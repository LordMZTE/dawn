fn clamp_5f0819() {
  var arg_0 = vec3<i32>();
  var arg_1 = vec3<i32>();
  var arg_2 = vec3<i32>();
  var res : vec3<i32> = clamp(arg_0, arg_1, arg_2);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  clamp_5f0819();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  clamp_5f0819();
}

@compute @workgroup_size(1)
fn compute_main() {
  clamp_5f0819();
}
