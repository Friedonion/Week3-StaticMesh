// CustomComponent.h
#pragma once
#include "TextureComponent.h"
#include "UPrimitiveComponent.h"
#include "Data/MaterialData.h"
#include "Primitive/PrimitiveVertices.h"
#include "Static/ResourceManager.h"
#include "Data/ObjManager.h"
#include "Object/ObjectMacros.h"

struct FStaticMesh {
    const TArray<FVertexPNCT>* Vertices = nullptr;
    const TArray<uint32_t>* Indices = nullptr;
    const FMaterialData* Material = nullptr;
    uint32_t GUID = 0;
    std::string ObjName;
};

class UStaticMeshComponent : public UPrimitiveComponent
{
public:
    using Super = UPrimitiveComponent;
    DECLARE_CLASS(UStaticMeshComponent, UPrimitiveComponent)

    UStaticMeshComponent() {
        bIsDefaultRendered = true;
        bCanPick = true;
        SetPixelType(EPixelType::ETexture);
    }

    virtual ~UStaticMeshComponent() = default;

    EPrimitiveType GetType() override {
        return EPrimitiveType::EPT_Custom;
    }

    void LoadFromObj(const std::string& ObjName);

    std::string& GetObjName(){return CurrentObjName;}
  
    TArray<FStaticMesh>& GetRenderUnits() { return RenderUnits; }

    void SetRenderUnits(const TArray<FStaticMesh>& newRenderUnits) { RenderUnits = newRenderUnits; } //지금은 머테리얼만 바꾸는데 사용
    
private:
    TArray<FStaticMesh> RenderUnits;
    std::string CurrentObjName;
};