#ifndef ASSETSMANAGER_H
#define ASSETSMANAGER_H

#include <SFML/Graphics.hpp>
#include <string>
#include <map>

/**
 * @brief Manage game assets like textures and fonts
 */
class AssetsManager {
public:
    /**
     * @brief Construct a new Assets Manager
     */
    AssetsManager();

    /**
     * @brief Load a texture from a file
     * @param name Identifier for the texture
     * @param filepath Path to the texture file
     * @return true if loaded successfully
     */
    bool loadTexture(const std::string& name, const std::string& filepath);

    /**
     * @brief Get a texture by name
     * @param name Identifier of the texture
     * @return Reference to the texture
     */
    sf::Texture& getTexture(const std::string& name);

    /**
     * @brief Load a font from a file
     * @param name Identifier for the font
     * @param filepath Path to the font file
     * @return true if loaded successfully
     */
    bool loadFont(const std::string& name, const std::string& filepath);

    /**
     * @brief Get a font by name
     * @param name Identifier of the font
     * @return Reference to the font
     */
    sf::Font& getFont(const std::string& name);

    void setUseSDAssets(bool value) { mUseSD = value; }
    bool isUsingSDAssets() const { return mUseSD; }

private:
    std::map<std::string, sf::Texture> mTextures; ///< Loaded textures
    std::map<std::string, sf::Font> mFonts;       ///< Loaded fonts
    bool mUseSD = false;
};

#endif // ASSETSMANAGER_H
