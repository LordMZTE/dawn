TextureCube arg_0 : register(t0, space1);
SamplerState arg_1 : register(s1, space1);

void textureSampleLevel_ae92a2() {
  float3 arg_2 = (1.0f).xxx;
  uint arg_3 = 1u;
  float res = arg_0.SampleLevel(arg_1, arg_2, arg_3).x;
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  textureSampleLevel_ae92a2();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  textureSampleLevel_ae92a2();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureSampleLevel_ae92a2();
  return;
}