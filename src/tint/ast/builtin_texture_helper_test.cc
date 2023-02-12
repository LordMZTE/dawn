// Copyright 2020 The Tint Authors.
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

#include "src/tint/ast/builtin_texture_helper_test.h"

#include "src/tint/type/depth_texture.h"
#include "src/tint/type/multisampled_texture.h"
#include "src/tint/type/sampled_texture.h"
#include "src/tint/type/texture_dimension.h"

using namespace tint::number_suffixes;  // NOLINT

namespace tint::ast::builtin::test {

TextureOverloadCase::TextureOverloadCase(ValidTextureOverload o,
                                         const char* desc,
                                         TextureKind tk,
                                         type::SamplerKind sk,
                                         type::TextureDimension dims,
                                         TextureDataType datatype,
                                         const char* f,
                                         std::function<Args(ProgramBuilder*)> a)
    : overload(o),
      description(desc),
      texture_kind(tk),
      sampler_kind(sk),
      texture_dimension(dims),
      texture_data_type(datatype),
      function(f),
      args(std::move(a)) {}
TextureOverloadCase::TextureOverloadCase(ValidTextureOverload o,
                                         const char* desc,
                                         TextureKind tk,
                                         type::TextureDimension dims,
                                         TextureDataType datatype,
                                         const char* f,
                                         std::function<Args(ProgramBuilder*)> a)
    : overload(o),
      description(desc),
      texture_kind(tk),
      texture_dimension(dims),
      texture_data_type(datatype),
      function(f),
      args(std::move(a)) {}
TextureOverloadCase::TextureOverloadCase(ValidTextureOverload o,
                                         const char* d,
                                         type::Access acc,
                                         type::TexelFormat fmt,
                                         type::TextureDimension dims,
                                         TextureDataType datatype,
                                         const char* f,
                                         std::function<Args(ProgramBuilder*)> a)
    : overload(o),
      description(d),
      texture_kind(TextureKind::kStorage),
      access(acc),
      texel_format(fmt),
      texture_dimension(dims),
      texture_data_type(datatype),
      function(f),
      args(std::move(a)) {}
TextureOverloadCase::TextureOverloadCase(const TextureOverloadCase&) = default;
TextureOverloadCase::~TextureOverloadCase() = default;

std::ostream& operator<<(std::ostream& out, const TextureKind& kind) {
    switch (kind) {
        case TextureKind::kRegular:
            out << "regular";
            break;
        case TextureKind::kDepth:
            out << "depth";
            break;
        case TextureKind::kDepthMultisampled:
            out << "depth-multisampled";
            break;
        case TextureKind::kMultisampled:
            out << "multisampled";
            break;
        case TextureKind::kStorage:
            out << "storage";
            break;
    }
    return out;
}

std::ostream& operator<<(std::ostream& out, const TextureDataType& ty) {
    switch (ty) {
        case TextureDataType::kF32:
            out << "f32";
            break;
        case TextureDataType::kU32:
            out << "u32";
            break;
        case TextureDataType::kI32:
            out << "i32";
            break;
    }
    return out;
}

std::ostream& operator<<(std::ostream& out, const TextureOverloadCase& data) {
    out << "TextureOverloadCase " << static_cast<int>(data.overload) << "\n";
    out << data.description << "\n";
    out << "texture_kind:      " << data.texture_kind << "\n";
    out << "sampler_kind:      ";
    if (data.texture_kind != TextureKind::kStorage) {
        out << data.sampler_kind;
    } else {
        out << "<unused>";
    }
    out << "\n";
    out << "access:            " << data.access << "\n";
    out << "texel_format:      " << data.texel_format << "\n";
    out << "texture_dimension: " << data.texture_dimension << "\n";
    out << "texture_data_type: " << data.texture_data_type << "\n";
    return out;
}

const ast::Type* TextureOverloadCase::BuildResultVectorComponentType(ProgramBuilder* b) const {
    switch (texture_data_type) {
        case ast::builtin::test::TextureDataType::kF32:
            return b->ty.f32();
        case ast::builtin::test::TextureDataType::kU32:
            return b->ty.u32();
        case ast::builtin::test::TextureDataType::kI32:
            return b->ty.i32();
    }

    TINT_UNREACHABLE(AST, b->Diagnostics());
    return {};
}

const ast::Variable* TextureOverloadCase::BuildTextureVariable(ProgramBuilder* b) const {
    utils::Vector attrs{
        b->Group(0_u),
        b->Binding(0_a),
    };
    switch (texture_kind) {
        case ast::builtin::test::TextureKind::kRegular:
            return b->GlobalVar(
                kTextureName,
                b->ty.sampled_texture(texture_dimension, BuildResultVectorComponentType(b)), attrs);

        case ast::builtin::test::TextureKind::kDepth:
            return b->GlobalVar(kTextureName, b->ty.depth_texture(texture_dimension), attrs);

        case ast::builtin::test::TextureKind::kDepthMultisampled:
            return b->GlobalVar(kTextureName, b->ty.depth_multisampled_texture(texture_dimension),
                                attrs);

        case ast::builtin::test::TextureKind::kMultisampled:
            return b->GlobalVar(
                kTextureName,
                b->ty.multisampled_texture(texture_dimension, BuildResultVectorComponentType(b)),
                attrs);

        case ast::builtin::test::TextureKind::kStorage: {
            auto* st = b->ty.storage_texture(texture_dimension, texel_format, access);
            return b->GlobalVar(kTextureName, st, attrs);
        }
    }

    TINT_UNREACHABLE(AST, b->Diagnostics());
    return nullptr;
}

const ast::Variable* TextureOverloadCase::BuildSamplerVariable(ProgramBuilder* b) const {
    utils::Vector attrs = {b->Group(0_a), b->Binding(1_a)};
    return b->GlobalVar(kSamplerName, b->ty.sampler(sampler_kind), attrs);
}

std::vector<TextureOverloadCase> TextureOverloadCase::ValidCases() {
    return {
        {
            ValidTextureOverload::kDimensions1d,
            "textureDimensions(t : texture_1d<f32>) -> u32",
            TextureKind::kRegular,
            type::SamplerKind::kSampler,
            type::TextureDimension::k1d,
            TextureDataType::kF32,
            "textureDimensions",
            [](ProgramBuilder* b) { return b->ExprList(kTextureName); },
        },
        {
            ValidTextureOverload::kDimensions2d,
            "textureDimensions(t : texture_2d<f32>) -> vec2<u32>",
            TextureKind::kRegular,
            type::SamplerKind::kSampler,
            type::TextureDimension::k2d,
            TextureDataType::kF32,
            "textureDimensions",
            [](ProgramBuilder* b) { return b->ExprList(kTextureName); },
        },
        {
            ValidTextureOverload::kDimensions2dLevel,
            "textureDimensions(t     : texture_2d<f32>,\n"
            "                  level : i32) -> vec2<u32>",
            TextureKind::kRegular,
            type::SamplerKind::kSampler,
            type::TextureDimension::k2d,
            TextureDataType::kF32,
            "textureDimensions",
            [](ProgramBuilder* b) { return b->ExprList(kTextureName, 1_i); },
        },
        {
            ValidTextureOverload::kDimensions2dArray,
            "textureDimensions(t : texture_2d_array<f32>) -> vec2<u32>",
            TextureKind::kRegular,
            type::SamplerKind::kSampler,
            type::TextureDimension::k2dArray,
            TextureDataType::kF32,
            "textureDimensions",
            [](ProgramBuilder* b) { return b->ExprList(kTextureName); },
        },
        {
            ValidTextureOverload::kDimensions2dArrayLevel,
            "textureDimensions(t     : texture_2d_array<f32>,\n"
            "                  level : i32) -> vec2<u32>",
            TextureKind::kRegular,
            type::SamplerKind::kSampler,
            type::TextureDimension::k2dArray,
            TextureDataType::kF32,
            "textureDimensions",
            [](ProgramBuilder* b) { return b->ExprList(kTextureName, 1_i); },
        },
        {
            ValidTextureOverload::kDimensions3d,
            "textureDimensions(t : texture_3d<f32>) -> vec3<u32>",
            TextureKind::kRegular,
            type::SamplerKind::kSampler,
            type::TextureDimension::k3d,
            TextureDataType::kF32,
            "textureDimensions",
            [](ProgramBuilder* b) { return b->ExprList(kTextureName); },
        },
        {
            ValidTextureOverload::kDimensions3dLevel,
            "textureDimensions(t     : texture_3d<f32>,\n"
            "                  level : i32) -> vec3<u32>",
            TextureKind::kRegular,
            type::SamplerKind::kSampler,
            type::TextureDimension::k3d,
            TextureDataType::kF32,
            "textureDimensions",
            [](ProgramBuilder* b) { return b->ExprList(kTextureName, 1_i); },
        },
        {
            ValidTextureOverload::kDimensionsCube,
            "textureDimensions(t : texture_cube<f32>) -> vec2<u32>",
            TextureKind::kRegular,
            type::SamplerKind::kSampler,
            type::TextureDimension::kCube,
            TextureDataType::kF32,
            "textureDimensions",
            [](ProgramBuilder* b) { return b->ExprList(kTextureName); },
        },
        {
            ValidTextureOverload::kDimensionsCubeLevel,
            "textureDimensions(t     : texture_cube<f32>,\n"
            "                  level : i32) -> vec2<u32>",
            TextureKind::kRegular,
            type::SamplerKind::kSampler,
            type::TextureDimension::kCube,
            TextureDataType::kF32,
            "textureDimensions",
            [](ProgramBuilder* b) { return b->ExprList(kTextureName, 1_i); },
        },
        {
            ValidTextureOverload::kDimensionsCubeArray,
            "textureDimensions(t : texture_cube_array<f32>) -> vec2<u32>",
            TextureKind::kRegular,
            type::SamplerKind::kSampler,
            type::TextureDimension::kCubeArray,
            TextureDataType::kF32,
            "textureDimensions",
            [](ProgramBuilder* b) { return b->ExprList(kTextureName); },
        },
        {
            ValidTextureOverload::kDimensionsCubeArrayLevel,
            "textureDimensions(t     : texture_cube_array<f32>,\n"
            "                  level : i32) -> vec2<u32>",
            TextureKind::kRegular,
            type::SamplerKind::kSampler,
            type::TextureDimension::kCubeArray,
            TextureDataType::kF32,
            "textureDimensions",
            [](ProgramBuilder* b) { return b->ExprList(kTextureName, 1_i); },
        },
        {
            ValidTextureOverload::kDimensionsMultisampled2d,
            "textureDimensions(t : texture_multisampled_2d<f32>)-> vec2<u32>",
            TextureKind::kMultisampled,
            type::SamplerKind::kSampler,
            type::TextureDimension::k2d,
            TextureDataType::kF32,
            "textureDimensions",
            [](ProgramBuilder* b) { return b->ExprList(kTextureName); },
        },
        {
            ValidTextureOverload::kDimensionsDepth2d,
            "textureDimensions(t : texture_depth_2d) -> vec2<u32>",
            TextureKind::kDepth,
            type::SamplerKind::kSampler,
            type::TextureDimension::k2d,
            TextureDataType::kF32,
            "textureDimensions",
            [](ProgramBuilder* b) { return b->ExprList(kTextureName); },
        },
        {
            ValidTextureOverload::kDimensionsDepth2dLevel,
            "textureDimensions(t     : texture_depth_2d,\n"
            "                  level : i32) -> vec2<u32>",
            TextureKind::kDepth,
            type::SamplerKind::kSampler,
            type::TextureDimension::k2d,
            TextureDataType::kF32,
            "textureDimensions",
            [](ProgramBuilder* b) { return b->ExprList(kTextureName, 1_i); },
        },
        {
            ValidTextureOverload::kDimensionsDepth2dArray,
            "textureDimensions(t : texture_depth_2d_array) -> vec2<u32>",
            TextureKind::kDepth,
            type::SamplerKind::kSampler,
            type::TextureDimension::k2dArray,
            TextureDataType::kF32,
            "textureDimensions",
            [](ProgramBuilder* b) { return b->ExprList(kTextureName); },
        },
        {
            ValidTextureOverload::kDimensionsDepth2dArrayLevel,
            "textureDimensions(t     : texture_depth_2d_array,\n"
            "                  level : i32) -> vec2<u32>",
            TextureKind::kDepth,
            type::SamplerKind::kSampler,
            type::TextureDimension::k2dArray,
            TextureDataType::kF32,
            "textureDimensions",
            [](ProgramBuilder* b) { return b->ExprList(kTextureName, 1_i); },
        },
        {
            ValidTextureOverload::kDimensionsDepthCube,
            "textureDimensions(t : texture_depth_cube) -> vec2<u32>",
            TextureKind::kDepth,
            type::SamplerKind::kSampler,
            type::TextureDimension::kCube,
            TextureDataType::kF32,
            "textureDimensions",
            [](ProgramBuilder* b) { return b->ExprList(kTextureName); },
        },
        {
            ValidTextureOverload::kDimensionsDepthCubeLevel,
            "textureDimensions(t     : texture_depth_cube,\n"
            "                  level : i32) -> vec2<u32>",
            TextureKind::kDepth,
            type::SamplerKind::kSampler,
            type::TextureDimension::kCube,
            TextureDataType::kF32,
            "textureDimensions",
            [](ProgramBuilder* b) { return b->ExprList(kTextureName, 1_i); },
        },
        {
            ValidTextureOverload::kDimensionsDepthCubeArray,
            "textureDimensions(t : texture_depth_cube_array) -> vec2<u32>",
            TextureKind::kDepth,
            type::SamplerKind::kSampler,
            type::TextureDimension::kCubeArray,
            TextureDataType::kF32,
            "textureDimensions",
            [](ProgramBuilder* b) { return b->ExprList(kTextureName); },
        },
        {
            ValidTextureOverload::kDimensionsDepthCubeArrayLevel,
            "textureDimensions(t     : texture_depth_cube_array,\n"
            "                  level : i32) -> vec2<u32>",
            TextureKind::kDepth,
            type::SamplerKind::kSampler,
            type::TextureDimension::kCubeArray,
            TextureDataType::kF32,
            "textureDimensions",
            [](ProgramBuilder* b) { return b->ExprList(kTextureName, 1_i); },
        },
        {
            ValidTextureOverload::kDimensionsDepthMultisampled2d,
            "textureDimensions(t : texture_depth_multisampled_2d) -> vec2<u32>",
            TextureKind::kDepthMultisampled,
            type::SamplerKind::kSampler,
            type::TextureDimension::k2d,
            TextureDataType::kF32,
            "textureDimensions",
            [](ProgramBuilder* b) { return b->ExprList(kTextureName); },
        },
        {
            ValidTextureOverload::kDimensionsStorageWO1d,
            "textureDimensions(t : texture_storage_1d<rgba32float>) -> u32",
            type::Access::kWrite,
            type::TexelFormat::kRgba32Float,
            type::TextureDimension::k1d,
            TextureDataType::kF32,
            "textureDimensions",
            [](ProgramBuilder* b) { return b->ExprList(kTextureName); },
        },
        {
            ValidTextureOverload::kDimensionsStorageWO2d,
            "textureDimensions(t : texture_storage_2d<rgba32float>) -> vec2<u32>",
            type::Access::kWrite,
            type::TexelFormat::kRgba32Float,
            type::TextureDimension::k2d,
            TextureDataType::kF32,
            "textureDimensions",
            [](ProgramBuilder* b) { return b->ExprList(kTextureName); },
        },
        {
            ValidTextureOverload::kDimensionsStorageWO2dArray,
            "textureDimensions(t : texture_storage_2d_array<rgba32float>) -> vec2<u32>",
            type::Access::kWrite,
            type::TexelFormat::kRgba32Float,
            type::TextureDimension::k2dArray,
            TextureDataType::kF32,
            "textureDimensions",
            [](ProgramBuilder* b) { return b->ExprList(kTextureName); },
        },
        {
            ValidTextureOverload::kDimensionsStorageWO3d,
            "textureDimensions(t : texture_storage_3d<rgba32float>) -> vec3<u32>",
            type::Access::kWrite,
            type::TexelFormat::kRgba32Float,
            type::TextureDimension::k3d,
            TextureDataType::kF32,
            "textureDimensions",
            [](ProgramBuilder* b) { return b->ExprList(kTextureName); },
        },

        {
            ValidTextureOverload::kGather2dF32,
            "textureGather(component : i32,\n"
            "              t         : texture_2d<T>,\n"
            "              s         : sampler,\n"
            "              coords    : vec2<f32>) -> vec4<T>",
            TextureKind::kRegular,
            type::SamplerKind::kSampler,
            type::TextureDimension::k2d,
            TextureDataType::kF32,
            "textureGather",
            [](ProgramBuilder* b) {
                return b->ExprList(0_i,                      // component
                                   kTextureName,             // t
                                   kSamplerName,             // s
                                   b->vec2<f32>(1_f, 2_f));  // coords
            },
        },
        {
            ValidTextureOverload::kGather2dOffsetF32,
            "textureGather(component : u32,\n"
            "              t         : texture_2d<T>,\n"
            "              s         : sampler,\n"
            "              coords    : vec2<f32>,\n"
            "              offset    : vec2<i32>) -> vec4<T>",
            TextureKind::kRegular,
            type::SamplerKind::kSampler,
            type::TextureDimension::k2d,
            TextureDataType::kF32,
            "textureGather",
            [](ProgramBuilder* b) {
                return b->ExprList(0_u,                      // component
                                   kTextureName,             // t
                                   kSamplerName,             // s
                                   b->vec2<f32>(1_f, 2_f),   // coords
                                   b->vec2<i32>(3_i, 4_i));  // offset
            },
        },
        {
            ValidTextureOverload::kGather2dArrayF32,
            "textureGather(component   : i32,\n"
            "              t           : texture_2d_array<T>,\n"
            "              s           : sampler,\n"
            "              coords      : vec2<f32>,\n"
            "              array_index : i32) -> vec4<T>",
            TextureKind::kRegular,
            type::SamplerKind::kSampler,
            type::TextureDimension::k2dArray,
            TextureDataType::kF32,
            "textureGather",
            [](ProgramBuilder* b) {
                return b->ExprList(0_i,                     // component
                                   kTextureName,            // t
                                   kSamplerName,            // s
                                   b->vec2<f32>(1_f, 2_f),  // coords
                                   3_i);                    // array index
            },
        },
        {
            ValidTextureOverload::kGather2dArrayOffsetF32,
            "textureGather(component   : u32,\n"
            "              t           : texture_2d_array<T>,\n"
            "              s           : sampler,\n"
            "              coords      : vec2<f32>,\n"
            "              array_index : u32,\n"
            "              offset      : vec2<i32>) -> vec4<T>",
            TextureKind::kRegular,
            type::SamplerKind::kSampler,
            type::TextureDimension::k2dArray,
            TextureDataType::kF32,
            "textureGather",
            [](ProgramBuilder* b) {
                return b->ExprList(0_u,                      // component
                                   kTextureName,             // t
                                   kSamplerName,             // s
                                   b->vec2<f32>(1_f, 2_f),   // coords
                                   3_u,                      // array_index
                                   b->vec2<i32>(4_i, 5_i));  // offset
            },
        },
        {
            ValidTextureOverload::kGatherCubeF32,
            "textureGather(component : i32,\n"
            "              t         : texture_cube<T>,\n"
            "              s         : sampler,\n"
            "              coords    : vec3<f32>) -> vec4<T>",
            TextureKind::kRegular,
            type::SamplerKind::kSampler,
            type::TextureDimension::kCube,
            TextureDataType::kF32,
            "textureGather",
            [](ProgramBuilder* b) {
                return b->ExprList(0_i,                           // component
                                   kTextureName,                  // t
                                   kSamplerName,                  // s
                                   b->vec3<f32>(1_f, 2_f, 3_f));  // coords
            },
        },
        {
            ValidTextureOverload::kGatherCubeArrayF32,
            "textureGather(component   : u32,\n"
            "              t           : texture_cube_array<T>,\n"
            "              s           : sampler,\n"
            "              coords      : vec3<f32>,\n"
            "              array_index : u32) -> vec4<T>",
            TextureKind::kRegular,
            type::SamplerKind::kSampler,
            type::TextureDimension::kCubeArray,
            TextureDataType::kF32,
            "textureGather",
            [](ProgramBuilder* b) {
                return b->ExprList(0_u,                          // component
                                   kTextureName,                 // t
                                   kSamplerName,                 // s
                                   b->vec3<f32>(1_f, 2_f, 3_f),  // coords
                                   4_u);                         // array_index
            },
        },
        {
            ValidTextureOverload::kGatherDepth2dF32,
            "textureGather(t      : texture_depth_2d,\n"
            "              s      : sampler,\n"
            "              coords : vec2<f32>) -> vec4<f32>",
            TextureKind::kDepth,
            type::SamplerKind::kSampler,
            type::TextureDimension::k2d,
            TextureDataType::kF32,
            "textureGather",
            [](ProgramBuilder* b) {
                return b->ExprList(kTextureName,             // t
                                   kSamplerName,             // s
                                   b->vec2<f32>(1_f, 2_f));  // coords
            },
        },
        {
            ValidTextureOverload::kGatherDepth2dOffsetF32,
            "textureGather(t      : texture_depth_2d,\n"
            "              s      : sampler,\n"
            "              coords : vec2<f32>,\n"
            "              offset : vec2<i32>) -> vec4<f32>",
            TextureKind::kDepth,
            type::SamplerKind::kSampler,
            type::TextureDimension::k2d,
            TextureDataType::kF32,
            "textureGather",
            [](ProgramBuilder* b) {
                return b->ExprList(kTextureName,             // t
                                   kSamplerName,             // s
                                   b->vec2<f32>(1_f, 2_f),   // coords
                                   b->vec2<i32>(3_i, 4_i));  // offset
            },
        },
        {
            ValidTextureOverload::kGatherDepth2dArrayF32,
            "textureGather(t           : texture_depth_2d_array,\n"
            "              s           : sampler,\n"
            "              coords      : vec2<f32>,\n"
            "              array_index : u32) -> vec4<f32>",
            TextureKind::kDepth,
            type::SamplerKind::kSampler,
            type::TextureDimension::k2dArray,
            TextureDataType::kF32,
            "textureGather",
            [](ProgramBuilder* b) {
                return b->ExprList(kTextureName,            // t
                                   kSamplerName,            // s
                                   b->vec2<f32>(1_f, 2_f),  // coords
                                   3_u);                    // array_index
            },
        },
        {
            ValidTextureOverload::kGatherDepth2dArrayOffsetF32,
            "textureGather(t           : texture_depth_2d_array,\n"
            "              s           : sampler,\n"
            "              coords      : vec2<f32>,\n"
            "              array_index : i32,\n"
            "              offset      : vec2<i32>) -> vec4<f32>",
            TextureKind::kDepth,
            type::SamplerKind::kSampler,
            type::TextureDimension::k2dArray,
            TextureDataType::kF32,
            "textureGather",
            [](ProgramBuilder* b) {
                return b->ExprList(kTextureName,             // t
                                   kSamplerName,             // s
                                   b->vec2<f32>(1_f, 2_f),   // coords
                                   3_i,                      // array_index
                                   b->vec2<i32>(4_i, 5_i));  // offset
            },
        },
        {
            ValidTextureOverload::kGatherDepthCubeF32,
            "textureGather(t      : texture_depth_cube,\n"
            "              s      : sampler,\n"
            "              coords : vec3<f32>) -> vec4<f32>",
            TextureKind::kDepth,
            type::SamplerKind::kSampler,
            type::TextureDimension::kCube,
            TextureDataType::kF32,
            "textureGather",
            [](ProgramBuilder* b) {
                return b->ExprList(kTextureName,                  // t
                                   kSamplerName,                  // s
                                   b->vec3<f32>(1_f, 2_f, 3_f));  // coords
            },
        },
        {
            ValidTextureOverload::kGatherDepthCubeArrayF32,
            "textureGather(t           : texture_depth_cube_array,\n"
            "              s           : sampler,\n"
            "              coords      : vec3<f32>,\n"
            "              array_index : u32) -> vec4<f32>",
            TextureKind::kDepth,
            type::SamplerKind::kSampler,
            type::TextureDimension::kCubeArray,
            TextureDataType::kF32,
            "textureGather",
            [](ProgramBuilder* b) {
                return b->ExprList(kTextureName,                 // t
                                   kSamplerName,                 // s
                                   b->vec3<f32>(1_f, 2_f, 3_f),  // coords
                                   4_u);                         // array_index
            },
        },
        {
            ValidTextureOverload::kGatherCompareDepth2dF32,
            "textureGatherCompare(t         : texture_depth_2d,\n"
            "                     s         : sampler_comparison,\n"
            "                     coords    : vec2<f32>,\n"
            "                     depth_ref : f32) -> vec4<f32>",
            TextureKind::kDepth,
            type::SamplerKind::kComparisonSampler,
            type::TextureDimension::k2d,
            TextureDataType::kF32,
            "textureGatherCompare",
            [](ProgramBuilder* b) {
                return b->ExprList(kTextureName,            // t
                                   kSamplerName,            // s
                                   b->vec2<f32>(1_f, 2_f),  // coords
                                   3_f);                    // depth_ref
            },
        },
        {
            ValidTextureOverload::kGatherCompareDepth2dOffsetF32,
            "textureGatherCompare(t         : texture_depth_2d,\n"
            "                     s         : sampler_comparison,\n"
            "                     coords    : vec2<f32>,\n"
            "                     depth_ref : f32,\n"
            "                     offset    : vec2<i32>) -> vec4<f32>",
            TextureKind::kDepth,
            type::SamplerKind::kComparisonSampler,
            type::TextureDimension::k2d,
            TextureDataType::kF32,
            "textureGatherCompare",
            [](ProgramBuilder* b) {
                return b->ExprList(kTextureName,             // t
                                   kSamplerName,             // s
                                   b->vec2<f32>(1_f, 2_f),   // coords
                                   3_f,                      // depth_ref
                                   b->vec2<i32>(4_i, 5_i));  // offset
            },
        },
        {
            ValidTextureOverload::kGatherCompareDepth2dArrayF32,
            "textureGatherCompare(t           : texture_depth_2d_array,\n"
            "                     s           : sampler_comparison,\n"
            "                     coords      : vec2<f32>,\n"
            "                     array_index : i32,\n"
            "                     depth_ref   : f32) -> vec4<f32>",
            TextureKind::kDepth,
            type::SamplerKind::kComparisonSampler,
            type::TextureDimension::k2dArray,
            TextureDataType::kF32,
            "textureGatherCompare",
            [](ProgramBuilder* b) {
                return b->ExprList(kTextureName,            // t
                                   kSamplerName,            // s
                                   b->vec2<f32>(1_f, 2_f),  // coords
                                   3_i,                     // array_index
                                   4_f);                    // depth_ref
            },
        },
        {
            ValidTextureOverload::kGatherCompareDepth2dArrayOffsetF32,
            "textureGatherCompare(t           : texture_depth_2d_array,\n"
            "                     s           : sampler_comparison,\n"
            "                     coords      : vec2<f32>,\n"
            "                     array_index : i32,\n"
            "                     depth_ref   : f32,\n"
            "                     offset      : vec2<i32>) -> vec4<f32>",
            TextureKind::kDepth,
            type::SamplerKind::kComparisonSampler,
            type::TextureDimension::k2dArray,
            TextureDataType::kF32,
            "textureGatherCompare",
            [](ProgramBuilder* b) {
                return b->ExprList(kTextureName,             // t
                                   kSamplerName,             // s
                                   b->vec2<f32>(1_f, 2_f),   // coords
                                   3_i,                      // array_index
                                   4_f,                      // depth_ref
                                   b->vec2<i32>(5_i, 6_i));  // offset
            },
        },
        {
            ValidTextureOverload::kGatherCompareDepthCubeF32,
            "textureGatherCompare(t         : texture_depth_cube,\n"
            "                     s         : sampler_comparison,\n"
            "                     coords    : vec3<f32>,\n"
            "                     depth_ref : f32) -> vec4<f32>",
            TextureKind::kDepth,
            type::SamplerKind::kComparisonSampler,
            type::TextureDimension::kCube,
            TextureDataType::kF32,
            "textureGatherCompare",
            [](ProgramBuilder* b) {
                return b->ExprList(kTextureName,                 // t
                                   kSamplerName,                 // s
                                   b->vec3<f32>(1_f, 2_f, 3_f),  // coords
                                   4_f);                         // depth_ref
            },
        },
        {
            ValidTextureOverload::kGatherCompareDepthCubeArrayF32,
            "textureGatherCompare(t           : texture_depth_cube_array,\n"
            "                     s           : sampler_comparison,\n"
            "                     coords      : vec3<f32>,\n"
            "                     array_index : u32,\n"
            "                     depth_ref   : f32) -> vec4<f32>",
            TextureKind::kDepth,
            type::SamplerKind::kComparisonSampler,
            type::TextureDimension::kCubeArray,
            TextureDataType::kF32,
            "textureGatherCompare",
            [](ProgramBuilder* b) {
                return b->ExprList(kTextureName,                 // t
                                   kSamplerName,                 // s
                                   b->vec3<f32>(1_f, 2_f, 3_f),  // coords
                                   4_u,                          // array_index
                                   5_f);                         // depth_ref
            },
        },
        {
            ValidTextureOverload::kNumLayers2dArray,
            "textureNumLayers(t : texture_2d_array<f32>) -> u32",
            TextureKind::kRegular,
            type::SamplerKind::kSampler,
            type::TextureDimension::k2dArray,
            TextureDataType::kF32,
            "textureNumLayers",
            [](ProgramBuilder* b) { return b->ExprList(kTextureName); },
        },
        {
            ValidTextureOverload::kNumLayersCubeArray,
            "textureNumLayers(t : texture_cube_array<f32>) -> u32",
            TextureKind::kRegular,
            type::SamplerKind::kSampler,
            type::TextureDimension::kCubeArray,
            TextureDataType::kF32,
            "textureNumLayers",
            [](ProgramBuilder* b) { return b->ExprList(kTextureName); },
        },
        {
            ValidTextureOverload::kNumLayersDepth2dArray,
            "textureNumLayers(t : texture_depth_2d_array) -> u32",
            TextureKind::kDepth,
            type::SamplerKind::kSampler,
            type::TextureDimension::k2dArray,
            TextureDataType::kF32,
            "textureNumLayers",
            [](ProgramBuilder* b) { return b->ExprList(kTextureName); },
        },
        {
            ValidTextureOverload::kNumLayersDepthCubeArray,
            "textureNumLayers(t : texture_depth_cube_array) -> u32",
            TextureKind::kDepth,
            type::SamplerKind::kSampler,
            type::TextureDimension::kCubeArray,
            TextureDataType::kF32,
            "textureNumLayers",
            [](ProgramBuilder* b) { return b->ExprList(kTextureName); },
        },
        {
            ValidTextureOverload::kNumLayersStorageWO2dArray,
            "textureNumLayers(t : texture_storage_2d_array<rgba32float>) -> u32",
            type::Access::kWrite,
            type::TexelFormat::kRgba32Float,
            type::TextureDimension::k2dArray,
            TextureDataType::kF32,
            "textureNumLayers",
            [](ProgramBuilder* b) { return b->ExprList(kTextureName); },
        },
        {
            ValidTextureOverload::kNumLevels2d,
            "textureNumLevels(t : texture_2d<f32>) -> u32",
            TextureKind::kRegular,
            type::SamplerKind::kSampler,
            type::TextureDimension::k2d,
            TextureDataType::kF32,
            "textureNumLevels",
            [](ProgramBuilder* b) { return b->ExprList(kTextureName); },
        },
        {
            ValidTextureOverload::kNumLevels2dArray,
            "textureNumLevels(t : texture_2d_array<f32>) -> u32",
            TextureKind::kRegular,
            type::SamplerKind::kSampler,
            type::TextureDimension::k2dArray,
            TextureDataType::kF32,
            "textureNumLevels",
            [](ProgramBuilder* b) { return b->ExprList(kTextureName); },
        },
        {
            ValidTextureOverload::kNumLevels3d,
            "textureNumLevels(t : texture_3d<f32>) -> u32",
            TextureKind::kRegular,
            type::SamplerKind::kSampler,
            type::TextureDimension::k3d,
            TextureDataType::kF32,
            "textureNumLevels",
            [](ProgramBuilder* b) { return b->ExprList(kTextureName); },
        },
        {
            ValidTextureOverload::kNumLevelsCube,
            "textureNumLevels(t : texture_cube<f32>) -> u32",
            TextureKind::kRegular,
            type::SamplerKind::kSampler,
            type::TextureDimension::kCube,
            TextureDataType::kF32,
            "textureNumLevels",
            [](ProgramBuilder* b) { return b->ExprList(kTextureName); },
        },
        {
            ValidTextureOverload::kNumLevelsCubeArray,
            "textureNumLevels(t : texture_cube_array<f32>) -> u32",
            TextureKind::kRegular,
            type::SamplerKind::kSampler,
            type::TextureDimension::kCubeArray,
            TextureDataType::kF32,
            "textureNumLevels",
            [](ProgramBuilder* b) { return b->ExprList(kTextureName); },
        },
        {
            ValidTextureOverload::kNumLevelsDepth2d,
            "textureNumLevels(t : texture_depth_2d) -> u32",
            TextureKind::kDepth,
            type::SamplerKind::kSampler,
            type::TextureDimension::k2d,
            TextureDataType::kF32,
            "textureNumLevels",
            [](ProgramBuilder* b) { return b->ExprList(kTextureName); },
        },
        {
            ValidTextureOverload::kNumLevelsDepth2dArray,
            "textureNumLevels(t : texture_depth_2d_array) -> u32",
            TextureKind::kDepth,
            type::SamplerKind::kSampler,
            type::TextureDimension::k2dArray,
            TextureDataType::kF32,
            "textureNumLevels",
            [](ProgramBuilder* b) { return b->ExprList(kTextureName); },
        },
        {
            ValidTextureOverload::kNumLevelsDepthCube,
            "textureNumLevels(t : texture_depth_cube) -> u32",
            TextureKind::kDepth,
            type::SamplerKind::kSampler,
            type::TextureDimension::kCube,
            TextureDataType::kF32,
            "textureNumLevels",
            [](ProgramBuilder* b) { return b->ExprList(kTextureName); },
        },
        {
            ValidTextureOverload::kNumLevelsDepthCubeArray,
            "textureNumLevels(t : texture_depth_cube_array) -> u32",
            TextureKind::kDepth,
            type::SamplerKind::kSampler,
            type::TextureDimension::kCubeArray,
            TextureDataType::kF32,
            "textureNumLevels",
            [](ProgramBuilder* b) { return b->ExprList(kTextureName); },
        },
        {
            ValidTextureOverload::kNumSamplesMultisampled2d,
            "textureNumSamples(t : texture_multisampled_2d<f32>) -> u32",
            TextureKind::kMultisampled,
            type::SamplerKind::kSampler,
            type::TextureDimension::k2d,
            TextureDataType::kF32,
            "textureNumSamples",
            [](ProgramBuilder* b) { return b->ExprList(kTextureName); },
        },
        {
            ValidTextureOverload::kNumSamplesDepthMultisampled2d,
            "textureNumSamples(t : texture_depth_multisampled_2d<f32>) -> u32",
            TextureKind::kMultisampled,
            type::SamplerKind::kComparisonSampler,
            type::TextureDimension::k2d,
            TextureDataType::kF32,
            "textureNumSamples",
            [](ProgramBuilder* b) { return b->ExprList(kTextureName); },
        },
        {
            ValidTextureOverload::kSample1dF32,
            "textureSample(t      : texture_1d<f32>,\n"
            "              s      : sampler,\n"
            "              coords : f32) -> vec4<f32>",
            TextureKind::kRegular,
            type::SamplerKind::kSampler,
            type::TextureDimension::k1d,
            TextureDataType::kF32,
            "textureSample",
            [](ProgramBuilder* b) {
                return b->ExprList(kTextureName,  // t
                                   kSamplerName,  // s
                                   1_f);          // coords
            },
        },
        {
            ValidTextureOverload::kSample2dF32,
            "textureSample(t      : texture_2d<f32>,\n"
            "              s      : sampler,\n"
            "              coords : vec2<f32>) -> vec4<f32>",
            TextureKind::kRegular,
            type::SamplerKind::kSampler,
            type::TextureDimension::k2d,
            TextureDataType::kF32,
            "textureSample",
            [](ProgramBuilder* b) {
                return b->ExprList(kTextureName,             // t
                                   kSamplerName,             // s
                                   b->vec2<f32>(1_f, 2_f));  // coords
            },
        },
        {
            ValidTextureOverload::kSample2dOffsetF32,
            "textureSample(t      : texture_2d<f32>,\n"
            "              s      : sampler,\n"
            "              coords : vec2<f32>\n"
            "              offset : vec2<i32>) -> vec4<f32>",
            TextureKind::kRegular,
            type::SamplerKind::kSampler,
            type::TextureDimension::k2d,
            TextureDataType::kF32,
            "textureSample",
            [](ProgramBuilder* b) {
                return b->ExprList(kTextureName,             // t
                                   kSamplerName,             // s
                                   b->vec2<f32>(1_f, 2_f),   // coords
                                   b->vec2<i32>(3_i, 4_i));  // offset
            },
        },
        {
            ValidTextureOverload::kSample2dArrayF32,
            "textureSample(t           : texture_2d_array<f32>,\n"
            "              s           : sampler,\n"
            "              coords      : vec2<f32>,\n"
            "              array_index : i32) -> vec4<f32>",
            TextureKind::kRegular,
            type::SamplerKind::kSampler,
            type::TextureDimension::k2dArray,
            TextureDataType::kF32,
            "textureSample",
            [](ProgramBuilder* b) {
                return b->ExprList(kTextureName,            // t
                                   kSamplerName,            // s
                                   b->vec2<f32>(1_f, 2_f),  // coords
                                   3_i);                    // array_index
            },
        },
        {
            ValidTextureOverload::kSample2dArrayOffsetF32,
            "textureSample(t           : texture_2d_array<f32>,\n"
            "              s           : sampler,\n"
            "              coords      : vec2<f32>,\n"
            "              array_index : u32\n"
            "              offset      : vec2<i32>) -> vec4<f32>",
            TextureKind::kRegular,
            type::SamplerKind::kSampler,
            type::TextureDimension::k2dArray,
            TextureDataType::kF32,
            "textureSample",
            [](ProgramBuilder* b) {
                return b->ExprList(kTextureName,             // t
                                   kSamplerName,             // s
                                   b->vec2<f32>(1_f, 2_f),   // coords
                                   3_u,                      // array_index
                                   b->vec2<i32>(4_i, 5_i));  // offset
            },
        },
        {
            ValidTextureOverload::kSample3dF32,
            "textureSample(t      : texture_3d<f32>,\n"
            "              s      : sampler,\n"
            "              coords : vec3<f32>) -> vec4<f32>",
            TextureKind::kRegular,
            type::SamplerKind::kSampler,
            type::TextureDimension::k3d,
            TextureDataType::kF32,
            "textureSample",
            [](ProgramBuilder* b) {
                return b->ExprList(kTextureName,                  // t
                                   kSamplerName,                  // s
                                   b->vec3<f32>(1_f, 2_f, 3_f));  // coords
            },
        },
        {
            ValidTextureOverload::kSample3dOffsetF32,
            "textureSample(t      : texture_3d<f32>,\n"
            "              s      : sampler,\n"
            "              coords : vec3<f32>\n"
            "              offset : vec3<i32>) -> vec4<f32>",
            TextureKind::kRegular,
            type::SamplerKind::kSampler,
            type::TextureDimension::k3d,
            TextureDataType::kF32,
            "textureSample",
            [](ProgramBuilder* b) {
                return b->ExprList(kTextureName,                  // t
                                   kSamplerName,                  // s
                                   b->vec3<f32>(1_f, 2_f, 3_f),   // coords
                                   b->vec3<i32>(4_i, 5_i, 6_i));  // offset
            },
        },
        {
            ValidTextureOverload::kSampleCubeF32,
            "textureSample(t      : texture_cube<f32>,\n"
            "              s      : sampler,\n"
            "              coords : vec3<f32>) -> vec4<f32>",
            TextureKind::kRegular,
            type::SamplerKind::kSampler,
            type::TextureDimension::kCube,
            TextureDataType::kF32,
            "textureSample",
            [](ProgramBuilder* b) {
                return b->ExprList(kTextureName,                  // t
                                   kSamplerName,                  // s
                                   b->vec3<f32>(1_f, 2_f, 3_f));  // coords
            },
        },
        {
            ValidTextureOverload::kSampleCubeArrayF32,
            "textureSample(t           : texture_cube_array<f32>,\n"
            "              s           : sampler,\n"
            "              coords      : vec3<f32>,\n"
            "              array_index : i32) -> vec4<f32>",
            TextureKind::kRegular,
            type::SamplerKind::kSampler,
            type::TextureDimension::kCubeArray,
            TextureDataType::kF32,
            "textureSample",
            [](ProgramBuilder* b) {
                return b->ExprList(kTextureName,                 // t
                                   kSamplerName,                 // s
                                   b->vec3<f32>(1_f, 2_f, 3_f),  // coords
                                   4_i);                         // array_index
            },
        },
        {
            ValidTextureOverload::kSampleDepth2dF32,
            "textureSample(t      : texture_depth_2d,\n"
            "              s      : sampler,\n"
            "              coords : vec2<f32>) -> f32",
            TextureKind::kDepth,
            type::SamplerKind::kSampler,
            type::TextureDimension::k2d,
            TextureDataType::kF32,
            "textureSample",
            [](ProgramBuilder* b) {
                return b->ExprList(kTextureName,             // t
                                   kSamplerName,             // s
                                   b->vec2<f32>(1_f, 2_f));  // coords
            },
        },
        {
            ValidTextureOverload::kSampleDepth2dOffsetF32,
            "textureSample(t      : texture_depth_2d,\n"
            "              s      : sampler,\n"
            "              coords : vec2<f32>\n"
            "              offset : vec2<i32>) -> f32",
            TextureKind::kDepth,
            type::SamplerKind::kSampler,
            type::TextureDimension::k2d,
            TextureDataType::kF32,
            "textureSample",
            [](ProgramBuilder* b) {
                return b->ExprList(kTextureName,             // t
                                   kSamplerName,             // s
                                   b->vec2<f32>(1_f, 2_f),   // coords
                                   b->vec2<i32>(3_i, 4_i));  // offset
            },
        },
        {
            ValidTextureOverload::kSampleDepth2dArrayF32,
            "textureSample(t           : texture_depth_2d_array,\n"
            "              s           : sampler,\n"
            "              coords      : vec2<f32>,\n"
            "              array_index : i32) -> f32",
            TextureKind::kDepth,
            type::SamplerKind::kSampler,
            type::TextureDimension::k2dArray,
            TextureDataType::kF32,
            "textureSample",
            [](ProgramBuilder* b) {
                return b->ExprList(kTextureName,            // t
                                   kSamplerName,            // s
                                   b->vec2<f32>(1_f, 2_f),  // coords
                                   3_i);                    // array_index
            },
        },
        {
            ValidTextureOverload::kSampleDepth2dArrayOffsetF32,
            "textureSample(t           : texture_depth_2d_array,\n"
            "              s           : sampler,\n"
            "              coords      : vec2<f32>,\n"
            "              array_index : i32\n"
            "              offset      : vec2<i32>) -> f32",
            TextureKind::kDepth,
            type::SamplerKind::kSampler,
            type::TextureDimension::k2dArray,
            TextureDataType::kF32,
            "textureSample",
            [](ProgramBuilder* b) {
                return b->ExprList(kTextureName,             // t
                                   kSamplerName,             // s
                                   b->vec2<f32>(1_f, 2_f),   // coords
                                   3_i,                      // array_index
                                   b->vec2<i32>(4_i, 5_i));  // offset
            },
        },
        {
            ValidTextureOverload::kSampleDepthCubeF32,
            "textureSample(t      : texture_depth_cube,\n"
            "              s      : sampler,\n"
            "              coords : vec3<f32>) -> f32",
            TextureKind::kDepth,
            type::SamplerKind::kSampler,
            type::TextureDimension::kCube,
            TextureDataType::kF32,
            "textureSample",
            [](ProgramBuilder* b) {
                return b->ExprList(kTextureName,                  // t
                                   kSamplerName,                  // s
                                   b->vec3<f32>(1_f, 2_f, 3_f));  // coords
            },
        },
        {
            ValidTextureOverload::kSampleDepthCubeArrayF32,
            "textureSample(t           : texture_depth_cube_array,\n"
            "              s           : sampler,\n"
            "              coords      : vec3<f32>,\n"
            "              array_index : u32) -> f32",
            TextureKind::kDepth,
            type::SamplerKind::kSampler,
            type::TextureDimension::kCubeArray,
            TextureDataType::kF32,
            "textureSample",
            [](ProgramBuilder* b) {
                return b->ExprList(kTextureName,                 // t
                                   kSamplerName,                 // s
                                   b->vec3<f32>(1_f, 2_f, 3_f),  // coords
                                   4_u);                         // array_index
            },
        },
        {
            ValidTextureOverload::kSampleBias2dF32,
            "textureSampleBias(t      : texture_2d<f32>,\n"
            "                  s      : sampler,\n"
            "                  coords : vec2<f32>,\n"
            "                  bias   : f32) -> vec4<f32>",
            TextureKind::kRegular,
            type::SamplerKind::kSampler,
            type::TextureDimension::k2d,
            TextureDataType::kF32,
            "textureSampleBias",
            [](ProgramBuilder* b) {
                return b->ExprList(kTextureName,            // t
                                   kSamplerName,            // s
                                   b->vec2<f32>(1_f, 2_f),  // coords
                                   3_f);                    // bias
            },
        },
        {
            ValidTextureOverload::kSampleBias2dOffsetF32,
            "textureSampleBias(t      : texture_2d<f32>,\n"
            "                  s      : sampler,\n"
            "                  coords : vec2<f32>,\n"
            "                  bias   : f32,\n"
            "                  offset : vec2<i32>) -> vec4<f32>",
            TextureKind::kRegular,
            type::SamplerKind::kSampler,
            type::TextureDimension::k2d,
            TextureDataType::kF32,
            "textureSampleBias",
            [](ProgramBuilder* b) {
                return b->ExprList(kTextureName,             // t
                                   kSamplerName,             // s
                                   b->vec2<f32>(1_f, 2_f),   // coords
                                   3_f,                      // bias
                                   b->vec2<i32>(4_i, 5_i));  // offset
            },
        },
        {
            ValidTextureOverload::kSampleBias2dArrayF32,
            "textureSampleBias(t           : texture_2d_array<f32>,\n"
            "                  s           : sampler,\n"
            "                  coords      : vec2<f32>,\n"
            "                  array_index : u32,\n"
            "                  bias        : f32) -> vec4<f32>",
            TextureKind::kRegular,
            type::SamplerKind::kSampler,
            type::TextureDimension::k2dArray,
            TextureDataType::kF32,
            "textureSampleBias",
            [](ProgramBuilder* b) {
                return b->ExprList(kTextureName,            // t
                                   kSamplerName,            // s
                                   b->vec2<f32>(1_f, 2_f),  // coords
                                   4_u,                     // array_index
                                   3_f);                    // bias
            },
        },
        {
            ValidTextureOverload::kSampleBias2dArrayOffsetF32,
            "textureSampleBias(t           : texture_2d_array<f32>,\n"
            "                  s           : sampler,\n"
            "                  coords      : vec2<f32>,\n"
            "                  array_index : i32,\n"
            "                  bias        : f32,\n"
            "                  offset      : vec2<i32>) -> vec4<f32>",
            TextureKind::kRegular,
            type::SamplerKind::kSampler,
            type::TextureDimension::k2dArray,
            TextureDataType::kF32,
            "textureSampleBias",
            [](ProgramBuilder* b) {
                return b->ExprList(kTextureName,             // t
                                   kSamplerName,             // s
                                   b->vec2<f32>(1_f, 2_f),   // coords
                                   3_i,                      // array_index
                                   4_f,                      // bias
                                   b->vec2<i32>(5_i, 6_i));  // offset
            },
        },
        {
            ValidTextureOverload::kSampleBias3dF32,
            "textureSampleBias(t      : texture_3d<f32>,\n"
            "                  s      : sampler,\n"
            "                  coords : vec3<f32>,\n"
            "                  bias   : f32) -> vec4<f32>",
            TextureKind::kRegular,
            type::SamplerKind::kSampler,
            type::TextureDimension::k3d,
            TextureDataType::kF32,
            "textureSampleBias",
            [](ProgramBuilder* b) {
                return b->ExprList(kTextureName,                 // t
                                   kSamplerName,                 // s
                                   b->vec3<f32>(1_f, 2_f, 3_f),  // coords
                                   4_f);                         // bias
            },
        },
        {
            ValidTextureOverload::kSampleBias3dOffsetF32,
            "textureSampleBias(t      : texture_3d<f32>,\n"
            "                  s      : sampler,\n"
            "                  coords : vec3<f32>,\n"
            "                  bias   : f32,\n"
            "                  offset : vec3<i32>) -> vec4<f32>",
            TextureKind::kRegular,
            type::SamplerKind::kSampler,
            type::TextureDimension::k3d,
            TextureDataType::kF32,
            "textureSampleBias",
            [](ProgramBuilder* b) {
                return b->ExprList(kTextureName,                  // t
                                   kSamplerName,                  // s
                                   b->vec3<f32>(1_f, 2_f, 3_f),   // coords
                                   4_f,                           // bias
                                   b->vec3<i32>(5_i, 6_i, 7_i));  // offset
            },
        },
        {
            ValidTextureOverload::kSampleBiasCubeF32,
            "textureSampleBias(t      : texture_cube<f32>,\n"
            "                  s      : sampler,\n"
            "                  coords : vec3<f32>,\n"
            "                  bias   : f32) -> vec4<f32>",
            TextureKind::kRegular,
            type::SamplerKind::kSampler,
            type::TextureDimension::kCube,
            TextureDataType::kF32,
            "textureSampleBias",
            [](ProgramBuilder* b) {
                return b->ExprList(kTextureName,                 // t
                                   kSamplerName,                 // s
                                   b->vec3<f32>(1_f, 2_f, 3_f),  // coords
                                   4_f);                         // bias
            },
        },
        {
            ValidTextureOverload::kSampleBiasCubeArrayF32,
            "textureSampleBias(t           : texture_cube_array<f32>,\n"
            "                  s           : sampler,\n"
            "                  coords      : vec3<f32>,\n"
            "                  array_index : i32,\n"
            "                  bias        : f32) -> vec4<f32>",
            TextureKind::kRegular,
            type::SamplerKind::kSampler,
            type::TextureDimension::kCubeArray,
            TextureDataType::kF32,
            "textureSampleBias",
            [](ProgramBuilder* b) {
                return b->ExprList(kTextureName,                 // t
                                   kSamplerName,                 // s
                                   b->vec3<f32>(1_f, 2_f, 3_f),  // coords
                                   3_i,                          // array_index
                                   4_f);                         // bias
            },
        },
        {
            ValidTextureOverload::kSampleLevel2dF32,
            "textureSampleLevel(t      : texture_2d<f32>,\n"
            "                   s      : sampler,\n"
            "                   coords : vec2<f32>,\n"
            "                   level  : f32) -> vec4<f32>",
            TextureKind::kRegular,
            type::SamplerKind::kSampler,
            type::TextureDimension::k2d,
            TextureDataType::kF32,
            "textureSampleLevel",
            [](ProgramBuilder* b) {
                return b->ExprList(kTextureName,            // t
                                   kSamplerName,            // s
                                   b->vec2<f32>(1_f, 2_f),  // coords
                                   3_f);                    // level
            },
        },
        {
            ValidTextureOverload::kSampleLevel2dOffsetF32,
            "textureSampleLevel(t      : texture_2d<f32>,\n"
            "                   s      : sampler,\n"
            "                   coords : vec2<f32>,\n"
            "                   level  : f32,\n"
            "                   offset : vec2<i32>) -> vec4<f32>",
            TextureKind::kRegular,
            type::SamplerKind::kSampler,
            type::TextureDimension::k2d,
            TextureDataType::kF32,
            "textureSampleLevel",
            [](ProgramBuilder* b) {
                return b->ExprList(kTextureName,             // t
                                   kSamplerName,             // s
                                   b->vec2<f32>(1_f, 2_f),   // coords
                                   3_f,                      // level
                                   b->vec2<i32>(4_i, 5_i));  // offset
            },
        },
        {
            ValidTextureOverload::kSampleLevel2dArrayF32,
            "textureSampleLevel(t           : texture_2d_array<f32>,\n"
            "                   s           : sampler,\n"
            "                   coords      : vec2<f32>,\n"
            "                   array_index : i32,\n"
            "                   level       : f32) -> vec4<f32>",
            TextureKind::kRegular,
            type::SamplerKind::kSampler,
            type::TextureDimension::k2dArray,
            TextureDataType::kF32,
            "textureSampleLevel",
            [](ProgramBuilder* b) {
                return b->ExprList(kTextureName,            // t
                                   kSamplerName,            // s
                                   b->vec2<f32>(1_f, 2_f),  // coords
                                   3_i,                     // array_index
                                   4_f);                    // level
            },
        },
        {
            ValidTextureOverload::kSampleLevel2dArrayOffsetF32,
            "textureSampleLevel(t           : texture_2d_array<f32>,\n"
            "                   s           : sampler,\n"
            "                   coords      : vec2<f32>,\n"
            "                   array_index : i32,\n"
            "                   level       : f32,\n"
            "                   offset      : vec2<i32>) -> vec4<f32>",
            TextureKind::kRegular,
            type::SamplerKind::kSampler,
            type::TextureDimension::k2dArray,
            TextureDataType::kF32,
            "textureSampleLevel",
            [](ProgramBuilder* b) {
                return b->ExprList(kTextureName,             // t
                                   kSamplerName,             // s
                                   b->vec2<f32>(1_f, 2_f),   // coords
                                   3_i,                      // array_index
                                   4_f,                      // level
                                   b->vec2<i32>(5_i, 6_i));  // offset
            },
        },
        {
            ValidTextureOverload::kSampleLevel3dF32,
            "textureSampleLevel(t      : texture_3d<f32>,\n"
            "                   s      : sampler,\n"
            "                   coords : vec3<f32>,\n"
            "                   level  : f32) -> vec4<f32>",
            TextureKind::kRegular,
            type::SamplerKind::kSampler,
            type::TextureDimension::k3d,
            TextureDataType::kF32,
            "textureSampleLevel",
            [](ProgramBuilder* b) {
                return b->ExprList(kTextureName,                 // t
                                   kSamplerName,                 // s
                                   b->vec3<f32>(1_f, 2_f, 3_f),  // coords
                                   4_f);                         // level
            },
        },
        {
            ValidTextureOverload::kSampleLevel3dOffsetF32,
            "textureSampleLevel(t      : texture_3d<f32>,\n"
            "                   s      : sampler,\n"
            "                   coords : vec3<f32>,\n"
            "                   level  : f32,\n"
            "                   offset : vec3<i32>) -> vec4<f32>",
            TextureKind::kRegular,
            type::SamplerKind::kSampler,
            type::TextureDimension::k3d,
            TextureDataType::kF32,
            "textureSampleLevel",
            [](ProgramBuilder* b) {
                return b->ExprList(kTextureName,                  // t
                                   kSamplerName,                  // s
                                   b->vec3<f32>(1_f, 2_f, 3_f),   // coords
                                   4_f,                           // level
                                   b->vec3<i32>(5_i, 6_i, 7_i));  // offset
            },
        },
        {
            ValidTextureOverload::kSampleLevelCubeF32,
            "textureSampleLevel(t      : texture_cube<f32>,\n"
            "                   s      : sampler,\n"
            "                   coords : vec3<f32>,\n"
            "                   level  : f32) -> vec4<f32>",
            TextureKind::kRegular,
            type::SamplerKind::kSampler,
            type::TextureDimension::kCube,
            TextureDataType::kF32,
            "textureSampleLevel",
            [](ProgramBuilder* b) {
                return b->ExprList(kTextureName,                 // t
                                   kSamplerName,                 // s
                                   b->vec3<f32>(1_f, 2_f, 3_f),  // coords
                                   4_f);                         // level
            },
        },
        {
            ValidTextureOverload::kSampleLevelCubeArrayF32,
            "textureSampleLevel(t           : texture_cube_array<f32>,\n"
            "                   s           : sampler,\n"
            "                   coords      : vec3<f32>,\n"
            "                   array_index : i32,\n"
            "                   level       : f32) -> vec4<f32>",
            TextureKind::kRegular,
            type::SamplerKind::kSampler,
            type::TextureDimension::kCubeArray,
            TextureDataType::kF32,
            "textureSampleLevel",
            [](ProgramBuilder* b) {
                return b->ExprList(kTextureName,                 // t
                                   kSamplerName,                 // s
                                   b->vec3<f32>(1_f, 2_f, 3_f),  // coords
                                   4_i,                          // array_index
                                   5_f);                         // level
            },
        },
        {
            ValidTextureOverload::kSampleLevelDepth2dF32,
            "textureSampleLevel(t      : texture_depth_2d,\n"
            "                   s      : sampler,\n"
            "                   coords : vec2<f32>,\n"
            "                   level  : u32) -> f32",
            TextureKind::kDepth,
            type::SamplerKind::kSampler,
            type::TextureDimension::k2d,
            TextureDataType::kF32,
            "textureSampleLevel",
            [](ProgramBuilder* b) {
                return b->ExprList(kTextureName,            // t
                                   kSamplerName,            // s
                                   b->vec2<f32>(1_f, 2_f),  // coords
                                   3_u);                    // level
            },
        },
        {
            ValidTextureOverload::kSampleLevelDepth2dOffsetF32,
            "textureSampleLevel(t      : texture_depth_2d,\n"
            "                   s      : sampler,\n"
            "                   coords : vec2<f32>,\n"
            "                   level  : i32,\n"
            "                   offset : vec2<i32>) -> f32",
            TextureKind::kDepth,
            type::SamplerKind::kSampler,
            type::TextureDimension::k2d,
            TextureDataType::kF32,
            "textureSampleLevel",
            [](ProgramBuilder* b) {
                return b->ExprList(kTextureName,             // t
                                   kSamplerName,             // s
                                   b->vec2<f32>(1_f, 2_f),   // coords
                                   3_i,                      // level
                                   b->vec2<i32>(4_i, 5_i));  // offset
            },
        },
        {
            ValidTextureOverload::kSampleLevelDepth2dArrayF32,
            "textureSampleLevel(t           : texture_depth_2d_array,\n"
            "                   s           : sampler,\n"
            "                   coords      : vec2<f32>,\n"
            "                   array_index : u32,\n"
            "                   level       : u32) -> f32",
            TextureKind::kDepth,
            type::SamplerKind::kSampler,
            type::TextureDimension::k2dArray,
            TextureDataType::kF32,
            "textureSampleLevel",
            [](ProgramBuilder* b) {
                return b->ExprList(kTextureName,            // t
                                   kSamplerName,            // s
                                   b->vec2<f32>(1_f, 2_f),  // coords
                                   3_u,                     // array_index
                                   4_u);                    // level
            },
        },
        {
            ValidTextureOverload::kSampleLevelDepth2dArrayOffsetF32,
            "textureSampleLevel(t           : texture_depth_2d_array,\n"
            "                   s           : sampler,\n"
            "                   coords      : vec2<f32>,\n"
            "                   array_index : u32,\n"
            "                   level       : u32,\n"
            "                   offset      : vec2<i32>) -> f32",
            TextureKind::kDepth,
            type::SamplerKind::kSampler,
            type::TextureDimension::k2dArray,
            TextureDataType::kF32,
            "textureSampleLevel",
            [](ProgramBuilder* b) {
                return b->ExprList(kTextureName,             // t
                                   kSamplerName,             // s
                                   b->vec2<f32>(1_f, 2_f),   // coords
                                   3_u,                      // array_index
                                   4_u,                      // level
                                   b->vec2<i32>(5_i, 6_i));  // offset
            },
        },
        {
            ValidTextureOverload::kSampleLevelDepthCubeF32,
            "textureSampleLevel(t      : texture_depth_cube,\n"
            "                   s      : sampler,\n"
            "                   coords : vec3<f32>,\n"
            "                   level  : i32) -> f32",
            TextureKind::kDepth,
            type::SamplerKind::kSampler,
            type::TextureDimension::kCube,
            TextureDataType::kF32,
            "textureSampleLevel",
            [](ProgramBuilder* b) {
                return b->ExprList(kTextureName,                 // t
                                   kSamplerName,                 // s
                                   b->vec3<f32>(1_f, 2_f, 3_f),  // coords
                                   4_i);                         // level
            },
        },
        {
            ValidTextureOverload::kSampleLevelDepthCubeArrayF32,
            "textureSampleLevel(t           : texture_depth_cube_array,\n"
            "                   s           : sampler,\n"
            "                   coords      : vec3<f32>,\n"
            "                   array_index : i32,\n"
            "                   level       : i32) -> f32",
            TextureKind::kDepth,
            type::SamplerKind::kSampler,
            type::TextureDimension::kCubeArray,
            TextureDataType::kF32,
            "textureSampleLevel",
            [](ProgramBuilder* b) {
                return b->ExprList(kTextureName,                 // t
                                   kSamplerName,                 // s
                                   b->vec3<f32>(1_f, 2_f, 3_f),  // coords
                                   4_i,                          // array_index
                                   5_i);                         // level
            },
        },
        {
            ValidTextureOverload::kSampleGrad2dF32,
            "textureSampleGrad(t      : texture_2d<f32>,\n"
            "                  s      : sampler,\n"
            "                  coords : vec2<f32>\n"
            "                  ddx    : vec2<f32>,\n"
            "                  ddy    : vec2<f32>) -> vec4<f32>",
            TextureKind::kRegular,
            type::SamplerKind::kSampler,
            type::TextureDimension::k2d,
            TextureDataType::kF32,
            "textureSampleGrad",
            [](ProgramBuilder* b) {
                return b->ExprList(kTextureName,             // t
                                   kSamplerName,             // s
                                   b->vec2<f32>(1_f, 2_f),   // coords
                                   b->vec2<f32>(3_f, 4_f),   // ddx
                                   b->vec2<f32>(5_f, 6_f));  // ddy
            },
        },
        {
            ValidTextureOverload::kSampleGrad2dOffsetF32,
            "textureSampleGrad(t      : texture_2d<f32>,\n"
            "                  s      : sampler,\n"
            "                  coords : vec2<f32>,\n"
            "                  ddx    : vec2<f32>,\n"
            "                  ddy    : vec2<f32>,\n"
            "                  offset : vec2<i32>) -> vec4<f32>",
            TextureKind::kRegular,
            type::SamplerKind::kSampler,
            type::TextureDimension::k2d,
            TextureDataType::kF32,
            "textureSampleGrad",
            [](ProgramBuilder* b) {
                return b->ExprList(kTextureName,             // t
                                   kSamplerName,             // s
                                   b->vec2<f32>(1_f, 2_f),   // coords
                                   b->vec2<f32>(3_f, 4_f),   // ddx
                                   b->vec2<f32>(5_f, 6_f),   // ddy
                                   b->vec2<i32>(7_i, 7_i));  // offset
            },
        },
        {
            ValidTextureOverload::kSampleGrad2dArrayF32,
            "textureSampleGrad(t           : texture_2d_array<f32>,\n"
            "                  s           : sampler,\n"
            "                  coords      : vec2<f32>,\n"
            "                  array_index : i32,\n"
            "                  ddx         : vec2<f32>,\n"
            "                  ddy         : vec2<f32>) -> vec4<f32>",
            TextureKind::kRegular,
            type::SamplerKind::kSampler,
            type::TextureDimension::k2dArray,
            TextureDataType::kF32,
            "textureSampleGrad",
            [](ProgramBuilder* b) {
                return b->ExprList(kTextureName,             // t
                                   kSamplerName,             // s
                                   b->vec2<f32>(1_f, 2_f),   // coords
                                   3_i,                      // array_index
                                   b->vec2<f32>(4_f, 5_f),   // ddx
                                   b->vec2<f32>(6_f, 7_f));  // ddy
            },
        },
        {
            ValidTextureOverload::kSampleGrad2dArrayOffsetF32,
            "textureSampleGrad(t           : texture_2d_array<f32>,\n"
            "                  s           : sampler,\n"
            "                  coords      : vec2<f32>,\n"
            "                  array_index : u32,\n"
            "                  ddx         : vec2<f32>,\n"
            "                  ddy         : vec2<f32>,\n"
            "                  offset      : vec2<i32>) -> vec4<f32>",
            TextureKind::kRegular,
            type::SamplerKind::kSampler,
            type::TextureDimension::k2dArray,
            TextureDataType::kF32,
            "textureSampleGrad",
            [](ProgramBuilder* b) {
                return b->ExprList(kTextureName,             // t
                                   kSamplerName,             // s
                                   b->vec2<f32>(1_f, 2_f),   // coords
                                   3_u,                      // array_index
                                   b->vec2<f32>(4_f, 5_f),   // ddx
                                   b->vec2<f32>(6_f, 7_f),   // ddy
                                   b->vec2<i32>(6_i, 7_i));  // offset
            },
        },
        {
            ValidTextureOverload::kSampleGrad3dF32,
            "textureSampleGrad(t      : texture_3d<f32>,\n"
            "                  s      : sampler,\n"
            "                  coords : vec3<f32>,\n"
            "                  ddx    : vec3<f32>,\n"
            "                  ddy    : vec3<f32>) -> vec4<f32>",
            TextureKind::kRegular,
            type::SamplerKind::kSampler,
            type::TextureDimension::k3d,
            TextureDataType::kF32,
            "textureSampleGrad",
            [](ProgramBuilder* b) {
                return b->ExprList(kTextureName,                  // t
                                   kSamplerName,                  // s
                                   b->vec3<f32>(1_f, 2_f, 3_f),   // coords
                                   b->vec3<f32>(4_f, 5_f, 6_f),   // ddx
                                   b->vec3<f32>(7_f, 8_f, 9_f));  // ddy
            },
        },
        {
            ValidTextureOverload::kSampleGrad3dOffsetF32,
            "textureSampleGrad(t      : texture_3d<f32>,\n"
            "                  s      : sampler,\n"
            "                  coords : vec3<f32>,\n"
            "                  ddx    : vec3<f32>,\n"
            "                  ddy    : vec3<f32>,\n"
            "                  offset : vec3<i32>) -> vec4<f32>",
            TextureKind::kRegular,
            type::SamplerKind::kSampler,
            type::TextureDimension::k3d,
            TextureDataType::kF32,
            "textureSampleGrad",
            [](ProgramBuilder* b) {
                return b->ExprList(kTextureName,                  // t
                                   kSamplerName,                  // s
                                   b->vec3<f32>(1_f, 2_f, 3_f),   // coords
                                   b->vec3<f32>(4_f, 5_f, 6_f),   // ddx
                                   b->vec3<f32>(7_f, 8_f, 9_f),   // ddy
                                   b->vec3<i32>(0_i, 1_i, 2_i));  // offset
            },
        },
        {
            ValidTextureOverload::kSampleGradCubeF32,
            "textureSampleGrad(t      : texture_cube<f32>,\n"
            "                  s      : sampler,\n"
            "                  coords : vec3<f32>,\n"
            "                  ddx    : vec3<f32>,\n"
            "                  ddy    : vec3<f32>) -> vec4<f32>",
            TextureKind::kRegular,
            type::SamplerKind::kSampler,
            type::TextureDimension::kCube,
            TextureDataType::kF32,
            "textureSampleGrad",
            [](ProgramBuilder* b) {
                return b->ExprList(kTextureName,                  // t
                                   kSamplerName,                  // s
                                   b->vec3<f32>(1_f, 2_f, 3_f),   // coords
                                   b->vec3<f32>(4_f, 5_f, 6_f),   // ddx
                                   b->vec3<f32>(7_f, 8_f, 9_f));  // ddy
            },
        },
        {
            ValidTextureOverload::kSampleGradCubeArrayF32,
            "textureSampleGrad(t           : texture_cube_array<f32>,\n"
            "                  s           : sampler,\n"
            "                  coords      : vec3<f32>,\n"
            "                  array_index : u32,\n"
            "                  ddx         : vec3<f32>,\n"
            "                  ddy         : vec3<f32>) -> vec4<f32>",
            TextureKind::kRegular,
            type::SamplerKind::kSampler,
            type::TextureDimension::kCubeArray,
            TextureDataType::kF32,
            "textureSampleGrad",
            [](ProgramBuilder* b) {
                return b->ExprList(kTextureName,                   // t
                                   kSamplerName,                   // s
                                   b->vec3<f32>(1_f, 2_f, 3_f),    // coords
                                   4_u,                            // array_index
                                   b->vec3<f32>(5_f, 6_f, 7_f),    // ddx
                                   b->vec3<f32>(8_f, 9_f, 10_f));  // ddy
            },
        },
        {
            ValidTextureOverload::kSampleCompareDepth2dF32,
            "textureSampleCompare(t         : texture_depth_2d,\n"
            "                     s         : sampler_comparison,\n"
            "                     coords    : vec2<f32>,\n"
            "                     depth_ref : f32) -> f32",
            TextureKind::kDepth,
            type::SamplerKind::kComparisonSampler,
            type::TextureDimension::k2d,
            TextureDataType::kF32,
            "textureSampleCompare",
            [](ProgramBuilder* b) {
                return b->ExprList(kTextureName,            // t
                                   kSamplerName,            // s
                                   b->vec2<f32>(1_f, 2_f),  // coords
                                   3_f);                    // depth_ref
            },
        },
        {
            ValidTextureOverload::kSampleCompareDepth2dOffsetF32,
            "textureSampleCompare(t         : texture_depth_2d,\n"
            "                     s         : sampler_comparison,\n"
            "                     coords    : vec2<f32>,\n"
            "                     depth_ref : f32,\n"
            "                     offset    : vec2<i32>) -> f32",
            TextureKind::kDepth,
            type::SamplerKind::kComparisonSampler,
            type::TextureDimension::k2d,
            TextureDataType::kF32,
            "textureSampleCompare",
            [](ProgramBuilder* b) {
                return b->ExprList(kTextureName,             // t
                                   kSamplerName,             // s
                                   b->vec2<f32>(1_f, 2_f),   // coords
                                   3_f,                      // depth_ref
                                   b->vec2<i32>(4_i, 5_i));  // offset
            },
        },
        {
            ValidTextureOverload::kSampleCompareDepth2dArrayF32,
            "textureSampleCompare(t           : texture_depth_2d_array,\n"
            "                     s           : sampler_comparison,\n"
            "                     coords      : vec2<f32>,\n"
            "                     array_index : i32,\n"
            "                     depth_ref   : f32) -> f32",
            TextureKind::kDepth,
            type::SamplerKind::kComparisonSampler,
            type::TextureDimension::k2dArray,
            TextureDataType::kF32,
            "textureSampleCompare",
            [](ProgramBuilder* b) {
                return b->ExprList(kTextureName,            // t
                                   kSamplerName,            // s
                                   b->vec2<f32>(1_f, 2_f),  // coords
                                   4_i,                     // array_index
                                   3_f);                    // depth_ref
            },
        },
        {
            ValidTextureOverload::kSampleCompareDepth2dArrayOffsetF32,
            "textureSampleCompare(t           : texture_depth_2d_array,\n"
            "                     s           : sampler_comparison,\n"
            "                     coords      : vec2<f32>,\n"
            "                     array_index : u32,\n"
            "                     depth_ref   : f32,\n"
            "                     offset      : vec2<i32>) -> f32",
            TextureKind::kDepth,
            type::SamplerKind::kComparisonSampler,
            type::TextureDimension::k2dArray,
            TextureDataType::kF32,
            "textureSampleCompare",
            [](ProgramBuilder* b) {
                return b->ExprList(kTextureName,             // t
                                   kSamplerName,             // s
                                   b->vec2<f32>(1_f, 2_f),   // coords
                                   4_u,                      // array_index
                                   3_f,                      // depth_ref
                                   b->vec2<i32>(5_i, 6_i));  // offset
            },
        },
        {
            ValidTextureOverload::kSampleCompareDepthCubeF32,
            "textureSampleCompare(t         : texture_depth_cube,\n"
            "                     s         : sampler_comparison,\n"
            "                     coords    : vec3<f32>,\n"
            "                     depth_ref : f32) -> f32",
            TextureKind::kDepth,
            type::SamplerKind::kComparisonSampler,
            type::TextureDimension::kCube,
            TextureDataType::kF32,
            "textureSampleCompare",
            [](ProgramBuilder* b) {
                return b->ExprList(kTextureName,                 // t
                                   kSamplerName,                 // s
                                   b->vec3<f32>(1_f, 2_f, 3_f),  // coords
                                   4_f);                         // depth_ref
            },
        },
        {
            ValidTextureOverload::kSampleCompareDepthCubeArrayF32,
            "textureSampleCompare(t           : texture_depth_cube_array,\n"
            "                     s           : sampler_comparison,\n"
            "                     coords      : vec3<f32>,\n"
            "                     array_index : i32,\n"
            "                     depth_ref   : f32) -> f32",
            TextureKind::kDepth,
            type::SamplerKind::kComparisonSampler,
            type::TextureDimension::kCubeArray,
            TextureDataType::kF32,
            "textureSampleCompare",
            [](ProgramBuilder* b) {
                return b->ExprList(kTextureName,                 // t
                                   kSamplerName,                 // s
                                   b->vec3<f32>(1_f, 2_f, 3_f),  // coords
                                   4_i,                          // array_index
                                   5_f);                         // depth_ref
            },
        },
        {
            ValidTextureOverload::kSampleCompareLevelDepth2dF32,
            "textureSampleCompareLevel(t         : texture_depth_2d,\n"
            "                          s         : sampler_comparison,\n"
            "                          coords    : vec2<f32>,\n"
            "                          depth_ref : f32) -> f32",
            TextureKind::kDepth,
            type::SamplerKind::kComparisonSampler,
            type::TextureDimension::k2d,
            TextureDataType::kF32,
            "textureSampleCompareLevel",
            [](ProgramBuilder* b) {
                return b->ExprList(kTextureName,            // t
                                   kSamplerName,            // s
                                   b->vec2<f32>(1_f, 2_f),  // coords
                                   3_f);                    // depth_ref
            },
        },
        {
            ValidTextureOverload::kSampleCompareLevelDepth2dOffsetF32,
            "textureSampleCompareLevel(t         : texture_depth_2d,\n"
            "                          s         : sampler_comparison,\n"
            "                          coords    : vec2<f32>,\n"
            "                          depth_ref : f32,\n"
            "                          offset    : vec2<i32>) -> f32",
            TextureKind::kDepth,
            type::SamplerKind::kComparisonSampler,
            type::TextureDimension::k2d,
            TextureDataType::kF32,
            "textureSampleCompareLevel",
            [](ProgramBuilder* b) {
                return b->ExprList(kTextureName,             // t
                                   kSamplerName,             // s
                                   b->vec2<f32>(1_f, 2_f),   // coords
                                   3_f,                      // depth_ref
                                   b->vec2<i32>(4_i, 5_i));  // offset
            },
        },
        {
            ValidTextureOverload::kSampleCompareLevelDepth2dArrayF32,
            "textureSampleCompareLevel(t           : texture_depth_2d_array,\n"
            "                          s           : sampler_comparison,\n"
            "                          coords      : vec2<f32>,\n"
            "                          array_index : i32,\n"
            "                          depth_ref   : f32) -> f32",
            TextureKind::kDepth,
            type::SamplerKind::kComparisonSampler,
            type::TextureDimension::k2dArray,
            TextureDataType::kF32,
            "textureSampleCompareLevel",
            [](ProgramBuilder* b) {
                return b->ExprList(kTextureName,            // t
                                   kSamplerName,            // s
                                   b->vec2<f32>(1_f, 2_f),  // coords
                                   3_i,                     // array_index
                                   4_f);                    // depth_ref
            },
        },
        {
            ValidTextureOverload::kSampleCompareLevelDepth2dArrayOffsetF32,
            "textureSampleCompareLevel(t           : texture_depth_2d_array,\n"
            "                          s           : sampler_comparison,\n"
            "                          coords      : vec2<f32>,\n"
            "                          array_index : i32,\n"
            "                          depth_ref   : f32,\n"
            "                          offset      : vec2<i32>) -> f32",
            TextureKind::kDepth,
            type::SamplerKind::kComparisonSampler,
            type::TextureDimension::k2dArray,
            TextureDataType::kF32,
            "textureSampleCompareLevel",
            [](ProgramBuilder* b) {
                return b->ExprList(kTextureName,             // t
                                   kSamplerName,             // s
                                   b->vec2<f32>(1_f, 2_f),   // coords
                                   3_i,                      // array_index
                                   4_f,                      // depth_ref
                                   b->vec2<i32>(5_i, 6_i));  // offset
            },
        },
        {
            ValidTextureOverload::kSampleCompareLevelDepthCubeF32,
            "textureSampleCompareLevel(t           : texture_depth_cube,\n"
            "                          s           : sampler_comparison,\n"
            "                          coords      : vec3<f32>,\n"
            "                          depth_ref   : f32) -> f32",
            TextureKind::kDepth,
            type::SamplerKind::kComparisonSampler,
            type::TextureDimension::kCube,
            TextureDataType::kF32,
            "textureSampleCompareLevel",
            [](ProgramBuilder* b) {
                return b->ExprList(kTextureName,                 // t
                                   kSamplerName,                 // s
                                   b->vec3<f32>(1_f, 2_f, 3_f),  // coords
                                   4_f);                         // depth_ref
            },
        },
        {
            ValidTextureOverload::kSampleCompareLevelDepthCubeArrayF32,
            "textureSampleCompareLevel(t           : texture_depth_cube_array,\n"
            "                          s           : sampler_comparison,\n"
            "                          coords      : vec3<f32>,\n"
            "                          array_index : i32,\n"
            "                          depth_ref   : f32) -> f32",
            TextureKind::kDepth,
            type::SamplerKind::kComparisonSampler,
            type::TextureDimension::kCubeArray,
            TextureDataType::kF32,
            "textureSampleCompareLevel",
            [](ProgramBuilder* b) {
                return b->ExprList(kTextureName,                 // t
                                   kSamplerName,                 // s
                                   b->vec3<f32>(1_f, 2_f, 3_f),  // coords
                                   4_i,                          // array_index
                                   5_f);                         // depth_ref
            },
        },
        {
            ValidTextureOverload::kLoad1dLevelF32,
            "textureLoad(t      : texture_1d<f32>,\n"
            "            coords : u32,\n"
            "            level  : u32) -> vec4<f32>",
            TextureKind::kRegular,
            type::TextureDimension::k1d,
            TextureDataType::kF32,
            "textureLoad",
            [](ProgramBuilder* b) {
                return b->ExprList(kTextureName,  // t
                                   1_u,           // coords
                                   3_u);          // level
            },
        },
        {
            ValidTextureOverload::kLoad1dLevelU32,
            "textureLoad(t      : texture_1d<u32>,\n"
            "            coords : i32,\n"
            "            level  : i32) -> vec4<u32>",
            TextureKind::kRegular,
            type::TextureDimension::k1d,
            TextureDataType::kU32,
            "textureLoad",
            [](ProgramBuilder* b) {
                return b->ExprList(kTextureName,  // t
                                   1_i,           // coords
                                   3_i);          // level
            },
        },
        {
            ValidTextureOverload::kLoad1dLevelI32,
            "textureLoad(t      : texture_1d<i32>,\n"
            "            coords : i32,\n"
            "            level  : i32) -> vec4<i32>",
            TextureKind::kRegular,
            type::TextureDimension::k1d,
            TextureDataType::kI32,
            "textureLoad",
            [](ProgramBuilder* b) {
                return b->ExprList(kTextureName,  // t
                                   1_i,           // coords
                                   3_i);          // level
            },
        },
        {
            ValidTextureOverload::kLoad2dLevelF32,
            "textureLoad(t      : texture_2d<f32>,\n"
            "            coords : vec2<u32>,\n"
            "            level  : u32) -> vec4<f32>",
            TextureKind::kRegular,
            type::TextureDimension::k2d,
            TextureDataType::kF32,
            "textureLoad",
            [](ProgramBuilder* b) {
                return b->ExprList(kTextureName,            // t
                                   b->vec2<u32>(1_u, 2_u),  // coords
                                   3_u);                    // level
            },
        },
        {
            ValidTextureOverload::kLoad2dLevelU32,
            "textureLoad(t      : texture_2d<u32>,\n"
            "            coords : vec2<i32>,\n"
            "            level  : i32) -> vec4<u32>",
            TextureKind::kRegular,
            type::TextureDimension::k2d,
            TextureDataType::kU32,
            "textureLoad",
            [](ProgramBuilder* b) {
                return b->ExprList(kTextureName,            // t
                                   b->vec2<i32>(1_i, 2_i),  // coords
                                   3_i);                    // level
            },
        },
        {
            ValidTextureOverload::kLoad2dLevelI32,
            "textureLoad(t      : texture_2d<i32>,\n"
            "            coords : vec2<u32>,\n"
            "            level  : u32) -> vec4<i32>",
            TextureKind::kRegular,
            type::TextureDimension::k2d,
            TextureDataType::kI32,
            "textureLoad",
            [](ProgramBuilder* b) {
                return b->ExprList(kTextureName,            // t
                                   b->vec2<u32>(1_u, 2_u),  // coords
                                   3_u);                    // level
            },
        },
        {
            ValidTextureOverload::kLoad2dArrayLevelF32,
            "textureLoad(t           : texture_2d_array<f32>,\n"
            "            coords      : vec2<i32>,\n"
            "            array_index : i32,\n"
            "            level       : i32) -> vec4<f32>",
            TextureKind::kRegular,
            type::TextureDimension::k2dArray,
            TextureDataType::kF32,
            "textureLoad",
            [](ProgramBuilder* b) {
                return b->ExprList(kTextureName,            // t
                                   b->vec2<i32>(1_i, 2_i),  // coords
                                   3_i,                     // array_index
                                   4_i);                    // level
            },
        },
        {
            ValidTextureOverload::kLoad2dArrayLevelU32,
            "textureLoad(t           : texture_2d_array<u32>,\n"
            "            coords      : vec2<i32>,\n"
            "            array_index : i32,\n"
            "            level       : i32) -> vec4<u32>",
            TextureKind::kRegular,
            type::TextureDimension::k2dArray,
            TextureDataType::kU32,
            "textureLoad",
            [](ProgramBuilder* b) {
                return b->ExprList(kTextureName,            // t
                                   b->vec2<i32>(1_i, 2_i),  // coords
                                   3_i,                     // array_index
                                   4_i);                    // level
            },
        },
        {
            ValidTextureOverload::kLoad2dArrayLevelI32,
            "textureLoad(t           : texture_2d_array<i32>,\n"
            "            coords      : vec2<u32>,\n"
            "            array_index : u32,\n"
            "            level       : u32) -> vec4<i32>",
            TextureKind::kRegular,
            type::TextureDimension::k2dArray,
            TextureDataType::kI32,
            "textureLoad",
            [](ProgramBuilder* b) {
                return b->ExprList(kTextureName,            // t
                                   b->vec2<u32>(1_u, 2_u),  // coords
                                   3_u,                     // array_index
                                   4_u);                    // level
            },
        },
        {
            ValidTextureOverload::kLoad3dLevelF32,
            "textureLoad(t      : texture_3d<f32>,\n"
            "            coords : vec3<i32>,\n"
            "            level  : i32) -> vec4<f32>",
            TextureKind::kRegular,
            type::TextureDimension::k3d,
            TextureDataType::kF32,
            "textureLoad",
            [](ProgramBuilder* b) {
                return b->ExprList(kTextureName,                 // t
                                   b->vec3<i32>(1_i, 2_i, 3_i),  // coords
                                   4_i);                         // level
            },
        },
        {
            ValidTextureOverload::kLoad3dLevelU32,
            "textureLoad(t      : texture_3d<u32>,\n"
            "            coords : vec3<i32>,\n"
            "            level  : i32) -> vec4<u32>",
            TextureKind::kRegular,
            type::TextureDimension::k3d,
            TextureDataType::kU32,
            "textureLoad",
            [](ProgramBuilder* b) {
                return b->ExprList(kTextureName,                 // t
                                   b->vec3<i32>(1_i, 2_i, 3_i),  // coords
                                   4_i);                         // level
            },
        },
        {
            ValidTextureOverload::kLoad3dLevelI32,
            "textureLoad(t      : texture_3d<i32>,\n"
            "            coords : vec3<u32>,\n"
            "            level  : u32) -> vec4<i32>",
            TextureKind::kRegular,
            type::TextureDimension::k3d,
            TextureDataType::kI32,
            "textureLoad",
            [](ProgramBuilder* b) {
                return b->ExprList(kTextureName,                 // t
                                   b->vec3<u32>(1_u, 2_u, 3_u),  // coords
                                   4_u);                         // level
            },
        },
        {
            ValidTextureOverload::kLoadMultisampled2dF32,
            "textureLoad(t            : texture_multisampled_2d<f32>,\n"
            "            coords       : vec2<i32>,\n"
            "            sample_index : i32) -> vec4<f32>",
            TextureKind::kMultisampled,
            type::TextureDimension::k2d,
            TextureDataType::kF32,
            "textureLoad",
            [](ProgramBuilder* b) {
                return b->ExprList(kTextureName,            // t
                                   b->vec2<i32>(1_i, 2_i),  // coords
                                   3_i);                    // sample_index
            },
        },
        {
            ValidTextureOverload::kLoadMultisampled2dU32,
            "textureLoad(t            : texture_multisampled_2d<u32>,\n"
            "            coords       : vec2<i32>,\n"
            "            sample_index : i32) -> vec4<u32>",
            TextureKind::kMultisampled,
            type::TextureDimension::k2d,
            TextureDataType::kU32,
            "textureLoad",
            [](ProgramBuilder* b) {
                return b->ExprList(kTextureName,            // t
                                   b->vec2<i32>(1_i, 2_i),  // coords
                                   3_i);                    // sample_index
            },
        },
        {
            ValidTextureOverload::kLoadMultisampled2dI32,
            "textureLoad(t            : texture_multisampled_2d<i32>,\n"
            "            coords       : vec2<u32>,\n"
            "            sample_index : u32) -> vec4<i32>",
            TextureKind::kMultisampled,
            type::TextureDimension::k2d,
            TextureDataType::kI32,
            "textureLoad",
            [](ProgramBuilder* b) {
                return b->ExprList(kTextureName,            // t
                                   b->vec2<u32>(1_u, 2_u),  // coords
                                   3_u);                    // sample_index
            },
        },
        {
            ValidTextureOverload::kLoadDepth2dLevelF32,
            "textureLoad(t      : texture_depth_2d,\n"
            "            coords : vec2<i32>,\n"
            "            level  : i32) -> f32",
            TextureKind::kDepth,
            type::TextureDimension::k2d,
            TextureDataType::kF32,
            "textureLoad",
            [](ProgramBuilder* b) {
                return b->ExprList(kTextureName,            // t
                                   b->vec2<i32>(1_i, 2_i),  // coords
                                   3_i);                    // level
            },
        },
        {
            ValidTextureOverload::kLoadDepth2dArrayLevelF32,
            "textureLoad(t           : texture_depth_2d_array,\n"
            "            coords      : vec2<u32>,\n"
            "            array_index : u32,\n"
            "            level       : u32) -> f32",
            TextureKind::kDepth,
            type::TextureDimension::k2dArray,
            TextureDataType::kF32,
            "textureLoad",
            [](ProgramBuilder* b) {
                return b->ExprList(kTextureName,            // t
                                   b->vec2<u32>(1_u, 2_u),  // coords
                                   3_u,                     // array_index
                                   4_u);                    // level
            },
        },
        {
            ValidTextureOverload::kLoadDepthMultisampled2dF32,
            "textureLoad(t            : texture_depth_multisampled_2d,\n"
            "            coords       : vec2<u32>,\n"
            "            sample_index : u32) -> f32",
            TextureKind::kDepthMultisampled,
            type::TextureDimension::k2d,
            TextureDataType::kF32,
            "textureLoad",
            [](ProgramBuilder* b) {
                return b->ExprList(kTextureName,            // t
                                   b->vec2<u32>(1_u, 2_u),  // coords
                                   3_u);                    // sample_index
            },
        },
        {
            ValidTextureOverload::kStoreWO1dRgba32float,
            "textureStore(t      : texture_storage_1d<rgba32float>,\n"
            "             coords : i32,\n"
            "             value  : vec4<T>)",
            type::Access::kWrite,
            type::TexelFormat::kRgba32Float,
            type::TextureDimension::k1d,
            TextureDataType::kF32,
            "textureStore",
            [](ProgramBuilder* b) {
                return b->ExprList(kTextureName,                       // t
                                   1_i,                                // coords
                                   b->vec4<f32>(2_f, 3_f, 4_f, 5_f));  // value
            },
        },
        {
            ValidTextureOverload::kStoreWO2dRgba32float,
            "textureStore(t      : texture_storage_2d<rgba32float>,\n"
            "             coords : vec2<i32>,\n"
            "             value  : vec4<T>)",
            type::Access::kWrite,
            type::TexelFormat::kRgba32Float,
            type::TextureDimension::k2d,
            TextureDataType::kF32,
            "textureStore",
            [](ProgramBuilder* b) {
                return b->ExprList(kTextureName,                       // t
                                   b->vec2<i32>(1_i, 2_i),             // coords
                                   b->vec4<f32>(3_f, 4_f, 5_f, 6_f));  // value
            },
        },
        {
            ValidTextureOverload::kStoreWO2dArrayRgba32float,
            "textureStore(t           : texture_storage_2d_array<rgba32float>,\n"
            "             coords      : vec2<u32>,\n"
            "             array_index : u32,\n"
            "             value       : vec4<T>)",
            type::Access::kWrite,
            type::TexelFormat::kRgba32Float,
            type::TextureDimension::k2dArray,
            TextureDataType::kF32,
            "textureStore",
            [](ProgramBuilder* b) {
                return b->ExprList(kTextureName,                       // t
                                   b->vec2<u32>(1_u, 2_u),             // coords
                                   3_u,                                // array_index
                                   b->vec4<f32>(4_f, 5_f, 6_f, 7_f));  // value
            },
        },
        {
            ValidTextureOverload::kStoreWO3dRgba32float,
            "textureStore(t      : texture_storage_3d<rgba32float>,\n"
            "             coords : vec3<u32>,\n"
            "             value  : vec4<T>)",
            type::Access::kWrite,
            type::TexelFormat::kRgba32Float,
            type::TextureDimension::k3d,
            TextureDataType::kF32,
            "textureStore",
            [](ProgramBuilder* b) {
                return b->ExprList(kTextureName,                       // t
                                   b->vec3<u32>(1_u, 2_u, 3_u),        // coords
                                   b->vec4<f32>(4_f, 5_f, 6_f, 7_f));  // value
            },
        },
    };
}

bool ReturnsVoid(ValidTextureOverload texture_overload) {
    switch (texture_overload) {
        case ValidTextureOverload::kStoreWO1dRgba32float:
        case ValidTextureOverload::kStoreWO2dRgba32float:
        case ValidTextureOverload::kStoreWO2dArrayRgba32float:
        case ValidTextureOverload::kStoreWO3dRgba32float:
            return true;
        default:
            return false;
    }
}

}  // namespace tint::ast::builtin::test
