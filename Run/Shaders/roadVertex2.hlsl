// Constant registers
//float4x4 viewMatrix       : register(c4);
//float4x4 projectionMatrix : register(c8);
//
float4x4 wvpMatrix : register(c0);
float4x4 worldMatrix      : register(c4);
//float4x4 texProjMatrix    : register(c8); // For projected noise texture
float4x4 worldViewMatrix      : register(c12);

struct VS_INPUT
{
    float3 position : POSITION;
    float4 color    : COLOR0;
    float2 texcoord : TEXCOORD0;
};

struct VS_OUTPUT
{
    float4 position : POSITION;
    float4 color    : COLOR0;
    float2 tex0     : TEXCOORD0;
    float3 WorldPos     : TEXCOORD1;
};

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT output;
    output.position = mul(float4(input.position,1.0), wvpMatrix); // standard MVP transform
    output.color = input.color;
    output.tex0 = input.texcoord;

    output.WorldPos = mul(float4(input.position,1.0), worldMatrix).xyz; // pos in world space
    return output;
}

