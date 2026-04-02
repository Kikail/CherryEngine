#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
in vec3 FragPos;
in mat3 TBN;
in vec3 iNormal;

uniform vec3 viewPos;
uniform vec3 lightPos;

struct Material {
    bool usingDiffuseTexture;
    bool usingSpecularTexture;
    bool usingNormalTexture;

    sampler2D diffuseTexture;
    sampler2D normalTexture;
    sampler2D specularTexture;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

uniform Material material;

void main()
{
    // --- 1. GESTION DES NORMALES ---
    vec3 norm;
    if (material.usingNormalTexture) {
        vec3 localNormal = texture(material.normalTexture, TexCoords).rgb;
        localNormal = normalize(localNormal * 2.0 - 1.0);
        norm = normalize(TBN * localNormal);
    } else {
        norm = normalize(iNormal);
    }

    // --- 2. COULEURS DE BASE (TEXTURES) ---
    vec3 diffuseTexColor = vec3(1.0);
    if (material.usingDiffuseTexture) {
        diffuseTexColor = texture(material.diffuseTexture, TexCoords).rgb;
    }

    vec3 specularTexColor = vec3(1.0);
    if (material.usingSpecularTexture) {
        specularTexColor = texture(material.specularTexture, TexCoords).rgb;
    }

    vec3 lightColor = vec3(1.0);

    // --- 3. CALCULS D'ÉCLAIRAGE ---

    // Ambient
    vec3 ambient = lightColor * material.ambient * diffuseTexColor;

    // Diffuse
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = lightColor * (diff * material.diffuse) * diffuseTexColor;

    // Specular
    vec3 viewDir = normalize(viewPos - FragPos);
    // On garde ta version sans le signe '-' comme demandé
    vec3 reflectDir = reflect(lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = lightColor * (spec * material.specular) * specularTexColor;

    // --- 4. RÉSULTAT FINAL ---
    vec3 result = ambient + diffuse + specular;
    FragColor = vec4(result, 1.0);
}