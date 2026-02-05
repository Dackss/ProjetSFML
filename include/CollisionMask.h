#ifndef COLLISIONMASK_H
#define COLLISIONMASK_H

#include <SFML/Graphics.hpp>
#include <string>
#include <cstdint> // <--- IMPORTANT : Nécessaire pour std::uint8_t

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
    sf::Vector2u worldToImage(sf::Vector2f pos) const;
    sf::Image mImage;
    float mScale;

    const std::uint8_t* mPixelData = nullptr;
    sf::Vector2u mSize;
};

#endif // COLLISIONMASK_H