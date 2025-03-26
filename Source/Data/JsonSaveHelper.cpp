#include "JsonSaveHelper.h"

#include <fstream>

#include "Core/EngineStatics.h"
#include "Debug/DebugConsole.h"
#include "SimpleJSON/Json.hpp"
#include "Core/Container/String.h"
#include "Object/PrimitiveComponent/StaticMeshComponent.h"

using json::JSON;

std::string JsonSaveHelper::SceneDir = "Scene/";

// SceneName - 확장자 제외
UWorldInfo* JsonSaveHelper::LoadScene(std::string SceneName)
{
    std::ifstream Input(SceneDir + SceneName + ".scene");

    if (!Input.is_open())
    {
        UE_LOG("Scene file not found");
        return nullptr;
    }
    
    std::string Contents;
    Input.seekg( 0, std::ios::end );
    Contents.reserve( Input.tellg() );
    Input.seekg( 0, std::ios::beg );

    Contents.assign( std::istreambuf_iterator<char>( Input ), std::istreambuf_iterator<char>() );

    JSON Json = JSON::Load(Contents);

    UWorldInfo* WorldInfo = new UWorldInfo();
     
    WorldInfo->Version = Json["Version"].ToInt();
    WorldInfo->SceneName = Json["SceneName"].ToString();
    WorldInfo->ActorCount = Json["ActorCount"].ToInt();
    WorldInfo->CameraCount = Json["CameraCount"].ToInt();
    WorldInfo->StaticMeshCount = Json["StaticMeshCount"].ToInt();
    WorldInfo->ObjctInfos = new UObjectInfo*[WorldInfo->ActorCount];
    WorldInfo->CameraInfos = new ACameraInfo*[WorldInfo->CameraCount];
    WorldInfo->StaticMeshInfos = new AStaticMeshInfo*[WorldInfo->StaticMeshCount];
    WorldInfo->NextUUID = Json["NextUUID"].ToInt();
    
    int ObjIndex = 0;
    int CameraIndex = 0;
    int StaticMeshIndex = 0;
    
    for (int i = 0; i < WorldInfo->NextUUID ; i++){
        if (Json["Actors"].hasKey(std::to_string(i)))
        {
            std::string ObjectUUID = std::to_string(i);
            
            UObjectInfo* ObjectInfo = new UObjectInfo();
            WorldInfo->ObjctInfos[ObjIndex] = ObjectInfo;
            
            JSON ObjLocation = Json["Actors"][ObjectUUID]["Location"];
            JSON ObjRotation = Json["Actors"][ObjectUUID]["Rotation"];
            JSON ObjScale = Json["Actors"][ObjectUUID]["Scale"];
            ObjectInfo->Location = FVector(ObjLocation[0].ToFloat(), ObjLocation[1].ToFloat(), ObjLocation[2].ToFloat());
            ObjectInfo->Rotation = FQuat(ObjRotation[0].ToFloat(), ObjRotation[1].ToFloat(), ObjRotation[2].ToFloat(), ObjRotation[3].ToFloat());
            ObjectInfo->Scale = FVector(ObjScale[0].ToFloat(), ObjScale[1].ToFloat(), ObjScale[2].ToFloat());
            ObjectInfo->ObjectType = Json["Actors"][ObjectUUID]["Type"].ToString();
            ObjectInfo->UUID = i;

            ObjIndex++;
        }
        else if (Json["Cameras"].hasKey(std::to_string(i)))
        {
            std::string CameraUUID = std::to_string(i);

            ACameraInfo* CameraInfo = new ACameraInfo();
            WorldInfo->CameraInfos[CameraIndex] = CameraInfo;

            JSON CameraLocation = Json["Cameras"][CameraUUID]["Location"];
            JSON CameraRotation = Json["Cameras"][CameraUUID]["Rotation"];
            JSON CameraScale = Json["Cameras"][CameraUUID]["Scale"];
            CameraInfo->Location = FVector(CameraLocation[0].ToFloat(), CameraLocation[1].ToFloat(), CameraLocation[2].ToFloat());
            CameraInfo->Rotation = FQuat(CameraRotation[0].ToFloat(), CameraRotation[1].ToFloat(), CameraRotation[2].ToFloat(), CameraRotation[3].ToFloat());
            CameraInfo->Scale = FVector(CameraScale[0].ToFloat(), CameraScale[1].ToFloat(), CameraScale[2].ToFloat());
            CameraInfo->FOV = Json["Cameras"][CameraUUID]["FOV"].ToFloat();
            CameraInfo->FarClip = Json["Cameras"][CameraUUID]["FarClip"].ToFloat();
            CameraInfo->NearClip = Json["Cameras"][CameraUUID]["NearClip"].ToFloat();
            CameraInfo->ObjectType = Json["Cameras"][CameraUUID]["Type"].ToString();
            CameraInfo->UUID = i;
            CameraInfo->ViewMode = static_cast<ECameraViewMode::Type>(Json["Cameras"][CameraUUID]["ViewMode"].ToInt());
            CameraInfo->IsMain = Json["Cameras"][CameraUUID]["IsMain"].ToBool();
            
            CameraIndex++;
        }
        else if (Json["StaticMeshes"].hasKey(std::to_string(i)))
        {
            std::string StaticMeshUUID = std::to_string(i);

            AStaticMeshInfo* StaticMeshInfo = new AStaticMeshInfo();
            WorldInfo->StaticMeshInfos[StaticMeshIndex] = StaticMeshInfo;

            JSON StaticMeshLocation = Json["StaticMeshes"][StaticMeshUUID]["Location"];
            JSON StaticMeshRotation = Json["StaticMeshes"][StaticMeshUUID]["Rotation"];
            JSON StaticMeshScale = Json["StaticMeshes"][StaticMeshUUID]["Scale"];
            StaticMeshInfo->Location = FVector(StaticMeshLocation[0].ToFloat(), StaticMeshLocation[1].ToFloat(), StaticMeshLocation[2].ToFloat());
            StaticMeshInfo->Rotation = FQuat(StaticMeshRotation[0].ToFloat(), StaticMeshRotation[1].ToFloat(), StaticMeshRotation[2].ToFloat(), StaticMeshRotation[3].ToFloat());
            StaticMeshInfo->Scale = FVector(StaticMeshScale[0].ToFloat(), StaticMeshScale[1].ToFloat(), StaticMeshScale[2].ToFloat());
            StaticMeshInfo->ObjectType = Json["StaticMeshes"][StaticMeshUUID]["Type"].ToString();
            StaticMeshInfo->ObjStaticMeshAsset = Json["StaticMeshes"][StaticMeshUUID]["ObjStaticMeshAsset"].ToString();
            StaticMeshInfo->UUID = i;
            
            StaticMeshIndex++;
        }
    }
    return WorldInfo;
}

