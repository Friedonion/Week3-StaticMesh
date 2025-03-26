#include "World.h"
#include <cassert>
#include "Data/JsonSavehelper.h"

#include "Core/Container/Map.h"
#include "Core/Input/PlayerInput.h"
#include "Object/Actor/Camera.h"
#include <Object/Gizmo/GizmoHandle.h>

#include "Object/Actor/Cone.h"
#include "Object/Actor/Cube.h"
#include "Object/Actor/Cylinder.h"
#include "Object/Actor/Sphere.h"
#include "Static/FEditorManager.h"
#include "Object/PrimitiveComponent/TextComponent.h"
#include <Object/PrimitiveComponent/BillBoardComponent.h>

#include "Object/Actor/Texture.h"
#include "Object/Cast.h"
#include "Debug/DebugConsole.h"
#include "Object/TObjectIterator.h"
#include "Object/Actor/StaticMesh.h"
#include "Object/PrimitiveComponent/StaticMeshComponent.h"
#include "Object/PrimitiveComponent/TextureComponent.h"

void UWorld::BeginPlay()
{
	for (const auto& Actor : ActorsToSpawn)
	{
		Actor->BeginPlay();
	}
	ActorsToSpawn.Empty();
}

void UWorld::Tick(float DeltaTime)
{
	for (const auto& Actor : ActorsToSpawn)
	{
		Actor->BeginPlay();
	}
	ActorsToSpawn.Empty();

	const auto CopyActors = Actors;
	for (const auto& Actor : CopyActors)
	{
		if (Actor->CanEverTick())
		{
			Actor->Tick(DeltaTime);
		}
	}
}

void UWorld::LateTick(float DeltaTime)
{
	const auto CopyActors = Actors;
	for (const auto& Actor : CopyActors)
	{
		if (Actor->CanEverTick())
		{
			Actor->LateTick(DeltaTime);
		}
	}

	for (const auto& PendingActor : PendingDestroyActors)
	{
		// Engine에서 제거
		GObjects.Remove(PendingActor->GetUUID());
	}
	PendingDestroyActors.Empty();
}

void UWorld::Render(float DeltaTime)
{
	URenderer* Renderer = UEngine::Get().GetRenderer();

	if (Renderer == nullptr)
	{
		return;
	}


	float initialHeight = UEngine::Get().GetScreenHeight();
	float initialWidth = UEngine::Get().GetScreenWidth();
	TMap<EViewport::Position, FViewport*> activeViewport = Renderer->GetActiveViewport();

	if (Renderer->activeFullViewport)
	{
		Renderer->activeFullViewport->SetViewportRendering();
		Renderer->PrepareShader();
		RenderMainTexture(*Renderer, DeltaTime);
	}
	else
	{
		for (auto& pair : activeViewport)
		{
 			pair.Value->SetViewportRendering();
			Renderer->PrepareShader();
			RenderMainTexture(*Renderer, DeltaTime);
		}

	}
	UEngine::Get().SetScreenHeight(initialHeight);
	UEngine::Get().SetScreenWidth(initialWidth);

}

void UWorld::RenderPickingTexture(URenderer& Renderer)
{
	// Renderer.PreparePicking();
	// Renderer.PreparePickingShader();

	
	// for (auto& RenderComponent : RenderComponents)
	// {
	// 	if (RenderComponent->GetOwner()->GetDepth() > 0)
	// 	{
	// 		continue;
	// 	}
	// 	uint32 UUID = RenderComponent->GetUUID();
	// 	RenderComponent->UpdateConstantPicking(Renderer, APicker::EncodeUUID(UUID));
	// 	RenderComponent->Render();
	// }
	//
	// Renderer.PrepareZIgnore();
	// for (auto& RenderComponent: ZIgnoreRenderComponents)
	// {
	// 	uint32 UUID = RenderComponent->GetUUID();
	// 	RenderComponent->UpdateConstantPicking(Renderer, APicker::EncodeUUID(UUID));
	// 	uint32 depth = RenderComponent->GetOwner()->GetDepth();
	// 	RenderComponent->Render();
	// }
}

