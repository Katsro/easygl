#version 460 core

layout (binding = 0, rgba16f) uniform readonly image2D data0;

out vec4 FragColor;

// df: visualise electromagnetic field
// .r = Ez (electric)  — mapped to warm red/green
// .b/.a = Hx/Hy (magnetic) — magnitude mapped to cool blue/cyan

void main() {
    ivec2 p = ivec2(gl_FragCoord.xy) & 1023;
    vec4 v = imageLoad(data0, p);

    float eScale = 5.0;
    float hMag = sqrt(v.b * v.b + v.a * v.a);

    vec3 col = eScale * vec3(max(v.r, 0.0), max(-v.r, 0.0), 0.0)
             + vec3(0.0, hMag * 1.75, hMag * 5.0);

    FragColor = vec4(col / (1.0 + col), 1.0);
}
