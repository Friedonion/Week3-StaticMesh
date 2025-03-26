﻿// ResourceManager.cpp
#include "ResourceManager.h"
#include "directxtk/WICTextureLoader.h"

unsigned int UResourceManager::GUID = 0;

void UResourceManager::Initialize(ID3D11Device* InDevice, ID3D11DeviceContext* InContext)
{
    Device = InDevice;
    DeviceContext = InContext;
}

void UResourceManager::Shutdown()
{
    ReleaseAllTextures();
    Device = nullptr;
    DeviceContext = nullptr;
}
void UResourceManager::LoadTexture(const std::string& Path)
{
    if (TextureMap.Contains(Path))
        return; // 이미 로드됨

    int size_needed = MultiByteToWideChar(CP_UTF8, 0, &Path[0], (int)Path.size(), NULL, 0);
    std::wstring wstr(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, &Path[0], (int)Path.size(), &wstr[0], size_needed);
    const wchar_t* TexturePathWCHAR = wstr.c_str();

    ID3D11ShaderResourceView* ResourceView = nullptr;
    HRESULT hr = DirectX::CreateWICTextureFromFile(Device, DeviceContext, TexturePathWCHAR, nullptr, &ResourceView);
    if (SUCCEEDED(hr))
    {
        TextureMap[Path] = ResourceView;
    }
}

ID3D11ShaderResourceView* UResourceManager::GetTexture(const std::string& Path) const
{
    auto found = TextureMap.Find(Path);
    return found ? *found : nullptr;
}


void UResourceManager::ReleaseAllTextures()
{
    for (auto& [Path, Texture] : TextureMap)
    {
        if (Texture)
            Texture->Release();
    }
    TextureMap.Empty();
}

TMap<std::string, FMaterialData>& UResourceManager::GetMaterials()
{
    return Materials;
}

const FMaterialData* UResourceManager::GetMaterial(const std::string& name) const {
    return Materials.Find(name);
}

void UResourceManager::SetMaterial(const std::string& name, FMaterialData& materialData)
{
    if (Materials.Contains(name))
    {
        Materials[name] = materialData;
    }else
    {
        materialData.GUID = GUID++;
        Materials.Add(name, materialData);
    }
}

TMap<std::string, TArray<FSubMeshData>>& UResourceManager::GetMeshDatas()
{
    return Meshes;
}

void UResourceManager::SetMeshData(const std::string& path, const TArray<FSubMeshData>& meshData)
{
    Meshes[path] = meshData;
}

const TArray<FSubMeshData>* UResourceManager::GetMeshData(const std::string& path) const
{
    return Meshes.Find(path);
}

bool UResourceManager::HasMeshData(const std::string& path) const
{
    return Meshes.Contains(path);
}


bool UResourceManager::LoadMtlFile(const std::string& path)
{
    return MaterialLoader.LoadMtlFile(path);
}


