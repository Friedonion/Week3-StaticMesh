#pragma once
#include "Actor.h"
#include "Object/ObjectMacros.h"
#include "Object/PrimitiveComponent/StaticMeshComponent.h"

class AStaticMesh : public AActor
{
public:
    using Super = AActor;
    DECLARE_CLASS(AStaticMesh, AActor)
public:
    AStaticMesh();
    virtual ~AStaticMesh() = default;
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    void SetObjName(const std::string& FileName);
    std::string GetObjName() {return GetComponentByClass<UStaticMeshComponent>()->GetObjName();}
private:
};
