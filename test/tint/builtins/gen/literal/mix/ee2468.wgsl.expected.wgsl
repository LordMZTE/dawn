enable f16;

fn mix_ee2468() {
  var res : vec4<f16> = mix(vec4<f16>(f16()), vec4<f16>(f16()), vec4<f16>(f16()));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  mix_ee2468();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  mix_ee2468();
}

@compute @workgroup_size(1)
fn compute_main() {
  mix_ee2468();
}
