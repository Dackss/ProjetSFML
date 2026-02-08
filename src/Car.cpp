#include "Car.h"
#include "Config.h"
#include <cmath>
#include <algorithm>

Car::Car(sf::Texture& texture)
    : mSprite(texture), mVelocity(0.f, 0.f), mCurrentSteer(0.f), mGrassIntensity(0.f)
{
    mSprite.setPosition({Config::CAR_INITIAL_POS_X, Config::CAR_INITIAL_POS_Y});
    mSprite.setRotation(sf::degrees(Config::CAR_INITIAL_ROTATION));
    mSprite.setScale({Config::CAR_SCALE, Config::CAR_SCALE});

    sf::Vector2u textureSize = texture.getSize();
    mSprite.setOrigin({textureSize.x / 2.f, textureSize.y / 2.f});

    mPreviousPosition = mSprite.getPosition();
    mPreviousRotation = mSprite.getRotation().asDegrees();
}

void Car::setupAudio(const sf::SoundBuffer& buffer) {
    mEngineSound = std::make_unique<sf::Sound>(buffer);
    mEngineSound->setLooping(true);
    mEngineSound->setVolume(50.f);
    mEngineSound->play();
}

// -----------------------------------------------------------------------
// Méthode UPDATE optimisée et découpée
// -----------------------------------------------------------------------
void Car::update(sf::Time deltaTime, const CarControls& inputs, const sf::FloatRect& trackBounds, const CollisionMask& mask) {
    // 1. Sauvegarde état précédent pour interpolation
    (void)trackBounds;
    mPreviousPosition = mSprite.getPosition();
    mPreviousRotation = mSprite.getRotation().asDegrees();

    float dt = deltaTime.asSeconds();

    // Calculs préliminaires (cache)
    // On calcule la vitesse une fois ici, elle sera mise à jour implicitement par la physique,
    // mais pour le steering, la vitesse du début de frame suffit souvent.
    float speedSq = mVelocity.x * mVelocity.x + mVelocity.y * mVelocity.y;
    float currentSpeed = std::sqrt(speedSq);

    // Vérification du terrain une seule fois pour cette frame
    bool onGrass = mask.isOnGrass(mSprite.getPosition());

    // 2. Gestion de la direction (Rotation)
    processSteering(dt, inputs, currentSpeed);

    // Recalcul du vecteur Forward après rotation
    float angleRad = mSprite.getRotation().asRadians();
    sf::Vector2f forward(std::cos(angleRad), std::sin(angleRad));

    // 3. Gestion Accélération, Friction et Vitesse Max
    processPhysics(dt, inputs, forward, onGrass);

    // Recalcul de la vitesse réelle après accélération/friction pour le drift et l'audio
    speedSq = mVelocity.x * mVelocity.x + mVelocity.y * mVelocity.y;
    currentSpeed = std::sqrt(speedSq);

    // 4. Gestion du Drift (Dérapage)
    applyDrift(forward, currentSpeed);

    // 5. Mise à jour de l'audio
    updateAudioPitch(currentSpeed);

    // 6. Mouvement final et résolution des collisions
    resolveCollisions(dt, forward, mask);
}

// -----------------------------------------------------------------------
// Sous-méthodes d'implémentation
// -----------------------------------------------------------------------

void Car::processSteering(float dt, const CarControls& inputs, float currentSpeed) {
    // 1. Détermination de la cible de direction (-1 = Gauche, 1 = Droite, 0 = Tout droit)
    float targetSteer = 0.f;
    if (inputs.turnLeft)  targetSteer = -1.f;
    if (inputs.turnRight) targetSteer = 1.f;

    // 2. Interpolation vers la cible (Braquage progressif)
    // Vitesse de braquage : 5.0f signifie qu'il faut 0.2 seconde pour braquer à fond
    float steerSpeed = 5.0f * dt;

    if (mCurrentSteer < targetSteer) {
        mCurrentSteer = std::min(mCurrentSteer + steerSpeed, targetSteer);
    } else if (mCurrentSteer > targetSteer) {
        mCurrentSteer = std::max(mCurrentSteer - steerSpeed, targetSteer);
    }

    // 3. Application de la rotation
    // Le taux de rotation dépend maintenant de l'angle de braquage réel (mCurrentSteer)
    if (std::abs(mCurrentSteer) > 0.01f) {
        float turnFactor = std::min(currentSpeed / 20.f, 1.f);

        // On multiplie par mCurrentSteer pour avoir la direction et l'intensité
        float rotationAmount = Config::CAR_MAX_TURN_RATE * dt * turnFactor * mCurrentSteer;

        mSprite.rotate(sf::degrees(rotationAmount));
    }
}

