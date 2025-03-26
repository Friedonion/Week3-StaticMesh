
#include "StaticMeshComponent.h"
#include "core/Engine.h"
#include "Static/ResourceManager.h"
#include <filesystem>

void UStaticMeshComponent::LoadFromObj(const std::string& ObjName)
{
    // Obj 파일 로드
    namespace fs = std::filesystem;
    std::string InObjName = fs::path(ObjName).stem().string();
    
    UEngine::Get().GetObjLoader()->LoadFromFile(InObjName);

    const TArray<FSubMeshData>* meshData = UResourceManager::Get().GetMeshData(InObjName);
    if (!meshData) return;

    CurrentObjName = InObjName;
    RenderUnits.Empty();

    for (const auto& sub : *meshData)
    {
        FStaticMesh unit;
        unit.Vertices = &sub.Vertices;
        unit.Indices = &sub.Indices;
        unit.Material = UResourceManager::Get().GetMaterial(sub.MaterialName);
        unit.GUID = unit.Material ? unit.Material->GUID : 0;
        unit.ObjName = InObjName;
        RenderUnits.Add(unit);
    }
}

