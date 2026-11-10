#pragma once

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>
#include <cmath>

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include "SOIL2/SOIL2.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Mesh.h"
#include "Shader.h"

using namespace std;

inline GLint TextureFromFile(const char* path, string directory);

class Model
{
public:
	/* ---------- Funciones publicas ---------- */

	// path: ruta RELATIVA al modelo, por ejemplo "Models/caminar.fbx"
	// fallbackTexture: nombre del archivo de textura (en la misma carpeta del modelo)
	//                  que se usa si el modelo no trae una textura valida, por ejemplo "mujer.png"
	Model(GLchar* path, const char* fallbackTexture = "")
	{
		if (fallbackTexture) m_FallbackTexture = fallbackTexture;
		this->loadModel(path);
	}

	// Dibuja el modelo (todas sus mallas).
	void Draw(Shader shader)
	{
		for (GLuint i = 0; i < this->meshes.size(); i++)
		{
			this->meshes[i].Draw(shader);
		}
	}

	// Acceso al mapa de huesos y a las matrices finales para enviarlas al shader.
	auto& GetBoneInfoMap() { return m_BoneMapping; }

	vector<glm::mat4> GetFinalBoneMatrices()
	{
		vector<glm::mat4> matrices;
		matrices.reserve(m_BoneInfo.size());
		for (const auto& info : m_BoneInfo)
		{
			matrices.push_back(info.FinalTransformation);
		}
		return matrices;
	}

	const aiScene* GetScenePointer() { return m_ScenePointer; }

	void SetRightArmGesture(float amount)
	{
		m_RightArmGesture = glm::clamp(amount, 0.0f, 1.0f);
	}

	// Avanza la animacion y recalcula las matrices de todos los huesos.
	void UpdateAnimation(float deltaTime, const aiScene* scene)
	{
		if (!scene || !scene->HasAnimations()) return;

		// Primera animacion del archivo
		aiAnimation* animation = scene->mAnimations[0];
		float ticksPerSecond = (float)(animation->mTicksPerSecond != 0 ? animation->mTicksPerSecond : 25.0);
		float duration = (float)animation->mDuration;
		if (duration <= 0.0f) return;

		m_AnimationTime += deltaTime * ticksPerSecond;
		m_AnimationTime = fmod(m_AnimationTime, duration);

		glm::mat4 identity(1.0f);
		ReadNodeHierarchy(m_AnimationTime, scene->mRootNode, animation, identity);
	}

	// Recorre el esqueleto, interpola cada canal y calcula la transformacion final de cada hueso.
	void ReadNodeHierarchy(float animationTime, const aiNode* node, const aiAnimation* animation, const glm::mat4& parentTransform)
	{
		string nodeName = node->mName.data;
		glm::mat4 nodeTransformation = AiToGlm(node->mTransformation);

		// Buscar el canal de animacion de este nodo
		const aiNodeAnim* nodeAnim = nullptr;
		for (GLuint i = 0; i < animation->mNumChannels; i++)
		{
			if (string(animation->mChannels[i]->mNodeName.data) == nodeName)
			{
				nodeAnim = animation->mChannels[i];
				break;
			}
		}

		if (nodeAnim)
		{
			glm::vec3 scale = InterpScale(animationTime, nodeAnim);
			glm::quat rotation = InterpRotation(animationTime, nodeAnim);
			glm::vec3 translation = InterpPosition(animationTime, nodeAnim);

			// ====================================================================
			// NUEVO FILTRO PARA EVITAR EL EFECTO DE REINICIO/PARPADEO
			// ====================================================================
			// Si el nodo actual es la cadera, anulamos su desplazamiento integrado en el FBX
			if (nodeName == "mixamorig:Hips" || nodeName == "Hips")
			{
				translation.z = 0.0f; // Bloquea el avance hacia adelante grabado por Mixamo
				translation.x = 0.0f; // Bloquea cualquier desviación lateral
			}
			// ====================================================================

			glm::mat4 mTranslation = glm::translate(glm::mat4(1.0f), translation);
			glm::mat4 mRotation = glm::mat4_cast(rotation);
			glm::mat4 mScale = glm::scale(glm::mat4(1.0f), scale);

			nodeTransformation = mTranslation * mRotation * mScale;
		}

		if (m_RightArmGesture > 0.0f)
		{
			float intensidad = m_RightArmGesture;

			if (nodeName == "mixamorig:RightShoulder")
			{
				nodeTransformation = nodeTransformation *
					glm::rotate(glm::mat4(1.0f),
						glm::radians(10.0f * intensidad),
						glm::vec3(0.0f, 0.0f, 1.0f));
			}

			if (nodeName == "mixamorig:RightArm")
			{
				nodeTransformation = nodeTransformation *
					glm::rotate(glm::mat4(1.0f),
						glm::radians(55.0f * intensidad),
						glm::vec3(0.0f, 0.0f, 1.0f));
			}

			if (nodeName == "mixamorig:RightForeArm")
			{
				nodeTransformation = nodeTransformation *
					glm::rotate(glm::mat4(1.0f),
						glm::radians(-15.0f * intensidad),
						glm::vec3(0.0f, 0.0f, 1.0f));
			}
		}

		glm::mat4 globalTransform = parentTransform * nodeTransformation;

		// Si el nodo es un hueso del modelo, guardar su transformacion final
		auto it = m_BoneMapping.find(nodeName);
		if (it != m_BoneMapping.end())
		{
			int boneID = it->second;
			m_BoneInfo[boneID].FinalTransformation =
				m_GlobalInverse * globalTransform * m_BoneInfo[boneID].BoneOffset;
		}

		// Hijos
		for (GLuint i = 0; i < node->mNumChildren; i++)
		{
			this->ReadNodeHierarchy(animationTime, node->mChildren[i], animation, globalTransform);
		}
	}


private:
	/* ---------- Datos del modelo ---------- */
	vector<Mesh> meshes;
	string directory;
	vector<Texture> textures_loaded; // Texturas ya cargadas (evita cargarlas dos veces)
	string m_FallbackTexture;        // Textura de respaldo (solo nombre de archivo)