void UWorld::RenderMainTexture(URenderer& Renderer, float DeltaTime)
{
	Renderer.PrepareMain();

	Renderer.RenderBatch();

//prepare texture하고 loadtexture 하고 텍스쳐렌더 돌면서

	Renderer.PrepareMainShader();
	
	for (auto& RenderComponent : RenderComponents)
	{
		if (!FEditorManager::Get().IsShowFlagSet((EEngineShowFlags::SF_Primitives)))
		{
			if (RenderComponent->IsCanPick()) continue;
		}
		{
			//분기해주고 로드텍스쳐까지
			if (RenderComponent->IsA(UTextureComponent::StaticClass()))
			{
				Renderer.PrepareTextureResource(RenderComponent->GetTextureResource());
			}
			if (RenderComponent->IsA(UStaticMeshComponent::StaticClass()))
			{
				Renderer.PrepareTextureResource(RenderComponent->GetTextureResource());
			}
			RenderComponent->Render();
		}
	}

	Renderer.PrepareZIgnore();
	for (auto& RenderComponent: ZIgnoreRenderComponents)
	{
		uint32 depth = RenderComponent->GetOwner()->GetDepth();
		RenderComponent->Render();
	}

	for (auto& RenderComponent : TextRenderComponents)
	{
		if (!FEditorManager::Get().IsShowFlagSet(EEngineShowFlags::SF_BillboardText))
			continue;

		if (UTextComponent* TextComponent = dynamic_cast<UTextComponent*>(RenderComponent))
			{
				TextComponent->RenderText(Renderer, TextComponent->GetText(),
				TextComponent->GetComponentTransformMatrix().GetTranslation(),
				TextComponent->GetTextSize());
			}
	}
	
	Renderer.RenderParticle(DeltaTime);

	for (auto& RenderComponent : BillBoardRenderComponents)
	{
		UBillBoardComponent* BillBoardComponent = Cast<UBillBoardComponent>(RenderComponent);
		Renderer.RenderTexture(BillBoardComponent->GetComponentTransformMatrix().GetTranslation());
	}

}

void UWorld::DisplayPickingTexture(URenderer& Renderer)
{
	Renderer.RenderPickingTexture();
}

void UWorld::ClearWorld()
{
	TArray CopyActors = Actors;
	for (AActor* Actor : CopyActors)
	{
		if (Actor->IsDontDestroy() == false)
		{
			DestroyActor(Actor);
		}
	}

	ACamera* Camera = FEditorManager::Get().GetCamera();
	if (Camera)
	{
		Camera->SetActorRelativeTransform(Camera->GetSpawnTransform());
	}

	UE_LOG("Clear World");
}


bool UWorld::DestroyActor(AActor* InActor)
{
	// 나중에 Destroy가 실패할 일이 있다면 return false; 하기
	assert(InActor);

	if (PendingDestroyActors.Find(InActor) != -1)
	{
		return true;
	}

	// 삭제될 때 Destroyed 호출
	InActor->Destroyed();
	
	// World에서 제거
	Actors.Remove(InActor);

	//Render할 목록에서 제거
	// UEngine::Get().GetRenderer()->ReleaseVertexBuffer(InActor->GetUUID());
	
	// 제거 대기열에 추가
	PendingDestroyActors.Add(InActor);
	return true;
}

void UWorld::SaveWorld(const std::string& SceneName)
{
	const UWorldInfo& WorldInfo = GetWorldInfo();
	JsonSaveHelper::SaveScene(WorldInfo, SceneName);
}

void UWorld::AddZIgnoreComponent(UPrimitiveComponent* InComponent)
{
	ZIgnoreRenderComponents.Add(InComponent);
	InComponent->SetIsOrthoGraphic(true);
}

void UWorld::AddTextComponent(UPrimitiveComponent* InComponent)
{
	TextRenderComponents.Add(InComponent);
}

void UWorld::AddBillBoardComponent(UPrimitiveComponent* InComponent)
{
	BillBoardRenderComponents.Add(InComponent);
}

