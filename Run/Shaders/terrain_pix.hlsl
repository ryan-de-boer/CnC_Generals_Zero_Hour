sampler2D tex0 : register(s0);

// Input structure from vertex shader
struct PSInput
{
    float4 color : COLOR0;      // v1.xyzw
    float2 texcoord : TEXCOORD0; // v3.xy
};

// Output structure
float4 main(PSInput input) : SV_Target
{
    // Sample the texture using the provided coordinates
    float4 texColor = tex2D(tex0, input.texcoord);

    // Multiply the sampled color with the input color (RGB) and clamp between 0 and 1
    float3 rgb = saturate(texColor.rgb * input.color.rgb);

    // Copy and clamp alpha from input color
    float a = saturate(input.color.a);


//return float4(0, 0, 1, 1); // blue
    return float4(rgb, a);
}
