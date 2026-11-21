// The simple material sSimpleMaterial: debug lines and sPainter text.
// Push constants: 64 bytes, mvp only (row_major, Altona sMatrix44 layout).
struct Env { row_major float4x4 mvp; };
[[vk::push_constant]] ConstantBuffer<Env> env;

[[vk::specialization_constant]] const bool COLOR = true;   // vertex has colour0
[[vk::specialization_constant]] const bool UV0   = false;  // vertex has uv0
[[vk::specialization_constant]] const bool T0MUL = false;  // multiply with texture 0

struct VSIn
{
    [[vk::location(0)]] float4 pos : POSITION;
    [[vk::location(1)]] float4 col : COLOR0;
    [[vk::location(2)]] float2 uv0 : TEXCOORD0;
};
struct VSOut { float4 pos : SV_Position; float4 col : COLOR0; float2 uv0 : TEXCOORD0; };

[shader("vertex")]
VSOut vsMain(VSIn i)
{
    VSOut o;
    o.pos = mul(i.pos, env.mvp);
    o.col = COLOR ? i.col : float4(1, 1, 1, 1);
    o.uv0 = UV0 ? i.uv0 : float2(0, 0);
    return o;
}

[[vk::binding(0, 0)]] Sampler2D s0;

[shader("fragment")]
float4 psMain(VSOut i) : SV_Target
{
    float4 r = i.col;
    if (T0MUL) r *= s0.Sample(i.uv0);
    return r;
}
