
#include "StaticMeshComponent.h"
#include "core/Engine.h"
#include "Static/ResourceManager.h"

void UStaticMeshComponent::LoadFromObj(const std::string& path)
{
    // Obj 파일 로드
    UEngine::Get().GetObjLoader()->LoadFromFile(path);
    const TArray<FSubMeshData>* meshData = UResourceManager::Get().GetMeshData(path);
    if (!meshData) return;

    RenderUnits.Empty();

    for (const auto& sub : *meshData)
    {
        FStaticMesh unit;
        unit.Vertices = &sub.Vertices;
        unit.Indices = &sub.Indices;
        unit.Material = UResourceManager::Get().GetMaterial(sub.MaterialName);
        unit.GUID = unit.Material ? unit.Material->GUID : 0;
        RenderUnits.Add(unit);
    }
}

