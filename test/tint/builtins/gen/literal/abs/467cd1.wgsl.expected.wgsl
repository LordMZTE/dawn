fn abs_467cd1() {
  var res : u32 = abs(1u);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : u32;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  abs_467cd1();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  abs_467cd1();
}

@compute @workgroup_size(1)
fn compute_main() {
  abs_467cd1();
}