	/* ---------- Datos de huesos / animacion ---------- */
	struct BoneInfo
	{
		glm::mat4 BoneOffset;          // Matriz de desplazamiento original del hueso
		glm::mat4 FinalTransformation; // Matriz calculada por la animacion
	};

	std::map<string, int> m_BoneMapping; // Nombre del hueso -> ID
	int m_BoneCounter = 0;
	vector<BoneInfo> m_BoneInfo;

	glm::mat4 m_GlobalInverse = glm::mat4(1.0f);
	float m_AnimationTime = 0.0f;
	float m_RightArmGesture = 0.0f;

	const aiScene* m_ScenePointer = nullptr;
	Assimp::Importer importer;

	/* ---------- Utilidades de matrices e interpolacion ---------- */

	// aiMatrix4x4 (row-major) -> glm::mat4 (column-major)
	static glm::mat4 AiToGlm(const aiMatrix4x4& m)
	{
		glm::mat4 r;
		for (int row = 0; row < 4; row++)
			for (int col = 0; col < 4; col++)
				r[row][col] = m[col][row];
		return r;
	}

	// Deja solo el nombre del archivo: "C:\Users\X\mujer.png" -> "mujer.png"
	static string FileNameOnly(const string& p)
	{
		size_t slash = p.find_last_of("/\\");
		return (slash == string::npos) ? p : p.substr(slash + 1);
	}

	// Indice del keyframe anterior al tiempo t
	template<typename T>
	static GLuint FindKey(float t, GLuint count, T* keys)
	{
		for (GLuint i = 0; i + 1 < count; i++)
		{
			if (t < (float)keys[i + 1].mTime) return i;
		}
		return count > 1 ? count - 2 : 0;
	}

	static glm::vec3 InterpPosition(float t, const aiNodeAnim* a)
	{
		if (a->mNumPositionKeys == 0) return glm::vec3(0.0f);
		if (a->mNumPositionKeys == 1)
		{
			aiVector3D v = a->mPositionKeys[0].mValue;
			return glm::vec3(v.x, v.y, v.z);
		}
		GLuint i = FindKey(t, a->mNumPositionKeys, a->mPositionKeys);
		float t0 = (float)a->mPositionKeys[i].mTime;
		float t1 = (float)a->mPositionKeys[i + 1].mTime;
		float f = (t1 > t0) ? glm::clamp((t - t0) / (t1 - t0), 0.0f, 1.0f) : 0.0f;
		aiVector3D s = a->mPositionKeys[i].mValue;
		aiVector3D e = a->mPositionKeys[i + 1].mValue;
		return glm::mix(glm::vec3(s.x, s.y, s.z), glm::vec3(e.x, e.y, e.z), f);
	}

	static glm::vec3 InterpScale(float t, const aiNodeAnim* a)
	{
		if (a->mNumScalingKeys == 0) return glm::vec3(1.0f);
		if (a->mNumScalingKeys == 1)
		{
			aiVector3D v = a->mScalingKeys[0].mValue;
			return glm::vec3(v.x, v.y, v.z);
		}
		GLuint i = FindKey(t, a->mNumScalingKeys, a->mScalingKeys);
		float t0 = (float)a->mScalingKeys[i].mTime;
		float t1 = (float)a->mScalingKeys[i + 1].mTime;
		float f = (t1 > t0) ? glm::clamp((t - t0) / (t1 - t0), 0.0f, 1.0f) : 0.0f;
		aiVector3D s = a->mScalingKeys[i].mValue;
		aiVector3D e = a->mScalingKeys[i + 1].mValue;
		return glm::mix(glm::vec3(s.x, s.y, s.z), glm::vec3(e.x, e.y, e.z), f);
	}

