﻿#include "Camera.h"

#include "Core/Rendering/URenderer.h"
#include "Object/PrimitiveComponent/UPrimitiveComponent.h"


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
