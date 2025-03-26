#pragma once

#include <string>

#include "Core/Container/String.h"
#include "Core/HAL/PlatformType.h"
#include "Core/Math/Plane.h"
#include "Core/Math/Vector.h"

namespace ECameraViewMode { enum class Type : uint8; }

struct UObjectInfo
{
    FVector Location; 
    FQuat Rotation;
    FVector Scale;
    std::string ObjectType;

    uint32 UUID;
};

struct ACameraInfo : UObjectInfo
{
    float FOV;
    float NearClip;
    float FarClip;
    ECameraViewMode::Type ViewMode;
    bool IsMain;
};

struct AStaticMeshInfo : UObjectInfo
{
    std::string ObjStaticMeshAsset;
};

struct UWorldInfo
{
    UObjectInfo** ObjctInfos;
    ACameraInfo** CameraInfos;
    AStaticMeshInfo** StaticMeshInfos;
    uint32 ActorCount;
    uint32 CameraCount;
    uint32 StaticMeshCount;
    uint32 Version;
    std::string SceneName;
    uint32 NextUUID;
};

class JsonSaveHelper
{
public:
    // SceneName - 확장자 제외
    static UWorldInfo* LoadScene(std::string SceneName);
    static void SaveScene(const UWorldInfo& WorldInfo, const std::string& SceneName);
    static std::string SceneDir;
};