void UWorld::LoadWorld(const char* SceneName)
{
	if (SceneName == nullptr || strcmp(SceneName, "") == 0){
		return;
	}

	if (ActorFactoryMap.empty())
	{
		ActorFactoryMap = {
			{ "AActor",    [this]() { return SpawnActor<AActor>(); } },
			{ "ASphere",   [this]() { return SpawnActor<ASphere>(); } },
			{ "ACube",     [this]() { return SpawnActor<ACube>(); } },
			{ "AArrow",    [this]() { return SpawnActor<AArrow>(); } },
			{ "ACylinder", [this]() { return SpawnActor<ACylinder>(); } },
			{ "ACone",     [this]() { return SpawnActor<ACone>(); } },
			{ "ATexture",  [this]() { return SpawnActor<ATexture>(); } },
		};
	}
	
	UWorldInfo* WorldInfo = JsonSaveHelper::LoadScene(SceneName);
	if (WorldInfo == nullptr) return;

	ClearWorld();

	Version = WorldInfo->Version;
	this->SceneName = WorldInfo->SceneName;
	uint32 ActorCount = WorldInfo->ActorCount;
	uint32 CameraCount = WorldInfo->CameraCount;
	uint32 StaticMeshCount = WorldInfo->StaticMeshCount;

	// Type 확인
	for (uint32 i = 0; i < ActorCount; i++)
	{
		UObjectInfo* ObjectInfo = WorldInfo->ObjctInfos[i];
		FTransform Transform = FTransform(ObjectInfo->Location, ObjectInfo->Rotation, ObjectInfo->Scale);
		
		if (ActorFactoryMap.contains(ObjectInfo->ObjectType))
		{
			AActor* Actor = ActorFactoryMap[ObjectInfo->ObjectType]();
			Actor->SetActorRelativeTransform(Transform);
			Actor->SetUUID(ObjectInfo->UUID);
		}
	}

	for (uint32 i=0;i<CameraCount;i++)
	{
		ACameraInfo* CameraInfo = WorldInfo->CameraInfos[i];
		FTransform Transform = FTransform(CameraInfo->Location, CameraInfo->Rotation, CameraInfo->Scale);
		float CameraFOV = CameraInfo->FOV;
		float CameraNearClip = CameraInfo->NearClip;
		float CameraFarClip = CameraInfo->FarClip;

		ACamera* Camera = FEditorManager::Get().GetViewCamera(CameraInfo->ViewMode);
		if (Camera == nullptr)
		{
			Camera = SpawnActor<ACamera>();
		}
		
		if (CameraInfo->IsMain)
		{
			FEditorManager::Get().SetCamera(Camera);
		}

		Camera->SetActorRelativeTransform(Transform);
		Camera->SetFieldOfView(CameraFOV);
		Camera->SetNear(CameraNearClip);
		Camera->SetFar(CameraFarClip);
		Camera->SetUUID(CameraInfo->UUID);
		Camera->SetCameraViewMode(CameraInfo->ViewMode);
		FEditorManager::Get().SetViewCamera(CameraInfo->ViewMode, Camera);
	}
	
	// if (ObjectInfo->ObjectType == "ACamera")
	// {
	// 	ACamera* Camera = FEditorManager::Get().GetCamera();
	// 	if (Camera == nullptr) //지금 없으면 스폰 있으면 스폰할필요 없음
	// 	{
	// 		Camera = SpawnActor<ACamera>();
	// 		FEditorManager::Get().SetCamera(Camera);
	// 	}
	// 	Actor = Camera;
	// }
	
	for (uint32 i=0;i<StaticMeshCount;i++)
	{
		AStaticMeshInfo* StaticMeshInfo = WorldInfo->StaticMeshInfos[i];
		FTransform Transform = FTransform(StaticMeshInfo->Location, StaticMeshInfo->Rotation, StaticMeshInfo->Scale);
		std::string ObjStaticMeshAsset = StaticMeshInfo->ObjStaticMeshAsset;

		AStaticMesh* StaticMesh = SpawnActor<AStaticMesh>();
		StaticMesh->SetActorRelativeTransform(Transform);
		StaticMesh->SetObjName(ObjStaticMeshAsset);
		StaticMesh->SetUUID(StaticMeshInfo->UUID);
	}
	
	UEngineStatics::NextUUID = WorldInfo->NextUUID;
}

