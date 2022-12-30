// Copyright 2022 The Dawn Authors
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <algorithm>
#include <vector>

#include "dawn/common/Math.h"
#include "dawn/tests/DawnTest.h"
#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/WGPUHelpers.h"

// 2D array textures with particular dimensions may corrupt on some devices. This test creates some
// 2d-array textures with different dimensions, and test them one by one. For each sub-test, the
// tested texture is written via different methods, then read back from the texture and verify the
// data.

namespace {
enum class WriteType {
    ClearTexture,
    WriteTexture,    // Write the tested texture via writeTexture API
    B2TCopy,         // Write the tested texture via B2T copy
    RenderConstant,  // Write the tested texture via rendering the whole rectangle with solid color
                     // (0xFFFFFFFF)
    RenderFromTextureSample,  // Write the tested texture via sampling from a temp texture and
                              // writing the sampled data
    RenderFromTextureLoad     // Write the tested texture via textureLoad() from a temp texture and
                              // writing the loaded data
};

constexpr wgpu::TextureFormat kDefaultFormat = wgpu::TextureFormat::RGBA8Unorm;
constexpr uint32_t kDefaultHeight = 100u;
constexpr uint32_t kDefaultArrayLayerCount = 2u;
constexpr uint32_t kDefaultMipLevelCount = 1u;
constexpr uint32_t kDefaultSampleCount = 1u;
constexpr WriteType kDefaultWriteType = WriteType::B2TCopy;

std::ostream& operator<<(std::ostream& o, WriteType writeType) {
    switch (writeType) {
        case WriteType::ClearTexture:
            o << "ClearTexture";
            break;
        case WriteType::WriteTexture:
            o << "WriteTexture";
            break;
        case WriteType::B2TCopy:
            o << "B2TCopy";
            break;
        case WriteType::RenderConstant:
            o << "RenderConstant";
            break;
        case WriteType::RenderFromTextureSample:
            o << "RenderFromTextureSample";
            break;
        case WriteType::RenderFromTextureLoad:
            o << "RenderFromTextureLoad";
            break;
    }
    return o;
}

using TextureFormat = wgpu::TextureFormat;
using TextureWidth = uint32_t;
using TextureHeight = uint32_t;
using ArrayLayerCount = uint32_t;
using MipLevelCount = uint32_t;
using SampleCount = uint32_t;

DAWN_TEST_PARAM_STRUCT(TextureCorruptionTestsParams,
                       TextureFormat,
                       TextureWidth,
                       TextureHeight,
                       ArrayLayerCount,
                       MipLevelCount,
                       SampleCount,
                       WriteType);

}  // namespace

