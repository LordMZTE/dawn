@group(1) @binding(0) var arg_0 : texture_storage_2d_array<rgba8unorm, write>;

fn textureNumLayers_8dbf23() {
  var res : u32 = textureNumLayers(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureNumLayers_8dbf23();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureNumLayers_8dbf23();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureNumLayers_8dbf23();
}
