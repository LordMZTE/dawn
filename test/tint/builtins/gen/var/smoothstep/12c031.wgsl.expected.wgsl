enable f16;

fn smoothstep_12c031() {
  var arg_0 = vec2<f16>(2.0h);
  var arg_1 = vec2<f16>(4.0h);
  var arg_2 = vec2<f16>(3.0h);
  var res : vec2<f16> = smoothstep(arg_0, arg_1, arg_2);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec2<f16>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  smoothstep_12c031();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  smoothstep_12c031();
}

@compute @workgroup_size(1)
fn compute_main() {
  smoothstep_12c031();
}
