#include "PlayerController.h"

#include <unordered_map>
#include "Object//Actor/Camera.h"
#include "PlayerInput.h"
#include "Static/FEditorManager.h"
#include "Core/Math/Plane.h"

APlayerController::APlayerController() {

}

void APlayerController::HandleCameraMovement(float DeltaTime) {

    FVector NewVelocity(0, 0, 0);

    float CameraSensitivySpeed = 10.f;
    float CameraMovementSpeed = 10.f;
    
    if (APlayerInput::Get().IsPressedMouse(true) == false)
    {
        // Camera->SetVelocity(NewVelocity);
        return;
    }

    ACamera* Camera = FEditorManager::Get().GetCamera();
    ECameraViewMode::Type cameraType = Camera->ViewMode;
    FVector MousePrePos = APlayerInput::Get().GetMousePrePos();
    FVector MousePos = APlayerInput::Get().GetMousePos();
    FVector DeltaPos = MousePos - MousePrePos;

    if (Camera->ViewMode != ECameraViewMode::Type::Perspective)
    {
        FEditorManager::Get().MoveAllOrthoCameras(cameraType, DeltaPos);
        return;
    }

    FTransform CameraTransform = Camera->GetActorRelativeTransform();

    FVector TargetRotation = CameraTransform.GetRotation().GetEuler();
    TargetRotation.Y += Camera->GetCameraSensitivity() * DeltaPos.Y * DeltaTime * CameraSensitivySpeed;
    TargetRotation.Z += Camera->GetCameraSensitivity() * DeltaPos.X * DeltaTime * CameraSensitivySpeed;
    
    TargetRotation.Y = FMath::Clamp(TargetRotation.Y, -Camera->MaxYDegree, Camera->MaxYDegree);
    CameraTransform.SetRotation(TargetRotation);

    float CamSpeed = Camera->CameraSpeed;
    
    if (APlayerInput::Get().IsPressedKey(EKeyCode::A)) {
        NewVelocity -= Camera->GetRight();
    }
    if (APlayerInput::Get().IsPressedKey(EKeyCode::D)) {
        NewVelocity += Camera->GetRight();
    }
    if (APlayerInput::Get().IsPressedKey(EKeyCode::W)) {
        NewVelocity += Camera->GetForward();
    }
    if (APlayerInput::Get().IsPressedKey(EKeyCode::S)) {
        NewVelocity -= Camera->GetForward();
    }
    if (APlayerInput::Get().IsPressedKey(EKeyCode::Q))
    {
        NewVelocity -= {0.0f, 0.0f, 1.0f};
    }
    if (APlayerInput::Get().IsPressedKey(EKeyCode::E))
    {
        NewVelocity += {0.0f, 0.0f, 1.0f};
    }
    if (NewVelocity.Length() > 0.001f)
    {
        NewVelocity = NewVelocity.GetSafeNormal();
    }

    //회전이랑 마우스클릭 구현 카메라로 해야할듯?
    CameraTransform.Translate(NewVelocity * DeltaTime * CamSpeed * CameraMovementSpeed);
    Camera->SetActorRelativeTransform(CameraTransform); //임시용
    // FCamera::Get().SetVelocity(NewVelocity);
}

void APlayerController::ProcessKeyBind(float DeltaTime)
{
    if (APlayerInput::Get().GetKeyDown(EKeyCode::Delete))
    {
        switch (FEditorManager::Get().GetPickState())
        {
        case PickState::Actor:
            if (AActor* SelectedActor = FEditorManager::Get().GetSelectedActor())
            {
                FEditorManager::Get().ReleasePick();
                SelectedActor->Destroy();
            }
            break;
        case PickState::Component:
            if (UPrimitiveComponent* SelectedComponent = FEditorManager::Get().GetSelectedComponent())
            {
                FEditorManager::Get().ReleasePick();
                AActor* ParentActor = SelectedComponent->GetOwner();
                ParentActor->RemoveComponent(SelectedComponent);
            }
            break;
        default:
            break;
        }
    }
}

void APlayerController::ProcessPlayerInput(float DeltaTime) {

    HandleCameraMovement(DeltaTime);
    ProcessKeyBind(DeltaTime);
}
