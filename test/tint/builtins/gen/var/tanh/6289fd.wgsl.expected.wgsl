fn tanh_6289fd() {
  const arg_0 = vec3(1.0);
  var res = tanh(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  tanh_6289fd();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  tanh_6289fd();
}

@compute @workgroup_size(1)
fn compute_main() {
  tanh_6289fd();
}
