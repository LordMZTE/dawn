enable f16;

fn normalize_7990f3() {
  var arg_0 = vec2<f16>(f16());
  var res : vec2<f16> = normalize(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  normalize_7990f3();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  normalize_7990f3();
}

@compute @workgroup_size(1)
fn compute_main() {
  normalize_7990f3();
}
