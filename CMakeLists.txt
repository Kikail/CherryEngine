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
        src/editor/ecs/componentPool.c
        src/editor/ecs/componentPool.h
        src/editor/ecs/components/transform.c
        src/editor/ecs/components/transform.h
        src/editor/ecs/components/meshRenderer.c
        src/editor/ecs/components/meshRenderer.h
        src/editor/ecs/components/spriteRenderer.c
        src/editor/ecs/components/spriteRenderer.h
        src/editor/ecs/components/playerController.c
        src/editor/ecs/components/playerController.h
        src/utils/types.h
        src/editor/ecs/component.h
        src/editor/ecs/gameObject.c
        src/editor/ecs/gameObject.h
        src/utils/maths/Vector3.c
        src/utils/maths/Vector3.h
        src/render/camera.c
        src/render/camera.h
        src/render/shader.c
        src/render/shader.h
        src/editor/core/shapes/shape.h
        src/editor/core/shapes/shape.c
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