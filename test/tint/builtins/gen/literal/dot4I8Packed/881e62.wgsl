// Copyright 2021 The Tint Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

////////////////////////////////////////////////////////////////////////////////
// File generated by tools/src/cmd/gen
// using the template:
//   test/tint/builtins/gen/gen.wgsl.tmpl
//
// Do not modify this file directly
////////////////////////////////////////////////////////////////////////////////


enable chromium_experimental_dp4a;

// fn dot4I8Packed(u32, u32) -> i32
fn dot4I8Packed_881e62() {
  var res: i32 = dot4I8Packed(1u, 1u);
  prevent_dce = res;
}
@group(2) @binding(0) var<storage, read_write> prevent_dce : i32;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  dot4I8Packed_881e62();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  dot4I8Packed_881e62();
}

@compute @workgroup_size(1)
fn compute_main() {
  dot4I8Packed_881e62();
}
