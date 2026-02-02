#ifndef COLLISIONMASK_H
#define COLLISIONMASK_H

#include <SFML/Graphics.hpp>
#include <string>

class CollisionMask {
public:
    CollisionMask();

    bool loadFromFile(const std::string& path);
    void setScale(float scale);

    bool isOnGrass(sf::Vector2f worldPos) const;
    bool isOnGreen(sf::Vector2f worldPos) const;
    bool isOnBlue(sf::Vector2f worldPos) const;
    bool isTraversable(sf::Vector2f worldPos) const;

private:
    sf::Image mImage;
    float mScale;

    // NOUVEAU : Stocke le ratio de conversion (ex: 0.5 sur Pi, 1.0 sur PC)
    sf::Vector2f mRatio;

    // Helper pour convertir la position
    sf::Vector2u worldToImage(sf::Vector2f pos) const;
};

#endif // COLLISIONMASK_H