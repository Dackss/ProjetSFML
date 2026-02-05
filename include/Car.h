#ifndef CAR_H
#define CAR_H

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include "CollisionMask.h"
#include <memory>

struct CarControls {
    bool accelerate = false;
    bool brake = false;
    bool turnLeft = false;
    bool turnRight = false;
};

class Car {
public:
    explicit Car(sf::Texture& texture);

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

    // --- Nouvelles méthodes internes pour découper update() ---
    void processSteering(float dt, const CarControls& inputs, float currentSpeed);
    void processPhysics(float dt, const CarControls& inputs, const sf::Vector2f& forwardVector, bool onGrass);
    void applyDrift(const sf::Vector2f& forwardVector, float currentSpeed);
    void resolveCollisions(float dt, const sf::Vector2f& forwardVector, const CollisionMask& mask);
    void updateAudioPitch(float currentSpeed);

private:
    sf::Sprite mSprite;
    sf::Vector2f mVelocity;

    sf::Vector2f mPreviousPosition;
    float mPreviousRotation;

    std::unique_ptr<sf::Sound> mEngineSound;
    float mLastPitch = 1.0f;

    float mCurrentSteer = 0.0f;
    float mGrassIntensity = 0.0f;
};

#endif