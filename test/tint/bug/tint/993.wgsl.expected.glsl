#version 310 es

struct Constants {
  uint zero;
  uint pad;
  uint pad_1;
  uint pad_2;
};

layout(binding = 0, std140) uniform constants_block_ubo {
  Constants inner;
} constants;

struct Result {
  uint value;
};

layout(binding = 1, std430) buffer result_block_ssbo {
  Result inner;
} result;

struct TestData {
  int data[3];
};

layout(binding = 0, std430) buffer s_block_ssbo {
  TestData inner;
} s;

int runTest() {
  return atomicOr(s.inner.data[(0u + uint(constants.inner.zero))], 0);
}

void tint_symbol() {
  int tint_symbol_1 = runTest();
  uint tint_symbol_2 = uint(tint_symbol_1);
  result.inner.value = tint_symbol_2;
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol();
  return;
}
