// c0–c3: WorldViewProjection
// c4–c7: WorldView
// c8–c11: Texture Transform 0 (D3DTS_TEXTURE0)
// c12–c15: Texture Transform 1 (D3DTS_TEXTURE1)

float4x4 g_mWVP        : register(c0);
float4x4 g_mWV         : register(c4);
float4x4 g_mTex0       : register(c8);
float4x4 g_mTex1       : register(c12);

struct VS_INPUT
{
    float3 position : POSITION; // v0
    float4 color    : COLOR0;   // v1
    float2 tex0     : TEXCOORD0; // v2
    float2 tex1     : TEXCOORD1; // v3
};

struct VS_OUTPUT
{
    float4 position    : POSITION;
    float4 color       : COLOR0;
    float2 texcoord0   : TEXCOORD0;
    float2 texcoord1   : TEXCOORD1;
};

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT output;

    float4 worldViewPos = mul(float4(input.position, 1.0f), g_mWV);

    // Now apply texture transforms manually
    float4 tex0 = mul(worldViewPos, g_mTex0);
    float4 tex1 = mul(worldViewPos, g_mTex1);

    output.texcoord0 = tex0.xy;
    output.texcoord1 = tex1.xy;

    output.color = input.color;
    output.position = mul(float4(input.position, 1.0f), g_mWVP);

    return output;
}
