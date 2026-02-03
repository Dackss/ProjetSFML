#include "Car.h"
#include "Config.h"
#include <cmath>
#include <algorithm> // pour std::min, std::max

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
}

void Car::setupAudio(const sf::SoundBuffer& buffer) {
    mEngineSound = std::make_unique<sf::Sound>(buffer);
    mEngineSound->setLooping(true);
    mEngineSound->setVolume(50.f);
    mEngineSound->play();
}

void Car::update(sf::Time deltaTime, const CarControls& inputs, const sf::FloatRect& trackBounds, const CollisionMask& mask) {
    // 1. Sauvegarde pour interpolation
    mPreviousPosition = mSprite.getPosition();
    mPreviousRotation = mSprite.getRotation().asDegrees();

    float dt = deltaTime.asSeconds();
    float speedSq = mVelocity.x * mVelocity.x + mVelocity.y * mVelocity.y;
    float speed = std::sqrt(speedSq);

    float turnFactor = std::min(speed / 20.f, 1.f);
    float rotationAmount = Config::CAR_MAX_TURN_RATE * dt * turnFactor;

    // --- INPUTS ---
    if (inputs.turnLeft) {
        mSprite.rotate(sf::degrees(-rotationAmount));
    }
    if (inputs.turnRight) {
        mSprite.rotate(sf::degrees(rotationAmount));
    }

    float angleRad = mSprite.getRotation().asRadians();
    sf::Vector2f forward(std::cos(angleRad), std::sin(angleRad));

    bool onGrass = mask.isOnGrass(mSprite.getPosition());
    float accel = onGrass ? Config::CAR_ACCELERATION * 0.4f : Config::CAR_ACCELERATION;

    if (inputs.accelerate) {
        mVelocity += forward * accel * dt;
    }
    if (inputs.brake) {
        mVelocity -= forward * Config::CAR_BRAKING * dt;
    }

    // --- AUDIO ---
    if (mEngineSound) {
        float targetPitch = (speed < 10.f) ? 0.8f : (0.8f + (speed / Config::CAR_MAX_SPEED) * 1.7f);
        if (std::abs(mLastPitch - targetPitch) > 0.05f) {
            mEngineSound->setPitch(targetPitch);
            mLastPitch = targetPitch;
        }
    }

    // --- PHYSIQUE ---
    speedSq = mVelocity.x * mVelocity.x + mVelocity.y * mVelocity.y;
    float maxSpeedGrassSq = Config::CAR_MAX_SPEED_GRASS * Config::CAR_MAX_SPEED_GRASS;

    if (onGrass && speedSq > maxSpeedGrassSq) {
        mVelocity *= (1.f - dt * 0.5f);
        speed = std::sqrt(speedSq);
        if (speed < Config::CAR_MAX_SPEED_GRASS + 1.f && speed > 0.1f) {
             mVelocity = (mVelocity / speed) * Config::CAR_MAX_SPEED_GRASS;
        }
        speedSq = mVelocity.x * mVelocity.x + mVelocity.y * mVelocity.y;
    }

    if (speedSq > 0.01f) {
        float friction = inputs.accelerate ? Config::CAR_FRICTION * 0.5f : Config::CAR_FRICTION;
        if (onGrass) friction *= 0.6f;
        speed = std::sqrt(speedSq);
        mVelocity -= (mVelocity / speed) * friction * dt;
    }

    speedSq = mVelocity.x * mVelocity.x + mVelocity.y * mVelocity.y;
    float maxSpeedSq = Config::CAR_MAX_SPEED * Config::CAR_MAX_SPEED;

    if (!onGrass && speedSq > maxSpeedSq) {
        speed = std::sqrt(speedSq);
        mVelocity = (mVelocity / speed) * Config::CAR_MAX_SPEED;
    }

    // Drift
    float forwardSpeed = mVelocity.x * forward.x + mVelocity.y * forward.y;
    sf::Vector2f lateral = mVelocity - forward * forwardSpeed;
    float drift = 0.7f - std::min(speed / 100.f, 0.4f);
    mVelocity = forward * forwardSpeed + lateral * drift;

    // Position
    sf::Vector2f newPos = mSprite.getPosition() + mVelocity * dt;
    sf::FloatRect bounds = mSprite.getGlobalBounds();
    float w = bounds.size.x / 2.f;
    float h = bounds.size.y / 2.f;

    // Clamp
    newPos.x = std::max(trackBounds.position.x + w, std::min(newPos.x, trackBounds.position.x + trackBounds.size.x - w));
    newPos.y = std::max(trackBounds.position.y + h, std::min(newPos.y, trackBounds.position.y + trackBounds.size.y - h));

    if (!mask.isTraversable(newPos)) {
        mVelocity = {0.f, 0.f};
        return;
    }

    mSprite.setPosition(newPos);
}

// C'est cette méthode qui manquait !
void Car::render(sf::RenderWindow& window, float alpha) {
    // Calculs d'interpolation
    sf::Vector2f currentPos = mSprite.getPosition();
    sf::Vector2f interpPos = mPreviousPosition * (1.f - alpha) + currentPos * alpha;

    float currentRot = mSprite.getRotation().asDegrees();
    float interpRot = lerpAngle(mPreviousRotation, currentRot, alpha);

    // Rendu
    mSprite.setPosition(interpPos);
    mSprite.setRotation(sf::degrees(interpRot));

    window.draw(mSprite);

    // Restauration pour la physique
    mSprite.setPosition(currentPos);
    mSprite.setRotation(sf::degrees(currentRot));
}

float Car::lerpAngle(float start, float end, float t) {
    float diff = end - start;
    while (diff < -180.f) diff += 360.f;
    while (diff > 180.f) diff -= 360.f;
    return start + diff * t;
}

sf::Vector2f Car::getPosition() const { return mSprite.getPosition(); }
sf::Vector2f Car::getInterpolatedPosition(float alpha) const { return mPreviousPosition * (1.f - alpha) + mSprite.getPosition() * alpha; }
void Car::setPosition(const sf::Vector2f& pos) { mSprite.setPosition(pos); mPreviousPosition = pos; }
const sf::Sprite& Car::getSprite() const { return mSprite; }
void Car::resetVelocity() { mVelocity = {0.f, 0.f}; }
void Car::setRotation(float angle) { mSprite.setRotation(sf::degrees(angle)); mPreviousRotation = angle; }
float Car::getRotation() const { return mSprite.getRotation().asDegrees(); }
float Car::getSpeed() const { return std::sqrt(mVelocity.x * mVelocity.x + mVelocity.y * mVelocity.y); }