#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoords;
layout (location = 3) in ivec4 boneIds;    // IDs de los 4 huesos que afectan al vertice
layout (location = 4) in vec4 weights;     // Peso de cada hueso

out vec3 Normal;
out vec3 FragPos;
out vec2 TexCoords;

const int MAX_BONES = 100;
const int MAX_BONE_INFLUENCE = 4;
uniform mat4 finalBoneMatrices[MAX_BONES];

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    vec4 totalPosition = vec4(0.0);
    vec3 totalNormal = vec3(0.0);
    float totalWeight = 0.0;

    for (int i = 0; i < MAX_BONE_INFLUENCE; i++)
    {
        // Sin hueso asignado, o hueso fuera del rango del array: se ignora
        if (boneIds[i] < 0 || boneIds[i] >= MAX_BONES)
            continue;

        mat4 bone = finalBoneMatrices[boneIds[i]];
        totalPosition += (bone * vec4(position, 1.0)) * weights[i];
        totalNormal   += (mat3(bone) * normal) * weights[i];
        totalWeight   += weights[i];
    }

    if (totalWeight > 0.0001)
    {
        // Renormaliza por si los pesos no suman exactamente 1
        totalPosition /= totalWeight;
        totalNormal   /= totalWeight;
    }
    else
    {
        // Malla estatica (edificios, muelle...) o vertice sin pesos: forma original
        totalPosition = vec4(position, 1.0);
        totalNormal   = normal;
    }

    vec4 worldPos = model * totalPosition;
    gl_Position = projection * view * worldPos;
    FragPos = vec3(worldPos);
    Normal = normalize(mat3(transpose(inverse(model))) * totalNormal);
    TexCoords = texCoords;
}