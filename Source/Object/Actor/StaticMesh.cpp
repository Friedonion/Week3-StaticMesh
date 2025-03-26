#include "StaticMesh.h"

#include "Object/PrimitiveComponent/StaticMeshComponent.h"
#include <cstdlib>
#include <ctime>    
#include <filesystem>

AStaticMesh::AStaticMesh()
{
    bCanEverTick = true;

    UStaticMeshComponent* CustomComp = AddComponent<UStaticMeshComponent>();
    CustomComp->SetupAttachment(RootComponent);
}

void AStaticMesh::BeginPlay()
{
    SetUUIDTag();

    Super::BeginPlay();
}

void AStaticMesh::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AStaticMesh::SetObjName(const std::string& FileName)
{
    namespace fs = std::filesystem;
    std::string InObjName = fs::path(FileName).stem().string();
    
    //잘못된 설계 액터는 1개의 컴포넌트만 가지고있는거 아니니까 나중에 고쳐줘야함. 지금은 액터없는 가정으로 하는거니까 컴포넌트취급해서 땜빵용 로직
    if (UStaticMeshComponent* CustomComp = GetComponentByClass<UStaticMeshComponent>())
    {
        CustomComp->LoadFromObj(FileName);
    }
}
