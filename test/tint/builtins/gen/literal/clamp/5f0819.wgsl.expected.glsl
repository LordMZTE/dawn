#version 310 es

void clamp_5f0819() {
  ivec3 res = clamp(ivec3(0), ivec3(0), ivec3(0));
}

vec4 vertex_main() {
  clamp_5f0819();
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

void clamp_5f0819() {
  ivec3 res = clamp(ivec3(0), ivec3(0), ivec3(0));
}

void fragment_main() {
  clamp_5f0819();
}

void main() {
  fragment_main();
  return;
}
#version 310 es

void clamp_5f0819() {
  ivec3 res = clamp(ivec3(0), ivec3(0), ivec3(0));
}

void compute_main() {
  clamp_5f0819();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
