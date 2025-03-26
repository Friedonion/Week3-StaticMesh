#pragma once
#include "Actor.h"
#include "Object/ObjectMacros.h"

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
    std::string GetObjName() {return ObjName;} //이거 컴포넌트 돌면서 이름 종합해서 가져와야함 아니면 액터가 컴포넌트 목록 갖고있어야함
    
private:
    std::string ObjName;
};
