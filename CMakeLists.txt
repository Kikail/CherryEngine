cmake_minimum_required(VERSION 3.20)

project(CherryEngine C)

# Standards C
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# =====================================================
# RESOURCES PATH
# =====================================================
add_compile_definitions(RESOURCES_PATH="${CMAKE_SOURCE_DIR}/resources")

# =====================================================
# DEPENDENCIES (FetchContent)
# =====================================================
include(FetchContent)

# GLFW
FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG 3.3.9
)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(glfw)

# CGLM (Maths library)
FetchContent_Declare(
        cglm
        GIT_REPOSITORY https://github.com/recp/cglm
        GIT_TAG v0.9.4
)
FetchContent_MakeAvailable(cglm)

# OpenGL
find_package(OpenGL REQUIRED)

find_package(assimp REQUIRED)

# =====================================================
# EXTERNAL LIBRARIES (GLAD)
# =====================================================
add_library(glad STATIC external/glad/glad.c)
target_include_directories(glad PUBLIC external)

# =====================================================
# MAIN EXECUTABLE
# =====================================================
add_executable(CherryEngine
        src/main.c
        src/utils/utils.h
        src/game/ecs/componentPool.c
        src/game/ecs/componentPool.h
        src/game/ecs/components/transform.c
        src/game/ecs/components/transform.h
        src/game/ecs/components/meshRenderer.c
        src/game/ecs/components/meshRenderer.h
        src/game/ecs/components/spriteRenderer.c
        src/game/ecs/components/spriteRenderer.h
        src/game/ecs/components/playerController.c
        src/game/ecs/components/playerController.h
        src/utils/types.h
        src/game/ecs/component.h
        src/game/scene/gameObject.c
        src/game/scene/gameObject.h
        src/utils/maths/Vector3.c
        src/utils/maths/Vector3.h
        src/render/camera.c
        src/render/camera.h
        src/render/shader.c
        src/render/shader.h
        src/resource/shapes/shape.h
        src/resource/shapes/shape.c
        src/physics/physicsObject.h
        src/physics/physicsWorld.h
        src/physics/physicsWorld.c
        src/physics/colliders.h
        src/physics/testCollision.h
        src/physics/testCollision.c
        src/physics/collision.h
        src/render/mesh.h
        src/render/mesh.c
        src/render/model.h
        src/render/model.c
        src/physics/octree.h
        src/physics/octree.c
        src/render/instanceMesh.h
        src/render/instanceMesh.c
        src/render/material.h
        src/render/material.c
        src/game/scene/scene.h
        src/game/scene/scene.c
        src/game/scene/game.h
        src/resource/resourceManager.h
        src/game/scene/game.c
        src/resource/resourceManager.c
        src/game/input/input.h
        src/game/input/input.c
        src/resource/filePicker.h
        src/resource/filePicker.c
        src/resource/fileSaver.h
        src/resource/fileSaver.c
        src/resource/serializer.h
        src/resource/serializer.c
)

# Configuration de l'inclusion des headers
target_include_directories(CherryEngine PRIVATE src)

# Optionnel : Permet d'utiliser les structs dans cglm (ex: vec.x au lieu de vec[0])
target_compile_definitions(CherryEngine PRIVATE CGLM_USE_STRUCTS)

# =====================================================
# LINK LIBRARIES
# =====================================================
target_link_libraries(CherryEngine
        PRIVATE
        glfw
        glad
        cglm
        OpenGL::GL
        assimp::assimp
        ${CMAKE_DL_LIBS}
)