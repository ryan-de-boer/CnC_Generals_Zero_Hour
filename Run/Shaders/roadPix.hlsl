sampler2D baseSampler : register(s0);   // texture 0
sampler2D noiseSampler : register(s1);  // texture 1 (optional)

struct PS_INPUT
{
    float4 position : POSITION;
    float4 color    : COLOR0;  // vertex diffuse color
    float2 tex0     : TEXCOORD0; // base texture UVs
    float4 tex1     : TEXCOORD1; // projected texture coords (homogeneous)
};

float4 main(PS_INPUT input) : COLOR
{
//return float4(1.0,1.0,0.0,1.0);

    // Sample base texture
    float4 baseColor = tex2D(baseSampler, input.tex0.xy);

    // Modulate with vertex diffuse color
    float4 finalColor = baseColor * input.color;

    // Optional second pass: noise modulation
    float4 noise = tex2Dproj(noiseSampler, input.tex1);
 //   float4 noise = tex2D(noiseSampler, input.tex1.xy);
    finalColor *= noise;

    return finalColor;

}
