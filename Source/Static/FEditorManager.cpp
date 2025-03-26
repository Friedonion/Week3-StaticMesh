#include "FEditorManager.h"
#include "Core/Engine.h"
#include "Object/World/World.h"
#include "Object/Gizmo/GizmoHandle.h"
#include "Core/Math/Vector.h"
#include "Core/Math/Transform.h"
#include "Object/Actor/Camera.h"

void FEditorManager::SelectComponent(UPrimitiveComponent* NewComponent)
{
    if (GizmoHandle == nullptr) //기즈모 띄우는거면 생성
    {
		GizmoHandle = UEngine::Get().GetWorld()->SpawnActor<AGizmoHandle>();
        GizmoHandle->SetActive(false);
    }

	if (NewComponent == nullptr || NewComponent == SelectedComponent) //이미 선택헀던 애면 선택 없애기
	{
		ReleasePick();
		return;
	}

	AActor* NewActor = NewComponent->GetOwner();

	switch (CurrentPickState)
	{
		case PickState::None:
			SetPickState(PickState::Actor);
			break;
		case PickState::Actor:
			if (SelectedActor == NewActor)
			{
				SetPickState(PickState::Component);
			}
			break;
		case PickState::Component:
			if (SelectedComponent == NewComponent)
			{
				SetPickState(PickState::None);
			}
			else if (SelectedActor != NewActor)
			{
				SetPickState(PickState::Actor);
			}
			break;
		default:
			break;
	}

	switch (CurrentPickState)
	{
		case PickState::None:
			ReleasePick();
			return;
		case PickState::Actor:
			//GizmoHandle->SetActive(true);
			SelectedActor = NewActor;
			NewActor->Pick();
			GizmoHandle->SetActive(true);
			if (SelectedComponent)
			{
				SelectedComponent->Pick(true);
				SelectedComponent = NewComponent;
			}
			break;
		case PickState::Component:
			GizmoHandle->SetActive(true);
			SelectedComponent = NewComponent;
			NewComponent->Pick(true);
			break;
	default:
		break;
	}
}

void FEditorManager::ReleasePick()
{
	SetPickState(PickState::None);
	
	if (SelectedActor)
	{
		SelectedActor->UnPick();
		SelectedActor = nullptr;
	}
	if (SelectedComponent)
	{
		SelectedComponent->Pick(false);
		SelectedComponent = nullptr;
	}
	if (GizmoHandle)
	{
		GizmoHandle->SetActive(false);
	}
}

void FEditorManager::SelectActor(AActor* NewActor)
{
	if (GizmoHandle == nullptr) //기즈모 띄우는거면 생성
	{
		GizmoHandle = UEngine::Get().GetWorld()->SpawnActor<AGizmoHandle>();
		GizmoHandle->SetActive(false);
	}

	if (NewActor == nullptr || NewActor == SelectedActor) //이미 선택헀던 애면 선택 없애기
	{
		ReleasePick();
		return;
	}

	SetPickState(PickState::Actor);

	SelectedActor = NewActor;
	NewActor->Pick();
	GizmoHandle->SetActive(true);
}

void FEditorManager::SetCamera(ACamera* NewCamera)
{
    Camera = NewCamera;
}

void FEditorManager::SetMainCameraByType(ECameraViewMode::Type type)
{
	Camera = OrthogonalCamera[type];
}

ACamera* FEditorManager::GetOrthoCamera(ECameraViewMode::Type type)
{
	return OrthogonalCamera[type];
}

void FEditorManager::AddOrthoCamera(ECameraViewMode::Type type, ACamera* NewMultiCamera)
{
	OrthogonalCamera.Add(type, NewMultiCamera);
}

