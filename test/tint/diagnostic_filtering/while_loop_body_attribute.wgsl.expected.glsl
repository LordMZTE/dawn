diagnostic_filtering/while_loop_body_attribute.wgsl:8:9 warning: 'textureSample' must only be called from uniform control flow
    v = textureSample(t, s, vec2(0, 0));
        ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

diagnostic_filtering/while_loop_body_attribute.wgsl:7:3 note: control flow depends on possibly non-uniform value
  while (x > v.x) @diagnostic(warning, derivative_uniformity) {
  ^^^^^

diagnostic_filtering/while_loop_body_attribute.wgsl:8:9 note: return value of 'textureSample' may be non-uniform
    v = textureSample(t, s, vec2(0, 0));
        ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

#version 310 es
precision highp float;

layout(location = 0) in float x_1;
uniform highp sampler2D t_s;

void tint_symbol(float x) {
  vec4 v = vec4(0.0f);
  while((x > v.x)) {
    v = texture(t_s, vec2(0.0f));
  }
}

void main() {
  tint_symbol(x_1);
  return;
}