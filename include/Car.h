#ifndef CAR_H
#define CAR_H

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include "CollisionMask.h"
#include <memory> // Nécessaire pour std::unique_ptr

class Car {
public:
    explicit Car(sf::Texture& texture);

    void update(sf::Time deltaTime, const sf::FloatRect& trackBounds, const CollisionMask& mask);

    // Rendu avec interpolation (alpha)
    void render(sf::RenderWindow& window, float alpha = 1.0f);

    sf::Vector2f getPosition() const;
    sf::Vector2f getInterpolatedPosition(float alpha) const;
    void setPosition(const sf::Vector2f& pos);

    const sf::Sprite& getSprite() const;
    void resetVelocity();
    void setRotation(float angle);
    float getRotation() const;
    float getSpeed() const;

    // Configuration audio différée
    void setupAudio(const sf::SoundBuffer& buffer);

private:
    // Helper pour l'interpolation d'angle
    static float lerpAngle(float start, float end, float t);

private:
    sf::Sprite mSprite;
    sf::Vector2f mVelocity;

    // État précédent pour interpolation
    sf::Vector2f mPreviousPosition;
    float mPreviousRotation;

    // SFML 3 : On utilise un pointeur pour pouvoir l'initialiser plus tard
    std::unique_ptr<sf::Sound> mEngineSound;
};

#endif