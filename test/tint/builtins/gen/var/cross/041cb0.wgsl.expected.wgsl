fn cross_041cb0() {
  var arg_0 = vec3<f32>(1.0f);
  var arg_1 = vec3<f32>(1.0f);
  var res : vec3<f32> = cross(arg_0, arg_1);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  cross_041cb0();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  cross_041cb0();
}

@compute @workgroup_size(1)
fn compute_main() {
  cross_041cb0();
}
