#version 310 es
precision mediump float;

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  vec2 inner;
} prevent_dce;

void fwidthFine_ff6aa0() {
  vec2 arg_0 = vec2(1.0f);
  vec2 res = fwidth(arg_0);
  prevent_dce.inner = res;
}

void fragment_main() {
  fwidthFine_ff6aa0();
}

void main() {
  fragment_main();
  return;
}
