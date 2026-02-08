#include "GhostManager.h"
#include "Config.h"
#include <cmath>
#include <iostream>
#include <fstream> // Nécessaire pour les fichiers

GhostManager::GhostManager(AssetsManager& assets)
    : mAssets(assets),
      mGhostSprite(assets.getTexture("voiture")),
      mIsRecording(false), // Faux par défaut, on attend la ligne de départ
      mIsActive(false),
      mHasGhost(false),
      mBestTime(99999.0f),
      mCurrentLapTime(0.0f),
      mRecordAccumulator(0.0f)
{
    mGhostSprite.setScale({Config::CAR_SCALE, Config::CAR_SCALE});
    sf::Vector2u size = mAssets.getTexture("voiture").getSize();
    mGhostSprite.setOrigin({size.x / 2.f, size.y / 2.f});

    // CORRECTION : Couleur Bleue (Cyan) semi-transparente
    mGhostSprite.setColor(sf::Color(0, 255, 255, 120));

    // Chargement du fantôme au démarrage
    loadGhost();
}

void GhostManager::update(float dt, const Car& playerCar) {
    // CORRECTION TIMING : Si la course n'est pas "Active" (ligne non franchie), on ne fait rien.
    // Cela empêche le fantôme d'accumuler du temps pendant le "rolling start".
    if (!mIsActive) return;

    mCurrentLapTime += dt;

    // --- ENREGISTREMENT ---
    if (mIsRecording) {
        float recordStep = 1.0f / Config::FPS;
        mRecordAccumulator += dt;

        while (mRecordAccumulator >= recordStep) {
            mCurrentGhost.addPoint(playerCar.getPosition(), playerCar.getRotation());
            mRecordAccumulator -= recordStep;
        }
    }

    // --- LECTURE ---
    if (mHasGhost && !mBestGhost.isEmpty()) {
        applyInterpolatedState(mCurrentLapTime);
    }
}

// NOUVEAU : Appelé par World quand la ligne de départ est franchie
void GhostManager::startRecording() {
    mIsActive = true;
    mIsRecording = true;
    mCurrentLapTime = 0.0f; // Reset précis du temps à 0.00s au top départ
    mRecordAccumulator = 0.0f;
    mCurrentGhost.reset();

    // On ajoute le point initial (la ligne de départ)
    // On ne peut pas récupérer la voiture ici facilement sans changer la signature,
    // mais elle sera capturée à la frame suivante, ce qui est négligeable (16ms).
}

void GhostManager::applyInterpolatedState(float time) {
    if (time >= mBestGhost.mTotalTime) {
        if (!mBestGhost.mPoints.empty()) {
            const auto& last = mBestGhost.mPoints.back();
            mGhostSprite.setPosition(last.position);
            mGhostSprite.setRotation(sf::degrees(last.rotation));
        }
        return;
    }

    float fps = Config::FPS;
    float exactIndex = time * fps;

    size_t indexA = static_cast<size_t>(exactIndex);
    size_t indexB = indexA + 1;

    if (indexA >= mBestGhost.mPoints.size()) return;
    if (indexB >= mBestGhost.mPoints.size()) indexB = indexA;

    const auto& pointA = mBestGhost.mPoints[indexA];
    const auto& pointB = mBestGhost.mPoints[indexB];

    float t = exactIndex - static_cast<float>(indexA);

    sf::Vector2f pos = pointA.position + (pointB.position - pointA.position) * t;

    float rotA = pointA.rotation;
    float rotB = pointB.rotation;

    float diff = rotB - rotA;
    while (diff < -180.f) diff += 360.f;
    while (diff > 180.f) diff -= 360.f;
    float rot = rotA + diff * t;

    mGhostSprite.setPosition(pos);
    mGhostSprite.setRotation(sf::degrees(rot));
}

void GhostManager::render(sf::RenderWindow& window, bool isPlaying) {
    // On n'affiche le fantôme que si la course est active (pas pendant le compte à rebours)
    if (isPlaying && mHasGhost && mIsActive) {
        window.draw(mGhostSprite);
    }
}

bool GhostManager::handleLapComplete() {
    float finalLapTime = mCurrentLapTime;
    mCurrentGhost.mTotalTime = finalLapTime;

    if (finalLapTime < mBestTime) {
        std::cout << "New Best Time Ghost! " << finalLapTime << "s" << std::endl;
        mBestTime = finalLapTime;
        mBestGhost = mCurrentGhost;
        mHasGhost = true;

        saveGhost(); // SAUVEGARDE SUR DISQUE

        reset(); // On stop tout en attendant le prochain passage de ligne
        return true;
    }

    reset();
    return false;
}

void GhostManager::reset() {
    mCurrentGhost.reset();
    mCurrentLapTime = 0.0f;
    mRecordAccumulator = 0.0f;
    mIsActive = false;    // On arrête le chrono interne
    mIsRecording = false; // On arrête l'enregistrement
}

float GhostManager::getBestLapTime() const {
    return mBestTime;
}

std::vector<sf::Time> GhostManager::getBestTimes() const {
    if (mBestTime > 9000.0f) return {};
    return { sf::seconds(mBestTime) };
}

// --- PERSISTANCE ---

void GhostManager::saveGhost() {
    std::ofstream file(GHOST_FILE, std::ios::binary);
    if (!file) return;

    // 1. Temps total
    file.write(reinterpret_cast<const char*>(&mBestTime), sizeof(float));

    // 2. Nombre de points
    size_t count = mBestGhost.mPoints.size();
    file.write(reinterpret_cast<const char*>(&count), sizeof(size_t));

    // 3. Les points (Raw Data)
    if (count > 0) {
        file.write(reinterpret_cast<const char*>(mBestGhost.mPoints.data()), count * sizeof(GhostPoint));
    }
    std::cout << "Ghost saved: " << count << " points, Time: " << mBestTime << std::endl;
}

void GhostManager::loadGhost() {
    std::ifstream file(GHOST_FILE, std::ios::binary);
    if (!file) return;

    file.read(reinterpret_cast<char*>(&mBestTime), sizeof(float));

    size_t count = 0;
    file.read(reinterpret_cast<char*>(&count), sizeof(size_t));

    mBestGhost.mPoints.resize(count);
    if (count > 0) {
        file.read(reinterpret_cast<char*>(mBestGhost.mPoints.data()), count * sizeof(GhostPoint));
    }

    mBestGhost.mTotalTime = mBestTime;
    mHasGhost = true;
    std::cout << "Ghost loaded: " << mBestTime << "s" << std::endl;
}