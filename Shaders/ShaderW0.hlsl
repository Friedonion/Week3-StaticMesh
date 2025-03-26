// ShaderW0.hlsl
Texture2D texDiffuse : register(t0);
SamplerState samLinear : register(s0);

cbuffer constants : register(b0)
{
    matrix MVP;
    float4 CustomColor;
    int PixelType;
    float3 CameraPos;
}

cbuffer materialConstants : register(b1)
{
    float3 AmbientColor;
    float3 DiffuseColor;
    float3 SpecularColor;
    float3 EmissiveColor;

    float Shininess;
    float OpticalDensity;
    float Transparency;
    float IlluminationModel;
}

struct VS_INPUT
{
    float4 position : POSITION; // Input position from vertex buffer
    float3 normal : NORMAL;
    float4 color : COLOR;       // Input color from vertex buffer
    float2 uv : UV;
};

struct PS_INPUT
{
    float4 position : SV_POSITION; // Transformed position to pass to the pixel shader
    float3 normal : NORMAL;
    float4 color : COLOR;          // Color to pass to the pixel shader
    float2 uv : UV;
    // float4 depthPosition : TEXCOORD0;
};

struct PS_OUTPUT
{
    float4 color : SV_TARGET;
    float depth : SV_Depth;
};

PS_INPUT mainVS(VS_INPUT input)
{
    PS_INPUT output;

    output.color = input.color;
    output.position = mul(input.position, MVP);
    output.normal = input.normal;
    output.uv = input.uv;
    
    return output;
}

float4 mainPS(PS_INPUT input) : SV_TARGET
{
    float3 FinalColor = input.color;
    
    switch (PixelType)
    {
    case 1: //CustomColor
        FinalColor = CustomColor.rgb;
        break;
    case 2: //Texture
        float3 lightDir = normalize(float3(-1, -1, -1));
        float3 normal = normalize(input.normal);
        float3 viewDir = normalize(CameraPos - input.position);

        float diff = max(dot(normal, lightDir), 0.0);
        float3 diffuse = diff * DiffuseColor;

        float3 reflectDir = reflect(-lightDir, normal);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), Shininess);
        float3 specular = spec * SpecularColor;

        float3 ambient = AmbientColor;
        
        FinalColor = saturate(diffuse + specular + ambient);

        float4 texColor = texDiffuse.Sample(samLinear, input.uv);
        // FinalColor += EmissiveColor.rgb;
        FinalColor *= texColor.rgb;

        // 방출광 추가
        break;
    default:
        break;
    }
    
    // 투명도 적용 (Transparency가 1에 가까울수록 투명)
    // Direct3D는 반대라는데
    float alpha = 1.0f - Transparency;

    return float4(FinalColor, alpha);
}