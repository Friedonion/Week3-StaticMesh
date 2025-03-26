#include "Camera.h"

#include "Core/Rendering/URenderer.h"
#include "Object/PrimitiveComponent/UPrimitiveComponent.h"
#include "Static/FEditorManager.h"

ACamera::ACamera()
{
    DontDestroy();
    bCanPick = false;
    
    Near = 0.1f;
    Far = 1000.f;
    FieldOfView = 45.f;
    ProjectionMode = ECameraProjectionMode::Perspective;
    
    SetActorRelativeTransform(GetSpawnTransform());
}

void ACamera::SetFieldOfView(float Fov)
{
    FieldOfView = Fov;
}

void ACamera::SetFar(float Far)
{
    this->Far = Far;
}

void ACamera::SetNear(float Near)
{
    this->Near = Near;
}

void ACamera::SetCameraSensitivity(float sensitivity)
{
    float clampedSensitivity = FMath::Clamp(sensitivity, 0.1f, 10.0f);
    this->CameraSensitivity = clampedSensitivity;
}

void ACamera::SetCameraViewMode(ECameraViewMode::Type CameraViewMode)
{
    ViewMode = CameraViewMode;
    FQuat Rotation;
    FTransform CameraTransform = GetActorRelativeTransform();

    float d = 10.f;
    switch (CameraViewMode)
    {
    case ECameraViewMode::Type::Front:
        {
            Rotation = FQuat::AxisAngleToQuaternion(FVector(0, 0, 1), 180);
            ProjectionMode = ECameraProjectionMode::Type::Orthographic;
            CameraTransform.SetPosition(FVector(d,0,0));
            break;
        }
    case ECameraViewMode::Type::Back:
        {
            Rotation = FQuat::AxisAngleToQuaternion(FVector(0, 0, 1), 0);
            ProjectionMode = ECameraProjectionMode::Type::Orthographic;
            CameraTransform.SetPosition(FVector(-d, 0, 0));
            break;
        }
    case ECameraViewMode::Type::Top:
        {
            Rotation = FQuat::AxisAngleToQuaternion(FVector(0, 1, 0), 90);
            ProjectionMode = ECameraProjectionMode::Type::Orthographic;
            CameraTransform.SetPosition(FVector(0, 0, d));
            break;
        }
    case ECameraViewMode::Type::Bottom:
        {
            Rotation = FQuat::AxisAngleToQuaternion(FVector(0, 1, 0), -90);
            ProjectionMode = ECameraProjectionMode::Type::Orthographic;
            CameraTransform.SetPosition(FVector(0, 0, -d));
            break;
        }
    case ECameraViewMode::Type::Left:
        {
            Rotation = FQuat::AxisAngleToQuaternion(FVector(0, 0, 1), 90);
            ProjectionMode = ECameraProjectionMode::Type::Orthographic;
            CameraTransform.SetPosition(FVector(0, -d, 0));
            break;
        }
    case ECameraViewMode::Type::Right:
        {
            Rotation = FQuat::AxisAngleToQuaternion(FVector(0, 0, 1), -90);
            ProjectionMode = ECameraProjectionMode::Type::Orthographic;
            CameraTransform.SetPosition(FVector(0, d, 0));
            break;
        }
    case ECameraViewMode::Type::Perspective:
        {
            ProjectionMode = ECameraProjectionMode::Type::Perspective;
            break;
        }
    default:
        break;
    }

    ACamera* Cam = FEditorManager::Get().GetViewCamera(CameraViewMode);
    if (Cam == nullptr)
    {
        FEditorManager::Get().AddOrthoCamera(CameraViewMode, this);
    }
    
    CameraTransform.SetRotation(Rotation);
    SetActorRelativeTransform(CameraTransform);
}

float ACamera::GetFieldOfView() const
{
    return  FieldOfView;
}

float ACamera::GetNear() const
{
    return Near;
}

float ACamera::GetFar() const
{
    return Far;
}

float ACamera::GetCameraSensitivity() const
{
    return CameraSensitivity;
}
