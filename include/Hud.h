#ifndef HUD_H
#define HUD_H

#include <SFML/Graphics.hpp>
#include <vector>

/// @brief Manages heads-up display
class HUD {
public:
    /// @brief Constructor
    /// @param font Text font
    explicit HUD(const sf::Font& font);

    /// @brief Update HUD texts
    /// @param speedKmH Car speed in km/h
    /// @param raceTime Race time in seconds
    /// @param countdown Countdown value
    /// @param windowSize Window dimensions
    void update(float speedKmH, float raceTime, int countdown, sf::Vector2u windowSize);

    /// @brief Render HUD
    /// @param window Render target
    void render(sf::RenderWindow& window);

    /// @brief Set best times display
    /// @param times Best race times
    void setBestTimes(const std::vector<sf::Time>& times);

private:
    sf::Text mSpeedText;        ///< Speed display
    sf::Text mTimerText;        ///< Race timer display
    sf::Text mCountdownText;    ///< Countdown display
    std::vector<sf::Text> mBestTimesText; ///< Best times display
};

#endif // HUD_H