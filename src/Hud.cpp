#include "Hud.h"
#include <string>

/// @brief Constructor
/// @param font Text font
HUD::HUD(const sf::Font& font)
        : mSpeedText(font), mTimerText(font), mCountdownText(font) {
    /// Configure speed text
    mSpeedText.setCharacterSize(36);
    mSpeedText.setFillColor(sf::Color::Cyan);
    mSpeedText.setOutlineColor(sf::Color::Black);
    mSpeedText.setOutlineThickness(2.f);
    mSpeedText.setStyle(sf::Text::Bold);

    /// Configure timer text
    mTimerText.setCharacterSize(28);
    mTimerText.setFillColor(sf::Color::White);
    mTimerText.setOutlineColor(sf::Color::Black);
    mTimerText.setOutlineThickness(2.f);
    mTimerText.setStyle(sf::Text::Bold);

    /// Configure countdown text
    mCountdownText.setCharacterSize(120);
    mCountdownText.setFillColor(sf::Color::Yellow);
    mCountdownText.setOutlineColor(sf::Color::Black);
    mCountdownText.setOutlineThickness(5.f);
    mCountdownText.setStyle(sf::Text::Bold);
}

/// @brief Update HUD texts
/// @param speedKmH Car speed in km/h
/// @param raceTime Race time in seconds
/// @param countdown Countdown value
/// @param windowSize Window dimensions
void HUD::update(float speedKmH, float raceTime, int countdown, sf::Vector2u windowSize) {
    char speedStr[64]; // Augmenté de 16 à 64
    snprintf(speedStr, sizeof(speedStr), "Vitesse: %.2f km/h", speedKmH);
    mSpeedText.setString(speedStr);

    if (countdown < 0) {
        char timeStr[64]; // Augmenté de 16 à 64
        snprintf(timeStr, sizeof(timeStr), "Temps: %.2f s", raceTime);
        mTimerText.setString(timeStr);
    }

    /// Update countdown text
    if (countdown >= 0) {
        mCountdownText.setString(std::to_string(countdown));
    } else if (countdown == -1) {
        mCountdownText.setString("GO!");
    } else {
        mCountdownText.setString("");
    }

    /// Position timer text (top-left)
    mTimerText.setOrigin({0.f, 0.f});
    mTimerText.setPosition({15.f, 10.f});

    /// Position speed text (bottom-right)
    sf::FloatRect speedBounds = mSpeedText.getLocalBounds();
    mSpeedText.setOrigin({speedBounds.size.x, 0.f});
    mSpeedText.setPosition({float(windowSize.x) - 15.f, float(windowSize.y) - 50.f});

    /// Position countdown text (center)
    sf::FloatRect cdBounds = mCountdownText.getLocalBounds();
    mCountdownText.setOrigin(cdBounds.size / 2.f + cdBounds.position);
    mCountdownText.setPosition({windowSize.x / 2.f, windowSize.y / 2.f});
}

/// @brief Set best times display
/// @param times Best race times
void HUD::setBestTimes(const std::vector<sf::Time>& times) {
    mBestTimesText.clear();

    for (std::size_t i = 0; i < times.size(); ++i) {
        sf::Text text = mTimerText;
        char timeStr[64];
        snprintf(timeStr, sizeof(timeStr), "Top %zu: %.2f s", i + 1, times[i].asMilliseconds() / 1000.f);
        text.setString(timeStr);

        /// Position text (top-right)
        sf::FloatRect bounds = text.getLocalBounds();
        text.setOrigin({bounds.size.x, 0.f});
        text.setPosition({1280.f - 20.f, 10.f + i * 30.f});
        mBestTimesText.push_back(text);
    }
}

/// @brief Render HUD
/// @param window Render target
void HUD::render(sf::RenderWindow& window) {
    window.draw(mSpeedText);
    window.draw(mTimerText);
    if (!mCountdownText.getString().isEmpty()) {
        window.draw(mCountdownText);
    }
    for (const auto& text : mBestTimesText) {
        window.draw(text);
    }
}