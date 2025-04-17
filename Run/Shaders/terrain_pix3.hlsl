sampler2D cloudSampler : register(s0);  // cloud texture
sampler2D noiseSampler : register(s1);  // noise texture

// Constant buffer simulation for DX9
// cb0[1].w == blend alpha
float4 blendAlpha : register(c0);  // only .w is used

float4x4 texTransform0 : register(c1);  // texture transform for cloud
float4x4 texTransform1 : register(c5);  // texture transform for noise

struct PS_INPUT {
    float4 position  : POSITION;
    float4 diffuse   : COLOR0;

    float2 texcoord : TEXCOORD0;
    float2 texcoord1 : TEXCOORD1;
    float4 worldViewPos : TEXCOORD2; // camera-space position passed from VS
};

float4 main(PS_INPUT input) : COLOR
{
/*
    // Apply texture transform to camera-space position (2D project)
//float4 viewPos = input.worldViewPos.xyzw;// / input.worldViewPos.w;

//float3 viewPos = input.worldViewPos.xyz / input.worldViewPos.w;
float invW = 1.0 / max(input.worldViewPos.w, 0.0001);
float3 viewPos = input.worldViewPos.xyz * invW;

    float2 texCoord0 = mul(texTransform0, float4(viewPos, 1.0)).xy;
    float2 texCoord1 = mul(texTransform1, float4(viewPos, 1.0)).xy;

    // Sample both textures
    float4 cloudColor = tex2D(cloudSampler, texCoord0);
    float4 noiseColor = tex2D(noiseSampler, texCoord1);

    // Blend the two textures (like MODULATE in fixed-function)
//    float4 finalColor = saturate(cloudColor * noiseColor);
    float4 finalColor = cloudColor;
//finalColor.a = 0.0;
finalColor.a = cloudColor.a;
//    float4 finalColor = cloudColor * noiseColor;
//finalColor.a = diffuse.a;

    // Blend with dest framebuffer: D3DBLEND_DESTCOLOR + ZERO = modulate with dest
    // This happens outside the shader via render states

//return float4(1, 0, 0, 1); // red
    return finalColor;
//return float4(input.worldViewPos.xyz * 0.1, 1);

//float4 cloudColor22 = tex2D(cloudSampler, float2(0.3, 0.3));
//return cloudColor22;
*/

    // Sample textures
    float4 cloudColor = tex2D(cloudSampler, input.texcoord);
    float4 noiseColor = tex2D(noiseSampler, input.texcoord1);

    // Clamp to [0,1]
    float3 cloudRGB = saturate(cloudColor.rgb);
    float3 resultRGB = saturate(noiseColor.rgb * cloudRGB);

    // Alpha from constant (cb0[1].w)
    float alpha = saturate(blendAlpha.w);

    return float4(resultRGB, alpha);

}
