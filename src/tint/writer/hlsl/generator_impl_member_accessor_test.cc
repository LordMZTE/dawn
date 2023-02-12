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

#include "gmock/gmock.h"
#include "src/tint/ast/stage_attribute.h"
#include "src/tint/writer/hlsl/test_helper.h"

using ::testing::HasSubstr;

using namespace tint::number_suffixes;  // NOLINT

namespace tint::writer::hlsl {
namespace {

using create_type_func_ptr = const ast::Type* (*)(const ProgramBuilder::TypesBuilder& ty);

inline const ast::Type* ty_i32(const ProgramBuilder::TypesBuilder& ty) {
    return ty.i32();
}
inline const ast::Type* ty_u32(const ProgramBuilder::TypesBuilder& ty) {
    return ty.u32();
}
inline const ast::Type* ty_f32(const ProgramBuilder::TypesBuilder& ty) {
    return ty.f32();
}
inline const ast::Type* ty_f16(const ProgramBuilder::TypesBuilder& ty) {
    return ty.f16();
}
template <typename T>
inline const ast::Type* ty_vec2(const ProgramBuilder::TypesBuilder& ty) {
    return ty.vec2<T>();
}
template <typename T>
inline const ast::Type* ty_vec3(const ProgramBuilder::TypesBuilder& ty) {
    return ty.vec3<T>();
}
template <typename T>
inline const ast::Type* ty_vec4(const ProgramBuilder::TypesBuilder& ty) {
    return ty.vec4<T>();
}
template <typename T>
inline const ast::Type* ty_mat2x2(const ProgramBuilder::TypesBuilder& ty) {
    return ty.mat2x2<T>();
}
template <typename T>
inline const ast::Type* ty_mat2x3(const ProgramBuilder::TypesBuilder& ty) {
    return ty.mat2x3<T>();
}
template <typename T>
inline const ast::Type* ty_mat2x4(const ProgramBuilder::TypesBuilder& ty) {
    return ty.mat2x4<T>();
}
template <typename T>
inline const ast::Type* ty_mat3x2(const ProgramBuilder::TypesBuilder& ty) {
    return ty.mat3x2<T>();
}
template <typename T>
inline const ast::Type* ty_mat3x3(const ProgramBuilder::TypesBuilder& ty) {
    return ty.mat3x3<T>();
}
template <typename T>
inline const ast::Type* ty_mat3x4(const ProgramBuilder::TypesBuilder& ty) {
    return ty.mat3x4<T>();
}
template <typename T>
inline const ast::Type* ty_mat4x2(const ProgramBuilder::TypesBuilder& ty) {
    return ty.mat4x2<T>();
}
template <typename T>
inline const ast::Type* ty_mat4x3(const ProgramBuilder::TypesBuilder& ty) {
    return ty.mat4x3<T>();
}
template <typename T>
inline const ast::Type* ty_mat4x4(const ProgramBuilder::TypesBuilder& ty) {
    return ty.mat4x4<T>();
}

template <typename BASE>
class HlslGeneratorImplTest_MemberAccessorBase : public BASE {
  public:
    void SetupStorageBuffer(utils::VectorRef<const ast::StructMember*> members) {
        ProgramBuilder& b = *this;
        auto* s = b.Structure("Data", members);

        b.GlobalVar("data", b.ty.Of(s), type::AddressSpace::kStorage, type::Access::kReadWrite,
                    b.Group(1_a), b.Binding(0_a));
    }

    void SetupUniformBuffer(utils::VectorRef<const ast::StructMember*> members) {
        ProgramBuilder& b = *this;
        auto* s = b.Structure("Data", members);

        b.GlobalVar("data", b.ty.Of(s), type::AddressSpace::kUniform, type::Access::kUndefined,
                    b.Group(1_a), b.Binding(1_a));
    }

