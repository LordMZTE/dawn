void tan_244e2a() {
  float4 res = tan((0.0f).xxxx);
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  tan_244e2a();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  tan_244e2a();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  tan_244e2a();
  return;
}
