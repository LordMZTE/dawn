fn tanh_5663c5() {
  var res : vec4<f32> = tanh(vec4<f32>());
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  tanh_5663c5();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  tanh_5663c5();
}

@compute @workgroup_size(1)
fn compute_main() {
  tanh_5663c5();
}
