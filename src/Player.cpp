#include "Player.h"
#include "Config.h"
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Joystick.hpp>

Player::Player(sf::Texture& carTexture)
    : mCar(carTexture), mDistance(0.f), mLap(0) {}

void Player::update(sf::Time deltaTime, const sf::FloatRect& bounds, const CollisionMask& mask) {
    // 1. Lire les entrées (Clavier / Manette)
    CarControls inputs;
    unsigned int joystickId = 0;

    // Axe X du joystick pour tourner
    float joyX = sf::Joystick::getAxisPosition(joystickId, sf::Joystick::Axis::X);
    bool joyLeft = joyX < -40.f;
    bool joyRight = joyX > 40.f;

    // Boutons pour accélérer/freiner (A = 0, B = 1, X = 2 sur manette Xbox générique)
    bool btnAccel = sf::Joystick::isButtonPressed(joystickId, 0);
    bool btnBrake = sf::Joystick::isButtonPressed(joystickId, 1) || sf::Joystick::isButtonPressed(joystickId, 2);

    // Mapping des touches
    inputs.turnLeft = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Q) ||
                      sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left) ||
                      joyLeft;

    inputs.turnRight = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D) ||
                       sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right) ||
                       joyRight;

    inputs.accelerate = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Z) ||
                        sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up) ||
                        btnAccel;

    inputs.brake = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S) ||
                   sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down) ||
                   btnBrake;

    // 2. Envoyer les commandes à la voiture
    mCar.update(deltaTime, inputs, bounds, mask);

    // 3. Mise à jour stats
    mDistance += mCar.getSpeed() * deltaTime.asSeconds();
}

// Le reste reste inchangé
void Player::render(sf::RenderWindow& window, float alpha) {
    mCar.render(window, alpha);
}
void Player::reset() {
    mCar.setPosition({Config::CAR_INITIAL_POS_X, Config::CAR_INITIAL_POS_Y});
    mCar.setRotation(Config::CAR_INITIAL_ROTATION);
    mCar.resetVelocity();
    mDistance = 0.f;
    mLap = 0;
}
void Player::startClock() { mClock.restart(); }
Car& Player::getCar() { return mCar; }
float Player::getDistance() const { return mDistance; }
int Player::getLap() const { return mLap; }
sf::Time Player::getElapsedTime() const { return mClock.getElapsedTime(); }
const Car& Player::getCar() const {
    return mCar;
}