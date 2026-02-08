#ifndef WORLD_H
#define WORLD_H

#include <SFML/Graphics.hpp>
#include "Track.h"
#include "Player.h"
#include "AssetsManager.h"
#include "CollisionMask.h"
#include "CheckpointManager.h"
#include "GhostManager.h"

class World {
public:
    explicit World(sf::RenderWindow& window, AssetsManager& assetsManager);

    void update(sf::Time deltaTime, sf::View& camera);
    void render(bool isPlaying, float alpha = 1.0f);

    sf::FloatRect getTrackBounds() const;
    Player& getPlayer();
    Car& getCar();
    bool isLapComplete();
    void reset();
    int getLapCount() const;
    GhostManager& getGhost();
    bool isOnStartLine() const;

    // NOUVEAU : Signal de départ réel
    void startRace();

private:
    sf::RenderWindow& mWindow;
    AssetsManager& mAssetsManager;
    Track mTrack;
    Player mPlayer;
    CollisionMask mCollisionMask;
    CheckpointManager mCheckpoints;
    GhostManager mGhost;
    sf::Vector2f mTrackSize;
    int mLapCount;
};

#endif