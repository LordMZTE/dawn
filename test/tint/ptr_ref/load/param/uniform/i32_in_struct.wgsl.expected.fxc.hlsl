cbuffer cbuffer_S : register(b0, space0) {
  uint4 S[1];
};

int func_S_i() {
  return asint(S[0].x);
}

[numthreads(1, 1, 1)]
void main() {
  const int r = func_S_i();
  return;
}
