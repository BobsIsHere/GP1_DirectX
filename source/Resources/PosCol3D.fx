//--------------------------------------------
//   Global Variables
//--------------------------------------------
float3 gLightDirection : LigtDirection;
float3 gCameraPosition : CameraPosition;
float4x4 gWorldMatrix : WorldMatrix;
float4x4 gWorldViewProj : WorldViewProjection;
Texture2D gDiffuseMap : DiffuseMap;
Texture2D gSpecularMap : SpecularMap;
Texture2D gGlossinessMap : GlossinessMap;
SamplerState gSamplerState : Sample;

//--------------------------------------------
//   Sample State
//--------------------------------------------
SamplerState samPoint
{
    Filter = MIN_MAG_MIP_POINT;
    AddressU = Wrap; //or Mirror, Clamp, Border
    AddressV = Wrap; //or Mirror, Clamp, Border
};

//--------------------------------------------
//   Input/Output Structs
//--------------------------------------------
struct VS_INPUT
{
	float3 Position : POSITION;
	float3 Color : COLOR;
    float2 UV : TEXCOORD;
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
};

struct VS_OUTPUT
{
    float4 Position : SV_POSITION;
	float3 Color : COLOR;
    float2 UV : TEXCOORD;
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
};

//--------------------------------------------
//   Vertex Shader
//--------------------------------------------
VS_OUTPUT VS(VS_INPUT input)
{
	VS_OUTPUT output = (VS_OUTPUT)0;
    output.Position = mul(float4(input.Position, 1.f), gWorldViewProj);
	output.Color = input.Color;
    output.UV = input.UV;
    output.Normal = input.Normal;
    output.Tangent = input.Tangent;
	return output;
}

//--------------------------------------------
//   Pixel Shader
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
		SetVertexShader( CompileShader( vs_5_0, VS() ) );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader( ps_5_0, PS() ) );
	}
}