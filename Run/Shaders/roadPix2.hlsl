// === Vertex Shader Output ===
struct VS_OUTPUT
{
    float4 Pos      : POSITION;
    float2 Tex0     : TEXCOORD0; // UV for road texture
    float3 WorldPos   : TEXCOORD1; // Camera-space position (used for proj)
    float4 Diffuse  : COLOR0;
};

// === Samplers ===
sampler2D RoadSampler   : register(s0); // road texture
sampler2D NoiseSampler  : register(s1); // noise overlay texture

// === Constant ===
float4x4 gTexProjMatrix: register(c8); // from CPU - result of noise texture matrix transform

// === Pixel Shader ===
float4 main(VS_OUTPUT input) : COLOR
{
    // Sample road texture
    float4 roadTex = tex2D(RoadSampler, input.Tex0);

    // Create projection UVs from camera-space position
float4 worldPos = float4(input.WorldPos, 1.0);
float2 projectedUV = mul(worldPos, gTexProjMatrix).xy;

    // Sample noise texture
    float4 noiseTex = tex2D(NoiseSampler, projectedUV);

    // Stage 0: copy alpha into roadTex.a, force RGB = 1.0 (white)
    float4 stage0 = float4(1.0, 1.0, 1.0, roadTex.a);

    // Stage 1: blend noise into current using current alpha (road alpha)
    float4 finalColor;
    finalColor.rgb = lerp(stage0.rgb, noiseTex.rgb, stage0.a); // BlendCurrentAlpha
    finalColor.a = noiseTex.a; // Optional: could be anything, not used in color blending

    return finalColor;
}
