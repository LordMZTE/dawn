SKIP: FAILED

void dot_8e40f1() {
  float16_t res = dot((float16_t(0.0h)).xxx, (float16_t(0.0h)).xxx);
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  dot_8e40f1();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  dot_8e40f1();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  dot_8e40f1();
  return;
}
FXC validation failure:
D:\Projects\RampUp\dawn\test\tint\builtins\Shader@0x000001E9C49996F0(2,3-11): error X3000: unrecognized identifier 'float16_t'
D:\Projects\RampUp\dawn\test\tint\builtins\Shader@0x000001E9C49996F0(2,13-15): error X3000: unrecognized identifier 'res'

