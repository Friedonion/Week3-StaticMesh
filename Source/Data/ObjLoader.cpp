#include "ObjLoader.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Core/Rendering/URenderer.h"
#include "Primitive/PrimitiveVertices.h"

struct MeshGroup {
    TArray<FVertexSimple> Vertices;
    TArray<uint32_t> Indices;
    int MaterialIndex;
};

bool ObjLoader::LoadFromFile(const std::string& filename)
{
    Assimp::Importer importer;
    importer.SetPropertyInteger(AI_CONFIG_PP_RVC_FLAGS, 1);

    const aiScene* scene = importer.ReadFile(filename, 
        aiProcess_Triangulate | aiProcess_ConvertToLeftHanded);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::string errorMessage = importer.GetErrorString();
        std::cout << "Assimp 오류: " << errorMessage << std::endl;
        return false;
    }

    URenderer* Renderer = UEngine::Get().GetRenderer();
    // 메시 데이터 추출

    TArray<MeshGroup> MeshGroups;

    for (unsigned int i = 0; i < scene->mNumMeshes; i++) {
        aiMesh* mesh = scene->mMeshes[i];
        MeshGroup currentGroup;
        currentGroup.MaterialIndex = mesh->mMaterialIndex;

        // 정점 데이터 처리
        for (unsigned int j = 0; j < mesh->mNumVertices; j++) {
            FVertexSimple Vertex;
            Vertex.SetPos({ mesh->mVertices[j].x, mesh->mVertices[j].y, mesh->mVertices[j].z });
            Vertex.SetVertexColor({0.f,0.f,0.f,1.f});
            if (mesh->HasNormals()) {
                Vertex.SetNormal({ mesh->mNormals[j].x, mesh->mNormals[j].y, mesh->mNormals[j].z});
            }
            if (mesh->HasTextureCoords(0)) {
                Vertex.SetUV({ mesh->mTextureCoords[0][j].x, mesh->mTextureCoords[0][j].y });
            }
            currentGroup.Vertices.Add(Vertex);
        }

        // 인덱스 데이터 처리
        for (unsigned int j = 0; j < mesh->mNumFaces; j++) {
            aiFace face = mesh->mFaces[j];
            for (unsigned int k = 0; k < face.mNumIndices; k++) {
                currentGroup.Indices.Add(face.mIndices[k]);
            }
        }

        MeshGroups.Add(currentGroup);
    }

    // 머테리얼 정보 처리
    for (unsigned int i = 0; i < scene->mNumMaterials; i++) {
        aiMaterial* material = scene->mMaterials[i];
        // 여기서 머테리얼 정보를 추출하고 저장할 수 있습니다.
        // 예: 디퓨즈 색상, 텍스처 경로 등
    }

    // MeshGroups를 사용하여 OriginVertices와 OriginIndices 업데이트
    for (const auto& group : MeshGroups) {
    //??
        // 여기서 머테리얼 정보도 저장할 수 있습니다.
    }

    return true;
}
