#ifndef MENU_H
#define MENU_H

#include <SFML/Graphics.hpp>

/// @brief Manages game menu
class Menu {
public:
    /// @brief Constructor
    /// @param font Text font
    explicit Menu(const sf::Font& font);

    /// @brief Render menu
    /// @param window Render target
    /// @param showResult Show result text
    void render(sf::RenderWindow& window, bool showResult = false);

    /// @brief Check if button is clicked
    /// @param mousePos Mouse position
    /// @return True if button clicked
    bool isButtonClicked(sf::Vector2f mousePos) const;

    /// @brief Set result text
    /// @param result Result string
    void setResultText(const std::string& result);

    /// @brief Set button text
    /// @param text Button text
    void setButtonText(const std::string& text);

private:
    sf::Text mTitleText;     ///< Title text
    sf::Text mButtonText;    ///< Button text
    sf::Text mResultText;    ///< Result text
    sf::RectangleShape mButton; ///< Button shape
};

#endif // MENU_H