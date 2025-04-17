// DX9 Pixel Shader - HLSL version with 4 textures
sampler2D Tex0 : register(s0); // big texture
sampler2D Tex1 : register(s1); // big texture
sampler2D Tex2 : register(s2); // big texture

// Only just have enough texture samplers for the 5 textures.
sampler2D TexA : register(s3);
sampler2D TexB : register(s4);
sampler2D TexC : register(s5);
sampler2D TexD : register(s6);
sampler2D TexE : register(s7);

struct PS_INPUT
{
    float2 TexCoord : TEXCOORD0;
    float2 TexCoord1 : TEXCOORD1;
    float4 Diffuse  : COLOR0; // Diffuse color; its alpha is used as blend factor
};

float4 SampleColor(float2 uv) {

float U = uv.x;
float V = uv.y;

bool inAU =  (U >= 0.0 && U <= 0.19091796875);
bool inAV = (V >= 0.0 && V <= 0.763671875);
bool useAltTexA = inAU && inAV;
bool inBU = (U >= 0.2109375 && U <= 0.33935546875);
bool inBV = (V >= 0.0 && V <= 0.513671875);
bool useAltTexB = inBU && inBV;
bool inCU = (U >= 0.3515625 && U <= 0.47998046875);
bool inCV = (V >= 0.0 && V <= 0.513671875);
bool useAltTexC = inCU && inCV;
bool inDU = (U >= 0.4921875 && U <= 0.55810546875);
bool inDV = (V >= 0.0 && V <= 0.263671875);
bool useAltTexD = inDU && inDV;
bool inEU = (U >= 0.5625 && U <= 0.62841796875);
bool inEV = (V>= 0.0 && V <= 0.263671875);
bool useAltTexE = inEU && inEV;

// Scale UVs if within range
if (useAltTexA)
{
    U *= 5.23785166240409;
    V *= 1.30946291560102;
}
else if (useAltTexB)
{
    U = (U-0.2109375)*7.787072243346;
   V *= 1.9467680608365;
}
else if (useAltTexC)
{
    U = (U-0.3515625)*7.787072243346;
    V *= 1.9467680608365;
}
else if (useAltTexD)
{
    U = (U-0.4921875)*15.17037037037;
    V *= 3.7925925925926;
}
else if (useAltTexE)
{
    U = (U-0.5625)*15.17037037037;
    V *= 3.7925925925926;

    U = (U - 0.0288) / 0.9488;
    V = (V - 0.0296) / 0.948;
}

float2 newUV = float2(U, V);

// Sample appropriate texture
float4 col;
if (useAltTexA)
{
    col = tex2D(TexA, newUV);
}
else if (useAltTexB)
{
    col = tex2D(TexB, newUV);
}
else if (useAltTexC)
{
    col = tex2D(TexC, newUV);
}
else if (useAltTexD)
{
    col = tex2D(TexD, newUV); 
}
else if (useAltTexE)
{
    col = col = tex2D(TexE, newUV); 
}
else
{
    col = tex2D(TexA, newUV);
}

return col;
}

float4 main(PS_INPUT input) : COLOR
{
    // Sample all four textures
//    float4 color0 = tex2D(Tex0, input.TexCoord);
//    float4 color1 = tex2D(Tex1, input.TexCoord1);
//    float4 color2 = tex2D(Tex2, input.TexCoord);

//    float4 color3 = tex2D(Tex2, input.TexCoord);

    float4 color0 = SampleColor(input.TexCoord);
    float4 color1 = SampleColor(input.TexCoord1);
    float4 color2 = SampleColor(input.TexCoord);
    
    // Perform linear interpolation between texture1 and texture0 using diffuse alpha.
    // This matches the DX8 "lrp r0, v0.a, t1, t0" instruction:
    // blended = (1 - input.Diffuse.a) * color1 + input.Diffuse.a * color0
    float4 blended = lerp(color1, color0, input.Diffuse.a);
    
    // Apply diffuse lighting
    float4 lit = blended * input.Diffuse;

    // Modulate with texture 2 and texture 3 sequentially
    float4 modulated = lit * color2;
    return modulated;
//    return float4(1.0,1.0,0.0,1.0);
//    return color3;
}
