#version 460 core

layout (binding = 0, rgba16f) uniform readonly image2D data0;

out vec4 FragColor;

// df: visualise electromagnetic field
// .r = Ez (electric)  — mapped to warm red/green
// .b.a = Hx.Hy (magnetic) — magnitude mapped to cool blue/cyan

void main() {
    ivec2 p = ivec2(gl_FragCoord.xy) & 1023;
    vec4 v = imageLoad(data0, p);

    float Ez  = v.r;
    float Hx  = v.b;
    float Hy  = v.a;

    // ── Electric: Ez > 0 → red,  Ez < 0 → green ──
    float eScale = 5.0;                       // boost for visibility
    vec3 eCol = eScale * vec3(max(Ez, 0.0), max(-Ez, 0.0), 0.0);

    // ── Magnetic: ｜H｜ → blue/cyan ──
    float hMag  = sqrt(Hx * Hx + Hy * Hy);
    float hScale = 5.0;
    vec3 hCol = hScale * vec3(0.0, hMag * 0.35, hMag);

    // Combine; soft tone-map to avoid hard clipping
    vec3 col = eCol + hCol;
    col = col / (1.0 + col);

    FragColor = vec4(col, 1.0);
}
