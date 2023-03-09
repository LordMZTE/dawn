@group(1) @binding(0) var arg_0 : texture_3d<f32>;

@group(1) @binding(1) var arg_1 : sampler;

fn textureSampleLevel_abfcc0() {
  var arg_2 = vec3<f32>(1.0f);
  var arg_3 = 1.0f;
  var res : vec4<f32> = textureSampleLevel(arg_0, arg_1, arg_2, arg_3);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec4<f32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureSampleLevel_abfcc0();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureSampleLevel_abfcc0();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureSampleLevel_abfcc0();
}
