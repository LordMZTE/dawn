struct SB_RW {
  arg_0 : atomic<u32>,
}

@group(0) @binding(0) var<storage, read_write> sb_rw : SB_RW;

fn atomicXor_54510e() {
  var arg_1 = 1u;
  var res : u32 = atomicXor(&(sb_rw.arg_0), arg_1);
}

@fragment
fn fragment_main() {
  atomicXor_54510e();
}

@compute @workgroup_size(1)
fn compute_main() {
  atomicXor_54510e();
}
