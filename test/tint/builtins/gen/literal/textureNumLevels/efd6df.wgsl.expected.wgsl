@group(1) @binding(0) var arg_0 : texture_2d<u32>;

fn textureNumLevels_efd6df() {
  var res : u32 = textureNumLevels(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureNumLevels_efd6df();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureNumLevels_efd6df();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureNumLevels_efd6df();
}
