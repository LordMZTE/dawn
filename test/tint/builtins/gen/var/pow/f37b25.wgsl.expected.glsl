#version 310 es
#extension GL_AMD_gpu_shader_half_float : require

void pow_f37b25() {
  f16vec2 arg_0 = f16vec2(0.0hf);
  f16vec2 arg_1 = f16vec2(0.0hf);
  f16vec2 res = pow(arg_0, arg_1);
}

vec4 vertex_main() {
  pow_f37b25();
  return vec4(0.0f);
}

void main() {
  gl_PointSize = 1.0;
  vec4 inner_result = vertex_main();
  gl_Position = inner_result;
  gl_Position.y = -(gl_Position.y);
  gl_Position.z = ((2.0f * gl_Position.z) - gl_Position.w);
  return;
}
#version 310 es
#extension GL_AMD_gpu_shader_half_float : require
precision mediump float;

void pow_f37b25() {
  f16vec2 arg_0 = f16vec2(0.0hf);
  f16vec2 arg_1 = f16vec2(0.0hf);
  f16vec2 res = pow(arg_0, arg_1);
}

void fragment_main() {
  pow_f37b25();
}

void main() {
  fragment_main();
  return;
}
#version 310 es
#extension GL_AMD_gpu_shader_half_float : require

void pow_f37b25() {
  f16vec2 arg_0 = f16vec2(0.0hf);
  f16vec2 arg_1 = f16vec2(0.0hf);
  f16vec2 res = pow(arg_0, arg_1);
}

void compute_main() {
  pow_f37b25();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
