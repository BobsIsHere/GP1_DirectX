//--------------------------------------------
//   Global Variables
//--------------------------------------------
float3 gCameraPosition : CAMERA;
float4x4 gWorldMatrix : WORLD;
float4x4 gWorldViewProj : WorldViewProjection;
Texture2D gDiffuseMap : DiffuseMap;
Texture2D gSpecularMap : SpecularMap;
Texture2D gGlossinessMap : GlossinessMap;
Texture2D gNormalMap : NormalMap;
SamplerState gSamplerState : Sample;

float3 gLightDirection = { 0.577f, -0.577f, 0.577f };
float gPI = 3.14159265359f;
float gLightIntensity = 7.0f;
float gShininess = 25.f;

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
    float3 Position      : POSITION;
    float2 UV            : TEXCOORD;
    float3 Normal        : NORMAL;
    float3 Tangent       : TANGENT;
};

struct VS_OUTPUT
{
    float4 Position      : SV_POSITION0;
    float4 WorldPosition : TEXCOORD;
    float2 UV            : TEXCOORD1;
    float3 Normal        : NORMAL;
    float3 Tangent       : TANGENT;
};

//--------------------------------------------
//   Vertex Shader
//--------------------------------------------
VS_OUTPUT VS(VS_INPUT input)
{
	VS_OUTPUT output = (VS_OUTPUT)0;
    output.Position = mul(float4(input.Position, 1.f), gWorldViewProj);
    output.WorldPosition = mul(float4(input.Position, 1.f), gWorldMatrix);
    output.UV = input.UV;
    output.Normal = mul(normalize(input.Normal), (float3x3) gWorldMatrix);
    output.Tangent = mul(normalize(input.Tangent), (float3x3) gWorldMatrix);
	return output;
}

//--------------------------------------------
//   PIXEL SHADING

//--------------------------------------------
//   Lambert Shader
//--------------------------------------------
float3 LambertShading(float kd, float3 cd)
{
    float3 lambertDiffuse = (cd * kd) / gPI;
    return lambertDiffuse;
}

//--------------------------------------------
//   Phong Reflection
//--------------------------------------------
float3 PhongReflection(float ks, float exponent, float3 lightVector, float3 viewVector, float3 normal)
{    
    const float3 reflection = lightVector - 2.f * (dot(normal, lightVector)) * normal;
    const float angle = dot(reflection, viewVector);
    const float phong = ks * pow(angle, exponent);
    
    return float3(phong, phong, phong);
}

//--------------------------------------------
//   Pixel Shader
//--------------------------------------------
float4 PS(VS_OUTPUT input) : SV_TARGET
{
    //variables
    const float3 invViewDirection = normalize(gCameraPosition - input.WorldPosition.xyz);
    const float4 diffuseSample = gDiffuseMap.Sample(gSamplerState, input.UV);
    const float4 normalSample = gNormalMap.Sample(gSamplerState, input.UV);
    const float4 glossinessSample = gGlossinessMap.Sample(gSamplerState, input.UV);
    const float4 specularSamlpe = gSpecularMap.Sample(gSamplerState, input.UV);
    
    //create tangent space transformation matrix
    const float3 binormal = cross(input.Normal, input.Tangent);
    const float3x3 tangentSpaceAxis = float3x3(input.Tangent, binormal, input.Normal);
    
    //sample from normal map and multiply it with matrix
    //change range [0, 1] to [-1, 1]
    const float3 normalMap = 2.f * normalSample.rgb - float3(1.f, 1.f, 1.f);
    const float3 sampledNormal = normalize(mul(normalMap, tangentSpaceAxis));
    
    float observedArea = dot(sampledNormal, normalize(gLightDirection) * -1.f);
    
    if(observedArea <= 0)
    {
        return (0.f, 0.f, 0.f, 0.f);
    }
    
    float3 lambert = LambertShading(gLightIntensity, diffuseSample.rgb);
    float3 phong = PhongReflection(specularSamlpe.r, glossinessSample.r * gShininess, gLightDirection, invViewDirection, sampledNormal);
    float3 result = (lambert + phong) * observedArea;
    
    return float4(result, 1.f);
    //return float4(phong, 1.f);
}

//--------------------------------------------

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