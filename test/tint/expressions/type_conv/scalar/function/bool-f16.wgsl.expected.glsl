#version 310 es
#extension GL_AMD_gpu_shader_half_float : require

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void unused_entry_point() {
  return;
}
bool t = false;
bool m() {
  t = true;
  return bool(t);
}

void f() {
  bool tint_symbol = m();
  float16_t v = float16_t(tint_symbol);
}

