#version 310 es
#extension GL_AMD_gpu_shader_half_float : require

layout(binding = 0, std140) uniform u_block_std140_ubo {
  f16vec2 inner_0;
  f16vec2 inner_1;
  f16vec2 inner_2;
  f16vec2 inner_3;
} u;

shared f16mat4x2 w;
f16mat4x2 load_u_inner() {
  return f16mat4x2(u.inner_0, u.inner_1, u.inner_2, u.inner_3);
}

void f(uint local_invocation_index) {
  {
    w = f16mat4x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf));
  }
  barrier();
  w = load_u_inner();
  w[1] = u.inner_0;
  w[1] = u.inner_0.yx;
  w[0][1] = u.inner_1[0u];
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f(gl_LocalInvocationIndex);
  return;
}
