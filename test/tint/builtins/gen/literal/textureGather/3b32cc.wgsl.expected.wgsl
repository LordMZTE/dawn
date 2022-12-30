@group(1) @binding(1) var arg_1 : texture_cube<u32>;

@group(1) @binding(2) var arg_2 : sampler;

fn textureGather_3b32cc() {
  var res : vec4<u32> = textureGather(1i, arg_1, arg_2, vec3<f32>(1.0f));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureGather_3b32cc();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureGather_3b32cc();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureGather_3b32cc();
}
