@group(1) @binding(0) var arg_0 : texture_storage_2d_array<r32uint, write>;

fn textureStore_38e8d7() {
  textureStore(arg_0, vec2<i32>(1i), 1i, vec4<u32>(1u));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureStore_38e8d7();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureStore_38e8d7();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_38e8d7();
}
