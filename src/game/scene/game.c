//
// Created by killian on 4/9/26.
//
#include "game.h"

int Game_initWindow(Game* game) {
    #ifdef DEBUG
        if (game == NULL)
            DEBUG_LOG("GAME::Game_initWindow game is NULL");
    #endif

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
    #ifdef DEBUG
        if (game == NULL)
            DEBUG_LOG("GAME::Game_init failed to malloc game");
    #endif

    game->input = Input_create();

    // Initialisation Factice de ta structure Camera (si processInput en a besoin)
    game->camera = Camera_createCamera(
        glms_vec3_zero(), (vec3s)VECTOR_UP, (vec3s)VECTOR_FRONT,
        CAMERA_YAW, CAMERA_PITCH, CAMERA_SPEED, CAMERA_SENSIVITY, CAMERA_ZOOM
    );

    Game_initWindow(game);

    //game->physicsWorld = PhysicsWorld_create();
    game->resourceManager = ResourceManager_create();

    // Creation de la matrice de perspective
    float aspect = (float)WIDTH / (float)HEIGHT;
    float fov = glm_rad(45.0f);
    game->perspective = glms_perspective(fov, aspect, 0.1f, 300.0f);

    return game;
}

void Game_update(Game* game, float deltaTime) {
    #ifdef DEBUG
        if (game == NULL)
            DEBUG_LOG("GAME::Game_update game is NULL");
    #endif
    // On actualise le monde physique
    //PhysicsWorld_step(game->physicsWorld, deltaTime);
    // On actualise les inputs
    Input_update(game->input, game->window);
}


Camera* Game_getCamera(Game* game) {
    #ifdef DEBUG
        if (game == NULL)
            DEBUG_LOG("GAME::Game_getCamera game is NULL");
    #endif
    return &game->camera;
}
GLFWwindow* Game_getWindow(Game* game) {
    #ifdef DEBUG
        if (game == NULL || game->window == NULL)
            DEBUG_LOG("GAME::Game_getWindow game or window is NULL");
    #endif
    return game->window;
}
PhysicsWorld* Game_getPhysicsWorld(Game* game) {
    #ifdef DEBUG
        if (game == NULL || game->physicsWorld)
            DEBUG_LOG("GAME::Game_getPhysicsWorld game or physicsWorld is NULL");
    #endif
    return game->physicsWorld;
}
ResourceManager* Game_getResourceManager(Game* game) {
    #ifdef DEBUG
        if (game == NULL || game->resourceManager == NULL)
            DEBUG_LOG("GAME::Game_getResourceManager game or resourceManager is NULL");
    #endif
    return game->resourceManager;
}
mat4s Game_getPerspective(Game* game) {
    #ifdef DEBUG
        if (game == NULL)
            DEBUG_LOG("GAME::Game_getPerspective game is NULL");
    #endif
    return game->perspective;
}
Input* Game_getInput(Game* game) {
    #ifdef DEBUG
        if (game == NULL || game->input == NULL)
            DEBUG_LOG("GAME::Game_getInput game or input is NULL");
    #endif
    return game->input;
}