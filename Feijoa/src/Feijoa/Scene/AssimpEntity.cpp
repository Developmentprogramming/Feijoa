#include "fjpch.h"
#include "AssimpEntity.h"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <glm/gtc/matrix_transform.hpp>

namespace Feijoa
{
	AssimpEntity::AssimpEntity(const std::string& path)
	{
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);
		FJ_CORE_ASSERT(scene || scene->mRootNode || ((scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) != AI_SCENE_FLAGS_INCOMPLETE), "Failed to load model!");

		m_Directory = path.substr(0, path.find_last_of("/"));
		ProcessNode(scene->mRootNode, scene);
	}

	void AssimpEntity::ProcessNode(aiNode* node, const aiScene* scene)
	{
		for (uint32_t i = 0; i < node->mNumMeshes; i++)
		{
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			m_Meshes.push_back(ProcessMesh(mesh, scene));
		}

		for (uint32_t i = 0; i < node->mNumChildren; i++)
			ProcessNode(node->mChildren[i], scene);
	}

	RenderMesh AssimpEntity::ProcessMesh(aiMesh* mesh, const aiScene* scene)
	{
		aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

		bool hasTexture = mesh->mTextureCoords[0] != nullptr;

		glm::vec4 ambient = GetMaterialColor(material, AI_MATKEY_COLOR_AMBIENT);
		glm::vec4 diffuse = GetMaterialColor(material, AI_MATKEY_COLOR_DIFFUSE);
		glm::vec4 specular = GetMaterialColor(material, AI_MATKEY_COLOR_SPECULAR);
		glm::vec4 color = ambient + diffuse + specular;
		if (color == glm::vec4(0.0f))
			color = glm::vec4(1.0f);

		std::vector<ModelVertex> vertices(mesh->mNumVertices);
		for (uint32_t i = 0; i < mesh->mNumVertices; i++)
		{
			vertices[i] = { { mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z }, glm::mat4(1.0f), color };
			if (hasTexture)
				vertices[i].TexCoord = glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
		}
		
		std::vector<uint32_t> indices;
		for (uint32_t i = 0; i < mesh->mNumFaces; i++)
		{
			aiFace face = mesh->mFaces[i];
			for (uint32_t j = 0; j < face.mNumIndices; j++)
				indices.push_back(face.mIndices[j]);
		}

		if (hasTexture)
			return RenderMesh(vertices, indices, LoadTextures(material, aiTextureType_DIFFUSE));

		return RenderMesh(vertices, indices);
	}

	std::vector<Ref<Texture2D>> AssimpEntity::LoadTextures(const aiMaterial* material, aiTextureType type)
	{
		std::vector<Ref<Texture2D>> textures;

		for (uint32_t i = 0; i < material->GetTextureCount(type); i++)
		{
			aiString str;
			material->GetTexture(type, i, &str);

			// Prevent duplicate loading
			bool skip = false;
			for (uint32_t j = 0; j < m_Textures.size(); j++)
			{
				if (strcmp(m_Textures[i]->GetPath().data(), str.C_Str()) == 0)
				{
					textures.push_back(m_Textures[i]);
					skip = true;
					break;
				}
			}

			if (!skip)
			{
				Ref<Texture2D> texture = Texture2D::Create(m_Directory + "/" + str.C_Str(), false);
				textures.push_back(texture);
				m_Textures.push_back(texture);
			}
		}

		return textures;
	}

	glm::vec4 AssimpEntity::GetMaterialColor(const aiMaterial* material, const char* pKey, uint32_t type, uint32_t idx)
	{
		aiColor4D color;
		if (material->Get(pKey, type, idx, color) != aiReturn_FAILURE)
			return glm::vec4((float)color.r, (float)color.g, (float)color.b, (float)color.a);

		return glm::vec4(0.0f);
	}

}