    void SetupFunction(utils::VectorRef<const ast::Statement*> statements) {
        ProgramBuilder& b = *this;
        utils::Vector attrs{
            b.Stage(ast::PipelineStage::kFragment),
        };
        b.Func("main", utils::Empty, b.ty.void_(), std::move(statements), std::move(attrs));
    }
};

using HlslGeneratorImplTest_MemberAccessor = HlslGeneratorImplTest_MemberAccessorBase<TestHelper>;

template <typename T>
using HlslGeneratorImplTest_MemberAccessorWithParam =
    HlslGeneratorImplTest_MemberAccessorBase<TestParamHelper<T>>;

TEST_F(HlslGeneratorImplTest_MemberAccessor, EmitExpression_MemberAccessor) {
    auto* s = Structure("Data", utils::Vector{Member("mem", ty.f32())});
    GlobalVar("str", ty.Of(s), type::AddressSpace::kPrivate);

    auto* expr = MemberAccessor("str", "mem");
    WrapInFunction(Var("expr", ty.f32(), expr));

    GeneratorImpl& gen = SanitizeAndBuild();

    ASSERT_TRUE(gen.Generate()) << gen.error();
    EXPECT_EQ(gen.result(), R"(struct Data {
  float mem;
};

static Data str = (Data)0;

[numthreads(1, 1, 1)]
void test_function() {
  float expr = str.mem;
  return;
}
)");
}

struct TypeCase {
    create_type_func_ptr member_type;
    std::string expected;
};
inline std::ostream& operator<<(std::ostream& out, TypeCase c) {
    ProgramBuilder b;
    auto* ty = c.member_type(b.ty);
    out << ty->FriendlyName(b.Symbols());
    return out;
}

using HlslGeneratorImplTest_MemberAccessor_StorageBufferLoad_ConstantOffset =
    HlslGeneratorImplTest_MemberAccessorWithParam<TypeCase>;

TEST_P(HlslGeneratorImplTest_MemberAccessor_StorageBufferLoad_ConstantOffset, Test) {
    // struct Data {
    //   a : i32,
    //   b : <type>,
    // };
    // var<storage> data : Data;
    // data.b;

    auto p = GetParam();

    Enable(ast::Extension::kF16);

    SetupStorageBuffer(utils::Vector{
        Member("a", ty.i32()),
        Member("b", p.member_type(ty)),
    });

    SetupFunction(utils::Vector{
        Decl(Var("x", MemberAccessor("data", "b"))),
    });

    GeneratorImpl& gen = SanitizeAndBuild();

    ASSERT_TRUE(gen.Generate()) << gen.error();
    EXPECT_THAT(gen.result(), HasSubstr(p.expected));
}

INSTANTIATE_TEST_SUITE_P(
    HlslGeneratorImplTest_MemberAccessor,
    HlslGeneratorImplTest_MemberAccessor_StorageBufferLoad_ConstantOffset,
    testing::Values(TypeCase{ty_u32, "data.Load(4u)"},
                    TypeCase{ty_f32, "asfloat(data.Load(4u))"},
                    TypeCase{ty_i32, "asint(data.Load(4u))"},
                    TypeCase{ty_f16, "data.Load<float16_t>(4u)"},
                    TypeCase{ty_vec2<u32>, "data.Load2(8u)"},
                    TypeCase{ty_vec2<f32>, "asfloat(data.Load2(8u))"},
                    TypeCase{ty_vec2<i32>, "asint(data.Load2(8u))"},
                    TypeCase{ty_vec2<f16>, "data.Load<vector<float16_t, 2> >(4u)"},
                    TypeCase{ty_vec3<u32>, "data.Load3(16u)"},
                    TypeCase{ty_vec3<f32>, "asfloat(data.Load3(16u))"},
                    TypeCase{ty_vec3<i32>, "asint(data.Load3(16u))"},
                    TypeCase{ty_vec3<f16>, "data.Load<vector<float16_t, 3> >(8u)"},
                    TypeCase{ty_vec4<u32>, "data.Load4(16u)"},
                    TypeCase{ty_vec4<f32>, "asfloat(data.Load4(16u))"},
                    TypeCase{ty_vec4<i32>, "asint(data.Load4(16u))"},
                    TypeCase{ty_vec4<f16>, "data.Load<vector<float16_t, 4> >(8u)"},
                    TypeCase{ty_mat2x2<f32>,
                             "return float2x2(asfloat(buffer.Load2((offset + 0u))), "
                             "asfloat(buffer.Load2((offset + 8u))));"},
                    TypeCase{ty_mat2x3<f32>,
                             "return float2x3(asfloat(buffer.Load3((offset + 0u))), "
                             "asfloat(buffer.Load3((offset + 16u))));"},
                    TypeCase{ty_mat2x4<f32>,
                             "return float2x4(asfloat(buffer.Load4((offset + 0u))), "
                             "asfloat(buffer.Load4((offset + 16u))));"},
                    TypeCase{ty_mat3x2<f32>,
                             "return float3x2(asfloat(buffer.Load2((offset + 0u))), "
                             "asfloat(buffer.Load2((offset + 8u))), "
                             "asfloat(buffer.Load2((offset + 16u))));"},
                    TypeCase{ty_mat3x3<f32>,
                             "return float3x3(asfloat(buffer.Load3((offset + 0u))), "
                             "asfloat(buffer.Load3((offset + 16u))), "
                             "asfloat(buffer.Load3((offset + 32u))));"},
                    TypeCase{ty_mat3x4<f32>,
                             "return float3x4(asfloat(buffer.Load4((offset + 0u))), "
                             "asfloat(buffer.Load4((offset + 16u))), "
                             "asfloat(buffer.Load4((offset + 32u))));"},
                    TypeCase{ty_mat4x2<f32>,
                             "return float4x2(asfloat(buffer.Load2((offset + 0u))), "
                             "asfloat(buffer.Load2((offset + 8u))), "
                             "asfloat(buffer.Load2((offset + 16u))), "
                             "asfloat(buffer.Load2((offset + 24u))));"},
                    TypeCase{ty_mat4x3<f32>,
                             "return float4x3(asfloat(buffer.Load3((offset + 0u))), "
                             "asfloat(buffer.Load3((offset + 16u))), "
                             "asfloat(buffer.Load3((offset + 32u))), "
                             "asfloat(buffer.Load3((offset + 48u))));"},
                    TypeCase{ty_mat4x4<f32>,
                             "return float4x4(asfloat(buffer.Load4((offset + 0u))), "
                             "asfloat(buffer.Load4((offset + 16u))), "
                             "asfloat(buffer.Load4((offset + 32u))), "
                             "asfloat(buffer.Load4((offset + 48u))));"},
                    TypeCase{ty_mat2x2<f16>,
                             "return matrix<float16_t, 2, 2>("
                             "buffer.Load<vector<float16_t, 2> >((offset + 0u)), "
                             "buffer.Load<vector<float16_t, 2> >((offset + 4u)));"},
                    TypeCase{ty_mat2x3<f16>,
                             "return matrix<float16_t, 2, 3>("
                             "buffer.Load<vector<float16_t, 3> >((offset + 0u)), "
                             "buffer.Load<vector<float16_t, 3> >((offset + 8u)));"},
                    TypeCase{ty_mat2x4<f16>,
                             "return matrix<float16_t, 2, 4>("
                             "buffer.Load<vector<float16_t, 4> >((offset + 0u)), "
                             "buffer.Load<vector<float16_t, 4> >((offset + 8u)));"},
                    TypeCase{ty_mat3x2<f16>,
                             "return matrix<float16_t, 3, 2>("
                             "buffer.Load<vector<float16_t, 2> >((offset + 0u)), "
                             "buffer.Load<vector<float16_t, 2> >((offset + 4u)), "
                             "buffer.Load<vector<float16_t, 2> >((offset + 8u)));"},
                    TypeCase{ty_mat3x3<f16>,
                             "return matrix<float16_t, 3, 3>("
                             "buffer.Load<vector<float16_t, 3> >((offset + 0u)), "
                             "buffer.Load<vector<float16_t, 3> >((offset + 8u)), "
                             "buffer.Load<vector<float16_t, 3> >((offset + 16u)));"},
                    TypeCase{ty_mat3x4<f16>,
                             "return matrix<float16_t, 3, 4>("
                             "buffer.Load<vector<float16_t, 4> >((offset + 0u)), "
                             "buffer.Load<vector<float16_t, 4> >((offset + 8u)), "
                             "buffer.Load<vector<float16_t, 4> >((offset + 16u)));"},
                    TypeCase{ty_mat4x2<f16>,
                             "return matrix<float16_t, 4, 2>("
                             "buffer.Load<vector<float16_t, 2> >((offset + 0u)), "
                             "buffer.Load<vector<float16_t, 2> >((offset + 4u)), "
                             "buffer.Load<vector<float16_t, 2> >((offset + 8u)), "
                             "buffer.Load<vector<float16_t, 2> >((offset + 12u)));"},
                    TypeCase{ty_mat4x3<f16>,
                             "return matrix<float16_t, 4, 3>("
                             "buffer.Load<vector<float16_t, 3> >((offset + 0u)), "
                             "buffer.Load<vector<float16_t, 3> >((offset + 8u)), "
                             "buffer.Load<vector<float16_t, 3> >((offset + 16u)), "
                             "buffer.Load<vector<float16_t, 3> >((offset + 24u)));"},
                    TypeCase{ty_mat4x4<f16>,
                             "return matrix<float16_t, 4, 4>("
                             "buffer.Load<vector<float16_t, 4> >((offset + 0u)), "
                             "buffer.Load<vector<float16_t, 4> >((offset + 8u)), "
                             "buffer.Load<vector<float16_t, 4> >((offset + 16u)), "
                             "buffer.Load<vector<float16_t, 4> >((offset + 24u)));"}));

using HlslGeneratorImplTest_MemberAccessor_StorageBufferLoad_DynamicOffset =
    HlslGeneratorImplTest_MemberAccessorWithParam<TypeCase>;

TEST_P(HlslGeneratorImplTest_MemberAccessor_StorageBufferLoad_DynamicOffset, Test) {
    // struct Inner {
    //   a : i32,
    //   b : <type>,
    //   c : vec4<i32>,
    // };
    // struct Data {
    //  arr : array<Inner, 4i>,
    // }
    // var<storage> data : Data;
    // data.arr[i].b;

    auto p = GetParam();

    Enable(ast::Extension::kF16);

    auto* inner = Structure("Inner", utils::Vector{
                                         Member("a", ty.i32()),
                                         Member("b", p.member_type(ty)),
                                         Member("c", ty.vec4(ty.i32())),
                                     });

    SetupStorageBuffer(utils::Vector{
        Member("arr", ty.array(ty.Of(inner), 4_i)),
    });

    auto* i = Var("i", Expr(2_i));

    SetupFunction(utils::Vector{
        Decl(i),
        Decl(Var("x", MemberAccessor(IndexAccessor(MemberAccessor("data", "arr"), i), "b"))),
    });

    GeneratorImpl& gen = SanitizeAndBuild();

    ASSERT_TRUE(gen.Generate()) << gen.error();
    EXPECT_THAT(gen.result(), HasSubstr(p.expected));
}

INSTANTIATE_TEST_SUITE_P(
    HlslGeneratorImplTest_MemberAccessor,
    HlslGeneratorImplTest_MemberAccessor_StorageBufferLoad_DynamicOffset,
    testing::Values(
        TypeCase{ty_u32, "data.Load(((32u * uint(i)) + 4u))"},
        TypeCase{ty_f32, "asfloat(data.Load(((32u * uint(i)) + 4u)))"},
        TypeCase{ty_i32, "asint(data.Load(((32u * uint(i)) + 4u)))"},
        TypeCase{ty_f16, "data.Load<float16_t>(((32u * uint(i)) + 4u))"},
        TypeCase{ty_vec2<u32>, "data.Load2(((32u * uint(i)) + 8u))"},
        TypeCase{ty_vec2<f32>, "asfloat(data.Load2(((32u * uint(i)) + 8u)))"},
        TypeCase{ty_vec2<i32>, "asint(data.Load2(((32u * uint(i)) + 8u)))"},
        TypeCase{ty_vec2<f16>, "data.Load<vector<float16_t, 2> >(((32u * uint(i)) + 4u))"},
        TypeCase{ty_vec3<u32>, "data.Load3(((48u * uint(i)) + 16u))"},
        TypeCase{ty_vec3<f32>, "asfloat(data.Load3(((48u * uint(i)) + 16u)))"},
        TypeCase{ty_vec3<i32>, "asint(data.Load3(((48u * uint(i)) + 16u)))"},
        TypeCase{ty_vec3<f16>, "data.Load<vector<float16_t, 3> >(((32u * uint(i)) + 8u))"},
        TypeCase{ty_vec4<u32>, "data.Load4(((48u * uint(i)) + 16u))"},
        TypeCase{ty_vec4<f32>, "asfloat(data.Load4(((48u * uint(i)) + 16u)))"},
        TypeCase{ty_vec4<i32>, "asint(data.Load4(((48u * uint(i)) + 16u)))"},
        TypeCase{ty_vec4<f16>, "data.Load<vector<float16_t, 4> >(((32u * uint(i)) + 8u))"},
        TypeCase{ty_mat2x2<f32>,
                 "return float2x2(asfloat(buffer.Load2((offset + 0u))), "
                 "asfloat(buffer.Load2((offset + 8u))));"},
        TypeCase{ty_mat2x3<f32>,
                 "return float2x3(asfloat(buffer.Load3((offset + 0u))), "
                 "asfloat(buffer.Load3((offset + 16u))));"},
        TypeCase{ty_mat2x4<f32>,
                 "return float2x4(asfloat(buffer.Load4((offset + 0u))), "
                 "asfloat(buffer.Load4((offset + 16u))));"},
        TypeCase{ty_mat3x2<f32>,
                 "return float3x2(asfloat(buffer.Load2((offset + 0u))), "
                 "asfloat(buffer.Load2((offset + 8u))), "
                 "asfloat(buffer.Load2((offset + 16u))));"},
        TypeCase{ty_mat3x3<f32>,
                 "return float3x3(asfloat(buffer.Load3((offset + 0u))), "
                 "asfloat(buffer.Load3((offset + 16u))), "
                 "asfloat(buffer.Load3((offset + 32u))));"},
        TypeCase{ty_mat3x4<f32>,
                 "return float3x4(asfloat(buffer.Load4((offset + 0u))), "
                 "asfloat(buffer.Load4((offset + 16u))), "
                 "asfloat(buffer.Load4((offset + 32u))));"},
        TypeCase{ty_mat4x2<f32>,
                 "return float4x2(asfloat(buffer.Load2((offset + 0u))), "
                 "asfloat(buffer.Load2((offset + 8u))), "
                 "asfloat(buffer.Load2((offset + 16u))), "
                 "asfloat(buffer.Load2((offset + 24u))));"},
        TypeCase{ty_mat4x3<f32>,
                 "return float4x3(asfloat(buffer.Load3((offset + 0u))), "
                 "asfloat(buffer.Load3((offset + 16u))), "
                 "asfloat(buffer.Load3((offset + 32u))), "
                 "asfloat(buffer.Load3((offset + 48u))));"},
        TypeCase{ty_mat4x4<f32>,
                 "return float4x4(asfloat(buffer.Load4((offset + 0u))), "
                 "asfloat(buffer.Load4((offset + 16u))), "
                 "asfloat(buffer.Load4((offset + 32u))), "
                 "asfloat(buffer.Load4((offset + 48u))));"},
        TypeCase{ty_mat2x2<f16>,
                 "return matrix<float16_t, 2, 2>("
                 "buffer.Load<vector<float16_t, 2> >((offset + 0u)), "
                 "buffer.Load<vector<float16_t, 2> >((offset + 4u)));"},
        TypeCase{ty_mat2x3<f16>,
                 "return matrix<float16_t, 2, 3>("
                 "buffer.Load<vector<float16_t, 3> >((offset + 0u)), "
                 "buffer.Load<vector<float16_t, 3> >((offset + 8u)));"},
        TypeCase{ty_mat2x4<f16>,
                 "return matrix<float16_t, 2, 4>("
                 "buffer.Load<vector<float16_t, 4> >((offset + 0u)), "
                 "buffer.Load<vector<float16_t, 4> >((offset + 8u)));"},
        TypeCase{ty_mat3x2<f16>,
                 "return matrix<float16_t, 3, 2>("
                 "buffer.Load<vector<float16_t, 2> >((offset + 0u)), "
                 "buffer.Load<vector<float16_t, 2> >((offset + 4u)), "
                 "buffer.Load<vector<float16_t, 2> >((offset + 8u)));"},
        TypeCase{ty_mat3x3<f16>,
                 "return matrix<float16_t, 3, 3>("
                 "buffer.Load<vector<float16_t, 3> >((offset + 0u)), "
                 "buffer.Load<vector<float16_t, 3> >((offset + 8u)), "
                 "buffer.Load<vector<float16_t, 3> >((offset + 16u)));"},
        TypeCase{ty_mat3x4<f16>,
                 "return matrix<float16_t, 3, 4>("
                 "buffer.Load<vector<float16_t, 4> >((offset + 0u)), "
                 "buffer.Load<vector<float16_t, 4> >((offset + 8u)), "
                 "buffer.Load<vector<float16_t, 4> >((offset + 16u)));"},
        TypeCase{ty_mat4x2<f16>,
                 "return matrix<float16_t, 4, 2>("
                 "buffer.Load<vector<float16_t, 2> >((offset + 0u)), "
                 "buffer.Load<vector<float16_t, 2> >((offset + 4u)), "
                 "buffer.Load<vector<float16_t, 2> >((offset + 8u)), "
                 "buffer.Load<vector<float16_t, 2> >((offset + 12u)));"},
        TypeCase{ty_mat4x3<f16>,
                 "return matrix<float16_t, 4, 3>("
                 "buffer.Load<vector<float16_t, 3> >((offset + 0u)), "
                 "buffer.Load<vector<float16_t, 3> >((offset + 8u)), "
                 "buffer.Load<vector<float16_t, 3> >((offset + 16u)), "
                 "buffer.Load<vector<float16_t, 3> >((offset + 24u)));"},
        TypeCase{ty_mat4x4<f16>,
                 "return matrix<float16_t, 4, 4>("
                 "buffer.Load<vector<float16_t, 4> >((offset + 0u)), "
                 "buffer.Load<vector<float16_t, 4> >((offset + 8u)), "
                 "buffer.Load<vector<float16_t, 4> >((offset + 16u)), "
                 "buffer.Load<vector<float16_t, 4> >((offset + 24u)));"}));

using HlslGeneratorImplTest_MemberAccessor_UniformBufferLoad_ConstantOffset =
    HlslGeneratorImplTest_MemberAccessorWithParam<TypeCase>;
TEST_P(HlslGeneratorImplTest_MemberAccessor_UniformBufferLoad_ConstantOffset, Test) {
    // struct Data {
    //   a : i32,
    //   b : <type>,
    // };
    // var<uniform> data : Data;
    // data.b;

    auto p = GetParam();

    Enable(ast::Extension::kF16);

    SetupUniformBuffer(utils::Vector{
        Member("a", ty.i32()),
        Member("b", p.member_type(ty)),
    });

    SetupFunction(utils::Vector{
        Decl(Var("x", MemberAccessor("data", "b"))),
    });

    GeneratorImpl& gen = SanitizeAndBuild();

    ASSERT_TRUE(gen.Generate()) << gen.error();
    EXPECT_THAT(gen.result(), HasSubstr(p.expected));
}

INSTANTIATE_TEST_SUITE_P(
    HlslGeneratorImplTest_MemberAccessor,
    HlslGeneratorImplTest_MemberAccessor_UniformBufferLoad_ConstantOffset,
    testing::Values(TypeCase{ty_u32, "uint x = data[0].y;"},
                    TypeCase{ty_f32, "float x = asfloat(data[0].y);"},
                    TypeCase{ty_i32, "int x = asint(data[0].y);"},
                    TypeCase{ty_f16, "float16_t x = float16_t(f16tof32(((data[0].y) & 0xFFFF)));"},
                    TypeCase{ty_vec2<u32>, "uint2 x = data[0].zw;"},
                    TypeCase{ty_vec2<f32>, "float2 x = asfloat(data[0].zw);"},
                    TypeCase{ty_vec2<i32>, "int2 x = asint(data[0].zw);"},
                    TypeCase{ty_vec2<f16>, R"(uint ubo_load = data[0].y;
  vector<float16_t, 2> x = vector<float16_t, 2>(float16_t(f16tof32(ubo_load & 0xFFFF)), float16_t(f16tof32(ubo_load >> 16)));)"},
                    TypeCase{ty_vec3<u32>, "uint3 x = data[1].xyz;"},
                    TypeCase{ty_vec3<f32>, "float3 x = asfloat(data[1].xyz);"},
                    TypeCase{ty_vec3<i32>, "int3 x = asint(data[1].xyz);"},
                    TypeCase{ty_vec3<f16>, R"(uint2 ubo_load = data[0].zw;
  vector<float16_t, 2> ubo_load_xz = vector<float16_t, 2>(f16tof32(ubo_load & 0xFFFF));
  float16_t ubo_load_y = f16tof32(ubo_load[0] >> 16);
  vector<float16_t, 3> x = vector<float16_t, 3>(ubo_load_xz[0], ubo_load_y, ubo_load_xz[1]);)"},
                    TypeCase{ty_vec4<u32>, "uint4 x = data[1];"},
                    TypeCase{ty_vec4<f32>, "float4 x = asfloat(data[1]);"},
                    TypeCase{ty_vec4<i32>, "int4 x = asint(data[1]);"},
                    TypeCase{ty_vec4<f16>,
                             R"(uint2 ubo_load = data[0].zw;
  vector<float16_t, 2> ubo_load_xz = vector<float16_t, 2>(f16tof32(ubo_load & 0xFFFF));
  vector<float16_t, 2> ubo_load_yw = vector<float16_t, 2>(f16tof32(ubo_load >> 16));
  vector<float16_t, 4> x = vector<float16_t, 4>(ubo_load_xz[0], ubo_load_yw[0], ubo_load_xz[1], ubo_load_yw[1]);)"},
                    TypeCase{ty_mat2x2<f32>, R"(float2x2 tint_symbol(uint4 buffer[2], uint offset) {
  const uint scalar_offset = ((offset + 0u)) / 4;
  uint4 ubo_load = buffer[scalar_offset / 4];
  const uint scalar_offset_1 = ((offset + 8u)) / 4;
  uint4 ubo_load_1 = buffer[scalar_offset_1 / 4];
  return float2x2(asfloat(((scalar_offset & 2) ? ubo_load.zw : ubo_load.xy)), asfloat(((scalar_offset_1 & 2) ? ubo_load_1.zw : ubo_load_1.xy)));
})"},
                    TypeCase{ty_mat2x3<f32>, R"(float2x3 tint_symbol(uint4 buffer[3], uint offset) {
  const uint scalar_offset = ((offset + 0u)) / 4;
  const uint scalar_offset_1 = ((offset + 16u)) / 4;
  return float2x3(asfloat(buffer[scalar_offset / 4].xyz), asfloat(buffer[scalar_offset_1 / 4].xyz));
})"},
                    TypeCase{ty_mat2x4<f32>, R"(float2x4 tint_symbol(uint4 buffer[3], uint offset) {
  const uint scalar_offset = ((offset + 0u)) / 4;
  const uint scalar_offset_1 = ((offset + 16u)) / 4;
  return float2x4(asfloat(buffer[scalar_offset / 4]), asfloat(buffer[scalar_offset_1 / 4]));
})"},
                    TypeCase{ty_mat3x2<f32>, R"(float3x2 tint_symbol(uint4 buffer[2], uint offset) {
  const uint scalar_offset = ((offset + 0u)) / 4;
  uint4 ubo_load = buffer[scalar_offset / 4];
  const uint scalar_offset_1 = ((offset + 8u)) / 4;
  uint4 ubo_load_1 = buffer[scalar_offset_1 / 4];
  const uint scalar_offset_2 = ((offset + 16u)) / 4;
  uint4 ubo_load_2 = buffer[scalar_offset_2 / 4];
  return float3x2(asfloat(((scalar_offset & 2) ? ubo_load.zw : ubo_load.xy)), asfloat(((scalar_offset_1 & 2) ? ubo_load_1.zw : ubo_load_1.xy)), asfloat(((scalar_offset_2 & 2) ? ubo_load_2.zw : ubo_load_2.xy)));
})"},
                    TypeCase{ty_mat3x3<f32>, R"(float3x3 tint_symbol(uint4 buffer[4], uint offset) {
  const uint scalar_offset = ((offset + 0u)) / 4;
  const uint scalar_offset_1 = ((offset + 16u)) / 4;
  const uint scalar_offset_2 = ((offset + 32u)) / 4;
  return float3x3(asfloat(buffer[scalar_offset / 4].xyz), asfloat(buffer[scalar_offset_1 / 4].xyz), asfloat(buffer[scalar_offset_2 / 4].xyz));
})"},
                    TypeCase{ty_mat3x4<f32>, R"(float3x4 tint_symbol(uint4 buffer[4], uint offset) {
  const uint scalar_offset = ((offset + 0u)) / 4;
  const uint scalar_offset_1 = ((offset + 16u)) / 4;
  const uint scalar_offset_2 = ((offset + 32u)) / 4;
  return float3x4(asfloat(buffer[scalar_offset / 4]), asfloat(buffer[scalar_offset_1 / 4]), asfloat(buffer[scalar_offset_2 / 4]));
})"},
                    TypeCase{ty_mat4x2<f32>, R"(float4x2 tint_symbol(uint4 buffer[3], uint offset) {
  const uint scalar_offset = ((offset + 0u)) / 4;
  uint4 ubo_load = buffer[scalar_offset / 4];
  const uint scalar_offset_1 = ((offset + 8u)) / 4;
  uint4 ubo_load_1 = buffer[scalar_offset_1 / 4];
  const uint scalar_offset_2 = ((offset + 16u)) / 4;
  uint4 ubo_load_2 = buffer[scalar_offset_2 / 4];
  const uint scalar_offset_3 = ((offset + 24u)) / 4;
  uint4 ubo_load_3 = buffer[scalar_offset_3 / 4];
  return float4x2(asfloat(((scalar_offset & 2) ? ubo_load.zw : ubo_load.xy)), asfloat(((scalar_offset_1 & 2) ? ubo_load_1.zw : ubo_load_1.xy)), asfloat(((scalar_offset_2 & 2) ? ubo_load_2.zw : ubo_load_2.xy)), asfloat(((scalar_offset_3 & 2) ? ubo_load_3.zw : ubo_load_3.xy)));
})"},
                    TypeCase{ty_mat4x3<f32>, R"(float4x3 tint_symbol(uint4 buffer[5], uint offset) {
  const uint scalar_offset = ((offset + 0u)) / 4;
  const uint scalar_offset_1 = ((offset + 16u)) / 4;
  const uint scalar_offset_2 = ((offset + 32u)) / 4;
  const uint scalar_offset_3 = ((offset + 48u)) / 4;
  return float4x3(asfloat(buffer[scalar_offset / 4].xyz), asfloat(buffer[scalar_offset_1 / 4].xyz), asfloat(buffer[scalar_offset_2 / 4].xyz), asfloat(buffer[scalar_offset_3 / 4].xyz));
})"},
                    TypeCase{ty_mat4x4<f32>, R"(float4x4 tint_symbol(uint4 buffer[5], uint offset) {
  const uint scalar_offset = ((offset + 0u)) / 4;
  const uint scalar_offset_1 = ((offset + 16u)) / 4;
  const uint scalar_offset_2 = ((offset + 32u)) / 4;
  const uint scalar_offset_3 = ((offset + 48u)) / 4;
  return float4x4(asfloat(buffer[scalar_offset / 4]), asfloat(buffer[scalar_offset_1 / 4]), asfloat(buffer[scalar_offset_2 / 4]), asfloat(buffer[scalar_offset_3 / 4]));
})"},
                    TypeCase{ty_mat2x2<f16>,
                             R"(matrix<float16_t, 2, 2> tint_symbol(uint4 buffer[1], uint offset) {
  const uint scalar_offset = ((offset + 0u)) / 4;
  uint ubo_load = buffer[scalar_offset / 4][scalar_offset % 4];
  const uint scalar_offset_1 = ((offset + 4u)) / 4;
  uint ubo_load_1 = buffer[scalar_offset_1 / 4][scalar_offset_1 % 4];
  return matrix<float16_t, 2, 2>(vector<float16_t, 2>(float16_t(f16tof32(ubo_load & 0xFFFF)), float16_t(f16tof32(ubo_load >> 16))), vector<float16_t, 2>(float16_t(f16tof32(ubo_load_1 & 0xFFFF)), float16_t(f16tof32(ubo_load_1 >> 16))));
})"},
                    TypeCase{ty_mat2x3<f16>,
                             R"(matrix<float16_t, 2, 3> tint_symbol(uint4 buffer[2], uint offset) {
  const uint scalar_offset = ((offset + 0u)) / 4;
  uint4 ubo_load_1 = buffer[scalar_offset / 4];
  uint2 ubo_load = ((scalar_offset & 2) ? ubo_load_1.zw : ubo_load_1.xy);
  vector<float16_t, 2> ubo_load_xz = vector<float16_t, 2>(f16tof32(ubo_load & 0xFFFF));
  float16_t ubo_load_y = f16tof32(ubo_load[0] >> 16);
  const uint scalar_offset_1 = ((offset + 8u)) / 4;
  uint4 ubo_load_3 = buffer[scalar_offset_1 / 4];
  uint2 ubo_load_2 = ((scalar_offset_1 & 2) ? ubo_load_3.zw : ubo_load_3.xy);
  vector<float16_t, 2> ubo_load_2_xz = vector<float16_t, 2>(f16tof32(ubo_load_2 & 0xFFFF));
  float16_t ubo_load_2_y = f16tof32(ubo_load_2[0] >> 16);
  return matrix<float16_t, 2, 3>(vector<float16_t, 3>(ubo_load_xz[0], ubo_load_y, ubo_load_xz[1]), vector<float16_t, 3>(ubo_load_2_xz[0], ubo_load_2_y, ubo_load_2_xz[1]));
})"},
                    TypeCase{ty_mat2x4<f16>,
                             R"(matrix<float16_t, 2, 4> tint_symbol(uint4 buffer[2], uint offset) {
  const uint scalar_offset = ((offset + 0u)) / 4;
  uint4 ubo_load_1 = buffer[scalar_offset / 4];
  uint2 ubo_load = ((scalar_offset & 2) ? ubo_load_1.zw : ubo_load_1.xy);
  vector<float16_t, 2> ubo_load_xz = vector<float16_t, 2>(f16tof32(ubo_load & 0xFFFF));
  vector<float16_t, 2> ubo_load_yw = vector<float16_t, 2>(f16tof32(ubo_load >> 16));
  const uint scalar_offset_1 = ((offset + 8u)) / 4;
  uint4 ubo_load_3 = buffer[scalar_offset_1 / 4];
  uint2 ubo_load_2 = ((scalar_offset_1 & 2) ? ubo_load_3.zw : ubo_load_3.xy);
  vector<float16_t, 2> ubo_load_2_xz = vector<float16_t, 2>(f16tof32(ubo_load_2 & 0xFFFF));
  vector<float16_t, 2> ubo_load_2_yw = vector<float16_t, 2>(f16tof32(ubo_load_2 >> 16));
  return matrix<float16_t, 2, 4>(vector<float16_t, 4>(ubo_load_xz[0], ubo_load_yw[0], ubo_load_xz[1], ubo_load_yw[1]), vector<float16_t, 4>(ubo_load_2_xz[0], ubo_load_2_yw[0], ubo_load_2_xz[1], ubo_load_2_yw[1]));
})"},
                    TypeCase{ty_mat3x2<f16>,
                             R"(matrix<float16_t, 3, 2> tint_symbol(uint4 buffer[1], uint offset) {
  const uint scalar_offset = ((offset + 0u)) / 4;
  uint ubo_load = buffer[scalar_offset / 4][scalar_offset % 4];
  const uint scalar_offset_1 = ((offset + 4u)) / 4;
  uint ubo_load_1 = buffer[scalar_offset_1 / 4][scalar_offset_1 % 4];
  const uint scalar_offset_2 = ((offset + 8u)) / 4;
  uint ubo_load_2 = buffer[scalar_offset_2 / 4][scalar_offset_2 % 4];
  return matrix<float16_t, 3, 2>(vector<float16_t, 2>(float16_t(f16tof32(ubo_load & 0xFFFF)), float16_t(f16tof32(ubo_load >> 16))), vector<float16_t, 2>(float16_t(f16tof32(ubo_load_1 & 0xFFFF)), float16_t(f16tof32(ubo_load_1 >> 16))), vector<float16_t, 2>(float16_t(f16tof32(ubo_load_2 & 0xFFFF)), float16_t(f16tof32(ubo_load_2 >> 16))));
})"},
                    TypeCase{ty_mat3x3<f16>,
                             R"(matrix<float16_t, 3, 3> tint_symbol(uint4 buffer[2], uint offset) {
  const uint scalar_offset = ((offset + 0u)) / 4;
  uint4 ubo_load_1 = buffer[scalar_offset / 4];
  uint2 ubo_load = ((scalar_offset & 2) ? ubo_load_1.zw : ubo_load_1.xy);
  vector<float16_t, 2> ubo_load_xz = vector<float16_t, 2>(f16tof32(ubo_load & 0xFFFF));
  float16_t ubo_load_y = f16tof32(ubo_load[0] >> 16);
  const uint scalar_offset_1 = ((offset + 8u)) / 4;
  uint4 ubo_load_3 = buffer[scalar_offset_1 / 4];
  uint2 ubo_load_2 = ((scalar_offset_1 & 2) ? ubo_load_3.zw : ubo_load_3.xy);
  vector<float16_t, 2> ubo_load_2_xz = vector<float16_t, 2>(f16tof32(ubo_load_2 & 0xFFFF));
  float16_t ubo_load_2_y = f16tof32(ubo_load_2[0] >> 16);
  const uint scalar_offset_2 = ((offset + 16u)) / 4;
  uint4 ubo_load_5 = buffer[scalar_offset_2 / 4];
  uint2 ubo_load_4 = ((scalar_offset_2 & 2) ? ubo_load_5.zw : ubo_load_5.xy);
  vector<float16_t, 2> ubo_load_4_xz = vector<float16_t, 2>(f16tof32(ubo_load_4 & 0xFFFF));
  float16_t ubo_load_4_y = f16tof32(ubo_load_4[0] >> 16);
  return matrix<float16_t, 3, 3>(vector<float16_t, 3>(ubo_load_xz[0], ubo_load_y, ubo_load_xz[1]), vector<float16_t, 3>(ubo_load_2_xz[0], ubo_load_2_y, ubo_load_2_xz[1]), vector<float16_t, 3>(ubo_load_4_xz[0], ubo_load_4_y, ubo_load_4_xz[1]));
})"},
                    TypeCase{ty_mat3x4<f16>,
                             R"(matrix<float16_t, 3, 4> tint_symbol(uint4 buffer[2], uint offset) {
  const uint scalar_offset = ((offset + 0u)) / 4;
  uint4 ubo_load_1 = buffer[scalar_offset / 4];
  uint2 ubo_load = ((scalar_offset & 2) ? ubo_load_1.zw : ubo_load_1.xy);
  vector<float16_t, 2> ubo_load_xz = vector<float16_t, 2>(f16tof32(ubo_load & 0xFFFF));
  vector<float16_t, 2> ubo_load_yw = vector<float16_t, 2>(f16tof32(ubo_load >> 16));
  const uint scalar_offset_1 = ((offset + 8u)) / 4;
  uint4 ubo_load_3 = buffer[scalar_offset_1 / 4];
  uint2 ubo_load_2 = ((scalar_offset_1 & 2) ? ubo_load_3.zw : ubo_load_3.xy);
  vector<float16_t, 2> ubo_load_2_xz = vector<float16_t, 2>(f16tof32(ubo_load_2 & 0xFFFF));
  vector<float16_t, 2> ubo_load_2_yw = vector<float16_t, 2>(f16tof32(ubo_load_2 >> 16));
  const uint scalar_offset_2 = ((offset + 16u)) / 4;
  uint4 ubo_load_5 = buffer[scalar_offset_2 / 4];
  uint2 ubo_load_4 = ((scalar_offset_2 & 2) ? ubo_load_5.zw : ubo_load_5.xy);
  vector<float16_t, 2> ubo_load_4_xz = vector<float16_t, 2>(f16tof32(ubo_load_4 & 0xFFFF));
  vector<float16_t, 2> ubo_load_4_yw = vector<float16_t, 2>(f16tof32(ubo_load_4 >> 16));
  return matrix<float16_t, 3, 4>(vector<float16_t, 4>(ubo_load_xz[0], ubo_load_yw[0], ubo_load_xz[1], ubo_load_yw[1]), vector<float16_t, 4>(ubo_load_2_xz[0], ubo_load_2_yw[0], ubo_load_2_xz[1], ubo_load_2_yw[1]), vector<float16_t, 4>(ubo_load_4_xz[0], ubo_load_4_yw[0], ubo_load_4_xz[1], ubo_load_4_yw[1]));)"},
                    TypeCase{ty_mat4x2<f16>,
                             R"(matrix<float16_t, 4, 2> tint_symbol(uint4 buffer[2], uint offset) {
  const uint scalar_offset = ((offset + 0u)) / 4;
  uint ubo_load = buffer[scalar_offset / 4][scalar_offset % 4];
  const uint scalar_offset_1 = ((offset + 4u)) / 4;
  uint ubo_load_1 = buffer[scalar_offset_1 / 4][scalar_offset_1 % 4];
  const uint scalar_offset_2 = ((offset + 8u)) / 4;
  uint ubo_load_2 = buffer[scalar_offset_2 / 4][scalar_offset_2 % 4];
  const uint scalar_offset_3 = ((offset + 12u)) / 4;
  uint ubo_load_3 = buffer[scalar_offset_3 / 4][scalar_offset_3 % 4];
  return matrix<float16_t, 4, 2>(vector<float16_t, 2>(float16_t(f16tof32(ubo_load & 0xFFFF)), float16_t(f16tof32(ubo_load >> 16))), vector<float16_t, 2>(float16_t(f16tof32(ubo_load_1 & 0xFFFF)), float16_t(f16tof32(ubo_load_1 >> 16))), vector<float16_t, 2>(float16_t(f16tof32(ubo_load_2 & 0xFFFF)), float16_t(f16tof32(ubo_load_2 >> 16))), vector<float16_t, 2>(float16_t(f16tof32(ubo_load_3 & 0xFFFF)), float16_t(f16tof32(ubo_load_3 >> 16))));
})"},
                    TypeCase{ty_mat4x3<f16>,
                             R"(matrix<float16_t, 4, 3> tint_symbol(uint4 buffer[3], uint offset) {
  const uint scalar_offset = ((offset + 0u)) / 4;
  uint4 ubo_load_1 = buffer[scalar_offset / 4];
  uint2 ubo_load = ((scalar_offset & 2) ? ubo_load_1.zw : ubo_load_1.xy);
  vector<float16_t, 2> ubo_load_xz = vector<float16_t, 2>(f16tof32(ubo_load & 0xFFFF));
  float16_t ubo_load_y = f16tof32(ubo_load[0] >> 16);
  const uint scalar_offset_1 = ((offset + 8u)) / 4;
  uint4 ubo_load_3 = buffer[scalar_offset_1 / 4];
  uint2 ubo_load_2 = ((scalar_offset_1 & 2) ? ubo_load_3.zw : ubo_load_3.xy);
  vector<float16_t, 2> ubo_load_2_xz = vector<float16_t, 2>(f16tof32(ubo_load_2 & 0xFFFF));
  float16_t ubo_load_2_y = f16tof32(ubo_load_2[0] >> 16);
  const uint scalar_offset_2 = ((offset + 16u)) / 4;
  uint4 ubo_load_5 = buffer[scalar_offset_2 / 4];
  uint2 ubo_load_4 = ((scalar_offset_2 & 2) ? ubo_load_5.zw : ubo_load_5.xy);
  vector<float16_t, 2> ubo_load_4_xz = vector<float16_t, 2>(f16tof32(ubo_load_4 & 0xFFFF));
  float16_t ubo_load_4_y = f16tof32(ubo_load_4[0] >> 16);
  const uint scalar_offset_3 = ((offset + 24u)) / 4;
  uint4 ubo_load_7 = buffer[scalar_offset_3 / 4];
  uint2 ubo_load_6 = ((scalar_offset_3 & 2) ? ubo_load_7.zw : ubo_load_7.xy);
  vector<float16_t, 2> ubo_load_6_xz = vector<float16_t, 2>(f16tof32(ubo_load_6 & 0xFFFF));
  float16_t ubo_load_6_y = f16tof32(ubo_load_6[0] >> 16);
  return matrix<float16_t, 4, 3>(vector<float16_t, 3>(ubo_load_xz[0], ubo_load_y, ubo_load_xz[1]), vector<float16_t, 3>(ubo_load_2_xz[0], ubo_load_2_y, ubo_load_2_xz[1]), vector<float16_t, 3>(ubo_load_4_xz[0], ubo_load_4_y, ubo_load_4_xz[1]), vector<float16_t, 3>(ubo_load_6_xz[0], ubo_load_6_y, ubo_load_6_xz[1]));
})"},
                    TypeCase{ty_mat4x4<f16>,
                             R"(matrix<float16_t, 4, 4> tint_symbol(uint4 buffer[3], uint offset) {
  const uint scalar_offset = ((offset + 0u)) / 4;
  uint4 ubo_load_1 = buffer[scalar_offset / 4];
  uint2 ubo_load = ((scalar_offset & 2) ? ubo_load_1.zw : ubo_load_1.xy);
  vector<float16_t, 2> ubo_load_xz = vector<float16_t, 2>(f16tof32(ubo_load & 0xFFFF));
  vector<float16_t, 2> ubo_load_yw = vector<float16_t, 2>(f16tof32(ubo_load >> 16));
  const uint scalar_offset_1 = ((offset + 8u)) / 4;
  uint4 ubo_load_3 = buffer[scalar_offset_1 / 4];
  uint2 ubo_load_2 = ((scalar_offset_1 & 2) ? ubo_load_3.zw : ubo_load_3.xy);
  vector<float16_t, 2> ubo_load_2_xz = vector<float16_t, 2>(f16tof32(ubo_load_2 & 0xFFFF));
  vector<float16_t, 2> ubo_load_2_yw = vector<float16_t, 2>(f16tof32(ubo_load_2 >> 16));
  const uint scalar_offset_2 = ((offset + 16u)) / 4;
  uint4 ubo_load_5 = buffer[scalar_offset_2 / 4];
  uint2 ubo_load_4 = ((scalar_offset_2 & 2) ? ubo_load_5.zw : ubo_load_5.xy);
  vector<float16_t, 2> ubo_load_4_xz = vector<float16_t, 2>(f16tof32(ubo_load_4 & 0xFFFF));
  vector<float16_t, 2> ubo_load_4_yw = vector<float16_t, 2>(f16tof32(ubo_load_4 >> 16));
  const uint scalar_offset_3 = ((offset + 24u)) / 4;
  uint4 ubo_load_7 = buffer[scalar_offset_3 / 4];
  uint2 ubo_load_6 = ((scalar_offset_3 & 2) ? ubo_load_7.zw : ubo_load_7.xy);
  vector<float16_t, 2> ubo_load_6_xz = vector<float16_t, 2>(f16tof32(ubo_load_6 & 0xFFFF));
  vector<float16_t, 2> ubo_load_6_yw = vector<float16_t, 2>(f16tof32(ubo_load_6 >> 16));
  return matrix<float16_t, 4, 4>(vector<float16_t, 4>(ubo_load_xz[0], ubo_load_yw[0], ubo_load_xz[1], ubo_load_yw[1]), vector<float16_t, 4>(ubo_load_2_xz[0], ubo_load_2_yw[0], ubo_load_2_xz[1], ubo_load_2_yw[1]), vector<float16_t, 4>(ubo_load_4_xz[0], ubo_load_4_yw[0], ubo_load_4_xz[1], ubo_load_4_yw[1]), vector<float16_t, 4>(ubo_load_6_xz[0], ubo_load_6_yw[0], ubo_load_6_xz[1], ubo_load_6_yw[1]));
})"}));