void Car::processPhysics(float dt, const CarControls& inputs, const sf::Vector2f& forward, bool onGrass) {
    // 1. Mise à jour progressive de l'intensité (Transition douce 1.5s)
    if (onGrass) {
        mGrassIntensity = std::min(mGrassIntensity + dt * 0.8f, 1.0f);
    } else {
        mGrassIntensity = std::max(mGrassIntensity - dt * 2.0f, 0.0f);
    }

    // --- A. ACCELERATION ---
    float baseAccel = Config::CAR_ACCELERATION * 1.1f;
    float accelPower = baseAccel;

    // Perte de puissance progressive (max 10% de perte)
    float grassPowerFactor = 1.0f - (0.1f * mGrassIntensity);
    accelPower *= grassPowerFactor;

    if (inputs.accelerate) {
        float steerFactor = std::abs(mCurrentSteer);
        accelPower *= (1.0f - (0.05f * steerFactor));
        mVelocity += forward * accelPower * dt;
    }

    // --- B. FREINAGE ---
    if (inputs.brake) {
        sf::Vector2f velocityDir = mVelocity;
        float speed = std::sqrt(mVelocity.x * mVelocity.x + mVelocity.y * mVelocity.y);

        if (speed > 0.1f) {
            velocityDir /= speed;
            mVelocity -= velocityDir * (Config::CAR_BRAKING * 2.0f) * dt;
        } else {
            mVelocity -= forward * (Config::CAR_ACCELERATION * 0.5f) * dt;
        }
    }

    // --- C. FRICTION & RESISTANCE (La clé de la souplesse) ---
    float currentSpeed = std::sqrt(mVelocity.x * mVelocity.x + mVelocity.y * mVelocity.y);

    if (currentSpeed > 0.0f) {
        float rollingResistance = Config::CAR_FRICTION; // Base

        // Friction progressive (plus faible qu'avant pour éviter le mur)
        // Mais on compense par le Drag (voir plus bas)
        rollingResistance += (3.0f * mGrassIntensity);

        // Résistance virage
        float steerFactor = std::abs(mCurrentSteer);
        rollingResistance += (steerFactor * 4.0f);

        // Frein moteur
        if (!inputs.accelerate && !inputs.brake) rollingResistance += 2.0f;

        // --- RESISTANCE DE L'AIR (DRAG) ---
        // C'est ici qu'on gère la vitesse max "naturelle" sur l'herbe
        float dragFactor = 0.002f;

        // Si on est sur l'herbe, l'air/sol résiste plus exponentiellement
        // Cela va "manger" la vitesse de pointe doucement sans à-coup
        if (onGrass) dragFactor = 0.005f;

        float airResistance = (currentSpeed * currentSpeed) * dragFactor;
        float totalDecel = (rollingResistance + airResistance) * dt;

        float newSpeed = currentSpeed - totalDecel;
        if (newSpeed < 0.0f) newSpeed = 0.0f;

        if (currentSpeed > 0.0001f) {
             mVelocity = (mVelocity / currentSpeed) * newSpeed;
        } else {
             mVelocity = {0.f, 0.f};
        }
    }

    // --- D. VITESSE MAX ABSOLUE (Seulement pour le max global) ---
    // SUPPRESSION DU CODE QUI LIMITAIT BRUTALEMENT SUR L'HERBE
    float maxSpeed = Config::CAR_MAX_SPEED;

    // On ne touche plus à maxSpeed en fonction de l'herbe.
    // C'est le "dragFactor" au-dessus qui empêchera naturellement d'atteindre
    // la vitesse max si on est dans l'herbe.

    float finalSpeedSq = mVelocity.x * mVelocity.x + mVelocity.y * mVelocity.y;
    if (finalSpeedSq > maxSpeed * maxSpeed) {
        float k = maxSpeed / std::sqrt(finalSpeedSq);
        mVelocity *= k;
    }
}

void Car::applyDrift(const sf::Vector2f& forward, float currentSpeed) {
    // Décomposition de la vitesse
    (void)currentSpeed;
    float forwardSpeed = mVelocity.x * forward.x + mVelocity.y * forward.y;
    sf::Vector2f forwardVelocity = forward * forwardSpeed;
    sf::Vector2f lateralVelocity = mVelocity - forwardVelocity;
    float gripFactor = 0.05f;

    mVelocity = forwardVelocity + lateralVelocity * gripFactor;
}

void Car::updateAudioPitch(float currentSpeed) {
    if (mEngineSound) {
        float targetPitch = (currentSpeed < 10.f) ? 0.8f : (0.8f + (currentSpeed / Config::CAR_MAX_SPEED) * 1.7f);

        // Lissage du pitch pour éviter les changements brusques
        if (std::abs(mLastPitch - targetPitch) > 0.05f) {
            mEngineSound->setPitch(targetPitch);
            mLastPitch = targetPitch;
        }
    }
}

void Car::resolveCollisions(float dt, const sf::Vector2f& forward, const CollisionMask& mask) {
    sf::Vector2f nextPos = mSprite.getPosition() + mVelocity * dt;

    // Calcul des bounds pour les collisions
    float scale = mSprite.getScale().x;
    float halfLength = (mSprite.getLocalBounds().size.x * scale) / 2.f;

    // Points de collision
    sf::Vector2f frontBumper = nextPos + forward * (halfLength * 0.9f);
    sf::Vector2f rearBumper = nextPos - forward * (halfLength * 0.9f);

    // Tests de collision via le masque
    bool frontHit = !mask.isTraversable(frontBumper);
    bool rearHit = !mask.isTraversable(rearBumper);
    bool centerHit = !mask.isTraversable(nextPos);

    bool collision = false;
    float speedProj = mVelocity.x * forward.x + mVelocity.y * forward.y;

    if (speedProj > 0 && frontHit) collision = true;
    else if (speedProj < 0 && rearHit) collision = true;
    else if (centerHit) collision = true;

    if (!collision) {
        mSprite.setPosition(nextPos);
    } else {
        // Rebond simple
        mVelocity = -mVelocity * 0.3f;
    }
}

// -----------------------------------------------------------------------
// Méthodes utilitaires et accesseurs existants
// -----------------------------------------------------------------------

void Car::render(sf::RenderWindow& window, float alpha) {
    sf::Vector2f currentPos = mSprite.getPosition();
    sf::Vector2f interpPos = mPreviousPosition * (1.f - alpha) + currentPos * alpha;

    float currentRot = mSprite.getRotation().asDegrees();
    float interpRot = lerpAngle(mPreviousRotation, currentRot, alpha);

    mSprite.setPosition(interpPos);
    mSprite.setRotation(sf::degrees(interpRot));

    window.draw(mSprite);

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