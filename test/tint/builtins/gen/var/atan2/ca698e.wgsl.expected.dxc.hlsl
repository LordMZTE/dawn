void atan2_ca698e() {
  float16_t arg_0 = float16_t(0.0h);
  float16_t arg_1 = float16_t(0.0h);
  float16_t res = atan2(arg_0, arg_1);
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  atan2_ca698e();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  atan2_ca698e();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  atan2_ca698e();
  return;
}
