#ifndef CONFIG_H
#define CONFIG_H

#include <string>

/// @brief Configuration constants
namespace Config {
    inline const std::string ASSETS_PATH = "../assets/";
    inline const std::string TEXTURES_PATH = ASSETS_PATH + "textures/";
    inline const std::string FONTS_PATH = ASSETS_PATH + "fonts/";

    inline constexpr int WINDOW_WIDTH = 1280;
    inline constexpr int WINDOW_HEIGHT = 720;

    inline constexpr float FPS = 60.0f;
    inline constexpr float TIME_PER_FRAME = 1.0f / FPS;

    inline constexpr float CAMERA_WIDTH = 120.0f;
    inline constexpr float CAMERA_HEIGHT = 67.5f;
    inline constexpr float CAMERA_RESIZE_FACTOR = 10.67f;

    inline constexpr float CAR_SCALE = 0.0075f;
    inline constexpr float CAR_INITIAL_POS_X = 782.082f;
    inline constexpr float CAR_INITIAL_POS_Y = 458.342f;
    inline constexpr float CAR_INITIAL_ROTATION = 145.0f;
    inline constexpr float CAR_MAX_SPEED = 80.0f;
    inline constexpr float CAR_ACCELERATION = 15.0f;
    inline constexpr float CAR_BRAKING = 30.0f;
    inline constexpr float CAR_MAX_TURN_RATE = 90.0f;
    inline constexpr float CAR_FRICTION = 6.0f;
    inline constexpr float CAR_MAX_SPEED_GRASS = 25.0f;

    /// @name Game Rules
    /// @{
    inline constexpr int COUNTDOWN_START_VALUE = 3;
    inline constexpr float COUNTDOWN_DURATION = 4.0f;
    /// @}
}

#endif // CONFIG_H