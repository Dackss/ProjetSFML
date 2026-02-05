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
    // 1. Sauvegarde pour interpolation (Toujours au début)
    mPreviousPosition = mSprite.getPosition();
    mPreviousRotation = mSprite.getRotation().asDegrees();

    float dt = deltaTime.asSeconds();

    // --- 2. INPUTS ROTATION ---
    // On calcule la vitesse au carré pour éviter un sqrt immédiat
    float speedSq = mVelocity.x * mVelocity.x + mVelocity.y * mVelocity.y;

    // Facteur de rotation basé sur une approximation (évite sqrt ici si possible, sinon on le fait une fois)
    // Astuce : Pour turnFactor, on a besoin de la vitesse réelle (magnitude)
    float speed = std::sqrt(speedSq);

    float turnFactor = std::min(speed / 20.f, 1.f);
    float rotationAmount = Config::CAR_MAX_TURN_RATE * dt * turnFactor;

    if (inputs.turnLeft)  mSprite.rotate(sf::degrees(-rotationAmount));
    if (inputs.turnRight) mSprite.rotate(sf::degrees(rotationAmount));

    // Vecteur direction (Forward)
    float angleRad = mSprite.getRotation().asRadians();
    sf::Vector2f forward(std::cos(angleRad), std::sin(angleRad));

    // --- 3. ACCELERATION ---
    bool onGrass = mask.isOnGrass(mSprite.getPosition());
    float accel = onGrass ? Config::CAR_ACCELERATION * 0.4f : Config::CAR_ACCELERATION;

    if (inputs.accelerate) mVelocity += forward * accel * dt;
    if (inputs.brake)      mVelocity -= forward * Config::CAR_BRAKING * dt;

    // --- 4. PHYSIQUE (Friction & Limites) ---
    // On met à jour speedSq après l'accélération
    speedSq = mVelocity.x * mVelocity.x + mVelocity.y * mVelocity.y;

    // Optimisation : On ne recalcul speed (sqrt) que si nécessaire pour normaliser
    // Seuil minimal pour éviter les divisions par zéro et l'instabilité à l'arrêt
    if (speedSq > 0.1f) {
        // Friction
        float friction = inputs.accelerate ? Config::CAR_FRICTION * 0.5f : Config::CAR_FRICTION;
        if (onGrass) friction *= 0.6f;

        // Formule d'amortissement sans sqrt: V_new = V_old * (1 - friction * dt)
        // C'est une approximation valide pour dt petit et évite de normaliser le vecteur
        float frictionFactor = 1.f - (friction / std::sqrt(speedSq)) * dt;
        if (frictionFactor < 0.f) frictionFactor = 0.f; // Évite l'inversion de mouvement
        mVelocity *= frictionFactor;

        // Recalcul après friction
        speedSq = mVelocity.x * mVelocity.x + mVelocity.y * mVelocity.y;
    }

    // Gestion Herbe & Max Speed
    float maxSpeed = onGrass ? Config::CAR_MAX_SPEED_GRASS : Config::CAR_MAX_SPEED;
    float maxSpeedSq = maxSpeed * maxSpeed;

    if (speedSq > maxSpeedSq) {
        // Normalisation (Coûteux mais nécessaire ici)
        float currentSpeed = std::sqrt(speedSq);
        mVelocity = (mVelocity / currentSpeed) * maxSpeed;
        speedSq = maxSpeedSq; // La vitesse est maintenant capée
    }

    // --- 5. DRIFT (Dérapage) ---
    // On a besoin de la vitesse réelle finale pour le drift
    speed = std::sqrt(speedSq); // Mise à jour finale de speed pour le drift et l'audio

    float forwardSpeed = mVelocity.x * forward.x + mVelocity.y * forward.y;
    sf::Vector2f lateral = mVelocity - forward * forwardSpeed;

    // Plus on va vite, moins on drift (adhérence) ? Ou l'inverse ?
    // Votre formule: 0.7 - (speed/100). Si speed=0, drift=0.7 (bcp de glisse latérale).
    // Si speed=100, drift=0.3 (peu de glisse). C'est un peu contre-intuitif (souvent on glisse plus à haute vitesse)
    // Mais gardons votre logique pour ne pas casser le gameplay.
    float driftFactor = 0.7f - std::min(speed / 100.f, 0.4f);
    mVelocity = forward * forwardSpeed + lateral * driftFactor;

    // --- 6. AUDIO ---
    if (mEngineSound) {
        float targetPitch = (speed < 10.f) ? 0.8f : (0.8f + (speed / Config::CAR_MAX_SPEED) * 1.7f);
        if (std::abs(mLastPitch - targetPitch) > 0.05f) {
            mEngineSound->setPitch(targetPitch);
            mLastPitch = targetPitch;
        }
    }

    // --- 7. MOUVEMENT & COLLISION ---
    sf::Vector2f nextPos = mSprite.getPosition() + mVelocity * dt;

    // Clamp aux limites du circuit (Bounding Box)
    // Utilisation de std::clamp (C++17) pour plus de propreté
    // Attention: trackBounds.width/height vs size.x/size.y selon SFML version
    sf::FloatRect bounds = mSprite.getGlobalBounds();
    float halfW = bounds.size.x / 2.f;
    float halfH = bounds.size.y / 2.f;

    nextPos.x = std::clamp(nextPos.x, trackBounds.position.x + halfW, trackBounds.position.x + trackBounds.size.x - halfW);
    nextPos.y = std::clamp(nextPos.y, trackBounds.position.y + halfH, trackBounds.position.y + trackBounds.size.y - halfH);

    // Vérification Masque de Collision
    if (mask.isTraversable(nextPos)) {
        mSprite.setPosition(nextPos);
    } else {
        // COLLISION MUR DETECTÉE
        // Option A (Simple) : On arrête tout (votre code actuel)
        // mVelocity = {0.f, 0.f};

        // Option B (Mieux) : On rebondit un peu et on perd de la vitesse
        mVelocity = -mVelocity * 0.3f; // Rebond inversé à 30% de la vitesse

        // On ne déplace PAS le sprite, il reste à mPreviousPosition (virtuellement)
        // Cela empêche de traverser le mur
    }
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