using HlslGeneratorImplTest_MemberAccessor_UniformBufferLoad_DynamicOffset =
    HlslGeneratorImplTest_MemberAccessorWithParam<TypeCase>;

TEST_P(HlslGeneratorImplTest_MemberAccessor_UniformBufferLoad_DynamicOffset, Test) {
    // struct Inner {
    //   a : i32,
    //   b : <type>,
    //   c : vec4<i32>,
    // };
    // struct Data {
    //  arr : array<Inner, 4i>,
    // }
    // var<uniform> data : Data;
    // data.arr[i].b;

    auto p = GetParam();

    Enable(ast::Extension::kF16);

    auto* inner = Structure("Inner", utils::Vector{
                                         Member("a", ty.i32()),
                                         Member("b", p.member_type(ty)),
                                         Member("c", ty.vec4(ty.i32())),
                                     });

    SetupUniformBuffer(utils::Vector{
        Member("arr", ty.array(ty.Of(inner), 4_i)),
    });

    auto* i = Var("i", Expr(2_i));

    SetupFunction(utils::Vector{
        Decl(i),
        Decl(Var("x", MemberAccessor(IndexAccessor(MemberAccessor("data", "arr"), i), "b"))),
    });

    GeneratorImpl& gen = SanitizeAndBuild();

    ASSERT_TRUE(gen.Generate()) << gen.error();
    EXPECT_THAT(gen.result(), HasSubstr(p.expected));
}

