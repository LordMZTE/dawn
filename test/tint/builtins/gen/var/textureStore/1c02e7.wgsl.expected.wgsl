@group(1) @binding(0) var arg_0 : texture_storage_2d_array<r32sint, write>;

fn textureStore_1c02e7() {
  var arg_1 = vec2<i32>();
  var arg_2 = 1;
  var arg_3 = vec4<i32>();
  textureStore(arg_0, arg_1, arg_2, arg_3);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureStore_1c02e7();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureStore_1c02e7();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_1c02e7();
}
