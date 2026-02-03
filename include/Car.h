#ifndef CAR_H
#define CAR_H

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include "CollisionMask.h"
#include <memory>

/// @brief Structure regroupant les commandes de pilotage
struct CarControls {
    bool accelerate = false;
    bool brake = false;
    bool turnLeft = false;
    bool turnRight = false;
};

class Car {
public:
    explicit Car(sf::Texture& texture);

    // Signature modifiée : on passe les inputs ici
    void update(sf::Time deltaTime, const CarControls& inputs, const sf::FloatRect& trackBounds, const CollisionMask& mask);

    void render(sf::RenderWindow& window, float alpha = 1.0f);

    sf::Vector2f getPosition() const;
    sf::Vector2f getInterpolatedPosition(float alpha) const;
    void setPosition(const sf::Vector2f& pos);

    const sf::Sprite& getSprite() const;
    void resetVelocity();
    void setRotation(float angle);
    float getRotation() const;
    float getSpeed() const;

    void setupAudio(const sf::SoundBuffer& buffer);

private:
    static float lerpAngle(float start, float end, float t);

private:
    sf::Sprite mSprite;
    sf::Vector2f mVelocity;

    sf::Vector2f mPreviousPosition;
    float mPreviousRotation;

    std::unique_ptr<sf::Sound> mEngineSound;
    float mLastPitch = 1.0f;
};

#endif