	static glm::quat InterpRotation(float t, const aiNodeAnim* a)
	{
		if (a->mNumRotationKeys == 0) return glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
		if (a->mNumRotationKeys == 1)
		{
			aiQuaternion q = a->mRotationKeys[0].mValue;
			return glm::quat(q.w, q.x, q.y, q.z);
		}
		GLuint i = FindKey(t, a->mNumRotationKeys, a->mRotationKeys);
		float t0 = (float)a->mRotationKeys[i].mTime;
		float t1 = (float)a->mRotationKeys[i + 1].mTime;
		float f = (t1 > t0) ? glm::clamp((t - t0) / (t1 - t0), 0.0f, 1.0f) : 0.0f;
		aiQuaternion s = a->mRotationKeys[i].mValue;
		aiQuaternion e = a->mRotationKeys[i + 1].mValue;
		return glm::normalize(glm::slerp(glm::quat(s.w, s.x, s.y, s.z),
			glm::quat(e.w, e.x, e.y, e.z), f));
	}

	/* ---------- Carga del modelo ---------- */

	void loadModel(string path)
	{
		m_ScenePointer = importer.ReadFile(path,
			aiProcess_Triangulate |
			aiProcess_FlipUVs |
			aiProcess_GenSmoothNormals |
			aiProcess_LimitBoneWeights);
		const aiScene* scene = m_ScenePointer;

		if (!scene || (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) || !scene->mRootNode)
		{
			cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << endl;
			cout << "Ruta intentada (relativa al directorio de trabajo): " << path << endl;
			return;
		}

		// Inversa de la transformacion del nodo raiz
		m_GlobalInverse = glm::inverse(AiToGlm(scene->mRootNode->mTransformation));

		// Carpeta del modelo, tomada de la ruta relativa que se paso (soporta / y \)
		size_t pos = path.find_last_of("/\\");
		this->directory = (pos == string::npos) ? "." : path.substr(0, pos);

		this->processNode(scene->mRootNode, scene);
	}

	// Procesa un nodo y sus hijos de forma recursiva.
	void processNode(aiNode* node, const aiScene* scene)
	{
		for (GLuint i = 0; i < node->mNumMeshes; i++)
		{
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			this->meshes.push_back(this->processMesh(mesh, scene));
		}

		for (GLuint i = 0; i < node->mNumChildren; i++)
		{
			this->processNode(node->mChildren[i], scene);
		}
	}

	Mesh processMesh(aiMesh* mesh, const aiScene* scene)
	{
		vector<Vertex> vertices;
		vector<GLuint> indices;
		vector<Texture> textures;

		// ----- Vertices -----
		for (GLuint i = 0; i < mesh->mNumVertices; i++)
		{
			Vertex vertex;
			glm::vec3 vec3;

			// Inicializar datos de huesos
			for (int k = 0; k < 4; k++)
			{
				vertex.m_BoneIDs[k] = -1;
				vertex.m_Weights[k] = 0.0f;
			}

			// Posicion
			vec3.x = mesh->mVertices[i].x;
			vec3.y = mesh->mVertices[i].y;
			vec3.z = mesh->mVertices[i].z;
			vertex.Position = vec3;

			// Normal
			if (mesh->HasNormals())
			{
				vec3.x = mesh->mNormals[i].x;
				vec3.y = mesh->mNormals[i].y;
				vec3.z = mesh->mNormals[i].z;
				vertex.Normal = vec3;
			}
			else
			{
				vertex.Normal = glm::vec3(0.0f, 1.0f, 0.0f);
			}

			// Coordenadas de textura
			if (mesh->mTextureCoords[0])
			{
				glm::vec2 vec2;
				vec2.x = mesh->mTextureCoords[0][i].x;
				vec2.y = mesh->mTextureCoords[0][i].y;
				vertex.TexCoords = vec2;
			}
			else
			{
				vertex.TexCoords = glm::vec2(0.0f, 0.0f);
			}

			vertices.push_back(vertex);
		}

		// ----- Indices -----
		for (GLuint i = 0; i < mesh->mNumFaces; i++)
		{
			aiFace face = mesh->mFaces[i];
			for (GLuint j = 0; j < face.mNumIndices; j++)
			{
				indices.push_back(face.mIndices[j]);
			}
		}

		// ----- Huesos y pesos -----
		for (GLuint i = 0; i < mesh->mNumBones; i++)
		{
			aiBone* bone = mesh->mBones[i];
			string boneName = bone->mName.C_Str();
			int boneID = -1;

			if (m_BoneMapping.find(boneName) == m_BoneMapping.end())
			{
				boneID = m_BoneCounter;
				m_BoneCounter++;

				BoneInfo newBoneInfo;
				newBoneInfo.BoneOffset = AiToGlm(bone->mOffsetMatrix);
				newBoneInfo.FinalTransformation = glm::mat4(1.0f);
				m_BoneInfo.push_back(newBoneInfo);
				m_BoneMapping[boneName] = boneID;
			}
			else
			{
				boneID = m_BoneMapping[boneName];
			}

			for (GLuint j = 0; j < bone->mNumWeights; j++)
			{
				GLuint vertexID = bone->mWeights[j].mVertexId;
				float weight = bone->mWeights[j].mWeight;

				if (vertexID < vertices.size())
				{
					for (int k = 0; k < 4; k++)
					{
						if (vertices[vertexID].m_BoneIDs[k] == -1)
						{
							vertices[vertexID].m_BoneIDs[k] = boneID;
							vertices[vertexID].m_Weights[k] = weight;
							break;
						}
					}
				}
			}
		}

		// ----- Materiales -----
		if (mesh->mMaterialIndex >= 0)
		{
			aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

			vector<Texture> diffuseMaps = this->loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
			textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());

			vector<Texture> specularMaps = this->loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
			textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
		}

