#version 310 es
precision mediump float;

uniform highp samplerCube arg_0_arg_1;

void textureSampleBias_53b9f7() {
  vec3 arg_2 = vec3(0.0f);
  float arg_3 = 1.0f;
  vec4 res = texture(arg_0_arg_1, arg_2, arg_3);
}

void fragment_main() {
  textureSampleBias_53b9f7();
}

void main() {
  fragment_main();
  return;
}
