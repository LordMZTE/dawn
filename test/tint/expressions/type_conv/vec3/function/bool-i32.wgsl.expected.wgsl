var<private> t : bool;

fn m() -> vec3<bool> {
  t = true;
  return vec3<bool>(t);
}

fn f() {
  var v : vec3<i32> = vec3<i32>(m());
}
