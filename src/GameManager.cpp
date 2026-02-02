#include "GameManager.h"
#include <string>

/// @brief Constructor
GameManager::GameManager()
        : mState(State::Menu), mCountdownValue(3), mLastRaceTime(0.f) {}

/// @brief Start countdown phase
void GameManager::startCountdown() {
    mState = State::Countdown;
    mCountdownValue = 3;
    mCountdownClock.restart();
}

/// @brief Start race phase
void GameManager::startRace() {
    mState = State::Playing;
    mRaceClock.restart();
}

/// @brief Update game state
void GameManager::update() {
    if (mState == State::Countdown) {
        /// Update countdown value
        int newVal = 3 - int(mCountdownClock.getElapsedTime().asSeconds());
        if (newVal != mCountdownValue) {
            mCountdownValue = newVal;
        }

        /// Start race after 4 seconds
        if (mCountdownClock.getElapsedTime().asSeconds() >= 4.f) {
            startRace();
        }
    }
}

/// @brief Check if in menu state
/// @return True if in menu
bool GameManager::isInMenu() const {
    return mState == State::Menu;
}

/// @brief Check if in countdown state
/// @return True if in countdown
bool GameManager::isCountdown() const {
    return mState == State::Countdown;
}

/// @brief Check if in playing state
/// @return True if playing
bool GameManager::isPlaying() const {
    return mState == State::Playing;
}

/// @brief Check if in finished state
/// @return True if finished
bool GameManager::isFinished() const {
    return mState == State::Finished;
}

/// @brief Mark lap as finished
/// @param raceTime Time to complete lap
void GameManager::markLapFinished(float raceTime) {
    mState = State::Finished;
    mLastRaceTime = raceTime;
    /// Format result text with 2 decimal places
    mResultText = "Tour termine en " + std::to_string(raceTime).substr(0, std::to_string(raceTime).find(".") + 3) + " s";
}

/// @brief Reset game state
void GameManager::reset() {
    mState = State::Menu;
    mCountdownValue = 3;
    mLastRaceTime = 0.f;
    mResultText.clear();
}

/// @brief Get countdown value
/// @return Current countdown value
int GameManager::getCountdownValue() const {
    return mCountdownValue;
}

/// @brief Get race time
/// @return Time in seconds
float GameManager::getRaceTime() const {
    return mRaceClock.getElapsedTime().asSeconds();
}

/// @brief Get result text
/// @return Result string
std::string GameManager::getResultText() const {
    return mResultText;
}

/// @brief Check if race just started
/// @return True if race started recently
bool GameManager::justStartedRace() const {
    return mState == State::Playing && mRaceClock.getElapsedTime().asSeconds() < 0.05f;
}