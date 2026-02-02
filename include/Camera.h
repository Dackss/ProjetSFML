#ifndef CAMERA_H
#define CAMERA_H

#include <SFML/Graphics.hpp>

/**
 * @brief Camera that follows the player within track bounds
 */
class Camera {
public:
    /**
     * @brief Initialize camera with a given size
     * @param width View width
     * @param height View height
     */
    Camera(float width, float height);

    /**
     * @brief Update view center on car while clamping to track bounds
     * @param view Reference to the view to update
     * @param carPosition Position of the car
     * @param trackSize Size of the track
     */
    void update(sf::View& view, sf::Vector2f carPosition, sf::Vector2f trackSize) const;

    /**
     * @brief Resize camera dimensions on window resize
     * @param resized Resize event from SFML
     */
    void handleResize(const sf::Event::Resized& resized);

private:
    float mWidth;  ///< Current view width
    float mHeight; ///< Current view height
};

#endif // CAMERA_H
