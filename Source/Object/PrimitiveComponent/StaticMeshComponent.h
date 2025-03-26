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

    void LoadFromObj(const std::string& path);
  
    TArray<FStaticMesh>& GetRenderUnits() { return RenderUnits; }

    void SetRenderUnits(const TArray<FStaticMesh>& newRenderUnits) { RenderUnits = newRenderUnits; }
    
private:
    TArray<FStaticMesh> RenderUnits;
};