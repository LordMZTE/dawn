#version 310 es

void tint_symbol() {
  vec2 tint_symbol_2 = vec2(0.625f, 0.9375f);
  ivec2 tint_symbol_3 = ivec2(1, 2);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol();
  return;
}
