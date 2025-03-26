// ShaderW0.hlsl
Texture2D texDiffuse : register(t0);
SamplerState samLinear : register(s0);

cbuffer constants : register(b0)
{
    // matrix MVP;
    matrix Model;
    matrix View;
    matrix Projection;
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
    float3 worldPos : POSITION;
};

struct PS_OUTPUT
{
    float4 color : SV_TARGET;
};

PS_INPUT mainVS(VS_INPUT input)
{
    PS_INPUT output;
    matrix MVP = mul(mul(Model, View), Projection);
    output.position = mul(input.position, MVP);
    // 법선을 월드 공간으로 변환
    output.worldPos = mul(input.position, Model).xyz;
    output.normal = normalize(mul(input.normal, (float3x3)Model));
    output.color = input.color;
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
        // 픽셀 위치에서의 정규화된 법선 벡터
        float3 normal = normalize(input.normal);
        // 뷰 벡터 계산 (카메라 위치에서 픽셀까지)
        float3 viewDir = normalize(CameraPos - input.worldPos);
        float3 lightDir = normalize(float3(-1, -1, -1));
        float3 LightColor = float3(1, 1, 1);
        float LightIntensity = 1.0f;
        
        // 확산광(Diffuse) 계산
        float diff = max(dot(normal, lightDir), 0.0);
        float3 diffuse = diff * DiffuseColor * LightColor * LightIntensity;

        // 반사광(Specular) 계산
        float3 reflectDir = reflect(-lightDir, normal);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), Shininess);
        float3 specular = spec * SpecularColor * LightColor * LightIntensity;

        // 주변광(Ambient) 계산
        float3 ambient = AmbientColor * LightColor;

        // 최종 조명 색상 적용
        FinalColor = saturate(ambient + diffuse + specular);

        // 텍스처 샘플링 및 적용
        float4 texColor = texDiffuse.Sample(samLinear, input.uv);
        FinalColor *= texColor.rgb;

        // 방출광(Emissive) 적용
        // FinalColor += EmissiveColor * texColor.rgb;

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