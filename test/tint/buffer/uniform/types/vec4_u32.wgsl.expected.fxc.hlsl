cbuffer cbuffer_u : register(b0, space0) {
  uint4 u[1];
};

[numthreads(1, 1, 1)]
void main() {
  const uint4 x = u[0];
  return;
}