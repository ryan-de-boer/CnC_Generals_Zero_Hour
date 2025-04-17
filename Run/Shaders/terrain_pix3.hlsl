sampler2D sClouds : register(s0); // Texture0 (clouds)
sampler2D sNoise  : register(s1); // Texture1 (noise)

struct PS_INPUT
{
    float4 color      : COLOR0;
    float2 texcoord0  : TEXCOORD0; // for clouds
    float2 texcoord1  : TEXCOORD1; // for noise
};

float4 main(PS_INPUT input) : COLOR
{
    // Sample cloud and noise textures
    float4 cloudColor = tex2D(sClouds, input.texcoord0);
    float4 noiseColor = tex2D(sNoise,  input.texcoord1);

    // Match the fixed-function: stage 0 selects cloud texture
    // Stage 1 modulates it with noise texture
    float3 finalColor = cloudColor.rgb * noiseColor.rgb;

    // Final output has no alpha; blending is done via render state
    return float4(finalColor, 1.0);
}