class TextureCorruptionTests : public DawnTestWithParams<TextureCorruptionTestsParams> {
  protected:
    virtual std::ostringstream& DoSingleTest(wgpu::Texture texture,
                                             const wgpu::Extent3D textureSize,
                                             uint32_t depthOrArrayLayer,
                                             uint32_t mipLevel,
                                             uint32_t sampleCount,
                                             uint32_t srcValue,
                                             wgpu::TextureFormat format) {
        wgpu::Extent3D levelSize = textureSize;
        if (mipLevel > 0) {
            levelSize.width = std::max(textureSize.width >> mipLevel, 1u);
            levelSize.height = std::max(textureSize.height >> mipLevel, 1u);
        }
        uint32_t bytesPerTexel = utils::GetTexelBlockSizeInBytes(format);
        uint32_t bytesPerRow = Align(levelSize.width * bytesPerTexel, 256);
        uint64_t bufferSize = bytesPerRow * levelSize.height;
        wgpu::BufferDescriptor descriptor;
        descriptor.size = bufferSize;
        descriptor.usage = wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst;
        wgpu::Buffer buffer = device.CreateBuffer(&descriptor);
        wgpu::Buffer resultBuffer = device.CreateBuffer(&descriptor);

        wgpu::ImageCopyTexture imageCopyTexture =
            utils::CreateImageCopyTexture(texture, mipLevel, {0, 0, depthOrArrayLayer});
        wgpu::ImageCopyBuffer imageCopyBuffer =
            utils::CreateImageCopyBuffer(buffer, 0, bytesPerRow);
        wgpu::ImageCopyBuffer imageCopyResult =
            utils::CreateImageCopyBuffer(resultBuffer, 0, bytesPerRow);

        WriteType type = GetParam().mWriteType;

        // Fill data into a buffer
        wgpu::Extent3D copySize = {levelSize.width, levelSize.height, 1};

        // Data is stored in a uint32_t vector, so a single texel may require multiple vector
        // elements for some formats or multiple texels may be combined into one vector element.
        uint32_t elementNumPerTexel = 1;
        uint32_t copyWidth = copySize.width;
        if (bytesPerTexel >= sizeof(uint32_t)) {
            elementNumPerTexel = bytesPerTexel / sizeof(uint32_t);
        } else {
            copyWidth = copyWidth * bytesPerTexel / sizeof(uint32_t);
        }

        uint32_t elementNumPerRow = bytesPerRow / sizeof(uint32_t);
        uint32_t elementNumInTotal = bufferSize / sizeof(uint32_t);
        std::vector<uint32_t> data(elementNumInTotal, 0);
        for (uint32_t i = 0; i < copySize.height; ++i) {
            for (uint32_t j = 0; j < copyWidth; ++j) {
                for (uint32_t k = 0; k < elementNumPerTexel; ++k) {
                    if (type == WriteType::RenderFromTextureSample ||
                        type == WriteType::RenderConstant) {
                        // Fill a simple and constant value (0xFFFFFFFF) in the whole buffer for
                        // texture sampling and rendering because either sampling operation will
                        // lead to precision loss or rendering a solid color is easier to implement
                        // and compare.
                        ASSERT(elementNumPerTexel == 1);
                        data[i * elementNumPerRow + j] = 0xFFFFFFFF;
                    } else if (type != WriteType::ClearTexture) {
                        data[i * elementNumPerRow + j * elementNumPerTexel + k] = srcValue;
                        srcValue++;
                    }
                }
            }
        }

        // Write data into the given layer via various write types
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        switch (type) {
            case WriteType::B2TCopy: {
                queue.WriteBuffer(buffer, 0, data.data(), bufferSize);
                encoder.CopyBufferToTexture(&imageCopyBuffer, &imageCopyTexture, &copySize);
                break;
            }
            case WriteType::WriteTexture: {
                wgpu::TextureDataLayout textureDataLayout =
                    utils::CreateTextureDataLayout(0, bytesPerRow);
                queue.WriteTexture(&imageCopyTexture, data.data(), bufferSize, &textureDataLayout,
                                   &copySize);
                break;
            }
            case WriteType::RenderConstant:
            case WriteType::RenderFromTextureSample:
            case WriteType::RenderFromTextureLoad: {
                // Write data into a single layer temp texture and read from this texture if needed
                ASSERT(format == wgpu::TextureFormat::RGBA8Unorm);
                wgpu::TextureView tempView;
                if (type != WriteType::RenderConstant) {
                    wgpu::Texture tempTexture = Create2DTexture(copySize, format, 1, 1);
                    wgpu::ImageCopyTexture imageCopyTempTexture =
                        utils::CreateImageCopyTexture(tempTexture, 0, {0, 0, 0});
                    wgpu::TextureDataLayout textureDataLayout =
                        utils::CreateTextureDataLayout(0, bytesPerRow);
                    queue.WriteTexture(&imageCopyTempTexture, data.data(), bufferSize,
                                       &textureDataLayout, &copySize);
                    tempView = tempTexture.CreateView();
                }

                // Write into the specified layer of a 2D array texture
                wgpu::TextureViewDescriptor viewDesc;
                viewDesc.format = format;
                viewDesc.dimension = wgpu::TextureViewDimension::e2D;
                viewDesc.baseMipLevel = 0;
                viewDesc.mipLevelCount = 1;
                viewDesc.baseArrayLayer = depthOrArrayLayer;
                viewDesc.arrayLayerCount = 1;
                CreatePipelineAndRender(texture.CreateView(&viewDesc), tempView, encoder, type,
                                        format);
                break;
            }
            default:
                break;
        }

        // Verify the data in texture via a T2B copy and comparison
        encoder.CopyTextureToBuffer(&imageCopyTexture, &imageCopyResult, &copySize);
        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);
        return EXPECT_BUFFER_U32_RANGE_EQ(data.data(), resultBuffer, 0, elementNumInTotal);
    }

    void CreatePipelineAndRender(wgpu::TextureView renderView,
                                 wgpu::TextureView samplerView,
                                 wgpu::CommandEncoder encoder,
                                 WriteType type,
                                 wgpu::TextureFormat format) {
        utils::ComboRenderPipelineDescriptor pipelineDescriptor;
        pipelineDescriptor.cTargets[0].format = format;

        // Draw the whole texture (a rectangle) via two triangles
        pipelineDescriptor.vertex.module = utils::CreateShaderModule(device, R"(
            @vertex
            fn main(@builtin(vertex_index) VertexIndex : u32) -> @builtin(position) vec4<f32> {
                var pos = array<vec2<f32>, 6>(
                    vec2<f32>(-1.0,  1.0),
                    vec2<f32>(-1.0, -1.0),
                    vec2<f32>( 1.0,  1.0),
                    vec2<f32>( 1.0,  1.0),
                    vec2<f32>(-1.0, -1.0),
                    vec2<f32>( 1.0, -1.0));
                return vec4<f32>(pos[VertexIndex], 0.0, 1.0);
            })");

        if (type == WriteType::RenderConstant) {
            pipelineDescriptor.cFragment.module = utils::CreateShaderModule(device, R"(
            @fragment
            fn main(@builtin(position) FragCoord : vec4<f32>) -> @location(0) vec4<f32> {
                return vec4<f32>(1.0, 1.0, 1.0, 1.0);
            })");
        } else if (type == WriteType::RenderFromTextureSample) {
            pipelineDescriptor.cFragment.module = utils::CreateShaderModule(device, R"(
            @group(0) @binding(0) var samp : sampler;
            @group(0) @binding(1) var tex : texture_2d<f32>;

            @fragment
            fn main(@builtin(position) FragCoord : vec4<f32>) -> @location(0) vec4<f32> {
                return textureSample(tex, samp, FragCoord.xy);
            })");
        } else {
            pipelineDescriptor.cFragment.module = utils::CreateShaderModule(device, R"(
            @group(0) @binding(0) var tex : texture_2d<f32>;

            @fragment
            fn main(@builtin(position) Fragcoord: vec4<f32>) -> @location(0) vec4<f32> {
                return textureLoad(tex, vec2<i32>(Fragcoord.xy), 0);
            })");
        }

        wgpu::RenderPipeline pipeline = device.CreateRenderPipeline(&pipelineDescriptor);

        utils::ComboRenderPassDescriptor renderPassDescriptor({renderView});
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPassDescriptor);
        pass.SetPipeline(pipeline);
        if (type != WriteType::RenderConstant) {
            wgpu::BindGroup bindGroup;
            if (type == WriteType::RenderFromTextureLoad) {
                bindGroup = utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                                 {{0, samplerView}});
            } else {
                bindGroup = utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                                 {{0, device.CreateSampler()}, {1, samplerView}});
            }
            pass.SetBindGroup(0, bindGroup);
        }
        pass.Draw(6);
        pass.End();
    }

    wgpu::Texture Create2DTexture(const wgpu::Extent3D size,
                                  wgpu::TextureFormat format,
                                  uint32_t mipLevelCount,
                                  uint32_t sampleCount) {
        wgpu::TextureDescriptor texDesc = {};
        texDesc.dimension = wgpu::TextureDimension::e2D;
        texDesc.size = size;
        texDesc.mipLevelCount = mipLevelCount;
        texDesc.format = format;
        texDesc.sampleCount = sampleCount;
        texDesc.usage = wgpu::TextureUsage::CopyDst | wgpu::TextureUsage::CopySrc |
                        wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::TextureBinding;
        return device.CreateTexture(&texDesc);
    }

    void DoTest() {
        DAWN_SUPPRESS_TEST_IF(IsWARP());
        uint32_t width = GetParam().mTextureWidth;
        uint32_t height = GetParam().mTextureHeight;
        uint32_t depthOrArrayLayerCount = GetParam().mArrayLayerCount;
        uint32_t mipLevelCount = GetParam().mMipLevelCount;
        uint32_t sampleCount = GetParam().mSampleCount;
        wgpu::Extent3D textureSize = {width, height, depthOrArrayLayerCount};

        // Pre-allocate textures. The incorrect write type may corrupt neighboring textures or
        // layers.
        std::vector<wgpu::Texture> textures;
        wgpu::TextureFormat format = GetParam().mTextureFormat;
        uint32_t texNum = 2;
        for (uint32_t i = 0; i < texNum; ++i) {
            textures.push_back(Create2DTexture(textureSize, format, mipLevelCount, sampleCount));
        }

        // Multisample textures have only 1 layer, while other textures being tested have 2 layers
        // at least.
        std::vector<uint32_t> testedLayers = {0};
        if (sampleCount == 1) {
            testedLayers.push_back(1);
        }

        // Most 2d-array textures being tested have only 2 layers. But if the texture has a lot of
        // layers, select a few layers to test.
        if (depthOrArrayLayerCount > 2) {
            ASSERT(sampleCount == 1);
            uint32_t divider = 4;
            for (uint32_t i = 1; i <= divider; ++i) {
                int32_t testedLayer = depthOrArrayLayerCount * i / divider - 1;
                if (testedLayer > 1) {
                    testedLayers.push_back(testedLayer);
                }
            }
        }

        // Write data and verify the result one by one for every layer of every texture
        uint32_t srcValue = 100000000;
        for (uint32_t i = 0; i < texNum; ++i) {
            for (uint32_t j = 0; j < testedLayers.size(); ++j) {
                for (uint32_t k = 0; k < mipLevelCount; ++k) {
                    DoSingleTest(textures[i], textureSize, testedLayers[j], k, sampleCount,
                                 srcValue, format)
                        << "texNum: " << i << ", layer: " << j << ", mip level: " << k;
                    srcValue += 100000000;
                }
            }
        }
    }
};

