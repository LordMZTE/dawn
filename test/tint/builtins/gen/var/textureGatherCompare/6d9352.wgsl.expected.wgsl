@group(1) @binding(0) var arg_0 : texture_depth_2d;

@group(1) @binding(1) var arg_1 : sampler_comparison;

fn textureGatherCompare_6d9352() {
  var arg_2 = vec2<f32>();
  var arg_3 = 1.0;
  var res : vec4<f32> = textureGatherCompare(arg_0, arg_1, arg_2, arg_3);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureGatherCompare_6d9352();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureGatherCompare_6d9352();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureGatherCompare_6d9352();
}
