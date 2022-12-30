#version 310 es

uniform highp isampler2DArray arg_0_1;
void textureNumLayers_77be7b() {
  uint res = uint(textureSize(arg_0_1, 0).z);
}

vec4 vertex_main() {
  textureNumLayers_77be7b();
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

uniform highp isampler2DArray arg_0_1;
void textureNumLayers_77be7b() {
  uint res = uint(textureSize(arg_0_1, 0).z);
}

void fragment_main() {
  textureNumLayers_77be7b();
}

void main() {
  fragment_main();
  return;
}
#version 310 es

uniform highp isampler2DArray arg_0_1;
void textureNumLayers_77be7b() {
  uint res = uint(textureSize(arg_0_1, 0).z);
}

void compute_main() {
  textureNumLayers_77be7b();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
