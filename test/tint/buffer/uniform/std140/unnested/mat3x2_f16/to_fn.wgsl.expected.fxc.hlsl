SKIP: FAILED

cbuffer cbuffer_u : register(b0, space0) {
  uint4 u[1];
};

void a(matrix<float16_t, 3, 2> m) {
}

void b(vector<float16_t, 2> v) {
}

void c(float16_t f_1) {
}

matrix<float16_t, 3, 2> tint_symbol(uint4 buffer[1], uint offset) {
  const uint scalar_offset = ((offset + 0u)) / 4;
  uint ubo_load = buffer[scalar_offset / 4][scalar_offset % 4];
  const uint scalar_offset_1 = ((offset + 4u)) / 4;
  uint ubo_load_1 = buffer[scalar_offset_1 / 4][scalar_offset_1 % 4];
  const uint scalar_offset_2 = ((offset + 8u)) / 4;
  uint ubo_load_2 = buffer[scalar_offset_2 / 4][scalar_offset_2 % 4];
  return matrix<float16_t, 3, 2>(vector<float16_t, 2>(float16_t(f16tof32(ubo_load & 0xFFFF)), float16_t(f16tof32(ubo_load >> 16))), vector<float16_t, 2>(float16_t(f16tof32(ubo_load_1 & 0xFFFF)), float16_t(f16tof32(ubo_load_1 >> 16))), vector<float16_t, 2>(float16_t(f16tof32(ubo_load_2 & 0xFFFF)), float16_t(f16tof32(ubo_load_2 >> 16))));
}

[numthreads(1, 1, 1)]
void f() {
  a(tint_symbol(u, 0u));
  uint ubo_load_3 = u[0].y;
  b(vector<float16_t, 2>(float16_t(f16tof32(ubo_load_3 & 0xFFFF)), float16_t(f16tof32(ubo_load_3 >> 16))));
  uint ubo_load_4 = u[0].y;
  b(vector<float16_t, 2>(float16_t(f16tof32(ubo_load_4 & 0xFFFF)), float16_t(f16tof32(ubo_load_4 >> 16))).yx);
  c(float16_t(f16tof32(((u[0].y) & 0xFFFF))));
  uint ubo_load_5 = u[0].y;
  c(vector<float16_t, 2>(float16_t(f16tof32(ubo_load_5 & 0xFFFF)), float16_t(f16tof32(ubo_load_5 >> 16))).yx.x);
  return;
}
FXC validation failure:
D:\Projects\RampUp\dawn\test\tint\buffer\Shader@0x0000014896F24370(5,15-23): error X3000: syntax error: unexpected token 'float16_t'
D:\Projects\RampUp\dawn\test\tint\buffer\Shader@0x0000014896F24370(8,15-23): error X3000: syntax error: unexpected token 'float16_t'
D:\Projects\RampUp\dawn\test\tint\buffer\Shader@0x0000014896F24370(11,8-16): error X3000: unrecognized identifier 'float16_t'

