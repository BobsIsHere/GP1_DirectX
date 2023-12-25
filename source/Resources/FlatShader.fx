//--------------------------------------------
//   Global Variables
//--------------------------------------------
float3 gCameraPosition : CAMERA;
float4x4 gWorldMatrix : WORLD;
float4x4 gWorldViewProj : WorldViewProjection;
Texture2D gDiffuseMap : DiffuseMap;
SamplerState gSamplerState : Sample;

//--------------------------------------------
//   Input/Output Structs
//--------------------------------------------
struct VS_INPUT
{
    float3 Position : POSITION;
    float2 UV : TEXCOORD;
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
};

struct VS_OUTPUT
{
    float4 Position : SV_POSITION0;
    float4 WorldPosition : TEXCOORD;
    float2 UV : TEXCOORD1;
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
};

//--------------------------------------------
//   Vertex Shader
//--------------------------------------------
VS_OUTPUT VS(VS_INPUT input)
{
    VS_OUTPUT output = (VS_OUTPUT) 0;
    output.Position = mul(float4(input.Position, 1.f), gWorldViewProj);
    output.WorldPosition = mul(float4(input.Position, 1.f), gWorldMatrix);
    output.UV = input.UV;
    output.Normal = mul(normalize(input.Normal), (float3x3) gWorldMatrix);
    output.Tangent = mul(normalize(input.Tangent), (float3x3) gWorldMatrix);
    return output;
}

//--------------------------------------------
//   Pixel Shading
//--------------------------------------------
float4 PS(VS_OUTPUT input) : SV_TARGET
{
    return gDiffuseMap.Sample(gSamplerState, input.UV);
}

//--------------------------------------------
//   Technique
//--------------------------------------------
technique11 DefaultTechnique
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS()));
    }
}