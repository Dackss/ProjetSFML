#include "Menu.h"

/// @brief Constructor
/// @param font Text font
Menu::Menu(const sf::Font& font)
        : mTitleText(font), mButtonText(font), mResultText(font) {
    /// Configure title text
    mTitleText.setString("RetroRush");
    mTitleText.setCharacterSize(60);
    mTitleText.setFillColor(sf::Color::White);
    mTitleText.setOutlineColor(sf::Color::Black);
    mTitleText.setOutlineThickness(2.f);

    /// Configure button text
    mButtonText.setString("Lancer");
    mButtonText.setCharacterSize(30);
    mButtonText.setFillColor(sf::Color::White);
    mButtonText.setOutlineColor(sf::Color::Black);
    mButtonText.setOutlineThickness(1.f);

    /// Configure result text
    mResultText.setCharacterSize(40);
    mResultText.setFillColor(sf::Color::White);
    mResultText.setOutlineColor(sf::Color::Black);
    mResultText.setOutlineThickness(1.f);

    /// Configure button
    mButton.setSize({300.f, 80.f});
    mButton.setFillColor(sf::Color(100, 100, 255));
}

/// @brief Render menu
/// @param window Render target
/// @param showResult Show result text
void Menu::render(sf::RenderWindow& window, bool showResult) {
    sf::Vector2u ws = window.getSize();

    /// Draw background
    sf::RectangleShape background(sf::Vector2f(float(ws.x), float(ws.y)));
    background.setFillColor(sf::Color::Black);
    window.draw(background);

    /// Update button color on hover
    sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
    mButton.setFillColor(mButton.getGlobalBounds().contains(mousePos) ? sf::Color(150, 150, 255) : sf::Color(100, 100, 255));

    /// Position title text (center, 25% height)
    sf::FloatRect tb = mTitleText.getLocalBounds();
    mTitleText.setOrigin(tb.position + tb.size / 2.f);
    mTitleText.setPosition(sf::Vector2f(ws.x / 2.f, ws.y * 0.25f));

    /// Position button (center, 50% height)
    sf::Vector2f bs = mButton.getSize();
    sf::Vector2f buttonPos((ws.x - bs.x) / 2.f, ws.y * 0.5f);
    mButton.setPosition(buttonPos);

    /// Position button text (center of button)
    sf::FloatRect bb = mButtonText.getLocalBounds();
    mButtonText.setOrigin(bb.position + bb.size / 2.f);
    mButtonText.setPosition(mButton.getPosition() + bs / 2.f);

    /// Position result text (center, 35% height)
    if (showResult) {
        sf::FloatRect rb = mResultText.getLocalBounds();
        mResultText.setOrigin(rb.position + rb.size / 2.f);
        mResultText.setPosition(sf::Vector2f(ws.x / 2.f, ws.y * 0.35f));
    }

    /// Draw all elements
    window.draw(mTitleText);
    window.draw(mButton);
    window.draw(mButtonText);
    if (showResult) {
        window.draw(mResultText);
    }
}

/// @brief Check if button is clicked
/// @param mousePos Mouse position
/// @return True if button clicked
bool Menu::isButtonClicked(sf::Vector2f mousePos) const {
    return mButton.getGlobalBounds().contains(mousePos);
}

/// @brief Set result text
/// @param result Result string
void Menu::setResultText(const std::string& result) {
    mResultText.setString(result);
}

/// @brief Set button text
/// @param text Button text
void Menu::setButtonText(const std::string& text) {
    mButtonText.setString(text);
}