UWorldInfo UWorld::GetWorldInfo() const
{
	UWorldInfo WorldInfo;
	WorldInfo.ActorCount = Actors.Num();
	WorldInfo.ObjctInfos = new UObjectInfo*[WorldInfo.ActorCount];
	WorldInfo.SceneName = *SceneName;
	WorldInfo.Version = 1;
	WorldInfo.NextUUID = UEngineStatics::NextUUID;
	
	uint32 CameraCount = 0;
	uint32 StaticMeshCount = 0;
	
	for (auto& Actor : Actors)
	{
		if (Actor->IsA(ACamera::StaticClass()))
		{
			CameraCount++;
		}
		if (Actor->IsA(AStaticMesh::StaticClass()))
		{
			StaticMeshCount++;
		}
	}

	WorldInfo.CameraCount = CameraCount;
	WorldInfo.StaticMeshCount = StaticMeshCount;
	
	WorldInfo.CameraInfos = new ACameraInfo*[CameraCount];
	WorldInfo.StaticMeshInfos = new AStaticMeshInfo*[StaticMeshCount];
	
	uint32 ObjIndex = 0;
	uint32 CameraIndex = 0;
	uint32 StaticMeshIndex = 0;
	
	for (auto& Actor : Actors)
	{
		if (Actor->IsDontDestroy()) //부수면 안되는애들이면
		{
			if (!Actor->IsA(ACamera::StaticClass())) //카메라도 부수면 안되는애여서 빼고
			{
				WorldInfo.ActorCount--;
				continue;
			}
		}

		if (Actor->IsA(ACamera::StaticClass()))//카메라면
		{
			ACamera* Cam = dynamic_cast<ACamera*>(Actor);
			FTransform CamTransform = Cam->GetActorRelativeTransform();
			
			WorldInfo.CameraInfos[CameraIndex] = new ACameraInfo();
			WorldInfo.CameraInfos[CameraIndex]->Location = CamTransform.GetPosition();
			WorldInfo.CameraInfos[CameraIndex]->Rotation = CamTransform.GetRotation();
			WorldInfo.CameraInfos[CameraIndex]->Scale = CamTransform.GetScale();
			WorldInfo.CameraInfos[CameraIndex]->FOV = Cam->GetFieldOfView();
			WorldInfo.CameraInfos[CameraIndex]->FarClip = Cam->GetFar();
			WorldInfo.CameraInfos[CameraIndex]->NearClip = Cam->GetNear();
			WorldInfo.CameraInfos[CameraIndex]->UUID = Cam->GetUUID();
			WorldInfo.CameraInfos[CameraIndex]->ObjectType = Actor->GetClass() ? *(Cam->GetClass()->GetName().GetString()) : "nullptr";
			WorldInfo.CameraInfos[CameraIndex]->IsMain = FEditorManager::Get().GetCamera() == Cam;
			WorldInfo.CameraInfos[CameraIndex]->ViewMode = Cam->ViewMode;

			WorldInfo.ActorCount--;
			CameraIndex++;
			continue;
		}

		if (Actor->IsA(AStaticMesh::StaticClass())) //스태틱메시면
		{
			AStaticMesh* StaticMesh = dynamic_cast<AStaticMesh*>(Actor);
			FTransform MeshTransform = StaticMesh->GetActorRelativeTransform();

			WorldInfo.StaticMeshInfos[StaticMeshIndex] = new AStaticMeshInfo();
			WorldInfo.StaticMeshInfos[StaticMeshIndex]->Location = MeshTransform.GetPosition();
			WorldInfo.StaticMeshInfos[StaticMeshIndex]->Rotation = MeshTransform.GetRotation();
			WorldInfo.StaticMeshInfos[StaticMeshIndex]->Scale = MeshTransform.GetScale();
			WorldInfo.StaticMeshInfos[StaticMeshIndex]->ObjStaticMeshAsset = StaticMesh->GetObjName();
			WorldInfo.StaticMeshInfos[StaticMeshIndex]->ObjectType = Actor->GetClass() ? *(StaticMesh->GetClass()->GetName().GetString()) : "nullptr";
			WorldInfo.StaticMeshInfos[StaticMeshIndex]->UUID = StaticMesh->GetUUID();

			WorldInfo.ActorCount--;
			StaticMeshIndex++;
			continue;
		}
		
		WorldInfo.ObjctInfos[ObjIndex] = new UObjectInfo();
		const FTransform& Transform = Actor->GetActorRelativeTransform();
		WorldInfo.ObjctInfos[ObjIndex]->Location = Transform.GetPosition();
		WorldInfo.ObjctInfos[ObjIndex]->Rotation = Transform.GetRotation();
		WorldInfo.ObjctInfos[ObjIndex]->Scale = Transform.GetScale();
		WorldInfo.ObjctInfos[ObjIndex]->ObjectType = Actor->GetClass() ? *(Actor->GetClass()->GetName().GetString()) : "nullptr";

		WorldInfo.ObjctInfos[ObjIndex]->UUID = Actor->GetUUID();
		ObjIndex++;
	}
	return WorldInfo;
}