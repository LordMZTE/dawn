ByteAddressBuffer data : register(t1, space0);

int foo() {
  return asint(data.Load(0u));
}

[numthreads(16, 16, 1)]
void main() {
  foo();
  return;
}
