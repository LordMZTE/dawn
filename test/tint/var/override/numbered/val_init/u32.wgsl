@id(1234) override o : u32 = 1u;

@compute @workgroup_size(1)
fn main() {
    _ = o;
}