class TextureCorruptionTests_Format : public TextureCorruptionTests {};

TEST_P(TextureCorruptionTests_Format, Tests) {
    DoTest();
}

DAWN_INSTANTIATE_TEST_P(TextureCorruptionTests_Format,
                        {D3D12Backend()},
                        {wgpu::TextureFormat::R8Unorm, wgpu::TextureFormat::RG8Unorm,
                         wgpu::TextureFormat::RGBA8Unorm, wgpu::TextureFormat::RGBA16Uint,
                         wgpu::TextureFormat::RGBA32Uint, wgpu::TextureFormat::Depth16Unorm,
                         wgpu::TextureFormat::Stencil8},
                        {100u, 600u, 1200u, 2400u, 4800u},
                        {kDefaultHeight},
                        {kDefaultArrayLayerCount},
                        {kDefaultMipLevelCount},
                        {kDefaultSampleCount},
                        {WriteType::ClearTexture});

class TextureCorruptionTests_WidthAndHeight : public TextureCorruptionTests {};

TEST_P(TextureCorruptionTests_WidthAndHeight, Tests) {
    DoTest();
}

DAWN_INSTANTIATE_TEST_P(TextureCorruptionTests_WidthAndHeight,
                        {D3D12Backend()},
                        {kDefaultFormat},
                        {100u, 200u, 300u, 400u, 500u, 600u, 700u, 800u, 900u, 1000u, 1200u},
                        {100u, 200u},
                        {kDefaultArrayLayerCount},
                        {kDefaultMipLevelCount},
                        {kDefaultSampleCount},
                        {kDefaultWriteType});

