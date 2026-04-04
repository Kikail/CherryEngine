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
    // -------------------------------------------------------------------------
    // 1. NORMAL MAPPING
    // -------------------------------------------------------------------------
    vec3 norm;
    if (material.usingNormalTexture) {
        vec3 localNormal = texture(material.normalTexture, TexCoords).rgb;
        localNormal = normalize(localNormal * 2.0 - 1.0);
        norm = normalize(TBN * localNormal);
    } else {
        norm = normalize(iNormal);
    }

    // -------------------------------------------------------------------------
    // 2. TEXTURE SAMPLING
    // -------------------------------------------------------------------------
    vec3 diffuseTexColor = vec3(1.0);
    if (material.usingDiffuseTexture) {
        diffuseTexColor = texture(material.diffuseTexture, TexCoords).rgb;
    }

    vec3 specularTexColor = vec3(1.0);
    if (material.usingSpecularTexture) {
        specularTexColor = texture(material.specularTexture, TexCoords).rgb;
    }

    float ao = 1.0;
    if (material.usingAOTexture) {
        float aoSample = texture(material.aoTexture, TexCoords).r;
        ao = mix(1.0, aoSample, material.aoIntensity);
    }

    // -------------------------------------------------------------------------
    // 3. CALCUL DE L'ÉCLAIRAGE (Phong)
    // -------------------------------------------------------------------------
    vec3 lightColor = vec3(1.0);

    vec3 ambient = lightColor * (material.ambient * ao) * diffuseTexColor;

    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = lightColor * (diff * material.diffuse) * diffuseTexColor * ao;

    // viewDir pointe du fragment VERS la caméra
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = lightColor * (spec * material.specular) * specularTexColor;

    vec3 result = ambient + diffuse + specular;

    // -------------------------------------------------------------------------
    // 4. COULEUR FINALE & REFLEXION
    // -------------------------------------------------------------------------
    // Le vecteur d'incidence I doit pointer de la caméra VERS le fragment.
    // C'est exactement l'inverse de viewDir que nous avons calculé juste au dessus !
    vec3 I = -viewDir;

    // Si la réflexion est trop déformée à cause de la normal map du rocher,
    // remplace "norm" par "normalize(iNormal)" ici pour avoir un reflet lisse.
    vec3 R;
    if(material.usingNormalTexture){
        vec3 smoothNormal = normalize(iNormal);
        vec3 finalNormal = normalize(mix(smoothNormal, norm, 0.5)); // ajuste 0.3
        R = reflect(I, finalNormal);
    }
    else{
        R = reflect(I, iNormal);
    }


    vec3 reflection = texture(skybox, R).rgb;

    // On mélange la couleur de base avec le reflet du ciel
    float fresnel = pow(1.0 - max(dot(viewDir, norm), 0.0), 5.0);
    float reflectFactor = mix(material.reflectionIntensity, 1.0, fresnel);
    vec3 finalColor = mix(result, reflection, reflectFactor);

    FragColor = vec4(finalColor, 1.0);
}