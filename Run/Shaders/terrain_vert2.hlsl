float4x4 worldViewProj : register(c0);
float4 SSBias : register(c4); // Additional transform scale/bias?
float4x4 worldView     : register(c5); // for camera-space transform

struct VSInput
{
    float4 position : POSITION; // v0
    float4 color    : COLOR0;   // v1
    float4 tex0     : TEXCOORD0; // v2
    float4 tex1     : TEXCOORD1; // v3
};

struct VSOutput
{
    float4 pos      : SV_POSITION;
    float4 color    : COLOR0;
    float2 texcoord : TEXCOORD0;
    float2 texcoord1 : TEXCOORD1;
    float4 worldViewPos : TEXCOORD2; // camera-space position
};

VSOutput main(VSInput input)
{
    VSOutput output;

    output.pos = mul(input.position, worldViewProj);
    output.worldViewPos = mul(input.position, worldView);

    // Apply additional transform to x/y using cb3[0] (likely screen-space scale/bias)
    output.pos.xy = SSBias.y * output.pos.ww + output.pos.xy; // mad r11.xy, cb3[0].xyyy, r11.wwww, r11.xyyy

    // Clamp color input
//    output.color = saturate(input.color);
    output.color = input.color;

    // Pass through texcoord.xy
    output.texcoord = input.tex0.xy;
    output.texcoord1 = input.tex1.xy;  //note pixel shader pass 2 must use this!

    return output;
}
