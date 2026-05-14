#version 330 core

out vec4 FragColor;

in vec2 TexCoords;
in vec3 FragPos;
in mat3 TBN;
in vec3 iNormal;

uniform vec3 viewPos;
uniform vec3 lightPos;
uniform samplerCube skybox;

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
    // On initialise avec la couleur diffuse de base du matériau
    vec3 color = vec3(1.0,1.0,1.0);

    // Affichage direct sans calcul de lumière
    FragColor = vec4(color, 1.0);
}