// The msg renderer's mesh material: sMsgMaterial and sMsgMaterialGrowable.
//
// The constant block is mvp (row_major float4x4), color and grow: 96 bytes,
// uploaded as Vulkan push constants.  Its C++ counterpart is sMsgMaterialEnvPara
// in libs/msglib/shaders.hpp; libs/msglib/shaders.asc describes the same block
// in Altona shader-compiler (asc) syntax.
//
// Row-vector convention: out_pos = mul(in_pos, mvp), with mvp stored row-major
// exactly as Altona's sMatrix44.  The picture is rendered top-down; the Y flip
// is done with a negative-height viewport on the C++ side, not here.

struct Env
{
    row_major float4x4 mvp;
    float4 color;   // material colour
    float4 grow;    // x: grow position  y: grow strength  z: scale  w: range
};
[[vk::push_constant]] ConstantBuffer<Env> env;

// Original sMtrl alpha-test: FuncFlags[ALPHA] = GREATEREQUAL, AlphaRef = 80.
// -1 disables the test.  Set per pipeline through a specialization constant.
[[vk::specialization_constant]] const int ALPHA_REF = -1;

struct VSIn
{
    [[vk::location(0)]] float4 pos : POSITION;   // float3 in memory, w defaults to 1
    [[vk::location(1)]] float4 col : COLOR0;     // unorm8x4
};

struct VSInUV
{
    [[vk::location(0)]] float4 pos : POSITION;
    [[vk::location(1)]] float4 col : COLOR0;
    [[vk::location(2)]] float2 uv0 : TEXCOORD0;
};

struct VSOut
{
    float4 pos : SV_Position;
    float4 col : COLOR0;
};

struct VSOutUV
{
    float4 pos : SV_Position;
    float4 col : COLOR0;
    float2 uv0 : TEXCOORD0;
};

// ---------------------------------------------------------------- sMsgMaterial

[shader("vertex")]
VSOut vsPlain(VSIn i)
{
    VSOut o;
    o.col = env.color * i.col;
    o.pos = mul(i.pos, env.mvp);
    return o;
}

[shader("fragment")]
float4 psPlain(VSOut i) : SV_Target
{
    if (ALPHA_REF >= 0 && i.col.a * 255.0 < float(ALPHA_REF))
        discard;
    return i.col;
}

// ---------------------------------------------------- sMsgMaterialGrowable

[shader("vertex")]
VSOutUV vsGrowable(VSInUV i)
{
    VSOutUV o;
    float4 c  = env.color;
    float  gy = env.grow.y;
    float  g  = env.grow.x;

    o.uv0 = i.uv0;
    if (g <= 0.0)
        o.uv0.x += 0.6;
    else if (g >= 1.0)
        o.uv0.x -= 0.6;
    else
        o.uv0.x = (o.uv0.x - g) * env.grow.z * 64.0 + 0.5;

    float pos = clamp((g - i.uv0.x) * env.grow.w, -0.1, 1.1);
    o.uv0.y = o.uv0.y - 0.5 + pos * gy + 1.0 * (1.0 - gy);

    c *= i.col;
    o.col = c;
    o.pos = mul(i.pos, env.mvp);
    return o;
}

[[vk::binding(0, 0)]] Sampler2D s0;

[shader("fragment")]
float4 psGrowable(VSOutUV i) : SV_Target
{
    float4 r = i.col * s0.Sample(i.uv0);
    if (ALPHA_REF >= 0 && r.a * 255.0 < float(ALPHA_REF))
        discard;
    return r;
}