		// ----- Fallback: si no se cargo ninguna textura, usar la de respaldo -----
		if (textures.empty() && !m_FallbackTexture.empty())
		{
			textures.push_back(this->loadFallbackTexture());
		}

		return Mesh(vertices, indices, textures);
	}

	// Carga (una sola vez) la textura de respaldo desde la carpeta del modelo.
	Texture loadFallbackTexture()
	{
		aiString fallbackPath(m_FallbackTexture.c_str());

		for (GLuint j = 0; j < textures_loaded.size(); j++)
		{
			if (textures_loaded[j].path == fallbackPath && textures_loaded[j].type == "texture_diffuse")
			{
				return textures_loaded[j];
			}
		}

		Texture texture;
		texture.id = TextureFromFile(m_FallbackTexture.c_str(), this->directory);
		texture.type = "texture_diffuse";
		texture.path = fallbackPath;
		this->textures_loaded.push_back(texture);
		return texture;
	}

	// Carga las texturas de un material si aun no estan cargadas.
	vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, string typeName)
	{
		vector<Texture> textures;
		for (GLuint i = 0; i < mat->GetTextureCount(type); i++)
		{
			aiString str;
			mat->GetTexture(type, i, &str);

			// Texturas incrustadas en el FBX (rutas tipo "*0"): no se soportan, se usara el fallback
			if (str.length > 0 && str.C_Str()[0] == '*')
			{
				continue;
			}

			// Los FBX suelen guardar rutas absolutas de la PC donde se exporto
			// (ej. "C:\Users\...\mujer.png"). Nos quedamos solo con el nombre del archivo
			// para buscarlo en la carpeta del modelo, que es relativa al proyecto.
			string fileName = FileNameOnly(str.C_Str());
			aiString cleanPath(fileName.c_str());

			GLboolean skip = false;
			for (GLuint j = 0; j < textures_loaded.size(); j++)
			{
				if (textures_loaded[j].path == cleanPath && textures_loaded[j].type == typeName)
				{
					textures.push_back(textures_loaded[j]);
					skip = true;
					break;
				}
			}

			if (!skip)
			{
				GLint id = TextureFromFile(fileName.c_str(), this->directory);
				if (id == 0) continue; // no se pudo cargar: que actue el fallback

				Texture texture;
				texture.id = id;
				texture.type = typeName;
				texture.path = cleanPath;
				textures.push_back(texture);
				this->textures_loaded.push_back(texture);
			}
		}
		return textures;
	}
};

// Devuelve el ID de la textura, o 0 si no se pudo cargar el archivo.
inline GLint TextureFromFile(const char* path, string directory)
{
	string filename = directory + '/' + string(path);

	int width, height;
	unsigned char* image = SOIL_load_image(filename.c_str(), &width, &height, 0, SOIL_LOAD_RGB);
	if (!image)
	{
		cout << "ERROR::TEXTURA:: no se pudo cargar " << filename << endl;
		return 0;
	}

	GLuint textureID;
	glGenTextures(1, &textureID);

	glBindTexture(GL_TEXTURE_2D, textureID);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	glGenerateMipmap(GL_TEXTURE_2D);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBindTexture(GL_TEXTURE_2D, 0);
	SOIL_free_image_data(image);
	return textureID;
}