INSTANTIATE_TEST_SUITE_P(
    HlslGeneratorImplTest_MemberAccessor,
    HlslGeneratorImplTest_MemberAccessor_UniformBufferLoad_DynamicOffset,
    testing::Values(
        TypeCase{ty_u32, "x = data[scalar_offset / 4][scalar_offset % 4]"},
        TypeCase{ty_f32, "x = asfloat(data[scalar_offset / 4][scalar_offset % 4])"},
        TypeCase{ty_i32, "x = asint(data[scalar_offset / 4][scalar_offset % 4])"},
        TypeCase{ty_f16, R"(const uint scalar_offset_bytes = (((32u * uint(i)) + 4u));
  const uint scalar_offset_index = scalar_offset_bytes / 4;
  float16_t x = float16_t(f16tof32(((data[scalar_offset_index / 4][scalar_offset_index % 4] >> (scalar_offset_bytes % 4 == 0 ? 0 : 16)) & 0xFFFF)));)"},
        TypeCase{ty_vec2<u32>, R"(uint4 ubo_load = data[scalar_offset / 4];
  uint2 x = ((scalar_offset & 2) ? ubo_load.zw : ubo_load.xy);)"},
        TypeCase{ty_vec2<f32>, R"(uint4 ubo_load = data[scalar_offset / 4];
  float2 x = asfloat(((scalar_offset & 2) ? ubo_load.zw : ubo_load.xy));)"},
        TypeCase{ty_vec2<i32>, R"(uint4 ubo_load = data[scalar_offset / 4];
  int2 x = asint(((scalar_offset & 2) ? ubo_load.zw : ubo_load.xy));)"},
        TypeCase{ty_vec2<f16>, R"(const uint scalar_offset = (((32u * uint(i)) + 4u)) / 4;
  uint ubo_load = data[scalar_offset / 4][scalar_offset % 4];
  vector<float16_t, 2> x = vector<float16_t, 2>(float16_t(f16tof32(ubo_load & 0xFFFF)), float16_t(f16tof32(ubo_load >> 16)));)"},
        TypeCase{ty_vec3<u32>, "x = data[scalar_offset / 4].xyz"},
        TypeCase{ty_vec3<f32>, "x = asfloat(data[scalar_offset / 4].xyz)"},
        TypeCase{ty_vec3<i32>, "x = asint(data[scalar_offset / 4].xyz)"},
        TypeCase{ty_vec3<f16>, R"(const uint scalar_offset = (((32u * uint(i)) + 8u)) / 4;
  uint4 ubo_load_1 = data[scalar_offset / 4];
  uint2 ubo_load = ((scalar_offset & 2) ? ubo_load_1.zw : ubo_load_1.xy);
  vector<float16_t, 2> ubo_load_xz = vector<float16_t, 2>(f16tof32(ubo_load & 0xFFFF));
  float16_t ubo_load_y = f16tof32(ubo_load[0] >> 16);
  vector<float16_t, 3> x = vector<float16_t, 3>(ubo_load_xz[0], ubo_load_y, ubo_load_xz[1]);)"},
        TypeCase{ty_vec4<u32>, "x = data[scalar_offset / 4]"},
        TypeCase{ty_vec4<f32>, "x = asfloat(data[scalar_offset / 4])"},
        TypeCase{ty_vec4<i32>, "x = asint(data[scalar_offset / 4])"},
        TypeCase{ty_vec4<f16>, R"(const uint scalar_offset = (((32u * uint(i)) + 8u)) / 4;
  uint4 ubo_load_1 = data[scalar_offset / 4];
  uint2 ubo_load = ((scalar_offset & 2) ? ubo_load_1.zw : ubo_load_1.xy);
  vector<float16_t, 2> ubo_load_xz = vector<float16_t, 2>(f16tof32(ubo_load & 0xFFFF));
  vector<float16_t, 2> ubo_load_yw = vector<float16_t, 2>(f16tof32(ubo_load >> 16));
  vector<float16_t, 4> x = vector<float16_t, 4>(ubo_load_xz[0], ubo_load_yw[0], ubo_load_xz[1], ubo_load_yw[1]);)"},
        TypeCase{ty_mat2x2<f32>, R"(float2x2 tint_symbol(uint4 buffer[12], uint offset) {
  const uint scalar_offset = ((offset + 0u)) / 4;
  uint4 ubo_load = buffer[scalar_offset / 4];
  const uint scalar_offset_1 = ((offset + 8u)) / 4;
  uint4 ubo_load_1 = buffer[scalar_offset_1 / 4];
  return float2x2(asfloat(((scalar_offset & 2) ? ubo_load.zw : ubo_load.xy)), asfloat(((scalar_offset_1 & 2) ? ubo_load_1.zw : ubo_load_1.xy)));
})"},
        TypeCase{ty_mat2x3<f32>, R"(float2x3 tint_symbol(uint4 buffer[16], uint offset) {
  const uint scalar_offset = ((offset + 0u)) / 4;
  const uint scalar_offset_1 = ((offset + 16u)) / 4;
  return float2x3(asfloat(buffer[scalar_offset / 4].xyz), asfloat(buffer[scalar_offset_1 / 4].xyz));
})"},
        TypeCase{ty_mat2x4<f32>, R"(float2x4 tint_symbol(uint4 buffer[16], uint offset) {
  const uint scalar_offset = ((offset + 0u)) / 4;
  const uint scalar_offset_1 = ((offset + 16u)) / 4;
  return float2x4(asfloat(buffer[scalar_offset / 4]), asfloat(buffer[scalar_offset_1 / 4]));
})"},
        TypeCase{ty_mat3x2<f32>, R"(float3x2 tint_symbol(uint4 buffer[12], uint offset) {
  const uint scalar_offset = ((offset + 0u)) / 4;
  uint4 ubo_load = buffer[scalar_offset / 4];
  const uint scalar_offset_1 = ((offset + 8u)) / 4;
  uint4 ubo_load_1 = buffer[scalar_offset_1 / 4];
  const uint scalar_offset_2 = ((offset + 16u)) / 4;
  uint4 ubo_load_2 = buffer[scalar_offset_2 / 4];
  return float3x2(asfloat(((scalar_offset & 2) ? ubo_load.zw : ubo_load.xy)), asfloat(((scalar_offset_1 & 2) ? ubo_load_1.zw : ubo_load_1.xy)), asfloat(((scalar_offset_2 & 2) ? ubo_load_2.zw : ubo_load_2.xy)));
})"},
        TypeCase{ty_mat3x3<f32>, R"(float3x3 tint_symbol(uint4 buffer[20], uint offset) {
  const uint scalar_offset = ((offset + 0u)) / 4;
  const uint scalar_offset_1 = ((offset + 16u)) / 4;
  const uint scalar_offset_2 = ((offset + 32u)) / 4;
  return float3x3(asfloat(buffer[scalar_offset / 4].xyz), asfloat(buffer[scalar_offset_1 / 4].xyz), asfloat(buffer[scalar_offset_2 / 4].xyz));
})"},
        TypeCase{ty_mat3x4<f32>, R"(float3x4 tint_symbol(uint4 buffer[20], uint offset) {
  const uint scalar_offset = ((offset + 0u)) / 4;
  const uint scalar_offset_1 = ((offset + 16u)) / 4;
  const uint scalar_offset_2 = ((offset + 32u)) / 4;
  return float3x4(asfloat(buffer[scalar_offset / 4]), asfloat(buffer[scalar_offset_1 / 4]), asfloat(buffer[scalar_offset_2 / 4]));
})"},
        TypeCase{ty_mat4x2<f32>, R"(float4x2 tint_symbol(uint4 buffer[16], uint offset) {
  const uint scalar_offset = ((offset + 0u)) / 4;
  uint4 ubo_load = buffer[scalar_offset / 4];
  const uint scalar_offset_1 = ((offset + 8u)) / 4;
  uint4 ubo_load_1 = buffer[scalar_offset_1 / 4];
  const uint scalar_offset_2 = ((offset + 16u)) / 4;
  uint4 ubo_load_2 = buffer[scalar_offset_2 / 4];
  const uint scalar_offset_3 = ((offset + 24u)) / 4;
  uint4 ubo_load_3 = buffer[scalar_offset_3 / 4];
  return float4x2(asfloat(((scalar_offset & 2) ? ubo_load.zw : ubo_load.xy)), asfloat(((scalar_offset_1 & 2) ? ubo_load_1.zw : ubo_load_1.xy)), asfloat(((scalar_offset_2 & 2) ? ubo_load_2.zw : ubo_load_2.xy)), asfloat(((scalar_offset_3 & 2) ? ubo_load_3.zw : ubo_load_3.xy)));
})"},
        TypeCase{ty_mat4x3<f32>, R"(float4x3 tint_symbol(uint4 buffer[24], uint offset) {
  const uint scalar_offset = ((offset + 0u)) / 4;
  const uint scalar_offset_1 = ((offset + 16u)) / 4;
  const uint scalar_offset_2 = ((offset + 32u)) / 4;
  const uint scalar_offset_3 = ((offset + 48u)) / 4;
  return float4x3(asfloat(buffer[scalar_offset / 4].xyz), asfloat(buffer[scalar_offset_1 / 4].xyz), asfloat(buffer[scalar_offset_2 / 4].xyz), asfloat(buffer[scalar_offset_3 / 4].xyz));
})"},
        TypeCase{ty_mat4x4<f32>, R"(float4x4 tint_symbol(uint4 buffer[24], uint offset) {
  const uint scalar_offset = ((offset + 0u)) / 4;
  const uint scalar_offset_1 = ((offset + 16u)) / 4;
  const uint scalar_offset_2 = ((offset + 32u)) / 4;
  const uint scalar_offset_3 = ((offset + 48u)) / 4;
  return float4x4(asfloat(buffer[scalar_offset / 4]), asfloat(buffer[scalar_offset_1 / 4]), asfloat(buffer[scalar_offset_2 / 4]), asfloat(buffer[scalar_offset_3 / 4]));
})"},
        TypeCase{ty_mat2x2<f16>,
                 R"(matrix<float16_t, 2, 2> tint_symbol(uint4 buffer[8], uint offset) {
  const uint scalar_offset = ((offset + 0u)) / 4;
  uint ubo_load = buffer[scalar_offset / 4][scalar_offset % 4];
  const uint scalar_offset_1 = ((offset + 4u)) / 4;
  uint ubo_load_1 = buffer[scalar_offset_1 / 4][scalar_offset_1 % 4];
  return matrix<float16_t, 2, 2>(vector<float16_t, 2>(float16_t(f16tof32(ubo_load & 0xFFFF)), float16_t(f16tof32(ubo_load >> 16))), vector<float16_t, 2>(float16_t(f16tof32(ubo_load_1 & 0xFFFF)), float16_t(f16tof32(ubo_load_1 >> 16))));
})"},
        TypeCase{ty_mat2x3<f16>,
                 R"(matrix<float16_t, 2, 3> tint_symbol(uint4 buffer[12], uint offset) {
  const uint scalar_offset = ((offset + 0u)) / 4;
  uint4 ubo_load_1 = buffer[scalar_offset / 4];
  uint2 ubo_load = ((scalar_offset & 2) ? ubo_load_1.zw : ubo_load_1.xy);
  vector<float16_t, 2> ubo_load_xz = vector<float16_t, 2>(f16tof32(ubo_load & 0xFFFF));
  float16_t ubo_load_y = f16tof32(ubo_load[0] >> 16);
  const uint scalar_offset_1 = ((offset + 8u)) / 4;
  uint4 ubo_load_3 = buffer[scalar_offset_1 / 4];
  uint2 ubo_load_2 = ((scalar_offset_1 & 2) ? ubo_load_3.zw : ubo_load_3.xy);
  vector<float16_t, 2> ubo_load_2_xz = vector<float16_t, 2>(f16tof32(ubo_load_2 & 0xFFFF));
  float16_t ubo_load_2_y = f16tof32(ubo_load_2[0] >> 16);
  return matrix<float16_t, 2, 3>(vector<float16_t, 3>(ubo_load_xz[0], ubo_load_y, ubo_load_xz[1]), vector<float16_t, 3>(ubo_load_2_xz[0], ubo_load_2_y, ubo_load_2_xz[1]));
})"},
        TypeCase{ty_mat2x4<f16>,
                 R"(matrix<float16_t, 2, 4> tint_symbol(uint4 buffer[12], uint offset) {
  const uint scalar_offset = ((offset + 0u)) / 4;
  uint4 ubo_load_1 = buffer[scalar_offset / 4];
  uint2 ubo_load = ((scalar_offset & 2) ? ubo_load_1.zw : ubo_load_1.xy);
  vector<float16_t, 2> ubo_load_xz = vector<float16_t, 2>(f16tof32(ubo_load & 0xFFFF));
  vector<float16_t, 2> ubo_load_yw = vector<float16_t, 2>(f16tof32(ubo_load >> 16));
  const uint scalar_offset_1 = ((offset + 8u)) / 4;
  uint4 ubo_load_3 = buffer[scalar_offset_1 / 4];
  uint2 ubo_load_2 = ((scalar_offset_1 & 2) ? ubo_load_3.zw : ubo_load_3.xy);
  vector<float16_t, 2> ubo_load_2_xz = vector<float16_t, 2>(f16tof32(ubo_load_2 & 0xFFFF));
  vector<float16_t, 2> ubo_load_2_yw = vector<float16_t, 2>(f16tof32(ubo_load_2 >> 16));
  return matrix<float16_t, 2, 4>(vector<float16_t, 4>(ubo_load_xz[0], ubo_load_yw[0], ubo_load_xz[1], ubo_load_yw[1]), vector<float16_t, 4>(ubo_load_2_xz[0], ubo_load_2_yw[0], ubo_load_2_xz[1], ubo_load_2_yw[1]));
})"},
        TypeCase{ty_mat3x2<f16>,
                 R"(matrix<float16_t, 3, 2> tint_symbol(uint4 buffer[8], uint offset) {
  const uint scalar_offset = ((offset + 0u)) / 4;
  uint ubo_load = buffer[scalar_offset / 4][scalar_offset % 4];
  const uint scalar_offset_1 = ((offset + 4u)) / 4;
  uint ubo_load_1 = buffer[scalar_offset_1 / 4][scalar_offset_1 % 4];
  const uint scalar_offset_2 = ((offset + 8u)) / 4;
  uint ubo_load_2 = buffer[scalar_offset_2 / 4][scalar_offset_2 % 4];
  return matrix<float16_t, 3, 2>(vector<float16_t, 2>(float16_t(f16tof32(ubo_load & 0xFFFF)), float16_t(f16tof32(ubo_load >> 16))), vector<float16_t, 2>(float16_t(f16tof32(ubo_load_1 & 0xFFFF)), float16_t(f16tof32(ubo_load_1 >> 16))), vector<float16_t, 2>(float16_t(f16tof32(ubo_load_2 & 0xFFFF)), float16_t(f16tof32(ubo_load_2 >> 16))));
})"},
        TypeCase{ty_mat3x3<f16>,
                 R"(matrix<float16_t, 3, 3> tint_symbol(uint4 buffer[12], uint offset) {
  const uint scalar_offset = ((offset + 0u)) / 4;
  uint4 ubo_load_1 = buffer[scalar_offset / 4];
  uint2 ubo_load = ((scalar_offset & 2) ? ubo_load_1.zw : ubo_load_1.xy);
  vector<float16_t, 2> ubo_load_xz = vector<float16_t, 2>(f16tof32(ubo_load & 0xFFFF));
  float16_t ubo_load_y = f16tof32(ubo_load[0] >> 16);
  const uint scalar_offset_1 = ((offset + 8u)) / 4;
  uint4 ubo_load_3 = buffer[scalar_offset_1 / 4];
  uint2 ubo_load_2 = ((scalar_offset_1 & 2) ? ubo_load_3.zw : ubo_load_3.xy);
  vector<float16_t, 2> ubo_load_2_xz = vector<float16_t, 2>(f16tof32(ubo_load_2 & 0xFFFF));
  float16_t ubo_load_2_y = f16tof32(ubo_load_2[0] >> 16);
  const uint scalar_offset_2 = ((offset + 16u)) / 4;
  uint4 ubo_load_5 = buffer[scalar_offset_2 / 4];
  uint2 ubo_load_4 = ((scalar_offset_2 & 2) ? ubo_load_5.zw : ubo_load_5.xy);
  vector<float16_t, 2> ubo_load_4_xz = vector<float16_t, 2>(f16tof32(ubo_load_4 & 0xFFFF));
  float16_t ubo_load_4_y = f16tof32(ubo_load_4[0] >> 16);
  return matrix<float16_t, 3, 3>(vector<float16_t, 3>(ubo_load_xz[0], ubo_load_y, ubo_load_xz[1]), vector<float16_t, 3>(ubo_load_2_xz[0], ubo_load_2_y, ubo_load_2_xz[1]), vector<float16_t, 3>(ubo_load_4_xz[0], ubo_load_4_y, ubo_load_4_xz[1]));
})"},
        TypeCase{ty_mat3x4<f16>,
                 R"(matrix<float16_t, 3, 4> tint_symbol(uint4 buffer[12], uint offset) {
  const uint scalar_offset = ((offset + 0u)) / 4;
  uint4 ubo_load_1 = buffer[scalar_offset / 4];
  uint2 ubo_load = ((scalar_offset & 2) ? ubo_load_1.zw : ubo_load_1.xy);
  vector<float16_t, 2> ubo_load_xz = vector<float16_t, 2>(f16tof32(ubo_load & 0xFFFF));
  vector<float16_t, 2> ubo_load_yw = vector<float16_t, 2>(f16tof32(ubo_load >> 16));
  const uint scalar_offset_1 = ((offset + 8u)) / 4;
  uint4 ubo_load_3 = buffer[scalar_offset_1 / 4];
  uint2 ubo_load_2 = ((scalar_offset_1 & 2) ? ubo_load_3.zw : ubo_load_3.xy);
  vector<float16_t, 2> ubo_load_2_xz = vector<float16_t, 2>(f16tof32(ubo_load_2 & 0xFFFF));
  vector<float16_t, 2> ubo_load_2_yw = vector<float16_t, 2>(f16tof32(ubo_load_2 >> 16));
  const uint scalar_offset_2 = ((offset + 16u)) / 4;
  uint4 ubo_load_5 = buffer[scalar_offset_2 / 4];
  uint2 ubo_load_4 = ((scalar_offset_2 & 2) ? ubo_load_5.zw : ubo_load_5.xy);
  vector<float16_t, 2> ubo_load_4_xz = vector<float16_t, 2>(f16tof32(ubo_load_4 & 0xFFFF));
  vector<float16_t, 2> ubo_load_4_yw = vector<float16_t, 2>(f16tof32(ubo_load_4 >> 16));
  return matrix<float16_t, 3, 4>(vector<float16_t, 4>(ubo_load_xz[0], ubo_load_yw[0], ubo_load_xz[1], ubo_load_yw[1]), vector<float16_t, 4>(ubo_load_2_xz[0], ubo_load_2_yw[0], ubo_load_2_xz[1], ubo_load_2_yw[1]), vector<float16_t, 4>(ubo_load_4_xz[0], ubo_load_4_yw[0], ubo_load_4_xz[1], ubo_load_4_yw[1]));
})"},
        TypeCase{ty_mat4x2<f16>,
                 R"(matrix<float16_t, 4, 2> tint_symbol(uint4 buffer[12], uint offset) {
  const uint scalar_offset = ((offset + 0u)) / 4;
  uint ubo_load = buffer[scalar_offset / 4][scalar_offset % 4];
  const uint scalar_offset_1 = ((offset + 4u)) / 4;
  uint ubo_load_1 = buffer[scalar_offset_1 / 4][scalar_offset_1 % 4];
  const uint scalar_offset_2 = ((offset + 8u)) / 4;
  uint ubo_load_2 = buffer[scalar_offset_2 / 4][scalar_offset_2 % 4];
  const uint scalar_offset_3 = ((offset + 12u)) / 4;
  uint ubo_load_3 = buffer[scalar_offset_3 / 4][scalar_offset_3 % 4];
  return matrix<float16_t, 4, 2>(vector<float16_t, 2>(float16_t(f16tof32(ubo_load & 0xFFFF)), float16_t(f16tof32(ubo_load >> 16))), vector<float16_t, 2>(float16_t(f16tof32(ubo_load_1 & 0xFFFF)), float16_t(f16tof32(ubo_load_1 >> 16))), vector<float16_t, 2>(float16_t(f16tof32(ubo_load_2 & 0xFFFF)), float16_t(f16tof32(ubo_load_2 >> 16))), vector<float16_t, 2>(float16_t(f16tof32(ubo_load_3 & 0xFFFF)), float16_t(f16tof32(ubo_load_3 >> 16))));
})"},
        TypeCase{ty_mat4x3<f16>,
                 R"(matrix<float16_t, 4, 3> tint_symbol(uint4 buffer[16], uint offset) {
  const uint scalar_offset = ((offset + 0u)) / 4;
  uint4 ubo_load_1 = buffer[scalar_offset / 4];
  uint2 ubo_load = ((scalar_offset & 2) ? ubo_load_1.zw : ubo_load_1.xy);
  vector<float16_t, 2> ubo_load_xz = vector<float16_t, 2>(f16tof32(ubo_load & 0xFFFF));
  float16_t ubo_load_y = f16tof32(ubo_load[0] >> 16);
  const uint scalar_offset_1 = ((offset + 8u)) / 4;
  uint4 ubo_load_3 = buffer[scalar_offset_1 / 4];
  uint2 ubo_load_2 = ((scalar_offset_1 & 2) ? ubo_load_3.zw : ubo_load_3.xy);
  vector<float16_t, 2> ubo_load_2_xz = vector<float16_t, 2>(f16tof32(ubo_load_2 & 0xFFFF));
  float16_t ubo_load_2_y = f16tof32(ubo_load_2[0] >> 16);
  const uint scalar_offset_2 = ((offset + 16u)) / 4;
  uint4 ubo_load_5 = buffer[scalar_offset_2 / 4];
  uint2 ubo_load_4 = ((scalar_offset_2 & 2) ? ubo_load_5.zw : ubo_load_5.xy);
  vector<float16_t, 2> ubo_load_4_xz = vector<float16_t, 2>(f16tof32(ubo_load_4 & 0xFFFF));
  float16_t ubo_load_4_y = f16tof32(ubo_load_4[0] >> 16);
  const uint scalar_offset_3 = ((offset + 24u)) / 4;
  uint4 ubo_load_7 = buffer[scalar_offset_3 / 4];
  uint2 ubo_load_6 = ((scalar_offset_3 & 2) ? ubo_load_7.zw : ubo_load_7.xy);
  vector<float16_t, 2> ubo_load_6_xz = vector<float16_t, 2>(f16tof32(ubo_load_6 & 0xFFFF));
  float16_t ubo_load_6_y = f16tof32(ubo_load_6[0] >> 16);
  return matrix<float16_t, 4, 3>(vector<float16_t, 3>(ubo_load_xz[0], ubo_load_y, ubo_load_xz[1]), vector<float16_t, 3>(ubo_load_2_xz[0], ubo_load_2_y, ubo_load_2_xz[1]), vector<float16_t, 3>(ubo_load_4_xz[0], ubo_load_4_y, ubo_load_4_xz[1]), vector<float16_t, 3>(ubo_load_6_xz[0], ubo_load_6_y, ubo_load_6_xz[1]));
})"},
        TypeCase{ty_mat4x4<f16>,
                 R"(matrix<float16_t, 4, 4> tint_symbol(uint4 buffer[16], uint offset) {
  const uint scalar_offset = ((offset + 0u)) / 4;
  uint4 ubo_load_1 = buffer[scalar_offset / 4];
  uint2 ubo_load = ((scalar_offset & 2) ? ubo_load_1.zw : ubo_load_1.xy);
  vector<float16_t, 2> ubo_load_xz = vector<float16_t, 2>(f16tof32(ubo_load & 0xFFFF));
  vector<float16_t, 2> ubo_load_yw = vector<float16_t, 2>(f16tof32(ubo_load >> 16));
  const uint scalar_offset_1 = ((offset + 8u)) / 4;
  uint4 ubo_load_3 = buffer[scalar_offset_1 / 4];
  uint2 ubo_load_2 = ((scalar_offset_1 & 2) ? ubo_load_3.zw : ubo_load_3.xy);
  vector<float16_t, 2> ubo_load_2_xz = vector<float16_t, 2>(f16tof32(ubo_load_2 & 0xFFFF));
  vector<float16_t, 2> ubo_load_2_yw = vector<float16_t, 2>(f16tof32(ubo_load_2 >> 16));
  const uint scalar_offset_2 = ((offset + 16u)) / 4;
  uint4 ubo_load_5 = buffer[scalar_offset_2 / 4];
  uint2 ubo_load_4 = ((scalar_offset_2 & 2) ? ubo_load_5.zw : ubo_load_5.xy);
  vector<float16_t, 2> ubo_load_4_xz = vector<float16_t, 2>(f16tof32(ubo_load_4 & 0xFFFF));
  vector<float16_t, 2> ubo_load_4_yw = vector<float16_t, 2>(f16tof32(ubo_load_4 >> 16));
  const uint scalar_offset_3 = ((offset + 24u)) / 4;
  uint4 ubo_load_7 = buffer[scalar_offset_3 / 4];
  uint2 ubo_load_6 = ((scalar_offset_3 & 2) ? ubo_load_7.zw : ubo_load_7.xy);
  vector<float16_t, 2> ubo_load_6_xz = vector<float16_t, 2>(f16tof32(ubo_load_6 & 0xFFFF));
  vector<float16_t, 2> ubo_load_6_yw = vector<float16_t, 2>(f16tof32(ubo_load_6 >> 16));
  return matrix<float16_t, 4, 4>(vector<float16_t, 4>(ubo_load_xz[0], ubo_load_yw[0], ubo_load_xz[1], ubo_load_yw[1]), vector<float16_t, 4>(ubo_load_2_xz[0], ubo_load_2_yw[0], ubo_load_2_xz[1], ubo_load_2_yw[1]), vector<float16_t, 4>(ubo_load_4_xz[0], ubo_load_4_yw[0], ubo_load_4_xz[1], ubo_load_4_yw[1]), vector<float16_t, 4>(ubo_load_6_xz[0], ubo_load_6_yw[0], ubo_load_6_xz[1], ubo_load_6_yw[1]));
})"}));

