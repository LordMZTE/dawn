#version 310 es

void mix_6f8adc() {
  vec2 arg_0 = vec2(0.0f);
  vec2 arg_1 = vec2(0.0f);
  vec2 arg_2 = vec2(0.0f);
  vec2 res = mix(arg_0, arg_1, arg_2);
}

vec4 vertex_main() {
  mix_6f8adc();
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
precision mediump float;

void mix_6f8adc() {
  vec2 arg_0 = vec2(0.0f);
  vec2 arg_1 = vec2(0.0f);
  vec2 arg_2 = vec2(0.0f);
  vec2 res = mix(arg_0, arg_1, arg_2);
}

void fragment_main() {
  mix_6f8adc();
}

void main() {
  fragment_main();
  return;
}
#version 310 es

void mix_6f8adc() {
  vec2 arg_0 = vec2(0.0f);
  vec2 arg_1 = vec2(0.0f);
  vec2 arg_2 = vec2(0.0f);
  vec2 res = mix(arg_0, arg_1, arg_2);
}

void compute_main() {
  mix_6f8adc();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
