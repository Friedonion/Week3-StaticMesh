#include "MtlLoader.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include "Static/ResourceManager.h"
#include "Data//MaterialData.h"
#include "Static/Util.h"

namespace fs = std::filesystem;

bool MtlLoader::LoadMtlFile(const std::string& mtlPath) {
    fs::path finalMtlPath = fs::path(mtlPath);
    if (!finalMtlPath.is_absolute()) {
        finalMtlPath = fs::absolute(fs::path("Materials") / finalMtlPath);  //절대경로인지 확인 -> 아니면 Materials 폴더 내에 경로 만들어줌
    }

    std::ifstream file(finalMtlPath);
    if (!file.is_open()) {
        std::cerr << "Failed to open MTL file: " << finalMtlPath << std::endl;
        return false;
    }

    FMaterialData currentMaterial;
    std::string line;

    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string token;
        iss >> token;
        float r, g, b;

        size_t HashToken = Hash(token);
        
        if (HashToken == Hash("newmtl")) {
            if (!currentMaterial.Name.empty()) {
                UResourceManager::Get().SetMaterial(currentMaterial.Name, currentMaterial);
            }

            std::string newName;
            iss >> newName;
            currentMaterial = FMaterialData();
            currentMaterial.Name = newName;
        }
        else if (HashToken == Hash("Ka")) {
            iss >> r >> g >> b;
            currentMaterial.AmbientColor = { r, g, b };
        }
        else if (HashToken == Hash("Kd")) {
            iss >> r >> g >> b;
            currentMaterial.DiffuseColor = { r, g, b };
        }
        else if (HashToken == Hash("Ks")) {
            iss >> r >> g >> b;
            currentMaterial.SpecularColor = { r, g, b };
        }
        else if (HashToken == Hash("Ke")) {
            iss >> r >> g >> b;
            currentMaterial.EmissiveColor = { r, g, b };
        }
        else if (HashToken == Hash("Ns"))
        {
            iss >> r;
            currentMaterial.Shininess = r;
        }
        else if (HashToken == Hash("Ni"))
        {
            iss >> r;
            currentMaterial.OpticalDensity = r;
        }
        else if (HashToken == Hash("d"))
        {
            iss >> r;
            currentMaterial.Transparency = r;
        }
        else if (HashToken == Hash("illum"))
        {
            iss >> r;
            currentMaterial.IlluminationModel = r;
        }
        else if (HashToken == Hash("map_Ka")) {
            iss >> currentMaterial.AmbientTexturePath;
        }
        else if (HashToken == Hash("map_Kd")) {
            iss >> currentMaterial.DiffuseTexturePath;
        }
        else if (HashToken == Hash("map_Ks")) {
            iss >> currentMaterial.SpecularTexturePath;
        }
        else if (HashToken == Hash("map_bump") || HashToken == Hash("bump"))
        {
            iss >> currentMaterial.BumpMap;
        }
        else if (HashToken == Hash("map_d"))
        {
            iss >> currentMaterial.AlphaMap;
        }
    }

    if (!currentMaterial.Name.empty()) {
        UResourceManager::Get().SetMaterial(currentMaterial.Name, currentMaterial);
    }

    return true;
}