using HlslGeneratorImplTest_MemberAccessor_StorageBufferStore =
    HlslGeneratorImplTest_MemberAccessorWithParam<TypeCase>;
TEST_P(HlslGeneratorImplTest_MemberAccessor_StorageBufferStore, Test) {
    // struct Data {
    //   a : i32,
    //   b : <type>,
    // };
    // var<storage> data : Data;
    // data.b = <type>();

    auto p = GetParam();

    Enable(ast::Extension::kF16);

    SetupStorageBuffer(utils::Vector{
        Member("a", ty.i32()),
        Member("b", p.member_type(ty)),
    });

    SetupFunction(utils::Vector{
        Decl(Var("value", p.member_type(ty), Call(p.member_type(ty)))),
        Assign(MemberAccessor("data", "b"), Expr("value")),
    });

    GeneratorImpl& gen = SanitizeAndBuild();

    ASSERT_TRUE(gen.Generate()) << gen.error();
    EXPECT_THAT(gen.result(), HasSubstr(p.expected));
}

INSTANTIATE_TEST_SUITE_P(
    HlslGeneratorImplTest_MemberAccessor,
    HlslGeneratorImplTest_MemberAccessor_StorageBufferStore,
    testing::Values(TypeCase{ty_u32, "data.Store(4u, asuint(value))"},
                    TypeCase{ty_f32, "data.Store(4u, asuint(value))"},
                    TypeCase{ty_i32, "data.Store(4u, asuint(value))"},
                    TypeCase{ty_f16, "data.Store<float16_t>(4u, value)"},
                    TypeCase{ty_vec2<u32>, "data.Store2(8u, asuint(value))"},
                    TypeCase{ty_vec2<f32>, "data.Store2(8u, asuint(value))"},
                    TypeCase{ty_vec2<i32>, "data.Store2(8u, asuint(value))"},
                    TypeCase{ty_vec2<f16>, "data.Store<vector<float16_t, 2> >(4u, value)"},
                    TypeCase{ty_vec3<u32>, "data.Store3(16u, asuint(value))"},
                    TypeCase{ty_vec3<f32>, "data.Store3(16u, asuint(value))"},
                    TypeCase{ty_vec3<i32>, "data.Store3(16u, asuint(value))"},
                    TypeCase{ty_vec3<f16>, "data.Store<vector<float16_t, 3> >(8u, value)"},
                    TypeCase{ty_vec4<u32>, "data.Store4(16u, asuint(value))"},
                    TypeCase{ty_vec4<f32>, "data.Store4(16u, asuint(value))"},
                    TypeCase{ty_vec4<i32>, "data.Store4(16u, asuint(value))"},
                    TypeCase{ty_vec4<f16>, "data.Store<vector<float16_t, 4> >(8u, value)"},
                    TypeCase{ty_mat2x2<f32>, R"({
  buffer.Store2((offset + 0u), asuint(value[0u]));
  buffer.Store2((offset + 8u), asuint(value[1u]));
})"},
                    TypeCase{ty_mat2x3<f32>, R"({
  buffer.Store3((offset + 0u), asuint(value[0u]));
  buffer.Store3((offset + 16u), asuint(value[1u]));
})"},
                    TypeCase{ty_mat2x4<f32>, R"({
  buffer.Store4((offset + 0u), asuint(value[0u]));
  buffer.Store4((offset + 16u), asuint(value[1u]));
})"},
                    TypeCase{ty_mat3x2<f32>, R"({
  buffer.Store2((offset + 0u), asuint(value[0u]));
  buffer.Store2((offset + 8u), asuint(value[1u]));
  buffer.Store2((offset + 16u), asuint(value[2u]));
})"},
                    TypeCase{ty_mat3x3<f32>, R"({
  buffer.Store3((offset + 0u), asuint(value[0u]));
  buffer.Store3((offset + 16u), asuint(value[1u]));
  buffer.Store3((offset + 32u), asuint(value[2u]));
})"},
                    TypeCase{ty_mat3x4<f32>, R"({
  buffer.Store4((offset + 0u), asuint(value[0u]));
  buffer.Store4((offset + 16u), asuint(value[1u]));
  buffer.Store4((offset + 32u), asuint(value[2u]));
})"},
                    TypeCase{ty_mat4x2<f32>, R"({
  buffer.Store2((offset + 0u), asuint(value[0u]));
  buffer.Store2((offset + 8u), asuint(value[1u]));
  buffer.Store2((offset + 16u), asuint(value[2u]));
  buffer.Store2((offset + 24u), asuint(value[3u]));
})"},
                    TypeCase{ty_mat4x3<f32>, R"({
  buffer.Store3((offset + 0u), asuint(value[0u]));
  buffer.Store3((offset + 16u), asuint(value[1u]));
  buffer.Store3((offset + 32u), asuint(value[2u]));
  buffer.Store3((offset + 48u), asuint(value[3u]));
})"},
                    TypeCase{ty_mat4x4<f32>, R"({
  buffer.Store4((offset + 0u), asuint(value[0u]));
  buffer.Store4((offset + 16u), asuint(value[1u]));
  buffer.Store4((offset + 32u), asuint(value[2u]));
  buffer.Store4((offset + 48u), asuint(value[3u]));
})"},
                    TypeCase{ty_mat2x2<f16>, R"({
  buffer.Store<vector<float16_t, 2> >((offset + 0u), value[0u]);
  buffer.Store<vector<float16_t, 2> >((offset + 4u), value[1u]);
})"},
                    TypeCase{ty_mat2x3<f16>, R"({
  buffer.Store<vector<float16_t, 3> >((offset + 0u), value[0u]);
  buffer.Store<vector<float16_t, 3> >((offset + 8u), value[1u]);
})"},
                    TypeCase{ty_mat2x4<f16>, R"({
  buffer.Store<vector<float16_t, 4> >((offset + 0u), value[0u]);
  buffer.Store<vector<float16_t, 4> >((offset + 8u), value[1u]);
})"},
                    TypeCase{ty_mat3x2<f16>, R"({
  buffer.Store<vector<float16_t, 2> >((offset + 0u), value[0u]);
  buffer.Store<vector<float16_t, 2> >((offset + 4u), value[1u]);
  buffer.Store<vector<float16_t, 2> >((offset + 8u), value[2u]);
})"},
                    TypeCase{ty_mat3x3<f16>, R"({
  buffer.Store<vector<float16_t, 3> >((offset + 0u), value[0u]);
  buffer.Store<vector<float16_t, 3> >((offset + 8u), value[1u]);
  buffer.Store<vector<float16_t, 3> >((offset + 16u), value[2u]);
})"},
                    TypeCase{ty_mat3x4<f16>, R"({
  buffer.Store<vector<float16_t, 4> >((offset + 0u), value[0u]);
  buffer.Store<vector<float16_t, 4> >((offset + 8u), value[1u]);
  buffer.Store<vector<float16_t, 4> >((offset + 16u), value[2u]);
})"},
                    TypeCase{ty_mat4x2<f16>, R"({
  buffer.Store<vector<float16_t, 2> >((offset + 0u), value[0u]);
  buffer.Store<vector<float16_t, 2> >((offset + 4u), value[1u]);
  buffer.Store<vector<float16_t, 2> >((offset + 8u), value[2u]);
  buffer.Store<vector<float16_t, 2> >((offset + 12u), value[3u]);
})"},
                    TypeCase{ty_mat4x3<f16>, R"({
  buffer.Store<vector<float16_t, 3> >((offset + 0u), value[0u]);
  buffer.Store<vector<float16_t, 3> >((offset + 8u), value[1u]);
  buffer.Store<vector<float16_t, 3> >((offset + 16u), value[2u]);
  buffer.Store<vector<float16_t, 3> >((offset + 24u), value[3u]);
})"},
                    TypeCase{ty_mat4x4<f16>, R"({
  buffer.Store<vector<float16_t, 4> >((offset + 0u), value[0u]);
  buffer.Store<vector<float16_t, 4> >((offset + 8u), value[1u]);
  buffer.Store<vector<float16_t, 4> >((offset + 16u), value[2u]);
  buffer.Store<vector<float16_t, 4> >((offset + 24u), value[3u]);
})"}));

