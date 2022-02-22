#version 310 es

int tint_count_trailing_zeros(int v) {
  uint x = uint(v);
  uint b16 = (bool((x & 65535u)) ? 0u : 16u);
  x = (x >> b16);
  uint b8 = (bool((x & 255u)) ? 0u : 8u);
  x = (x >> b8);
  uint b4 = (bool((x & 15u)) ? 0u : 4u);
  x = (x >> b4);
  uint b2 = (bool((x & 3u)) ? 0u : 2u);
  x = (x >> b2);
  uint b1 = (bool((x & 1u)) ? 0u : 1u);
  uint is_zero = ((x == 0u) ? 1u : 0u);
  return int((((((b16 | b8) | b4) | b2) | b1) + is_zero));
}

void countTrailingZeros_42fed6() {
  int res = tint_count_trailing_zeros(1);
}

vec4 vertex_main() {
  countTrailingZeros_42fed6();
  return vec4(0.0f, 0.0f, 0.0f, 0.0f);
}

void main() {
  vec4 inner_result = vertex_main();
  gl_Position = inner_result;
  gl_Position.y = -(gl_Position.y);
  gl_Position.z = ((2.0f * gl_Position.z) - gl_Position.w);
  return;
}
#version 310 es
precision mediump float;

int tint_count_trailing_zeros(int v) {
  uint x = uint(v);
  uint b16 = (bool((x & 65535u)) ? 0u : 16u);
  x = (x >> b16);
  uint b8 = (bool((x & 255u)) ? 0u : 8u);
  x = (x >> b8);
  uint b4 = (bool((x & 15u)) ? 0u : 4u);
  x = (x >> b4);
  uint b2 = (bool((x & 3u)) ? 0u : 2u);
  x = (x >> b2);
  uint b1 = (bool((x & 1u)) ? 0u : 1u);
  uint is_zero = ((x == 0u) ? 1u : 0u);
  return int((((((b16 | b8) | b4) | b2) | b1) + is_zero));
}

void countTrailingZeros_42fed6() {
  int res = tint_count_trailing_zeros(1);
}

void fragment_main() {
  countTrailingZeros_42fed6();
}

void main() {
  fragment_main();
  return;
}
#version 310 es

int tint_count_trailing_zeros(int v) {
  uint x = uint(v);
  uint b16 = (bool((x & 65535u)) ? 0u : 16u);
  x = (x >> b16);
  uint b8 = (bool((x & 255u)) ? 0u : 8u);
  x = (x >> b8);
  uint b4 = (bool((x & 15u)) ? 0u : 4u);
  x = (x >> b4);
  uint b2 = (bool((x & 3u)) ? 0u : 2u);
  x = (x >> b2);
  uint b1 = (bool((x & 1u)) ? 0u : 1u);
  uint is_zero = ((x == 0u) ? 1u : 0u);
  return int((((((b16 | b8) | b4) | b2) | b1) + is_zero));
}

void countTrailingZeros_42fed6() {
  int res = tint_count_trailing_zeros(1);
}

void compute_main() {
  countTrailingZeros_42fed6();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
