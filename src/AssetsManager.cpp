#include "AssetsManager.h"
#include <stdexcept>

AssetsManager::AssetsManager() {}

bool AssetsManager::loadTexture(const std::string& name, const std::string& filepath) {
    sf::Texture texture;
    if (!texture.loadFromFile(filepath)) { // https://www.sfml-dev.org/documentation/3.0.0/classsf_1_1Texture.php#a75c2f3d8e4c1d3f4bbd7cd8f7e4d554e
        return false;
    }
    mTextures[name] = texture;
    return true;
}

sf::Texture& AssetsManager::getTexture(const std::string& name) {
    for (auto& pair : mTextures) {
        if (pair.first == name)
            return pair.second;
    }

    throw std::runtime_error("Texture not found: " + name);
}

bool AssetsManager::loadFont(const std::string& name, const std::string& filepath) {
    sf::Font font;
    if (!font.openFromFile(filepath)) { // https://www.sfml-dev.org/documentation/3.0.0/classsf_1_1Font.php#a3dfc31f9c1746a7ce03f45f1b2a4b7bf
        return false;
    }
    mFonts[name] = font;
    return true;
}

sf::Font& AssetsManager::getFont(const std::string& name) {
    for (auto& pair : mFonts) {
        if (pair.first == name)
            return pair.second;
    }

    throw std::runtime_error("Font not found: " + name);
}
