#include "Car.h"
#include "Config.h"
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Joystick.hpp>
#include <cmath>

Car::Car(sf::Texture& texture)
    : mSprite(texture), mVelocity(0.f, 0.f)
{
    mSprite.setPosition({Config::CAR_INITIAL_POS_X, Config::CAR_INITIAL_POS_Y});
    mSprite.setRotation(sf::degrees(Config::CAR_INITIAL_ROTATION));
    mSprite.setScale({Config::CAR_SCALE, Config::CAR_SCALE});

    sf::Vector2u textureSize = texture.getSize();
    mSprite.setOrigin({textureSize.x / 2.f, textureSize.y / 2.f});

    // Init previous state
    mPreviousPosition = mSprite.getPosition();
    mPreviousRotation = mSprite.getRotation().asDegrees();

    // Note: mEngineSound est un unique_ptr, il est nullptr par défaut (ce qui est valide)
}

void Car::setupAudio(const sf::SoundBuffer& buffer) {
    // C'est ici qu'on crée réellement le son avec son buffer (obligatoire en SFML 3)
    mEngineSound = std::make_unique<sf::Sound>(buffer);
    mEngineSound->setLoop(true);
    mEngineSound->setVolume(50.f);
    mEngineSound->play();
}

void Car::update(sf::Time deltaTime, const sf::FloatRect& trackBounds, const CollisionMask& mask) {
    // 1. Sauvegarde pour interpolation
    mPreviousPosition = mSprite.getPosition();
    mPreviousRotation = mSprite.getRotation().asDegrees();

    float dt = deltaTime.asSeconds();
    float speed = std::sqrt(mVelocity.x * mVelocity.x + mVelocity.y * mVelocity.y);
    float turnFactor = std::min(speed / 20.f, 1.f);

    // --- INPUTS ---
    unsigned int joystickId = 0;
    float joyX = sf::Joystick::getAxisPosition(joystickId, sf::Joystick::Axis::X);
    bool joyLeft = joyX < -40.f;
    bool joyRight = joyX > 40.f;

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Q) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left) || joyLeft) {
        mSprite.rotate(sf::degrees(-Config::CAR_MAX_TURN_RATE * dt * turnFactor));
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right) || joyRight) {
        mSprite.rotate(sf::degrees(Config::CAR_MAX_TURN_RATE * dt * turnFactor));
    }

    float angleDeg = mSprite.getRotation().asDegrees();
    float angleRad = angleDeg * 3.14159265359f / 180.f;
    sf::Vector2f forward(std::cos(angleRad), std::sin(angleRad));

    bool onGrass = mask.isOnGrass(mSprite.getPosition());
    float accel = onGrass ? Config::CAR_ACCELERATION * 0.4f : Config::CAR_ACCELERATION;

    bool btnAccel = sf::Joystick::isButtonPressed(joystickId, 0);
    bool btnBrake = sf::Joystick::isButtonPressed(joystickId, 1) || sf::Joystick::isButtonPressed(joystickId, 2);

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Z) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up) || btnAccel) {
        mVelocity += forward * accel * dt;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down) || btnBrake) {
        mVelocity -= forward * Config::CAR_BRAKING * dt;
    }

    // --- AUDIO DYNAMIQUE ---
    // On vérifie si le son est initialisé avant de l'utiliser
    if (mEngineSound) {
        if (speed < 10.f) {
            mEngineSound->setPitch(0.8f);
        } else {
            mEngineSound->setPitch(0.8f + (speed / Config::CAR_MAX_SPEED) * 1.7f);
        }
    }

    // --- PHYSIQUE ---
    speed = std::sqrt(mVelocity.x * mVelocity.x + mVelocity.y * mVelocity.y);

    if (onGrass && speed > Config::CAR_MAX_SPEED_GRASS) {
        mVelocity *= (1.f - dt * 0.5f);
        speed = std::sqrt(mVelocity.x * mVelocity.x + mVelocity.y * mVelocity.y);
        if (speed < Config::CAR_MAX_SPEED_GRASS + 1.f) {
            mVelocity = (mVelocity / speed) * Config::CAR_MAX_SPEED_GRASS;
        }
    }

    if (speed > 0.1f) {
        bool isAccelerating = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Z) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up) || btnAccel;
        float friction = isAccelerating ? Config::CAR_FRICTION * 0.5f : Config::CAR_FRICTION;
        if (onGrass) friction *= 0.6f;
        mVelocity -= (mVelocity / speed) * friction * dt;
    }

    if (!onGrass && speed > Config::CAR_MAX_SPEED) {
        mVelocity = (mVelocity / speed) * Config::CAR_MAX_SPEED;
    }

    float forwardSpeed = mVelocity.x * forward.x + mVelocity.y * forward.y;
    sf::Vector2f lateral = mVelocity - forward * forwardSpeed;
    float drift = 0.7f - std::min(speed / 100.f, 0.4f);
    mVelocity = forward * forwardSpeed + lateral * drift;

    sf::Vector2f newPos = mSprite.getPosition() + mVelocity * dt;

    sf::FloatRect bounds = mSprite.getGlobalBounds();
    float w = bounds.size.x / 2.f;
    float h = bounds.size.y / 2.f;
    newPos.x = std::max(trackBounds.position.x + w, std::min(newPos.x, trackBounds.position.x + trackBounds.size.x - w));
    newPos.y = std::max(trackBounds.position.y + h, std::min(newPos.y, trackBounds.position.y + trackBounds.size.y - h));

    if (!mask.isTraversable(newPos)) {
        mVelocity = {0.f, 0.f};
        return;
    }

    mSprite.setPosition(newPos);
}

// Implémentation de la méthode statique
float Car::lerpAngle(float start, float end, float t) {
    float diff = end - start;
    while (diff < -180.f) diff += 360.f;
    while (diff > 180.f) diff -= 360.f;
    return start + diff * t;
}

sf::Vector2f Car::getInterpolatedPosition(float alpha) const {
    sf::Vector2f currentPos = mSprite.getPosition();
    return mPreviousPosition * (1.f - alpha) + currentPos * alpha;
}

void Car::render(sf::RenderWindow& window, float alpha) {
    // 1. Sauvegarde État Réel
    sf::Vector2f realPos = mSprite.getPosition();
    float realRot = mSprite.getRotation().asDegrees();

    // 2. Calcul État Visuel Interpolé
    sf::Vector2f interpPos = getInterpolatedPosition(alpha);
    float interpRot = lerpAngle(mPreviousRotation, realRot, alpha);

    // 3. Dessin
    mSprite.setPosition(interpPos);
    mSprite.setRotation(sf::degrees(interpRot));

    window.draw(mSprite);

    // 4. Restauration État Réel
    mSprite.setPosition(realPos);
    mSprite.setRotation(sf::degrees(realRot));
}

sf::Vector2f Car::getPosition() const { return mSprite.getPosition(); }
void Car::setPosition(const sf::Vector2f& pos) { mSprite.setPosition(pos); }
const sf::Sprite& Car::getSprite() const { return mSprite; }
void Car::resetVelocity() { mVelocity = {0.f, 0.f}; }
void Car::setRotation(float angle) { mSprite.setRotation(sf::degrees(angle)); }
float Car::getRotation() const { return mSprite.getRotation().asDegrees(); }
float Car::getSpeed() const { return std::sqrt(mVelocity.x * mVelocity.x + mVelocity.y * mVelocity.y); }