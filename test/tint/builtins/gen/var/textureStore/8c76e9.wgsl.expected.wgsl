@group(1) @binding(0) var arg_0 : texture_storage_1d<rgba16sint, write>;

fn textureStore_8c76e9() {
  var arg_1 = 1u;
  var arg_2 = vec4<i32>(1i);
  textureStore(arg_0, arg_1, arg_2);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureStore_8c76e9();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureStore_8c76e9();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_8c76e9();
}
