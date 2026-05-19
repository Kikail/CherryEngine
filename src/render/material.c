//
// Created by killian on 4/2/26.
//
#include "material.h"
#include "../resource/resourceManager.h"
#include "utils/utils.h"

Material Material_create(vec3s a, vec3s d, vec3s s, float shininess, float aoIntensity, float displacementIntensity, float reflectionIntensity, Shader* shader) {
    Material mat;

    if (shader != NULL) {
        Shader_use(shader);
        Shader_setInt(shader, "material.diffuseTexture", 0);
        Shader_setInt(shader, "material.normalTexture", 1);
        Shader_setInt(shader, "material.specularTexture", 2);
        Shader_setInt(shader, "material.aoTexture", 3);
        Shader_setInt(shader, "material.displacementTexture", 4);
        mat.shaderSignature = SHADER_SIGNATURE_NULL;
    }

    mat.ambient = a;
    mat.diffuse = d;
    mat.specular = s;
    mat.aoIntensity = aoIntensity;
    mat.reflectionIntensity = reflectionIntensity;
    mat.shininess = shininess;
    mat.displacementIntensity = displacementIntensity;
    mat.usingDiffuseTexture = false;
    mat.usingSpecularTexture = false;
    mat.usingNormalTexture = false;
    mat.usingAOTexture = false;
    mat.usingDisplacementTexture = false;

    return mat;
}
void Material_attachDiffuseTexture(Material *material, unsigned int diffuseTexture) {
    material->diffuseTexture = diffuseTexture;
    material->usingDiffuseTexture = true;
}
void Material_attachSpecularTexture(Material *material, unsigned int specularTexture) {
    material->specularTexture = specularTexture;
    material->usingSpecularTexture = true;
}
void Material_attachNormalTexture(Material *material, unsigned int normalTexture) {
    material->normalTexture = normalTexture;
    material->usingNormalTexture = true;
}
void Material_attachAoTexture(Material *material, unsigned int aoTexture) {
    material->aoTexture = aoTexture;
    material->usingAOTexture = true;
}
void Material_attachDisplacementTexture(Material *material, unsigned int displacementTexture) {
    material->displacementTexture = displacementTexture;
    material->usingDisplacementTexture = true;
}
void Material_sendToShader(Material* mat, Shader* shader) {
    Shader_use(shader);
    Shader_setVec3(shader, "material.ambient", mat->ambient);
    Shader_setVec3(shader, "material.diffuse", mat->diffuse);
    Shader_setVec3(shader, "material.specular", mat->specular);
    Shader_setFloat(shader, "material.shininess", mat->shininess);
    Shader_setFloat(shader, "material.aoIntensity", mat->aoIntensity);
    Shader_setFloat(shader, "material.displacementIntensity", mat->displacementIntensity);
    Shader_setFloat(shader, "material.reflectionIntensity", mat->reflectionIntensity);

    Shader_setBool(shader, "material.usingDiffuseTexture", mat->usingDiffuseTexture);
    if (mat->usingDiffuseTexture) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, mat->diffuseTexture);
    }
    Shader_setBool(shader, "material.usingNormalTexture", mat->usingNormalTexture);
    if (mat->usingNormalTexture) {
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, mat->normalTexture);
    }
    Shader_setBool(shader, "material.usingSpecularTexture", mat->usingSpecularTexture);
    if (mat->usingSpecularTexture) {
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, mat->specularTexture);
    }
    Shader_setBool(shader, "material.usingAOTexture", mat->usingAOTexture);
    if (mat->usingAOTexture) {
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, mat->aoTexture);
    }
    Shader_setBool(shader, "material.usingDisplacementTexture", mat->usingDisplacementTexture);
    if (mat->usingDisplacementTexture) {
        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, mat->displacementTexture);
    }
}

Material Material_loadFromFile(char* path) {
    FILE* f = fopen(path, "r");
    SerialObject materialObject = SerialObject_DeserializeSingle(f);
    fclose(f);
    return Material_deserialize(&materialObject);
}

