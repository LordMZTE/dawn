SKIP: FAILED

vector<float16_t, 2> tint_atanh(vector<float16_t, 2> x) {
  return (log(((float16_t(1.0h) + x) / (float16_t(1.0h) - x))) * float16_t(0.5h));
}

void atanh_5bf88d() {
  vector<float16_t, 2> arg_0 = (float16_t(0.0h)).xx;
  vector<float16_t, 2> res = tint_atanh(arg_0);
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  atanh_5bf88d();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  atanh_5bf88d();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  atanh_5bf88d();
  return;
}
FXC validation failure:
D:\Projects\RampUp\dawn\test\tint\builtins\Shader@0x00000223A4C149E0(1,8-16): error X3000: syntax error: unexpected token 'float16_t'