class TextureCorruptionTests_ArrayLayer : public TextureCorruptionTests {};

TEST_P(TextureCorruptionTests_ArrayLayer, Tests) {
    DoTest();
}

DAWN_INSTANTIATE_TEST_P(TextureCorruptionTests_ArrayLayer,
                        {D3D12Backend()},
                        {kDefaultFormat},
                        {100u, 600u, 1200u},
                        {kDefaultHeight},
                        {6u, 12u, 40u, 256u},
                        {kDefaultMipLevelCount},
                        {kDefaultSampleCount},
                        {kDefaultWriteType});

class TextureCorruptionTests_Mipmap : public TextureCorruptionTests {};

TEST_P(TextureCorruptionTests_Mipmap, Tests) {
    DoTest();
}

DAWN_INSTANTIATE_TEST_P(TextureCorruptionTests_Mipmap,
                        {D3D12Backend()},
                        {kDefaultFormat},
                        {100u, 600u, 1200u},
                        {kDefaultHeight},
                        {kDefaultArrayLayerCount},
                        {2u, 6u},
                        {kDefaultSampleCount},
                        {kDefaultWriteType});

class TextureCorruptionTests_Multisample : public TextureCorruptionTests {
  protected:
    std::ostringstream& DoSingleTest(wgpu::Texture texture,
                                     const wgpu::Extent3D textureSize,
                                     uint32_t depthOrArrayLayer,
                                     uint32_t mipLevel,
                                     uint32_t sampleCount,
                                     uint32_t srcValue,
                                     wgpu::TextureFormat format) override {
        ASSERT(depthOrArrayLayer == 0);
        ASSERT(mipLevel == 0);
        uint32_t bytesPerTexel = utils::GetTexelBlockSizeInBytes(format);

        return ExpectMultisampledFloatData(texture, textureSize.width, textureSize.height,
                                           bytesPerTexel, sampleCount, 0, mipLevel,
                                           new detail::ExpectConstant<float>(0));
    }
};

TEST_P(TextureCorruptionTests_Multisample, Tests) {
    DoTest();
}

DAWN_INSTANTIATE_TEST_P(TextureCorruptionTests_Multisample,
                        {D3D12Backend()},
                        {kDefaultFormat},
                        {100u, 200u, 300u, 400u, 500u, 600u, 700u, 800u, 900u, 1000u, 1200u},
                        {100u, 200u},
                        {1u},
                        {kDefaultMipLevelCount},
                        {4u},
                        {WriteType::ClearTexture});

class TextureCorruptionTests_WriteType : public TextureCorruptionTests {};

TEST_P(TextureCorruptionTests_WriteType, Tests) {
    DoTest();
}

DAWN_INSTANTIATE_TEST_P(TextureCorruptionTests_WriteType,
                        {D3D12Backend()},
                        {kDefaultFormat},
                        {100u, 600u, 1200u},
                        {kDefaultHeight},
                        {kDefaultArrayLayerCount},
                        {kDefaultMipLevelCount},
                        {kDefaultSampleCount},
                        {WriteType::ClearTexture, WriteType::WriteTexture, WriteType::B2TCopy,
                         WriteType::RenderConstant, WriteType::RenderFromTextureSample,
                         WriteType::RenderFromTextureLoad});
