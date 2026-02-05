#ifndef GAMEMANAGER_H
#define GAMEMANAGER_H

#include <SFML/System.hpp>
#include <string>

/// @brief Manages game state and timing
class GameManager {
public:
    /// @brief Constructor
    GameManager();

    /// @brief Start countdown phase
    void startCountdown();

    /// @brief Start race phase
    void startRace();
    void startTimer();

    /// @brief Update game state
    void update();

    /// @brief Check if in menu state
    /// @return True if in menu
    bool isInMenu() const;

    /// @brief Check if in countdown state
    /// @return True if in countdown
    bool isCountdown() const;

    /// @brief Check if in playing state
    /// @return True if playing
    bool isPlaying() const;

    /// @brief Check if in finished state
    /// @return True if finished
    bool isFinished() const;

    /// @brief Mark lap as finished
    /// @param raceTime Time to complete lap
    void markLapFinished(float raceTime);

    /// @brief Reset game state
    void reset();

    /// @brief Get countdown value
    /// @return Current countdown value
    int getCountdownValue() const;

    /// @brief Get race time
    /// @return Time in seconds
    float getRaceTime() const;

    /// @brief Get result text
    /// @return Result string
    std::string getResultText() const;

    /// @brief Check if race just started
    /// @return True if race started recently
    bool justStartedRace() const;

    bool isTimerRunning() const;

private:
    /// @brief Game state enumeration
    enum class State {
        Menu,     ///< Menu state
        Countdown,///< Countdown state
        Playing,  ///< Playing state
        Finished  ///< Finished state
    } mState;     ///< Current state

    sf::Clock mCountdownClock; ///< Countdown timer
    sf::Clock mRaceClock;      ///< Race timer
    int mCountdownValue;       ///< Countdown value
    float mLastRaceTime;       ///< Last race time
    std::string mResultText;   ///< Result text
    bool mTimerRunning;
};

#endif // GAMEMANAGER_H