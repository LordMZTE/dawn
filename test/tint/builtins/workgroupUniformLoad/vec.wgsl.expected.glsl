#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void unused_entry_point() {
  return;
}
vec4 tint_workgroupUniformLoad(inout vec4 p) {
  barrier();
  vec4 result = p;
  barrier();
  return result;
}

shared vec4 v;
vec4 foo() {
  return tint_workgroupUniformLoad(v);
}