bool Material_loadTextures(Material* material, ResourceManager* res) {
    if (material->usingDiffuseTexture) {
        CherryTexture* tex = ResourceManager_getTextureBySignature(res, material->diffuseTextureSignature);
        if (tex == NULL) {
            #ifdef DEBUG
                  DEBUG_LOG_LINE("MATERIAL::Material_loadTextures failed to load diffuse texture");
                  printf(" %u\n",material->diffuseTextureSignature);
            #endif
            return false;
        }
        material->diffuseTexture = tex->id;
    }
    if (material->usingAOTexture) {
        CherryTexture* tex = ResourceManager_getTextureBySignature(res, material->aoTextureSignature);
        if (tex == NULL) {
            #ifdef DEBUG
                DEBUG_LOG("MATERIAL::Material_loadTextures failed to load ao texture");
            #endif
            return false;
        }
        material->aoTexture = tex->id;
    }
    if (material->usingDisplacementTexture) {
        CherryTexture* tex = ResourceManager_getTextureBySignature(res, material->displacementTextureSignature);
        if (tex == NULL) {
            #ifdef DEBUG
                DEBUG_LOG("MATERIAL::Material_loadTextures failed to load displacement texture");
            #endif
            return false;
        }
        material->displacementTexture = tex->id;
    }
    if (material->usingNormalTexture) {
        CherryTexture* tex = ResourceManager_getTextureBySignature(res, material->normalTextureSignature);
        if (tex == NULL) {
            #ifdef DEBUG
                DEBUG_LOG("MATERIAL::Material_loadTextures failed to load normal texture");
            #endif
            return false;
        }
        material->normalTexture = tex->id;
    }
    if (material->usingSpecularTexture) {
        CherryTexture* tex = ResourceManager_getTextureBySignature(res, material->specularTextureSignature);
        if (tex == NULL) {
            #ifdef DEBUG
                DEBUG_LOG("MATERIAL::Material_loadTextures failed to load specular texture");
            #endif
            return false;
        }
        material->specularTexture = tex->id;
    }
    // CHARGER LE SHADER
    material->shader = ResourceManager_getShaderBySignature(res, material->shaderSignature);
    if (material->shader == NULL) {
        #ifdef DEBUG
            DEBUG_LOG("MATERIAL::Material_loadTextures failed to load shader");
        #endif
        return false;
    }
    return true;
}
SerialObject Material_serialize(Material* material) {
    SerialObject serialObject = SerialObject_create("Material");

    SerialValue usingDiffuseTexture = SerialValue_create_int("usingDiffuseTexture", material->usingDiffuseTexture);
    SerialValue usingSpecularTexture = SerialValue_create_int("usingSpecularTexture", material->usingSpecularTexture);
    SerialValue usingNormalTexture = SerialValue_create_int("usingNormalTexture", material->usingNormalTexture);
    SerialValue usingAOTexture = SerialValue_create_int("usingAOTexture", material->usingAOTexture);
    SerialValue usingDisplacementTexture = SerialValue_create_int("usingDisplacementTexture", material->usingDisplacementTexture);
    SerialObject_AddSerialValue(&serialObject, &usingDiffuseTexture);
    SerialObject_AddSerialValue(&serialObject, &usingSpecularTexture);
    SerialObject_AddSerialValue(&serialObject, &usingNormalTexture);
    SerialObject_AddSerialValue(&serialObject, &usingAOTexture);
    SerialObject_AddSerialValue(&serialObject, &usingDisplacementTexture);

    SerialValue diffuseTextureSignature = SerialValue_create_uint("diffuseTextureSignature", material->diffuseTextureSignature);
    SerialValue specularTextureSignature = SerialValue_create_uint("specularTextureSignature", material->specularTextureSignature);
    SerialValue normalTextureSignature = SerialValue_create_uint("normalTextureSignature", material->normalTextureSignature);
    SerialValue aoTextureSignature = SerialValue_create_uint("aoTextureSignature", material->aoTextureSignature);
    SerialValue displacementTextureSignature = SerialValue_create_uint("displacementTextureSignature", material->displacementTextureSignature);
    SerialObject_AddSerialValue(&serialObject, &diffuseTextureSignature);
    SerialObject_AddSerialValue(&serialObject, &specularTextureSignature);
    SerialObject_AddSerialValue(&serialObject, &normalTextureSignature);
    SerialObject_AddSerialValue(&serialObject, &aoTextureSignature);
    SerialObject_AddSerialValue(&serialObject, &displacementTextureSignature);

    SerialValue ambiantValueX = SerialValue_create_double("ambientX", material->ambient.x);
    SerialValue ambiantValueY = SerialValue_create_double("ambientY", material->ambient.y);
    SerialValue ambiantValueZ = SerialValue_create_double("ambientZ", material->ambient.z);
    SerialObject_AddSerialValue(&serialObject, &ambiantValueX);
    SerialObject_AddSerialValue(&serialObject, &ambiantValueY);
    SerialObject_AddSerialValue(&serialObject, &ambiantValueZ);

    SerialValue diffuseValueX = SerialValue_create_double("diffuseX", material->diffuse.x);
    SerialValue diffuseValueY = SerialValue_create_double("diffuseY", material->diffuse.y);
    SerialValue diffuseValueZ = SerialValue_create_double("diffuseZ", material->diffuse.z);
    SerialObject_AddSerialValue(&serialObject, &diffuseValueX);
    SerialObject_AddSerialValue(&serialObject, &diffuseValueY);
    SerialObject_AddSerialValue(&serialObject, &diffuseValueZ);

    SerialValue specularValueX = SerialValue_create_double("specularX", material->specular.x);
    SerialValue specularValueY = SerialValue_create_double("specularY", material->specular.y);
    SerialValue specularValueZ = SerialValue_create_double("specularZ", material->specular.z);
    SerialObject_AddSerialValue(&serialObject, &specularValueX);
    SerialObject_AddSerialValue(&serialObject, &specularValueY);
    SerialObject_AddSerialValue(&serialObject, &specularValueZ);

    SerialValue aoIntensity = SerialValue_create_double("aoIntensity", material->aoIntensity);
    SerialValue shininessValue = SerialValue_create_double("shininess", material->shininess);
    SerialValue displacementIntensityValue = SerialValue_create_double("displacementIntensity", material->displacementIntensity);
    SerialValue reflectionIntensityValue = SerialValue_create_double("reflectionIntensity", material->reflectionIntensity);
    SerialObject_AddSerialValue(&serialObject, &aoIntensity);
    SerialObject_AddSerialValue(&serialObject, &shininessValue);
    SerialObject_AddSerialValue(&serialObject, &displacementIntensityValue);
    SerialObject_AddSerialValue(&serialObject, &reflectionIntensityValue);

    SerialValue shaderSignatureValue = SerialValue_create_uint("shaderSignature", material->shaderSignature);
    SerialObject_AddSerialValue(&serialObject, &shaderSignatureValue);

    return serialObject;
}
Material Material_deserialize(SerialObject* serialObject) {
    SerialValue usingDiffuseTexture = SerialObject_GetByName(serialObject, "usingDiffuseTexture");
    SerialValue usingSpecularTexture = SerialObject_GetByName(serialObject, "usingSpecularTexture");
    SerialValue usingNormalTexture = SerialObject_GetByName(serialObject, "usingNormalTexture");
    SerialValue usingAOTexture = SerialObject_GetByName(serialObject, "usingAOTexture");
    SerialValue usingDisplacementTexture = SerialObject_GetByName(serialObject, "usingDisplacementTexture");

    SerialValue diffuseTextureSignature = SerialObject_GetByName(serialObject, "diffuseTextureSignature");
    SerialValue specularTextureSignature = SerialObject_GetByName(serialObject, "specularTextureSignature");
    SerialValue normalTextureSignature = SerialObject_GetByName(serialObject, "normalTextureSignature");
    SerialValue aoTextureSignature = SerialObject_GetByName(serialObject, "aoTextureSignature");
    SerialValue displacementTextureSignature = SerialObject_GetByName(serialObject, "displacementTextureSignature");

    SerialValue ambiantValueX = SerialObject_GetByName(serialObject, "ambientX");
    SerialValue ambiantValueY = SerialObject_GetByName(serialObject, "ambientY");
    SerialValue ambiantValueZ = SerialObject_GetByName(serialObject, "ambientZ");
    SerialValue diffuseValueX = SerialObject_GetByName(serialObject, "diffuseX");
    SerialValue diffuseValueY = SerialObject_GetByName(serialObject, "diffuseY");
    SerialValue diffuseValueZ = SerialObject_GetByName(serialObject, "diffuseZ");
    SerialValue specularValueX = SerialObject_GetByName(serialObject, "specularX");
    SerialValue specularValueY = SerialObject_GetByName(serialObject, "specularY");
    SerialValue specularValueZ = SerialObject_GetByName(serialObject, "specularZ");

    SerialValue aoIntensity = SerialObject_GetByName(serialObject, "aoIntensity");
    SerialValue shininessValue = SerialObject_GetByName(serialObject, "shininess");
    SerialValue displacementIntensityValue = SerialObject_GetByName(serialObject, "displacementIntensity");
    SerialValue reflectionIntensityValue = SerialObject_GetByName(serialObject, "reflectionIntensity");

    SerialValue shaderSignatureValue = SerialObject_GetByName(serialObject, "shaderSignature");

    Material mat;
    mat.usingDiffuseTexture = SerialValue_GetIntValue(&usingDiffuseTexture);
    mat.usingSpecularTexture = SerialValue_GetIntValue(&usingSpecularTexture);
    mat.usingNormalTexture = SerialValue_GetIntValue(&usingNormalTexture);
    mat.usingAOTexture = SerialValue_GetIntValue(&usingAOTexture);
    mat.usingDisplacementTexture = SerialValue_GetIntValue(&usingDisplacementTexture);

    mat.diffuseTextureSignature = SerialValue_GetIntValue(&diffuseTextureSignature);
    mat.specularTextureSignature = SerialValue_GetIntValue(&specularTextureSignature);
    mat.normalTextureSignature = SerialValue_GetIntValue(&normalTextureSignature);
    mat.aoTextureSignature = SerialValue_GetIntValue(&aoTextureSignature);
    mat.displacementTextureSignature = SerialValue_GetIntValue(&displacementTextureSignature);

    mat.ambient = (vec3s){
    SerialValue_GetDoubleValue(&ambiantValueX),
    SerialValue_GetDoubleValue(&ambiantValueY),
    SerialValue_GetDoubleValue(&ambiantValueZ),
    };
    mat.diffuse = (vec3s){
        SerialValue_GetDoubleValue(&diffuseValueX),
        SerialValue_GetDoubleValue(&diffuseValueY),
        SerialValue_GetDoubleValue(&diffuseValueZ),
    };
    mat.specular = (vec3s){
        SerialValue_GetDoubleValue(&specularValueX),
        SerialValue_GetDoubleValue(&specularValueY),
        SerialValue_GetDoubleValue(&specularValueZ),
    };

    mat.aoIntensity = SerialValue_GetDoubleValue(&aoIntensity);
    mat.shininess = SerialValue_GetDoubleValue(&shininessValue);
    mat.displacementIntensity = SerialValue_GetDoubleValue(&displacementIntensityValue);
    mat.reflectionIntensity = SerialValue_GetDoubleValue(&reflectionIntensityValue);

    mat.shaderSignature = SerialValue_GetUintValue(&shaderSignatureValue);

    return mat;
}


































