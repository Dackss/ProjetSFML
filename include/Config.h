#ifndef CONFIG_H
#define CONFIG_H

#include <string>

namespace Config {
    // --- CHEMINS ---
    inline const std::string ASSETS_PATH = "../assets/";
    inline const std::string TEXTURES_PATH = ASSETS_PATH + "textures/";
    inline const std::string FONTS_PATH = ASSETS_PATH + "fonts/";

    inline const std::string FILE_CIRCUIT_HD = "circuit.png";
    inline const std::string FILE_CIRCUIT_SD = "circuit_sd.png";
    inline const std::string FILE_MASK_HD = "circuit_mask.png";
    inline const std::string FILE_MASK_SD = "circuit_mask_sd.png";

    // --- GRAPHIQUES & SYSTEME ---
    inline constexpr bool USE_BORDERLESS_FULLSCREEN = true;
    inline constexpr int WINDOW_WIDTH = 1280;
    inline constexpr int WINDOW_HEIGHT = 720;

    // --- PHYSIQUE (FIXE) ---
    inline constexpr float PHYSICS_FPS = 60.0f;
    inline constexpr float TIME_PER_FRAME = 1.0f / PHYSICS_FPS;

    // Alias pour compatibilité avec GhostManager.cpp
    inline constexpr float FPS = PHYSICS_FPS;

    // --- RENDU (VARIABLE) ---
    inline constexpr bool ENABLE_VSYNC = true;
    inline constexpr unsigned int FRAME_LIMIT = 0;
    inline constexpr unsigned int ANTIALIASING_LEVEL = 0;

    // --- CAMERA ---
    inline constexpr float CAMERA_WIDTH = 120.0f;
    inline constexpr float CAMERA_HEIGHT = 67.5f;
    // Restauration de la constante requise par Camera.cpp
    inline constexpr float CAMERA_RESIZE_FACTOR = 10.67f;

    // --- VOITURE ---
    inline constexpr float CAR_SCALE = 0.0075f;
    inline constexpr float CAR_INITIAL_POS_X = 782.082f;
    inline constexpr float CAR_INITIAL_POS_Y = 458.342f;
    inline constexpr float CAR_INITIAL_ROTATION = 145.0f;
    inline constexpr float CAR_MAX_SPEED = 80.0f;

    inline constexpr float CAR_ACCELERATION = 20.0f;
    inline constexpr float CAR_BRAKING = 30.0f;
    inline constexpr float CAR_MAX_TURN_RATE = 90.0f;
    inline constexpr float CAR_FRICTION = 6.0f;
    inline constexpr float CAR_MAX_SPEED_GRASS = 25.0f;
    inline constexpr unsigned int TEXTURE_LIMIT_THRESHOLD = 4096;

    // --- REGLES ---
    inline constexpr int COUNTDOWN_START_VALUE = 3;
    inline constexpr float COUNTDOWN_DURATION = 4.0f;
}

#endif // CONFIG_H