void JsonSaveHelper::SaveScene(const UWorldInfo& WorldInfo, const std::string& SceneName)
{
    if (SceneName.empty())
        return;
    JSON Json;
    
    Json["Version"] = WorldInfo.Version;
    Json["ActorCount"] = WorldInfo.ActorCount;
    Json["CameraCount"] = WorldInfo.CameraCount;
    Json["StaticMeshCount"] = WorldInfo.StaticMeshCount;
    Json["SceneName"] = SceneName;
    Json["NextUUID"] = WorldInfo.NextUUID;

    for (uint32 i = 0; i < WorldInfo.ActorCount; i++)
    {
        UObjectInfo* ObjectInfo = WorldInfo.ObjctInfos[i];
        std::string ObjectUUID = std::to_string(ObjectInfo->UUID);
        
        Json["Actors"][ObjectUUID]["Location"].append(ObjectInfo->Location.X, ObjectInfo->Location.Y, ObjectInfo->Location.Z);
        Json["Actors"][ObjectUUID]["Rotation"].append(ObjectInfo->Rotation.X, ObjectInfo->Rotation.Y, ObjectInfo->Rotation.Z, ObjectInfo->Rotation.W);
        Json["Actors"][ObjectUUID]["Scale"].append(ObjectInfo->Scale.X, ObjectInfo->Scale.Y, ObjectInfo->Scale.Z);
        Json["Actors"][ObjectUUID]["Type"] = ObjectInfo->ObjectType;
    }

    for (uint32 i=0;i<WorldInfo.CameraCount;i++)
    {
        ACameraInfo* CameraInfo = WorldInfo.CameraInfos[i];
        std::string CameraUUID = std::to_string(CameraInfo->UUID);

        Json["Cameras"][CameraUUID]["Location"].append(CameraInfo->Location.X, CameraInfo->Location.Y, CameraInfo->Location.Z);
        Json["Cameras"][CameraUUID]["Rotation"].append(CameraInfo->Rotation.X, CameraInfo->Rotation.Y, CameraInfo->Rotation.Z);
        Json["Cameras"][CameraUUID]["Scale"].append(CameraInfo->Scale.X, CameraInfo->Scale.Y, CameraInfo->Scale.Z);
        Json["Cameras"][CameraUUID]["FOV"] = CameraInfo->FOV;
        Json["Cameras"][CameraUUID]["NearClip"] = CameraInfo->NearClip;
        Json["Cameras"][CameraUUID]["FarClip"] = CameraInfo->FarClip;
        Json["Cameras"][CameraUUID]["Type"] = CameraInfo->ObjectType;
        Json["Cameras"][CameraUUID]["IsMain"] = CameraInfo->IsMain;
        Json["Cameras"][CameraUUID]["ViewMode"] = static_cast<int>(CameraInfo->ViewMode);
    }

    for (uint32 i=0;i<WorldInfo.StaticMeshCount;i++)
    {
        AStaticMeshInfo* StaticMeshInfo = WorldInfo.StaticMeshInfos[i];
        std::string StaticMeshUUID = std::to_string(StaticMeshInfo->UUID);

        Json["StaticMeshes"][StaticMeshUUID]["ObjStaticMeshAsset"] = StaticMeshInfo->ObjStaticMeshAsset;
        Json["StaticMeshes"][StaticMeshUUID]["Location"].append(StaticMeshInfo->Location.X, StaticMeshInfo->Location.Y, StaticMeshInfo->Location.Z);
        Json["StaticMeshes"][StaticMeshUUID]["Rotation"].append(StaticMeshInfo->Rotation.X, StaticMeshInfo->Rotation.Y, StaticMeshInfo->Rotation.Z);
        Json["StaticMeshes"][StaticMeshUUID]["Scale"].append(StaticMeshInfo->Scale.X, StaticMeshInfo->Scale.Y, StaticMeshInfo->Scale.Z);
        Json["StaticMeshes"][StaticMeshUUID]["Type"] = StaticMeshInfo->ObjectType;
    }
    
    std::ofstream Output(SceneDir + SceneName + ".scene");
    
    if (Output.is_open())
    {
        Output << Json;
    }
}
