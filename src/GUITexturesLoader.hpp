/*****************************************************************//**
 * @file   GUITexturesLoader.hpp
 * @brief  Contains functions that help to load textures and images for the GUI.
 * 
 * @author OmegaDIL.
 * @date   April 2025
 *********************************************************************/

#ifndef GUITEXTURESLOADER_HPP
#define GUITEXTURESLOADER_HPP

#include <SFML/Graphics.hpp>
#include <optional>
#include <string>
#include <memory>

///////////////////////////////////////////////////////////////////////////////////////////////////
/// Helper functions
///////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * @brief Converts a shape to a sprite with the same transformables.
 * 
 * @param[in] shape: The shape to convert.
 * @param[out] sprite: The sprite to create.
 * @param[out] texture: Its texture.
 * @param[in] smooth: If true, the texture is smoothed.
 * 
 * @note no effect if the shape is nullptr.
 * @note The content of the sprite and the texture is overwritten.
 * @note The shape is left unchanged at the end even though it is not const.
 * @note The shape is converted into a sf::Texture and a sf::Sprite by sf::RenderTexture, so please
 * 		 keep in mind that it is a slow action.
 */
void convertShapeToSprite(sf::Shape* shape, std::unique_ptr<sf::Sprite>& sprite, std::unique_ptr<sf::Texture>& texture, bool smooth = false) noexcept;

///////////////////////////////////////////////////////////////////////////////////////////////////
/// Helper functions
///////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////
/// Image loader functions
///////////////////////////////////////////////////////////////////////////////////////////////////

sf::Image loadDefaultSliderBackgroundTexture(sf::Vector2f size) noexcept;

sf::Image loadDefaultSliderCursorTexture(sf::Vector2f size) noexcept;

///////////////////////////////////////////////////////////////////////////////////////////////////
/// Image loader functions
///////////////////////////////////////////////////////////////////////////////////////////////////

#endif //GUITEXTURESLOADER_HPP