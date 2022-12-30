#version 310 es

struct modf_result_vec4_f32 {
  vec4 fract;
  vec4 whole;
};


void modf_4bfced() {
  modf_result_vec4_f32 res = modf_result_vec4_f32(vec4(-0.5f), vec4(-1.0f));
}

vec4 vertex_main() {
  modf_4bfced();
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

struct modf_result_vec4_f32 {
  vec4 fract;
  vec4 whole;
};


void modf_4bfced() {
  modf_result_vec4_f32 res = modf_result_vec4_f32(vec4(-0.5f), vec4(-1.0f));
}

void fragment_main() {
  modf_4bfced();
}

void main() {
  fragment_main();
  return;
}
#version 310 es

struct modf_result_vec4_f32 {
  vec4 fract;
  vec4 whole;
};


void modf_4bfced() {
  modf_result_vec4_f32 res = modf_result_vec4_f32(vec4(-0.5f), vec4(-1.0f));
}

void compute_main() {
  modf_4bfced();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
