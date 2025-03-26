
#include "StaticMeshComponent.h"
#include "core/Engine.h"
#include "Static/ResourceManager.h"

void UStaticMeshComponent::LoadFromObj(const std::string& ObjName)
{
    // Obj 파일 로드
    UEngine::Get().GetObjLoader()->LoadFromFile(ObjName);
    const TArray<FSubMeshData>* meshData = UResourceManager::Get().GetMeshData(ObjName);
    if (!meshData) return;

    CurrentObjName = ObjName;
    RenderUnits.Empty();

    for (const auto& sub : *meshData)
    {
        FStaticMesh unit;
        unit.Vertices = &sub.Vertices;
        unit.Indices = &sub.Indices;
        unit.Material = UResourceManager::Get().GetMaterial(sub.MaterialName);
        unit.GUID = unit.Material ? unit.Material->GUID : 0;
        unit.ObjName = ObjName;
        RenderUnits.Add(unit);
    }
}

