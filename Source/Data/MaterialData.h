#pragma once
#include<string>
#include "Core/Math/Vector.h"
#include "Core/Container/Array.h"
#include "Primitive/PrimitiveVertices.h"

struct FSubMeshData {
    TArray<FVertexPNCT> Vertices;
    TArray<uint32_t> Indices;
    std::string MaterialName;
};

struct FMaterialData {
    std::string Name;

    FVector AmbientColor = { 0.f,0.f,0.f };
    FVector DiffuseColor = { 1.f,1.f,1.f };
    FVector SpecularColor = { 0.f,0.f,0.f };
    FVector EmissiveColor = { 0.f,0.f,0.f };

    float Shininess = 32.f;
    float OpticalDensity = 1.f;
    float Transparency = 0.f;
    float IlluminationModel = 2.f;
    
    std::string AmbientTexturePath = "";
    std::string DiffuseTexturePath = "";
    std::string SpecularTexturePath = "";

    std::string BumpMap = "";
    std::string AlphaMap = "";
};