TEST_F(HlslGeneratorImplTest_MemberAccessor, StorageBuffer_Store_Matrix_Empty) {
    // struct Data {
    //   a : f32,
    //   b : mat2x3<f32>,
    // };
    // var<storage> data : Data;
    // data.b = mat2x3<f32>();

    SetupStorageBuffer(utils::Vector{
        Member("a", ty.i32()),
        Member("b", ty.mat2x3<f32>()),
    });

    SetupFunction(utils::Vector{
        Assign(MemberAccessor("data", "b"), Call(ty.mat2x3<f32>())),
    });

    GeneratorImpl& gen = SanitizeAndBuild();

    ASSERT_TRUE(gen.Generate()) << gen.error();
    auto* expected =
        R"(RWByteAddressBuffer data : register(u0, space1);

void tint_symbol(RWByteAddressBuffer buffer, uint offset, float2x3 value) {
  buffer.Store3((offset + 0u), asuint(value[0u]));
  buffer.Store3((offset + 16u), asuint(value[1u]));
}

void main() {
  tint_symbol(data, 16u, float2x3((0.0f).xxx, (0.0f).xxx));
  return;
}
)";
    EXPECT_EQ(gen.result(), expected);
}

TEST_F(HlslGeneratorImplTest_MemberAccessor, StorageBuffer_Load_Matrix_F32_Single_Element) {
    // struct Data {
    //   z : f32,
    //   a : mat4x3<f32>,
    // };
    // var<storage> data : Data;
    // data.a[2i][1i];

    SetupStorageBuffer(utils::Vector{
        Member("z", ty.f32()),
        Member("a", ty.mat4x3<f32>()),
    });

    SetupFunction(utils::Vector{
        Decl(Var("x", IndexAccessor(IndexAccessor(MemberAccessor("data", "a"), 2_i), 1_i))),
    });

    GeneratorImpl& gen = SanitizeAndBuild();

    ASSERT_TRUE(gen.Generate()) << gen.error();
    auto* expected =
        R"(RWByteAddressBuffer data : register(u0, space1);

void main() {
  float x = asfloat(data.Load(52u));
  return;
}
)";
    EXPECT_EQ(gen.result(), expected);
}

