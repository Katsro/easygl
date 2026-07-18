#version 460 core

out gl_PerVertex {
    vec4 gl_Position;
};
void main() {
    int vt = gl_VertexID;
    gl_Position = vec4((vt << 1 & 2) - 1,- (vt & 2) + 1, 0, 1);
}
