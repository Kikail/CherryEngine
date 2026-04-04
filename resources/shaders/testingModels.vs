#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBiTangent;

out vec2 TexCoords;
out vec3 iNormal;
out vec3 FragPos;
out mat3 TBN;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

struct Material {
    bool usingDiffuseTexture;
    bool usingSpecularTexture;
    bool usingNormalTexture;
    bool usingAOTexture;
    bool usingDisplacementTexture;

    sampler2D diffuseTexture;
    sampler2D normalTexture;
    sampler2D specularTexture;
    sampler2D aoTexture;
    sampler2D displacementTexture;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float aoIntensity;
    float shininess;
    float displacementIntensity;
    float reflectionIntensity;
};
uniform Material material;

void main()
{
    vec3 currentPos = aPos;
    if (material.usingDisplacementTexture) {
        float height = texture(material.displacementTexture, aTexCoords).r;
        currentPos += aNormal * (height * material.displacementIntensity);
    }
    FragPos = vec3(model * vec4(currentPos, 1.0));
    TexCoords = aTexCoords;
    mat3 normalMatrix = mat3(transpose(inverse(model)));
    vec3 T = normalize(normalMatrix * aTangent);
    vec3 N = normalize(normalMatrix * aNormal);
    T = normalize(T - dot(T, N) * N);
    vec3 B = cross(N, T);
    TBN = mat3(T, B, N);
    iNormal = normalize(mat3(transpose(inverse(model))) * aNormal);
    gl_Position = projection * view * vec4(FragPos, 1.0);
}