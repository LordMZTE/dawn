fn log2_aea659() {
  var res : vec2<f32> = log2(vec2<f32>());
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  log2_aea659();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  log2_aea659();
}

@compute @workgroup_size(1)
fn compute_main() {
  log2_aea659();
}
