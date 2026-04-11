//
// Created by killian on 4/9/26.
//
#include "game.h"

int Game_initWindow(Game* game) {
    // GLFW init
    if (!glfwInit()) {
        DEBUG_LOG("Failed to initialize GLFW");
        return EXIT_FAILURE;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    game->window = glfwCreateWindow(WIDTH, HEIGHT, "CherryEngine", NULL, NULL);
    if (game->window  == NULL) {
        DEBUG_LOG("Failed to create GLFW window");
        glfwTerminate();
        return EXIT_FAILURE;
    }
    glfwMakeContextCurrent(game->window );
    // NOUVEAU : Curseur normal et ajout des callbacks
    glfwSetInputMode(game->window , GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    glfwSetCursorPosCallback(game->window , mouse_callback);
    glfwSetMouseButtonCallback(game->window , mouse_button_callback);
    glfwSetScrollCallback(game->window , scroll_callback);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        DEBUG_LOG("Erreur de Init Glad");
        cleanup(game->window );
        return EXIT_FAILURE;
    }
}

Game* Game_init() {
    Game* game = malloc(sizeof(Game));

    game->input = Input_create();

    // Initialisation Factice de ta structure Camera (si processInput en a besoin)
    game->camera = Camera_createCamera(
        glms_vec3_zero(), (vec3s)VECTOR_UP, (vec3s)VECTOR_FRONT,
        CAMERA_YAW, CAMERA_PITCH, CAMERA_SPEED, CAMERA_SENSIVITY, CAMERA_ZOOM
    );

    Game_initWindow(game);

    game->physicsWorld = PhysicsWorld_create();
    game->resourceManager = ResourceManager_create();



    // Creation de la matrice de perspective
    float aspect = (float)WIDTH / (float)HEIGHT;
    float fov = glm_rad(45.0f);
    game->perspective = glms_perspective(fov, aspect, 0.1f, 300.0f);

    return game;
}

void Game_update(Game* game, float deltaTime) {
    // On actualise le monde physique
    PhysicsWorld_step(game->physicsWorld, deltaTime);
    // On actualise les inputs
    Input_update(game->input, game->window);
}


Camera* Game_getCamera(Game* game) {
    return &game->camera;
}
GLFWwindow* Game_getWindow(Game* game) {
    return game->window;
}
PhysicsWorld* Game_getPhysicsWorld(Game* game) {
    return game->physicsWorld;
}
ResourceManager* Game_getResourceManager(Game* game) {
    return game->resourceManager;
}
mat4s Game_getPerspective(Game* game) {
    return game->perspective;
}
Input* Game_getInput(Game* game) {
    return game->input;
}