#version 310 es

void tanh_ac5d33() {
  vec4 res = vec4(0.76159417629241943359f);
}

vec4 vertex_main() {
  tanh_ac5d33();
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

void tanh_ac5d33() {
  vec4 res = vec4(0.76159417629241943359f);
}

void fragment_main() {
  tanh_ac5d33();
}

void main() {
  fragment_main();
  return;
}
#version 310 es

void tanh_ac5d33() {
  vec4 res = vec4(0.76159417629241943359f);
}

void compute_main() {
  tanh_ac5d33();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
