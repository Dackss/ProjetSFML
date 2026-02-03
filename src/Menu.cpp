#include "Menu.h"
#include "ScoreManager.h"
#include <cmath>
#include <iomanip>
#include <sstream>
#include <algorithm> // pour std::max
#include <cstdint>   // pour std::uint8_t

/// @brief Constructor
Menu::Menu(const sf::Font& font, const sf::Texture& bgTexture)
        : mFont(font),
          mBackground(bgTexture), // SFML 3: Sprite doit être init avec une texture
          mTitleText(font),       // SFML 3: Text doit être init avec une font
          mPressStartText(font),
          mResultText(font),
          mHighScoreTitle(font)
{
    // Configurer le fond (assombri pour lisibilité)
    mBackground.setColor(sf::Color(100, 100, 100));

    // Titre Stylisé
    mTitleText.setString("RETRO RUSH");
    mTitleText.setCharacterSize(100);
    mTitleText.setFillColor(sf::Color(255, 200, 0)); // Or
    mTitleText.setOutlineColor(sf::Color::Red);
    mTitleText.setOutlineThickness(5.f);
    mTitleText.setStyle(sf::Text::Bold | sf::Text::Italic);

    // Texte "Press Start" clignotant
    mPressStartText.setString("PRESS START / ENTER");
    mPressStartText.setCharacterSize(40);
    mPressStartText.setFillColor(sf::Color::White);
    mPressStartText.setOutlineColor(sf::Color::Black);
    mPressStartText.setOutlineThickness(2.f);

    // Titre des High Scores
    mHighScoreTitle.setString("TOP RECORDS");
    mHighScoreTitle.setCharacterSize(30);
    mHighScoreTitle.setFillColor(sf::Color::Cyan);
    mHighScoreTitle.setStyle(sf::Text::Underlined);

    // Texte résultat (fin de course)
    mResultText.setCharacterSize(50);
    mResultText.setFillColor(sf::Color::Green);
    mResultText.setOutlineColor(sf::Color::Black);
    mResultText.setOutlineThickness(3.f);

    updateHighScores();
}

void Menu::updateHighScores() {
    mHighScoresList.clear();
    std::vector<float> scores = ScoreManager::loadScores();

    for (size_t i = 0; i < scores.size(); ++i) {
        // SFML 3: Constructeur avec font obligatoire
        sf::Text text(mFont);
        text.setCharacterSize(25);
        text.setFillColor(sf::Color::White);

        std::stringstream ss;
        ss << i + 1 << ". " << std::fixed << std::setprecision(2) << scores[i] << " s";
        text.setString(ss.str());

        mHighScoresList.push_back(text);
    }
}

void Menu::render(sf::RenderWindow& window, bool showResult) {
    sf::Vector2u ws = window.getSize();

    // SFML 3: getTexture() retourne une référence (const Texture&), donc on utilise '.'
    float scaleX = (float)ws.x / mBackground.getTexture().getSize().x;
    float scaleY = (float)ws.y / mBackground.getTexture().getSize().y;
    mBackground.setScale({std::max(scaleX, scaleY), std::max(scaleX, scaleY)});
    window.draw(mBackground);

    // Centrer le titre
    sf::FloatRect tb = mTitleText.getLocalBounds();
    mTitleText.setOrigin(tb.position + tb.size / 2.f);
    // SFML 3: setPosition attend un sf::Vector2f
    mTitleText.setPosition({ws.x / 2.f, ws.y * 0.2f});
    window.draw(mTitleText);

    // Faire clignoter "Press Start"
    float alpha = (std::sin(mBlinkClock.getElapsedTime().asSeconds() * 5.f) + 1.f) / 2.f; // 0 à 1
    // SFML 3: sf::Uint8 n'existe plus, utiliser std::uint8_t
    mPressStartText.setFillColor(sf::Color(255, 255, 255, static_cast<std::uint8_t>(alpha * 255)));

    sf::FloatRect pb = mPressStartText.getLocalBounds();
    mPressStartText.setOrigin(pb.position + pb.size / 2.f);
    mPressStartText.setPosition({ws.x / 2.f, ws.y * 0.8f});
    window.draw(mPressStartText);

    // Afficher les High Scores
    // SFML 3: setPosition attend un sf::Vector2f
    mHighScoreTitle.setPosition({ws.x * 0.1f, ws.y * 0.4f});
    window.draw(mHighScoreTitle);

    for (size_t i = 0; i < mHighScoresList.size(); ++i) {
        mHighScoresList[i].setPosition({ws.x * 0.1f, ws.y * 0.45f + i * 35.f});
        window.draw(mHighScoresList[i]);
    }

    // Afficher le résultat si nécessaire
    if (showResult) {
        sf::FloatRect rb = mResultText.getLocalBounds();
        mResultText.setOrigin(rb.position + rb.size / 2.f);
        mResultText.setPosition({ws.x / 2.f, ws.y * 0.5f});
        window.draw(mResultText);
    }
}

void Menu::setResultText(const std::string& result) {
    mResultText.setString(result);
}