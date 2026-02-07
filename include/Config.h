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

    // --- GRAPHIQUES & PERFORMANCES (FIX WSL) ---

    // Si true : Utilise le mode "Faux Plein Écran" (Résolution native + Sans bordure).
    // C'est la SEULE façon d'avoir un jeu fluide sous WSL/Laptop.
    inline constexpr bool USE_BORDERLESS_FULLSCREEN = true;

    // Résolution fenêtre classique
    inline constexpr int WINDOW_WIDTH = 1280;
    inline constexpr int WINDOW_HEIGHT = 720;

    // Réglages de fluidité
    // Note : Sous WSL, la VSync peut échouer, mais le Borderless compensera via Windows DWM.
    inline constexpr bool ENABLE_VSYNC = true;
    inline constexpr unsigned int FRAME_LIMIT = 165; // 60 FPS est impératif pour ne pas surchauffer en résolution native
    inline constexpr unsigned int ANTIALIASING_LEVEL = 0;

    // Physique
    inline constexpr float FPS = 165.0f;
    inline constexpr float TIME_PER_FRAME = 1.0f / FPS;

    inline constexpr unsigned int RENDER_FPS = 144;

    // --- CAMERA ---
    inline constexpr float CAMERA_WIDTH = 120.0f;
    inline constexpr float CAMERA_HEIGHT = 67.5f;
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

    // --- REGLES DU JEU ---
    inline constexpr int COUNTDOWN_START_VALUE = 3;
    inline constexpr float COUNTDOWN_DURATION = 4.0f;
}

#endif // CONFIG_H