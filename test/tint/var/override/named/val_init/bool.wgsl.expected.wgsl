override o : bool = true;

@compute @workgroup_size(1)
fn main() {
  _ = o;
}
