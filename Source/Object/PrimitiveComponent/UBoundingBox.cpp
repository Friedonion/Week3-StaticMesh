#include "UBoundingBox.h"
#include "BillBoardComponent.h"
#include "Object/PrimitiveComponent/StaticMeshComponent.h"
#include "Object/Cast.h"
#include "Static/FEditorManager.h" // 이게 없으면 include 추가

void UBoundingBoxComponent::Tick(float DeltaTime)
{
	if (!Parent)
	{
		bIsDefaultRendered = false;
		return;
	}

	PickState State = FEditorManager::Get().GetPickState();
	switch (State)
	{
	case PickState::Actor:
		if (FEditorManager::Get().GetSelectedActor() != nullptr)
		{
			bIsDefaultRendered = true;
			UpdateMinMax();
		}
		else
		{
			bIsDefaultRendered = false;
		}
		break;
	case PickState::Component:
		if (TargetPrimitive == FEditorManager::Get().GetSelectedComponent())
		{
			bIsDefaultRendered = true;
			UpdateMinMax();
		}
		else
		{
			bIsDefaultRendered = false;
		}
		break;
	default:
		bIsDefaultRendered = false;
		return;
	}
	//bIsDefaultRendered = true;
}




// 부모 component의 scale을 나눠서 구할 수 없음 -> override
FMatrix UBoundingBoxComponent::GetComponentTransformMatrix()
{
	return OverrideBoxTransform.GetMatrix();
}

void UBoundingBoxComponent::UpdateMinMax()
{
	BBox = FBox(); // 초기화
	PickState State = FEditorManager::Get().GetPickState();

	if (State == PickState::Actor)
	{
		// Actor 선택: 하위 컴포넌트 전체 순회
		AActor* Actor = FEditorManager::Get().GetSelectedActor();
		if (!Actor) return;

		for (UActorComponent* Component : Actor->GetComponents())
		{
			UPrimitiveComponent* Primitive = Cast<UPrimitiveComponent>(Component);
			if (!Primitive || Primitive->IsA(UBoundingBoxComponent::StaticClass())) continue;

			FBox TempBox;
			if (UStaticMeshComponent* Mesh = Cast<UStaticMeshComponent>(Primitive))
			{
				TempBox = FBox(Mesh->GetRenderUnits(), Mesh->GetComponentTransformMatrix());
			}
			else
			{
				TempBox = FBox(OriginVertices[Primitive->GetType()], Primitive->GetComponentTransformMatrix());
			}

			if (TempBox.IsValid())
			{
				if (!BBox.IsValid())
					BBox = TempBox;
				else
					BBox = FBox(BBox, TempBox);
			}
		}
	}
	else if (State == PickState::Component)
	{
		if (!TargetPrimitive)
			return;

		if (TargetPrimitive->IsA(UBillBoardComponent::StaticClass()))
		{
			BBox = FBox();
			return;
		}
		else if (UStaticMeshComponent* Mesh = Cast<UStaticMeshComponent>(TargetPrimitive))
		{
			BBox = FBox(Mesh->GetRenderUnits(), Mesh->GetComponentTransformMatrix());
		}
		else
		{
			BBox = FBox(OriginVertices[TargetPrimitive->GetType()], TargetPrimitive->GetComponentTransformMatrix());
		}
	}

	if (BBox.IsValid())
	{
		OverrideBoxTransform = FTransform(BBox.GetCenter(), FQuat(), BBox.GetExtent());
	}
}

