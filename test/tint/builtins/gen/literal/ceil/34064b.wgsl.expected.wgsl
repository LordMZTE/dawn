fn ceil_34064b() {
  var res : vec3<f32> = ceil(vec3<f32>(1.5f));
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec3<f32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  ceil_34064b();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  ceil_34064b();
}

@compute @workgroup_size(1)
fn compute_main() {
  ceil_34064b();
}
