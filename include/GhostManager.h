#ifndef GHOSTMANAGER_H
#define GHOSTMANAGER_H

#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include "Car.h"
#include "CheckpointManager.h"
#include "AssetsManager.h"

struct GhostPoint {
    sf::Vector2f position;
    float rotation;
};

class GhostData {
public:
    std::vector<GhostPoint> mPoints;
    float mTotalTime = 0.0f;

    void addPoint(const sf::Vector2f& pos, float rot) {
        mPoints.push_back({pos, rot});
    }

    void reset() {
        mPoints.clear();
        mTotalTime = 0.0f;
    }

    bool isEmpty() const { return mPoints.empty(); }
};

class GhostManager {
public:
    explicit GhostManager(AssetsManager& assets);

    void update(float dt, const Car& playerCar);
    void render(sf::RenderWindow& window, bool isPlaying);

    // Réinitialise l'état interne (accumulateurs)
    void reset();

    // NOUVEAU : Déclenche l'enregistrement réel (au franchissement de ligne)
    void startRecording();

    // Gère la fin de tour et la sauvegarde
    bool handleLapComplete();

    float getBestLapTime() const;
    std::vector<sf::Time> getBestTimes() const;

private:
    void applyInterpolatedState(float time);

    // NOUVEAU : Persistance fichier
    void saveGhost();
    void loadGhost();

private:
    AssetsManager& mAssets;
    sf::Sprite mGhostSprite;

    GhostData mBestGhost;
    GhostData mCurrentGhost;

    bool mIsRecording; // Si on est autorisé à enregistrer
    bool mIsActive;    // NOUVEAU : Si la course a vraiment commencé (Timer lancé)
    bool mHasGhost;

    float mBestTime;
    float mCurrentLapTime;
    float mRecordAccumulator;

    const std::string GHOST_FILE = "ghost.dat";
};

#endif