TEST_F(HlslGeneratorImplTest_MemberAccessor, StorageBuffer_Load_Matrix_F16_Single_Element) {
    // struct Data {
    //   z : f16,
    //   a : mat4x3<f16>,
    // };
    // var<storage> data : Data;
    // data.a[2i][1i];

    Enable(ast::Extension::kF16);

    SetupStorageBuffer(utils::Vector{
        Member("z", ty.f16()),
        Member("a", ty.mat4x3<f16>()),
    });

    SetupFunction(utils::Vector{
        Decl(Var("x", IndexAccessor(IndexAccessor(MemberAccessor("data", "a"), 2_i), 1_i))),
    });

    GeneratorImpl& gen = SanitizeAndBuild();

    ASSERT_TRUE(gen.Generate()) << gen.error();
    auto* expected =
        R"(RWByteAddressBuffer data : register(u0, space1);

void main() {
  float16_t x = data.Load<float16_t>(26u);
  return;
}
)";
    EXPECT_EQ(gen.result(), expected);
}

TEST_F(HlslGeneratorImplTest_MemberAccessor, UniformBuffer_Load_Matrix_F32_Single_Element) {
    // struct Data {
    //   z : f32,
    //   a : mat4x3<f32>,
    // };
    // var<uniform> data : Data;
    // data.a[2i][1i];

    SetupUniformBuffer(utils::Vector{
        Member("z", ty.f32()),
        Member("a", ty.mat4x3<f32>()),
    });

    SetupFunction(utils::Vector{
        Decl(Var("x", IndexAccessor(IndexAccessor(MemberAccessor("data", "a"), 2_i), 1_i))),
    });

    GeneratorImpl& gen = SanitizeAndBuild();

    ASSERT_TRUE(gen.Generate()) << gen.error();
    auto* expected =
        R"(cbuffer cbuffer_data : register(b1, space1) {
  uint4 data[5];
};

void main() {
  float x = asfloat(data[3].y);
  return;
}
)";
    EXPECT_EQ(gen.result(), expected);
}

TEST_F(HlslGeneratorImplTest_MemberAccessor, UniformBuffer_Load_Matrix_F16_Single_Element) {
    // struct Data {
    //   z : f16,
    //   a : mat4x3<f16>,
    // };
    // var<uniform> data : Data;
    // data.a[2i][1i];

    Enable(ast::Extension::kF16);

    SetupUniformBuffer(utils::Vector{
        Member("z", ty.f16()),
        Member("a", ty.mat4x3<f16>()),
    });

    SetupFunction(utils::Vector{
        Decl(Var("x", IndexAccessor(IndexAccessor(MemberAccessor("data", "a"), 2_i), 1_i))),
    });

    GeneratorImpl& gen = SanitizeAndBuild();

    ASSERT_TRUE(gen.Generate()) << gen.error();
    auto* expected =
        R"(cbuffer cbuffer_data : register(b1, space1) {
  uint4 data[3];
};

void main() {
  float16_t x = float16_t(f16tof32(((data[1].z >> 16) & 0xFFFF)));
  return;
}
)";
    EXPECT_EQ(gen.result(), expected);
}

TEST_F(HlslGeneratorImplTest_MemberAccessor,
       EmitExpression_IndexAccessor_StorageBuffer_Load_I32_FromArray) {
    // struct Data {
    //   z : f32,
    //   a : array<i32, 5i>,
    // };
    // var<storage> data : Data;
    // data.a[2];

    SetupStorageBuffer(utils::Vector{
        Member("z", ty.f32()),
        Member("a", ty.array<i32, 5>()),
    });

    SetupFunction(utils::Vector{
        Decl(Var("x", IndexAccessor(MemberAccessor("data", "a"), 2_i))),
    });

    GeneratorImpl& gen = SanitizeAndBuild();

    ASSERT_TRUE(gen.Generate()) << gen.error();
    auto* expected =
        R"(RWByteAddressBuffer data : register(u0, space1);

void main() {
  int x = asint(data.Load(12u));
  return;
}
)";
    EXPECT_EQ(gen.result(), expected);
}

TEST_F(HlslGeneratorImplTest_MemberAccessor,
       EmitExpression_IndexAccessor_UniformBuffer_Load_Vec4_I32_FromArray) {
    // struct Data {
    //   z : f32,
    //   a : array<vec4<i32>, 5i>,
    // };
    // var<uniform> data : Data;
    // data.a[2];

    SetupUniformBuffer(utils::Vector{
        Member("z", ty.f32()),
        Member("a", ty.array(ty.vec4(ty.i32()), 5_i)),
    });

    SetupFunction(utils::Vector{
        Decl(Var("x", IndexAccessor(MemberAccessor("data", "a"), 2_i))),
    });

    GeneratorImpl& gen = SanitizeAndBuild();

    ASSERT_TRUE(gen.Generate()) << gen.error();
    auto* expected =
        R"(cbuffer cbuffer_data : register(b1, space1) {
  uint4 data[6];
};

void main() {
  int4 x = asint(data[3]);
  return;
}
)";
    EXPECT_EQ(gen.result(), expected);
}

TEST_F(HlslGeneratorImplTest_MemberAccessor,
       EmitExpression_IndexAccessor_StorageBuffer_Load_Struct_FromArray) {
    // struct Inner {
    //   @size(16i) @align(16i)
    //   v : i32,
    // };
    // struct Data {
    //   z : f32,
    //   a : array<Inner, 5i>,
    // };
    // var<storage> data : Data;
    // data.a[2i];

    auto* elem_type = Structure(
        "Inner", utils::Vector{
                     Member("v", ty.i32(), utils::Vector{MemberSize(16_i), MemberAlign(16_i)}),
                 });

    SetupStorageBuffer(utils::Vector{
        Member("z", ty.f32()),
        Member("a", ty.array(ty.Of(elem_type), 5_i)),
    });

    SetupFunction(utils::Vector{
        Decl(Var("x", IndexAccessor(MemberAccessor("data", "a"), 2_i))),
    });

    GeneratorImpl& gen = SanitizeAndBuild();

    ASSERT_TRUE(gen.Generate()) << gen.error();
    auto* expected =
        R"(struct Inner {
  int v;
};

RWByteAddressBuffer data : register(u0, space1);

Inner tint_symbol(RWByteAddressBuffer buffer, uint offset) {
  const Inner tint_symbol_2 = {asint(buffer.Load((offset + 0u)))};
  return tint_symbol_2;
}

void main() {
  Inner x = tint_symbol(data, 48u);
  return;
}
)";
    EXPECT_EQ(gen.result(), expected);
}

TEST_F(HlslGeneratorImplTest_MemberAccessor,
       EmitExpression_IndexAccessor_UniformBuffer_Load_Struct_FromArray) {
    // struct Inner {
    //   @size(16i) @align(16i)
    //   v : i32,
    // };
    // struct Data {
    //   z : f32,
    //   a : array<Inner, 5i>,
    // };
    // var<uniform> data : Data;
    // data.a[2i];

    auto* elem_type = Structure(
        "Inner", utils::Vector{
                     Member("v", ty.i32(), utils::Vector{MemberSize(16_i), MemberAlign(16_i)}),
                 });

    SetupUniformBuffer(utils::Vector{
        Member("z", ty.f32()),
        Member("a", ty.array(ty.Of(elem_type), 5_i)),
    });

    SetupFunction(utils::Vector{
        Decl(Var("x", IndexAccessor(MemberAccessor("data", "a"), 2_i))),
    });

    GeneratorImpl& gen = SanitizeAndBuild();

    ASSERT_TRUE(gen.Generate()) << gen.error();
    auto* expected =
        R"(struct Inner {
  int v;
};

cbuffer cbuffer_data : register(b1, space1) {
  uint4 data[6];
};

Inner tint_symbol(uint4 buffer[6], uint offset) {
  const uint scalar_offset = ((offset + 0u)) / 4;
  const Inner tint_symbol_2 = {asint(buffer[scalar_offset / 4][scalar_offset % 4])};
  return tint_symbol_2;
}

void main() {
  Inner x = tint_symbol(data, 48u);
  return;
}
)";
    EXPECT_EQ(gen.result(), expected);
}

TEST_F(HlslGeneratorImplTest_MemberAccessor,
       EmitExpression_IndexAccessor_StorageBuffer_Load_I32_FromArray_ExprIdx) {
    // struct Data {
    //   z : f32,
    //   a : array<i32, 5i>,
    // };
    // var<storage> data : Data;
    // data.a[(2i + 4i) - 3i];

    SetupStorageBuffer(utils::Vector{
        Member("z", ty.f32()),
        Member("a", ty.array<i32, 5>()),
    });

    SetupFunction(utils::Vector{
        Decl(Var("a", Expr(2_i))),
        Decl(Var("b", Expr(4_i))),
        Decl(Var("c", Expr(3_i))),
        Decl(Var("x", IndexAccessor(MemberAccessor("data", "a"), Sub(Add("a", "b"), "c")))),
    });

    GeneratorImpl& gen = SanitizeAndBuild();

    ASSERT_TRUE(gen.Generate()) << gen.error();
    auto* expected =
        R"(RWByteAddressBuffer data : register(u0, space1);

void main() {
  int a = 2;
  int b = 4;
  int c = 3;
  int x = asint(data.Load((4u + (4u * uint(((a + b) - c))))));
  return;
}
)";
    EXPECT_EQ(gen.result(), expected);
}

TEST_F(HlslGeneratorImplTest_MemberAccessor,
       EmitExpression_IndexAccessor_UniformBuffer_Load_Vec4_I32_FromArray_ExprIdx) {
    // struct Data {
    //   z : f32,
    //   a : array<vec4<i32>, 5i>,
    // };
    // var<uniform> data : Data;
    // data.a[(2i + 4i) - 3i];

    SetupUniformBuffer(utils::Vector{
        Member("z", ty.f32()),
        Member("a", ty.array(ty.vec4(ty.i32()), 5_i)),
    });

    SetupFunction(utils::Vector{
        Decl(Var("a", Expr(2_i))),
        Decl(Var("b", Expr(4_i))),
        Decl(Var("c", Expr(3_i))),
        Decl(Var("x", IndexAccessor(MemberAccessor("data", "a"), Sub(Add("a", "b"), "c")))),
    });

    GeneratorImpl& gen = SanitizeAndBuild();

    ASSERT_TRUE(gen.Generate()) << gen.error();
    auto* expected =
        R"(cbuffer cbuffer_data : register(b1, space1) {
  uint4 data[6];
};

void main() {
  int a = 2;
  int b = 4;
  int c = 3;
  const uint scalar_offset = ((16u + (16u * uint(((a + b) - c))))) / 4;
  int4 x = asint(data[scalar_offset / 4]);
  return;
}
)";
    EXPECT_EQ(gen.result(), expected);
}

TEST_F(HlslGeneratorImplTest_MemberAccessor, StorageBuffer_Store_ToArray) {
    // struct Data {
    //   a : array<i32, 5i>,
    // };
    // var<storage> data : Data;
    // data.a[2i] = 2i;

    SetupStorageBuffer(utils::Vector{
        Member("z", ty.f32()),
        Member("a", ty.array<i32, 5>()),
    });

    SetupFunction(utils::Vector{
        Assign(IndexAccessor(MemberAccessor("data", "a"), 2_i), 2_i),
    });

    GeneratorImpl& gen = SanitizeAndBuild();

    ASSERT_TRUE(gen.Generate()) << gen.error();
    auto* expected =
        R"(RWByteAddressBuffer data : register(u0, space1);

void main() {
  data.Store(12u, asuint(2));
  return;
}
)";
    EXPECT_EQ(gen.result(), expected);
}

