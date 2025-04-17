// Constant registers
//float4x4 viewMatrix       : register(c4);
//float4x4 projectionMatrix : register(c8);
//
float4x4 wvpMatrix : register(c0);
float4x4 worldMatrix      : register(c4);
float4x4 texProjMatrix    : register(c8); // For projected noise texture

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
    float4 tex1     : TEXCOORD1;
};

VS_OUTPUT main(VS_INPUT input)
{
/*
    VS_OUTPUT output;

    // Convert to float4 for matrix multiplication
    float4 posW = mul(float4(input.position, 1.0), worldMatrix);
    float4 posV = mul(posW, viewMatrix);
    output.position = mul(posV, projectionMatrix);

    // Diffuse color passthrough
    output.color = input.color;

    // Base UVs for texture 0
    output.tex0 = input.texcoord;

    // Projected UVs for texture 1 (clouds/noise)
    output.tex1 = mul(posW, texProjMatrix); // Needs to be float4 for tex2Dproj

    return output;
*/

    VS_OUTPUT output;

    // Transform position to clip space
    output.position = mul(float4(input.position, 1.0), wvpMatrix);

    // Pass through color and texcoord
    output.color = input.color;
    output.tex0 = input.texcoord;

    float4 posW = mul(float4(input.position, 1.0), worldMatrix);

    // Projected UVs for texture 1 (clouds/noise)
    output.tex1 = mul(posW, texProjMatrix); // Needs to be float4 for tex2Dproj
    //output.tex1 = float4(0,0,0,0);

    return output;

}
