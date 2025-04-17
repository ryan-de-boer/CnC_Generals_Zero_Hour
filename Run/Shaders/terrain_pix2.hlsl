sampler2D tex0 : register(s0);

// Input structure from vertex shader
struct PSInput
{
    float4 color : COLOR0;      
    float2 texcoord : TEXCOORD0; 
    float2 texcoord1 : TEXCOORD1; // pass 2 pixel shader must use this
};

// Output structure
float4 main(PSInput input) : SV_Target
{
    // Sample the texture using the provided coordinates
    float4 texColor = tex2D(tex0, input.texcoord1);

   // Multiply and saturate RGB and Alpha
    float3 rgb = saturate(texColor.rgb * input.color.rgb);
    float alpha = saturate(texColor.a * input.color.a);

//return float4(1, 1, 0, 1); // yellow
    return float4(rgb, alpha);
}
