[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

void f() {
  {
    const int vec3f_1 = 1;
    const int b = vec3f_1;
  }
  const float3 c = (0.0f).xxx;
  const float3 d = (0.0f).xxx;
}
