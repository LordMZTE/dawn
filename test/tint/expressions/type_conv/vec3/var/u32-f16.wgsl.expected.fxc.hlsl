SKIP: FAILED

[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

static uint3 u = (1u).xxx;

void f() {
  const vector<float16_t, 3> v = vector<float16_t, 3>(u);
}
FXC validation failure:
D:\Projects\RampUp\dawn\test\tint\expressions\type_conv\Shader@0x00000140A91D3590(9,16-24): error X3000: syntax error: unexpected token 'float16_t'

