// The post-pipeline material sIppToolMaterial: the motion-blur accumulation
// blits, the glare combine (Normal) and the separable glare blur (Spread8).
// The other permutations in libs/msglib/ippmtrl.asc (Burnout, Gray, Dof,
// Spread8Far, Boost) are never invoked by msglib and are not implemented here.
//
// ColorAdd/Color0/Color1/ColorMul are read from push constants: the VS block
// (mvp) and the PS block (colours) follow each other, 64 + 64 = 128 bytes.

struct Env
{
    row_major float4x4 mvp;
    float4 ColorAdd;
    float4 Color0;
    float4 Color1;
    float4 ColorMul;
};
[[vk::push_constant]] ConstantBuffer<Env> env;

// Permutation, chosen per pipeline.
[[vk::specialization_constant]] const bool T0 = true;
[[vk::specialization_constant]] const bool T1 = false;
[[vk::specialization_constant]] const bool T2 = false;
[[vk::specialization_constant]] const bool T3 = false;
[[vk::specialization_constant]] const int  SPECIAL = 0;   // 0 = Normal, 1 = Spread8

struct VSIn
{
    [[vk::location(0)]] float4 pos : POSITION;
    // location 1 is the (unused) vertex colour of sVertexFormatDouble
    [[vk::location(2)]] float2 uv0 : TEXCOORD0;
    [[vk::location(3)]] float2 uv1 : TEXCOORD1;
};

struct VSOut
{
    float4 pos : SV_Position;
    float2 uv0 : TEXCOORD0;
    float2 uv1 : TEXCOORD1;
};

[shader("vertex")]
VSOut vsMain(VSIn i)
{
    VSOut o;
    o.uv0 = i.uv0;
    o.uv1 = i.uv1;
    o.pos = mul(i.pos, env.mvp);
    return o;
}

[[vk::binding(0, 0)]] Sampler2D s0;
[[vk::binding(1, 0)]] Sampler2D s1;
[[vk::binding(2, 0)]] Sampler2D s2;
[[vk::binding(3, 0)]] Sampler2D s3;

[shader("fragment")]
float4 psMain(VSOut i) : SV_Target
{
    float4 result;

    if (SPECIAL == 1)
    {
        // Spread8: eight taps along uv1, weights in Color0.xyzw / Color1.xyzw
        result  = s0.Sample(i.uv0 + 0.0 * i.uv1) * env.Color0.x;
        result += s0.Sample(i.uv0 + 1.0 * i.uv1) * env.Color0.y;
        result += s0.Sample(i.uv0 + 2.0 * i.uv1) * env.Color0.z;
        result += s0.Sample(i.uv0 + 3.0 * i.uv1) * env.Color0.w;
        result += s0.Sample(i.uv0 + 4.0 * i.uv1) * env.Color1.x;
        result += s0.Sample(i.uv0 + 5.0 * i.uv1) * env.Color1.y;
        result += s0.Sample(i.uv0 + 6.0 * i.uv1) * env.Color1.z;
        result += s0.Sample(i.uv0 + 7.0 * i.uv1) * env.Color1.w;
    }
    else
    {
        float4 t0 = T0 ? s0.Sample(i.uv0) : float4(0, 0, 0, 0);
        float4 t1 = T1 ? s1.Sample(i.uv1) : float4(0, 0, 0, 0);
        float4 t2 = T2 ? s2.Sample(i.uv0) : float4(0, 0, 0, 0);   // sic: uv0, as in the original
        float4 t3 = T3 ? s3.Sample(i.uv1) : float4(0, 0, 0, 0);
        result = env.ColorAdd
               + t0 * env.Color0 + t1 * env.Color1
               + t2 * env.Color0 + t3 * env.Color1
               + t0 * t1 * env.ColorMul;
    }
    return result;
}
