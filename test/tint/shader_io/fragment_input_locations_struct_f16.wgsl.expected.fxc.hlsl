SKIP: FAILED

struct FragmentInputs {
  int loc0;
  uint loc1;
  float loc2;
  float4 loc3;
  float16_t loc4;
  vector<float16_t, 3> loc5;
};
struct tint_symbol_1 {
  nointerpolation int loc0 : TEXCOORD0;
  nointerpolation uint loc1 : TEXCOORD1;
  float loc2 : TEXCOORD2;
  float4 loc3 : TEXCOORD3;
  float16_t loc4 : TEXCOORD4;
  vector<float16_t, 3> loc5 : TEXCOORD5;
};

void main_inner(FragmentInputs inputs) {
  const int i = inputs.loc0;
  const uint u = inputs.loc1;
  const float f = inputs.loc2;
  const float4 v = inputs.loc3;
  const float16_t x = inputs.loc4;
  const vector<float16_t, 3> y = inputs.loc5;
}

void main(tint_symbol_1 tint_symbol) {
  const FragmentInputs tint_symbol_2 = {tint_symbol.loc0, tint_symbol.loc1, tint_symbol.loc2, tint_symbol.loc3, tint_symbol.loc4, tint_symbol.loc5};
  main_inner(tint_symbol_2);
  return;
}
FXC validation failure:
D:\Projects\RampUp\dawn\test\tint\shader_io\Shader@0x000001A7231510D0(6,3-11): error X3000: unrecognized identifier 'float16_t'

