fn transpose_31e37e() {
  var res : mat2x4<f32> = transpose(mat4x2<f32>());
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  transpose_31e37e();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  transpose_31e37e();
}

@compute @workgroup_size(1)
fn compute_main() {
  transpose_31e37e();
}
