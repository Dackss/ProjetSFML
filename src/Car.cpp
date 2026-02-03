#include "Car.h"
#include "Config.h"
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Joystick.hpp>
#include <cmath>

Car::Car(sf::Texture& texture)
    : mSprite(texture),  mVelocity(0.f, 0.f), mShadow(texture) // <--- AJOUT DE mShadow(texture)
{
    mSprite.setPosition({Config::CAR_INITIAL_POS_X, Config::CAR_INITIAL_POS_Y});
    mSprite.setRotation(sf::degrees(Config::CAR_INITIAL_ROTATION));
    mSprite.setScale({Config::CAR_SCALE, Config::CAR_SCALE});

    sf::Vector2u textureSize = texture.getSize();
    mSprite.setOrigin({textureSize.x / 2.f, textureSize.y / 2.f});

    // Init previous state
    mPreviousPosition = mSprite.getPosition();
    mPreviousRotation = mSprite.getRotation().asDegrees();

    // Remplacer la configuration de l'ombre par :
    mShadow.setTexture(texture);
    mShadow.setOrigin(mSprite.getOrigin());

    // Une ombre semi-transparente un peu plus sombre
    mShadow.setColor(sf::Color(0, 0, 0, 90));

    // On garde une taille lègèrement supérieure (105%) pour que l'ombre déborde un peu
    float shadowScaleFactor = 1.05f;
    mShadow.setScale({Config::CAR_SCALE * shadowScaleFactor, Config::CAR_SCALE * shadowScaleFactor});

    // La position initiale importe peu car écrasée dans render, mais on peut l'initier
    mShadow.setPosition(mSprite.getPosition());
    mShadow.setRotation(mSprite.getRotation());
}
void Car::setupAudio(const sf::SoundBuffer& buffer) {
    // C'est ici qu'on crée réellement le son avec son buffer (obligatoire en SFML 3)
    mEngineSound = std::make_unique<sf::Sound>(buffer);
    mEngineSound->setLooping(true);
    mEngineSound->setVolume(50.f);
    mEngineSound->play();
}

