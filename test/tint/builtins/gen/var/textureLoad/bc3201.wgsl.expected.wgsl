@group(1) @binding(0) var arg_0 : texture_1d<u32>;

fn textureLoad_bc3201() {
  var arg_1 = 1u;
  var arg_2 = 1u;
  var res : vec4<u32> = textureLoad(arg_0, arg_1, arg_2);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureLoad_bc3201();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureLoad_bc3201();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureLoad_bc3201();
}