void FEditorManager::MoveAllOrthoCameras(ECameraViewMode::Type cameraType, FVector MouseDeltaPos)
{
	if (cameraType == ECameraViewMode::Type::Perspective)
	{
		return;
	}
	FVector deltaMove(0,0,0);
	float x = MouseDeltaPos.X*0.01f;
	float y = MouseDeltaPos.Y*0.01f;
	switch (cameraType)
	{
	case ECameraViewMode::Type::Front:
		deltaMove.Z += y;
		deltaMove.Y += -x;
		break;
	case ECameraViewMode::Type::Back:
		deltaMove.Z += y;
		deltaMove.Y += x;
		break;
	case ECameraViewMode::Type::Top:
		deltaMove.X += y;
		deltaMove.Y += x;
		break;
	case ECameraViewMode::Type::Bottom:
		deltaMove.X += -y;
		deltaMove.Y += x;
		break;
	case ECameraViewMode::Type::Left:
		deltaMove.Z += y;
		deltaMove.X += -x;
		break;
	case ECameraViewMode::Type::Right:
		deltaMove.Z += y;
		deltaMove.X += x;
		break;
	default:
		break;
	}

	for (int i = static_cast<int>(ECameraViewMode::Type::Front); 
		i < static_cast<int>(ECameraViewMode::Type::Perspective); ++i)
	{
		ECameraViewMode::Type mode = static_cast<ECameraViewMode::Type>(i);
		MoveOrthoCamera(mode, deltaMove);
	}

}

void FEditorManager::MoveOrthoCamera(ECameraViewMode::Type cameraType, FVector deltaMove)
{
	ACamera* camera = OrthogonalCamera[cameraType];
	FTransform cameraTransform = camera->GetActorRelativeTransform();
	cameraTransform.Translate(deltaMove);
	camera->SetActorRelativeTransform(cameraTransform);

}

ACamera* FEditorManager::GetCameraByViewMode(ECameraViewMode::Type cameraType)
{
	return OrthogonalCamera[cameraType];
}

void FEditorManager::SavePerspectiveCamera()
{
	ACamera* perspectiveCam = OrthogonalCamera[ECameraViewMode::Type::Perspective];

	json::JSON jsonArray = json::Array();

	json::JSON j;
	j["Near"] = perspectiveCam->GetNear();
	j["Far"] = perspectiveCam->GetFar();
	j["FOV"] = perspectiveCam->GetFieldOfView();
	j["Sensitivity"] = perspectiveCam->GetCameraSensitivity();

	FTransform transform = perspectiveCam->GetActorRelativeTransform();
	j["LocX"] = transform.GetPosition().X;
	j["LocY"] = transform.GetPosition().Y;
	j["LocZ"] = transform.GetPosition().Z;

	FVector rot = transform.GetRotation().GetEuler();
	j["RotX"] = rot.X;
	j["RotY"] = rot.Y;
	j["RotZ"] = rot.Z;

	std::string s = j.ToString();
	jsonArray.append(j);

	std::ofstream file("CameraSettings.ini");
	if (file.is_open())
	{
		file << jsonArray.dump(4); // 또는 file << jsonStr;
		file.close();
	}

}

void FEditorManager::LoadPerspectiveCamera()
{
	ACamera* perspectiveCam = OrthogonalCamera[ECameraViewMode::Type::Perspective];
	
	std::ifstream inFile("CameraSettings.ini");
	std::stringstream ss;
	ss << inFile.rdbuf();
	json::JSON layout = json::JSON::Load(ss.str());

	float Near;
	float Far;
	float FOV;
	float CamSens;

	float LocX;
	float LocY;
	float LocZ;

	float RotX;
	float RotY;
	float RotZ;

	for (const auto& item : layout.ArrayRange())
	{
		Near = item.at("Near").ToFloat();
		Far = item.at("Far").ToFloat();
		FOV = item.at("FOV").ToFloat();
		CamSens = item.at("Sensitivity").ToFloat();

		LocX = item.at("LocX").ToFloat();
		LocY = item.at("LocY").ToFloat();
		LocZ = item.at("LocZ").ToFloat();

		RotX = item.at("RotX").ToFloat();
		RotY = item.at("RotY").ToFloat();
		RotZ = item.at("RotZ").ToFloat();
	}

	perspectiveCam->SetNear(Near);
	perspectiveCam->SetFar(Far);
	perspectiveCam->SetFieldOfView(FOV);
	perspectiveCam->SetCameraSensitivity(CamSens);

	FTransform transform;
	transform.SetPosition(LocX, LocY, LocZ);
	transform.SetRotation(RotX, RotY, RotZ);
	perspectiveCam->SetActorRelativeTransform(transform);
}
