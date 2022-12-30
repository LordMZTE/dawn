#version 310 es

layout(binding = 0, std140) uniform u_block_ubo {
  mat4x3 inner;
} u;

layout(binding = 1, std430) buffer u_block_ssbo {
  mat4x3 inner;
} s;

void f() {
  s.inner = u.inner;
  s.inner[1] = u.inner[0];
  s.inner[1] = u.inner[0].zxy;
  s.inner[0][1] = u.inner[1][0];
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f();
  return;
}
