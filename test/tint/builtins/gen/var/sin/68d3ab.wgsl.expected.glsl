#version 310 es

void sin_68d3ab() {
  vec2 res = vec2(0.841470957f);
}

vec4 vertex_main() {
  sin_68d3ab();
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

void sin_68d3ab() {
  vec2 res = vec2(0.841470957f);
}

void fragment_main() {
  sin_68d3ab();
}

void main() {
  fragment_main();
  return;
}
#version 310 es

void sin_68d3ab() {
  vec2 res = vec2(0.841470957f);
}

void compute_main() {
  sin_68d3ab();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
