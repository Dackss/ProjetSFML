#ifndef CONFIG_H
#define CONFIG_H

#include <string>

/// @brief Configuration constants
namespace Config {
    /// @name Asset paths
    /// @{
    inline const std::string ASSETS_PATH = "../assets/"; ///< Base assets path
    inline const std::string TEXTURES_PATH = ASSETS_PATH + "textures/"; ///< Textures path
    inline const std::string FONTS_PATH = ASSETS_PATH + "fonts/"; ///< Fonts path
    /// @}

    /// @name Window settings
    /// @{
    inline constexpr int WINDOW_WIDTH = 1280; ///< Window width
    inline constexpr int WINDOW_HEIGHT = 720; ///< Window height
    /// @}

    /// @name Frame rate
    /// @{
    inline constexpr float FPS = 60.0f; ///< Frames per second
    inline constexpr float TIME_PER_FRAME = 1.0f / FPS; ///< Time per frame
    /// @}

    /// @name Camera settings
    /// @{
    inline constexpr float CAMERA_WIDTH = 120.0f; ///< Camera width
    inline constexpr float CAMERA_HEIGHT = 67.5f; ///< Camera height
    inline constexpr float CAMERA_RESIZE_FACTOR = 10.67f; ///< Camera resize factor
    /// @}

    /// @name Car settings
    /// @{
    inline constexpr float CAR_SCALE = 0.0075f; ///< Car scale
    inline constexpr float CAR_INITIAL_POS_X = 782.082f; ///< Car initial X position
    inline constexpr float CAR_INITIAL_POS_Y = 458.342f; ///< Car initial Y position
    inline constexpr float CAR_INITIAL_ROTATION = 145.0f; ///< Car initial rotation
    inline constexpr float CAR_MAX_SPEED = 80.0f; ///< Car max speed (pixels/s)
    inline constexpr float CAR_ACCELERATION = 15.0f; ///< Car acceleration (pixels/s²)
    inline constexpr float CAR_BRAKING = 30.0f; ///< Car braking (pixels/s²)
    inline constexpr float CAR_MAX_TURN_RATE = 90.0f; ///< Car max turn rate (degrees/s)
    inline constexpr float CAR_FRICTION = 6.0f; ///< Car friction (pixels/s²)
    inline constexpr float CAR_MAX_SPEED_GRASS = 25.0f; ///< Car max speed on grass
    /// @}
}

#endif // CONFIG_H