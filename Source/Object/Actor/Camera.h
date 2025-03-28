﻿#pragma once

#include "Core/HAL/PlatformType.h"
#include "Core/Math/Matrix.h"
#include "Core/Math/Transform.h"
#include "Core/Math/Vector.h"
#include "Object/Actor/Actor.h"
#include "Object/ObjectMacros.h"
namespace ECameraProjectionMode
{
    enum Type : uint8
    {
        Perspective,
        Orthographic
    };
}

namespace ECameraViewMode
{
    enum class Type : uint8
    {
        Front = 0,
        Back = 1,
        Top = 2,
        Bottom = 3,
        Left = 4,
        Right = 5,
        Perspective = 6,
        None = 7
    };

}

class ACamera : public AActor
{

    using Super = AActor;
	DECLARE_CLASS(ACamera, AActor)
public:
    ACamera();

private:    
    float Near;
    float Far;
    // 화면각
    float FieldOfView;
    float CameraSensitivity = 1.0f;

public:
    const float MaxYDegree = 89.8f;
    //카메라 스피드 IMGui용 나중에 Velocity로 관리하면 없어질애라 편하게 public에서 관리
    float CameraSpeed = 10.0f;
    
    // 투영 타입 - Perspective, Orthographic
    ECameraProjectionMode::Type ProjectionMode;
    ECameraViewMode::Type ViewMode;
    // float AspectRatio;	// 카메라 비율 (이번 프로젝트에서는 사용 안할듯) 

    void SetFieldOfView(float Fov);
    void SetFar(float Far);
    void SetNear(float Near);
    void SetCameraSensitivity(float sensitivity);
    void SetCameraViewMode(ECameraViewMode::Type CameraViewMode);
    float GetFieldOfView() const;
    float GetNear() const;
    float GetFar() const;
    float GetCameraSensitivity() const;

    FTransform GetSpawnTransform()
    {
        FTransform SpawnTransform = FTransform();
        SpawnTransform.SetPosition(FVector(-5, 0, 1));
        return SpawnTransform;
    }
    
    FVector GetForward() const
    {
        return GetActorRelativeTransform().GetForward();
    }
    
    FVector GetRight() const
    {
        return GetActorRelativeTransform().GetRight();
    }
    
    FVector GetUp() const
    {
        return GetActorRelativeTransform().GetUp();
    }

    FMatrix GetViewMatrix() const
    {
        return GetActorRelativeTransform().GetViewMatrix();
    }
};