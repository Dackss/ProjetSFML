#ifndef MENU_H
#define MENU_H

#include <SFML/Graphics.hpp>
#include <vector>

class Menu {
public:
    // On ajoute la texture de fond au constructeur
    Menu(const sf::Font& font, const sf::Texture& bgTexture);

    void render(sf::RenderWindow& window, bool showResult);
    void setResultText(const std::string& result);

    // Met à jour l'affichage des scores
    void updateHighScores();

private:
    // CORRECTION : On déclare mFont en premier pour correspondre à l'ordre d'initialisation
    const sf::Font& mFont;

    sf::Sprite mBackground;
    sf::Text mTitleText;
    sf::Text mPressStartText;
    sf::Text mResultText;

    sf::Text mHighScoreTitle;
    std::vector<sf::Text> mHighScoresList;

    sf::Clock mBlinkClock;
};

#endif // MENU_H