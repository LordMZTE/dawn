@group(1) @binding(0) var arg_0 : texture_storage_3d<rgba8uint, write>;

fn textureStore_d2b565() {
  textureStore(arg_0, vec3<u32>(1u), vec4<u32>(1u));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureStore_d2b565();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureStore_d2b565();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_d2b565();
}