void Car::update(sf::Time deltaTime, const sf::FloatRect& trackBounds, const CollisionMask& mask) {
    // 1. Sauvegarde pour interpolation (Fluidité)
    mPreviousPosition = mSprite.getPosition();
    mPreviousRotation = mSprite.getRotation().asDegrees();

    float dt = deltaTime.asSeconds();

    // OPTIMISATION MATH : Calcul unique de la vitesse (et de son carré)
    // On utilise speedSq pour les comparaisons quand c'est possible (évite sqrt)
    float speedSq = mVelocity.x * mVelocity.x + mVelocity.y * mVelocity.y;
    float speed = std::sqrt(speedSq);

    float turnFactor = std::min(speed / 20.f, 1.f);

    // --- INPUTS ---
    unsigned int joystickId = 0;
    float joyX = sf::Joystick::getAxisPosition(joystickId, sf::Joystick::Axis::X);
    bool joyLeft = joyX < -40.f;
    bool joyRight = joyX > 40.f;

    // Utilisation de constantes pré-calculées pour la rotation
    float rotationAmount = Config::CAR_MAX_TURN_RATE * dt * turnFactor;

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Q) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left) || joyLeft) {
        mSprite.rotate(sf::degrees(-rotationAmount));
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right) || joyRight) {
        mSprite.rotate(sf::degrees(rotationAmount));
    }

    // Calcul du vecteur avant (Forward)
    // Optimisation possible : stocker le vecteur forward si utilisé ailleurs, mais ici cos/sin est nécessaire après rotation
    float angleRad = mSprite.getRotation().asRadians(); // SFML 3 permet asRadians() direct souvent, sinon asDegrees() * conversion
    sf::Vector2f forward(std::cos(angleRad), std::sin(angleRad));

    bool onGrass = mask.isOnGrass(mSprite.getPosition());
    float accel = onGrass ? Config::CAR_ACCELERATION * 0.4f : Config::CAR_ACCELERATION;

    bool btnAccel = sf::Joystick::isButtonPressed(joystickId, 0);
    bool btnBrake = sf::Joystick::isButtonPressed(joystickId, 1) || sf::Joystick::isButtonPressed(joystickId, 2);

    // Regroupement des inputs booléens
    bool inputUp = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Z) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up) || btnAccel;
    bool inputDown = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down) || btnBrake;

    if (inputUp) {
        mVelocity += forward * accel * dt;
    }
    if (inputDown) {
        mVelocity -= forward * Config::CAR_BRAKING * dt;
    }

    // --- AUDIO DYNAMIQUE OPTIMISÉ ---
    if (mEngineSound) {
        float targetPitch = (speed < 10.f) ? 0.8f : (0.8f + (speed / Config::CAR_MAX_SPEED) * 1.7f);

        // On ne change le pitch que si la différence est audible (> 0.05)
        // Nécessite float mLastPitch dans Car.h, sinon retirez le if
        if (std::abs(mLastPitch - targetPitch) > 0.05f) {
            mEngineSound->setPitch(targetPitch);
            mLastPitch = targetPitch;
        }
    }

    // --- PHYSIQUE ---
    // Recalcul de speedSq uniquement si la vélocité a changé significativement (ici on assume que oui)
    speedSq = mVelocity.x * mVelocity.x + mVelocity.y * mVelocity.y;

    // Optimisation : Comparaison au carré pour éviter sqrt
    float maxSpeedGrassSq = Config::CAR_MAX_SPEED_GRASS * Config::CAR_MAX_SPEED_GRASS;

    if (onGrass && speedSq > maxSpeedGrassSq) {
        mVelocity *= (1.f - dt * 0.5f);

        // Ici on a besoin de la vraie vitesse pour normaliser
        speed = std::sqrt(speedSq);
        if (speed < Config::CAR_MAX_SPEED_GRASS + 1.f) {
            if (speed > 0.1f) // Protection division par zéro
                mVelocity = (mVelocity / speed) * Config::CAR_MAX_SPEED_GRASS;
        }
        // Mise à jour de speedSq après modif
        speedSq = mVelocity.x * mVelocity.x + mVelocity.y * mVelocity.y;
    }

    if (speedSq > 0.01f) { // 0.1f * 0.1f
        float friction = inputUp ? Config::CAR_FRICTION * 0.5f : Config::CAR_FRICTION;
        if (onGrass) friction *= 0.6f;

        // Friction simplifiée sans normalisation coûteuse si vitesse faible ?
        // On garde la logique originale mais on calcule speed une fois
        speed = std::sqrt(speedSq);
        mVelocity -= (mVelocity / speed) * friction * dt;
    }

    speedSq = mVelocity.x * mVelocity.x + mVelocity.y * mVelocity.y;
    float maxSpeedSq = Config::CAR_MAX_SPEED * Config::CAR_MAX_SPEED;

    if (!onGrass && speedSq > maxSpeedSq) {
        // Normalisation rapide
        speed = std::sqrt(speedSq);
        mVelocity = (mVelocity / speed) * Config::CAR_MAX_SPEED;
    }

    // Drift / Adhérence latérale
    float forwardSpeed = mVelocity.x * forward.x + mVelocity.y * forward.y;
    sf::Vector2f lateral = mVelocity - forward * forwardSpeed;

    // speed est à jour approximativement (suffisant pour le drift factor)
    float drift = 0.7f - std::min(speed / 100.f, 0.4f);
    mVelocity = forward * forwardSpeed + lateral * drift;

    sf::Vector2f newPos = mSprite.getPosition() + mVelocity * dt;

    // Bornes du circuit
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
    // 1. Calculs d'interpolation (comme avant)
    sf::Vector2f currentPos = mSprite.getPosition();
    sf::Vector2f interpPos = mPreviousPosition * (1.f - alpha) + currentPos * alpha;

    float currentRot = mSprite.getRotation().asDegrees();
    float interpRot = lerpAngle(mPreviousRotation, currentRot, alpha);

    // 2. RENDU DE L'OMBRE AVEC DÉCALAGE
    // On définit la direction de la lumière (ici, ombre vers le bas-droite)
    sf::Vector2f shadowOffset(6.f, 6.f);

    mShadow.setPosition(interpPos + shadowOffset);
    mShadow.setRotation(sf::degrees(interpRot)); // L'ombre tourne avec la voiture

    window.draw(mShadow);

    // 3. RENDU DE LA VOITURE (Au dessus)
    mSprite.setPosition(interpPos);
    mSprite.setRotation(sf::degrees(interpRot));

    window.draw(mSprite);

    // 4. Restauration pour la physique
    mSprite.setPosition(currentPos);
    mSprite.setRotation(sf::degrees(currentRot));
}

sf::Vector2f Car::getPosition() const { return mSprite.getPosition(); }
void Car::setPosition(const sf::Vector2f& pos) {
    mSprite.setPosition(pos);
    mPreviousPosition = pos;
}
const sf::Sprite& Car::getSprite() const { return mSprite; }
void Car::resetVelocity() { mVelocity = {0.f, 0.f}; }
void Car::setRotation(float angle) {
    mSprite.setRotation(sf::degrees(angle));
    mPreviousRotation = angle;
}
float Car::getRotation() const { return mSprite.getRotation().asDegrees(); }
float Car::getSpeed() const { return std::sqrt(mVelocity.x * mVelocity.x + mVelocity.y * mVelocity.y); }