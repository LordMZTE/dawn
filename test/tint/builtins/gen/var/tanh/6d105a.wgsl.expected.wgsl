enable f16;

fn tanh_6d105a() {
  var arg_0 = vec2<f16>(f16());
  var res : vec2<f16> = tanh(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  tanh_6d105a();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  tanh_6d105a();
}

@compute @workgroup_size(1)
fn compute_main() {
  tanh_6d105a();
}
