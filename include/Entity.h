#ifndef ENTITY_H
#define ENTITY_H

#include <SFML/Graphics.hpp>

class Entity {
public:
    virtual ~Entity() = default;

    virtual void update(sf::Time) {}
    virtual void render(sf::RenderWindow& window) = 0;

    virtual sf::Vector2f getPosition() const { return {}; }
    virtual void setPosition(const sf::Vector2f&) {}
};

#endif // ENTITY_H