TEST_F(HlslGeneratorImplTest_MemberAccessor, StorageBuffer_Load_MultiLevel) {
    // struct Inner {
    //   a : vec3<i32>,
    //   b : vec3<f32>,
    // };
    // struct Data {
    //   c : array<Inner, 4u>,
    // };
    //
    // var<storage> data : Data;
    // data.c[2i].b

    auto* inner = Structure("Inner", utils::Vector{
                                         Member("a", ty.vec3<i32>()),
                                         Member("b", ty.vec3<f32>()),
                                     });

    SetupStorageBuffer(utils::Vector{
        Member("c", ty.array(ty.Of(inner), 4_u)),
    });

    SetupFunction(utils::Vector{
        Decl(Var("x", MemberAccessor(IndexAccessor(MemberAccessor("data", "c"), 2_i), "b"))),
    });

    GeneratorImpl& gen = SanitizeAndBuild();

    ASSERT_TRUE(gen.Generate()) << gen.error();
    auto* expected =
        R"(RWByteAddressBuffer data : register(u0, space1);

void main() {
  float3 x = asfloat(data.Load3(80u));
  return;
}
)";
    EXPECT_EQ(gen.result(), expected);
}

TEST_F(HlslGeneratorImplTest_MemberAccessor, UniformBuffer_Load_MultiLevel) {
    // struct Inner {
    //   a : vec3<i32>,
    //   b : vec3<f32>,
    // };
    // struct Data {
    //   var c : array<Inner, 4u>,
    // };
    //
    // var<storage> data : Data;
    // data.c[2i].b

    auto* inner = Structure("Inner", utils::Vector{
                                         Member("a", ty.vec3<i32>()),
                                         Member("b", ty.vec3<f32>()),
                                     });

    SetupUniformBuffer(utils::Vector{
        Member("c", ty.array(ty.Of(inner), 4_u)),
    });

    SetupFunction(utils::Vector{
        Decl(Var("x", MemberAccessor(IndexAccessor(MemberAccessor("data", "c"), 2_i), "b"))),
    });

    GeneratorImpl& gen = SanitizeAndBuild();

    ASSERT_TRUE(gen.Generate()) << gen.error();
    auto* expected =
        R"(cbuffer cbuffer_data : register(b1, space1) {
  uint4 data[8];
};

void main() {
  float3 x = asfloat(data[5].xyz);
  return;
}
)";
    EXPECT_EQ(gen.result(), expected);
}

TEST_F(HlslGeneratorImplTest_MemberAccessor, StorageBuffer_Load_MultiLevel_Swizzle) {
    // struct Inner {
    //   a : vec3<i32>,
    //   b : vec3<f32>,
    // };
    // struct Data {
    //   var c : array<Inner, 4u>,
    // };
    //
    // var<storage> data : Data;
    // data.c[2i].b.yx

    auto* inner = Structure("Inner", utils::Vector{
                                         Member("a", ty.vec3<i32>()),
                                         Member("b", ty.vec3<f32>()),
                                     });

    SetupStorageBuffer(utils::Vector{
        Member("c", ty.array(ty.Of(inner), 4_u)),
    });

    SetupFunction(utils::Vector{
        Decl(Var("x",
                 MemberAccessor(
                     MemberAccessor(IndexAccessor(MemberAccessor("data", "c"), 2_i), "b"), "yx"))),
    });

    GeneratorImpl& gen = SanitizeAndBuild();

    ASSERT_TRUE(gen.Generate()) << gen.error();
    auto* expected =
        R"(RWByteAddressBuffer data : register(u0, space1);

void main() {
  float2 x = asfloat(data.Load3(80u)).yx;
  return;
}
)";
    EXPECT_EQ(gen.result(), expected);
}

TEST_F(HlslGeneratorImplTest_MemberAccessor, UniformBuffer_Load_MultiLevel_Swizzle) {
    // struct Inner {
    //   a : vec3<i32>,
    //   b : vec3<f32>,
    // };
    // struct Data {
    //   var c : array<Inner, 4u>,
    // };
    //
    // var<uniform> data : Data;
    // data.c[2i].b.yx

    auto* inner = Structure("Inner", utils::Vector{
                                         Member("a", ty.vec3<i32>()),
                                         Member("b", ty.vec3<f32>()),
                                     });

    SetupUniformBuffer(utils::Vector{
        Member("c", ty.array(ty.Of(inner), 4_u)),
    });

    SetupFunction(utils::Vector{
        Decl(Var("x",
                 MemberAccessor(
                     MemberAccessor(IndexAccessor(MemberAccessor("data", "c"), 2_i), "b"), "yx"))),
    });

    GeneratorImpl& gen = SanitizeAndBuild();

    ASSERT_TRUE(gen.Generate()) << gen.error();
    auto* expected =
        R"(cbuffer cbuffer_data : register(b1, space1) {
  uint4 data[8];
};

void main() {
  float2 x = asfloat(data[5].xyz).yx;
  return;
}
)";
    EXPECT_EQ(gen.result(), expected);
}

TEST_F(HlslGeneratorImplTest_MemberAccessor,
       StorageBuffer_Load_MultiLevel_Swizzle_SingleLetter) {  // NOLINT
    // struct Inner {
    //   a : vec3<i32>,
    //   b : vec3<f32>,
    // };
    // struct Data {
    //   var c : array<Inner, 4u>,
    // };
    //
    // var<storage> data : Data;
    // data.c[2i].b.g

    auto* inner = Structure("Inner", utils::Vector{
                                         Member("a", ty.vec3<i32>()),
                                         Member("b", ty.vec3<f32>()),
                                     });

    SetupStorageBuffer(utils::Vector{
        Member("c", ty.array(ty.Of(inner), 4_u)),
    });

    SetupFunction(utils::Vector{
        Decl(Var("x",
                 MemberAccessor(
                     MemberAccessor(IndexAccessor(MemberAccessor("data", "c"), 2_i), "b"), "g"))),
    });

    GeneratorImpl& gen = SanitizeAndBuild();

    ASSERT_TRUE(gen.Generate()) << gen.error();
    auto* expected =
        R"(RWByteAddressBuffer data : register(u0, space1);

void main() {
  float x = asfloat(data.Load(84u));
  return;
}
)";
    EXPECT_EQ(gen.result(), expected);
}

TEST_F(HlslGeneratorImplTest_MemberAccessor,
       UniformBuffer_Load_MultiLevel_Swizzle_SingleLetter) {  // NOLINT
    // struct Inner {
    //   a : vec3<i32>,
    //   b : vec3<f32>,
    // };
    // struct Data {
    //   var c : array<Inner, 4u>,
    // };
    //
    // var<uniform> data : Data;
    // data.c[2i].b.g

    auto* inner = Structure("Inner", utils::Vector{
                                         Member("a", ty.vec3<i32>()),
                                         Member("b", ty.vec3<f32>()),
                                     });

    SetupUniformBuffer(utils::Vector{
        Member("c", ty.array(ty.Of(inner), 4_u)),
    });

    SetupFunction(utils::Vector{
        Decl(Var("x",
                 MemberAccessor(
                     MemberAccessor(IndexAccessor(MemberAccessor("data", "c"), 2_i), "b"), "g"))),
    });

    GeneratorImpl& gen = SanitizeAndBuild();

    ASSERT_TRUE(gen.Generate()) << gen.error();
    auto* expected =
        R"(cbuffer cbuffer_data : register(b1, space1) {
  uint4 data[8];
};

void main() {
  float x = asfloat(data[5].y);
  return;
}
)";
    EXPECT_EQ(gen.result(), expected);
}

TEST_F(HlslGeneratorImplTest_MemberAccessor, StorageBuffer_Load_MultiLevel_Index) {
    // struct Inner {
    //   a : vec3<i32>,
    //   b : vec3<f32>,
    // };
    // struct Data {
    //   var c : array<Inner, 4u>,
    // };
    //
    // var<storage> data : Data;
    // data.c[2i].b[1i]

    auto* inner = Structure("Inner", utils::Vector{
                                         Member("a", ty.vec3<i32>()),
                                         Member("b", ty.vec3<f32>()),
                                     });

    SetupStorageBuffer(utils::Vector{
        Member("c", ty.array(ty.Of(inner), 4_u)),
    });

    SetupFunction(utils::Vector{
        Decl(Var("x",
                 IndexAccessor(MemberAccessor(IndexAccessor(MemberAccessor("data", "c"), 2_i), "b"),
                               1_i))),
    });

    GeneratorImpl& gen = SanitizeAndBuild();

    ASSERT_TRUE(gen.Generate()) << gen.error();
    auto* expected =
        R"(RWByteAddressBuffer data : register(u0, space1);

void main() {
  float x = asfloat(data.Load(84u));
  return;
}
)";
    EXPECT_EQ(gen.result(), expected);
}

TEST_F(HlslGeneratorImplTest_MemberAccessor, UniformBuffer_Load_MultiLevel_Index) {
    // struct Inner {
    //   a : vec3<i32>,
    //   b : vec3<f32>,
    // };
    // struct Data {
    //   var c : array<Inner, 4u>,
    // };
    //
    // var<uniform> data : Data;
    // data.c[2i].b[1i]

    auto* inner = Structure("Inner", utils::Vector{
                                         Member("a", ty.vec3<i32>()),
                                         Member("b", ty.vec3<f32>()),
                                     });

    SetupUniformBuffer(utils::Vector{
        Member("c", ty.array(ty.Of(inner), 4_u)),
    });

    SetupFunction(utils::Vector{
        Decl(Var("x",
                 IndexAccessor(MemberAccessor(IndexAccessor(MemberAccessor("data", "c"), 2_i), "b"),
                               1_i))),
    });

    GeneratorImpl& gen = SanitizeAndBuild();

    ASSERT_TRUE(gen.Generate()) << gen.error();
    auto* expected =
        R"(cbuffer cbuffer_data : register(b1, space1) {
  uint4 data[8];
};

void main() {
  float x = asfloat(data[5].y);
  return;
}
)";
    EXPECT_EQ(gen.result(), expected);
}

TEST_F(HlslGeneratorImplTest_MemberAccessor, StorageBuffer_Store_MultiLevel) {
    // struct Inner {
    //   a : vec3<i32>,
    //   b : vec3<f32>,
    // };
    // struct Data {
    //   var c : array<Inner, 4u>,
    // };
    //
    // var<storage> data : Pre;
    // data.c[2i].b = vec3<f32>(1_f, 2_f, 3_f);

    auto* inner = Structure("Inner", utils::Vector{
                                         Member("a", ty.vec3<i32>()),
                                         Member("b", ty.vec3<f32>()),
                                     });

    SetupStorageBuffer(utils::Vector{
        Member("c", ty.array(ty.Of(inner), 4_u)),
    });

    SetupFunction(utils::Vector{
        Assign(MemberAccessor(IndexAccessor(MemberAccessor("data", "c"), 2_i), "b"),
               vec3<f32>(1_f, 2_f, 3_f)),
    });

    GeneratorImpl& gen = SanitizeAndBuild();

    ASSERT_TRUE(gen.Generate()) << gen.error();
    auto* expected =
        R"(RWByteAddressBuffer data : register(u0, space1);

void main() {
  data.Store3(80u, asuint(float3(1.0f, 2.0f, 3.0f)));
  return;
}
)";
    EXPECT_EQ(gen.result(), expected);
}

TEST_F(HlslGeneratorImplTest_MemberAccessor, StorageBuffer_Store_Swizzle_SingleLetter) {
    // struct Inner {
    //   a : vec3<i32>,
    //   b : vec3<f32>,
    // };
    // struct Data {
    //   var c : array<Inner, 4u>,
    // };
    //
    // var<storage> data : Pre;
    // data.c[2i].b.y = 1.f;

    auto* inner = Structure("Inner", utils::Vector{
                                         Member("a", ty.vec3<i32>()),
                                         Member("b", ty.vec3<f32>()),
                                     });

    SetupStorageBuffer(utils::Vector{
        Member("c", ty.array(ty.Of(inner), 4_u)),
    });

    SetupFunction(utils::Vector{
        Assign(MemberAccessor(MemberAccessor(IndexAccessor(MemberAccessor("data", "c"), 2_i), "b"),
                              "y"),
               Expr(1_f)),
    });

    GeneratorImpl& gen = SanitizeAndBuild();

    ASSERT_TRUE(gen.Generate()) << gen.error();
    auto* expected =
        R"(RWByteAddressBuffer data : register(u0, space1);

void main() {
  data.Store(84u, asuint(1.0f));
  return;
}
)";
    EXPECT_EQ(gen.result(), expected);
}

TEST_F(HlslGeneratorImplTest_MemberAccessor, Swizzle_xyz) {
    auto* var = Var("my_vec", ty.vec4<f32>(), vec4<f32>(1_f, 2_f, 3_f, 4_f));
    auto* expr = MemberAccessor("my_vec", "xyz");
    WrapInFunction(var, expr);

    GeneratorImpl& gen = SanitizeAndBuild();
    ASSERT_TRUE(gen.Generate()) << gen.error();
    EXPECT_THAT(gen.result(), HasSubstr("my_vec.xyz"));
}

TEST_F(HlslGeneratorImplTest_MemberAccessor, Swizzle_gbr) {
    auto* var = Var("my_vec", ty.vec4<f32>(), vec4<f32>(1_f, 2_f, 3_f, 4_f));
    auto* expr = MemberAccessor("my_vec", "gbr");
    WrapInFunction(var, expr);

    GeneratorImpl& gen = SanitizeAndBuild();
    ASSERT_TRUE(gen.Generate()) << gen.error();
    EXPECT_THAT(gen.result(), HasSubstr("my_vec.gbr"));
}

}  // namespace
}  // namespace tint::writer::hlsl
