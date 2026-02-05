#include "GameManager.h"
#include "Config.h" // Inclusion nécessaire
#include <string>

GameManager::GameManager()
    : mState(State::Menu), mCountdownValue(Config::COUNTDOWN_START_VALUE),
      mLastRaceTime(0.f), mTimerRunning(false) {}

void GameManager::startCountdown() {
    mState = State::Countdown;
    mCountdownValue = Config::COUNTDOWN_START_VALUE;
    mCountdownClock.restart();
}

void GameManager::startRace() {
    mState = State::Playing;
    mRaceClock.restart(); // On restart pour l'initialisation physique
    mTimerRunning = false; // IMPORTANT : Le temps ne tourne pas encore
}

void GameManager::startTimer() {
    mRaceClock.restart(); // On remet à zéro au moment précis du franchissement
    mTimerRunning = true;
}

void GameManager::update() {
    if (mState == State::Countdown) {
        float elapsed = mCountdownClock.getElapsedTime().asSeconds();

        // Mise à jour de l'affichage (3.. 2.. 1..)
        int newVal = Config::COUNTDOWN_START_VALUE - static_cast<int>(elapsed);
        if (newVal != mCountdownValue) {
            mCountdownValue = newVal;
        }

        // Démarrage de la course
        if (elapsed >= Config::COUNTDOWN_DURATION) {
            startRace();
        }
    }
}

// Les getters restent identiques, mais sont maintenant plus cohérents
bool GameManager::isInMenu() const { return mState == State::Menu; }
bool GameManager::isCountdown() const { return mState == State::Countdown; }
bool GameManager::isPlaying() const { return mState == State::Playing; }
bool GameManager::isFinished() const { return mState == State::Finished; }

void GameManager::markLapFinished(float raceTime) {
    mState = State::Finished;
    mLastRaceTime = raceTime;
    mResultText = "Tour termine en " + std::to_string(raceTime).substr(0, std::to_string(raceTime).find(".") + 3) + " s";
}

void GameManager::reset() {
    mState = State::Menu;
    mCountdownValue = Config::COUNTDOWN_START_VALUE;
    mLastRaceTime = 0.f;
    mResultText.clear();
    mTimerRunning = false; // Reset du flag
}

int GameManager::getCountdownValue() const { return mCountdownValue; }

float GameManager::getRaceTime() const {
    if (mState == State::Menu || mState == State::Countdown) return 0.0f;
    if (mState == State::Finished) return mLastRaceTime;

    // Si le timer n'a pas démarré (mais qu'on joue), on retourne 0
    if (!mTimerRunning) return 0.0f;

    return mRaceClock.getElapsedTime().asSeconds();
}

std::string GameManager::getResultText() const { return mResultText; }

bool GameManager::justStartedRace() const {
    return mState == State::Playing && mRaceClock.getElapsedTime().asSeconds() < 0.05f;
}

bool GameManager::isTimerRunning() const {
    return mTimerRunning;
}