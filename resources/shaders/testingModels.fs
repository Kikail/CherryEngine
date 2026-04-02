#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
in vec3 FragPos;
in mat3 TBN;

uniform vec3 viewPos;        // Position de la caméra (monde)
uniform vec3 lightPos; // Direction de la lumière
uniform sampler2D diffuse;   // Slot 0
uniform sampler2D normal;    // Slot 1

struct Material{
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};
uniform Material material;

void main()
{
    vec3 localNormal = texture(normal, TexCoords).rgb;
    localNormal = normalize(localNormal * 2.0 - 1.0);
    vec3 worldNormal = normalize(TBN * localNormal);

    vec3 lightColor = vec3(1.0);

    // ambient
    vec3 ambient = lightColor * material.ambient;

    // diffuse
    vec3 norm = normalize(worldNormal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = lightColor * (diff * material.diffuse);

    // specular
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = lightColor * (spec * material.specular);

    vec3 result = ambient + diffuse + specular;
    FragColor = vec4(result, 1.0);
}
