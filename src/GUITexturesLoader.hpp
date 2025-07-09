/*****************************************************************//**
 * \file   GUITexturesLoader.hpp
 * \brief  Contains functions that help to load textures and images for the GUI.
 * 
 * \author OmegaDIL.
 * \date   April 2025
 *********************************************************************/

#ifndef GUITEXTURESLOADER_HPP
#define GUITEXTURESLOADER_HPP

#include <SFML/Graphics.hpp>
#include <concepts>
#include <optional>
#include <sstream>
#include <type_traits>
#include <algorithm>
#include <variant>


///////////////////////////////////////////////////////////////////////////////////////////////////
/// Helper functions
///////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * \brief Must be a sf::Drawable and a sf::Transformable type in sfml.
 */
template<typename T>
concept Drawable = std::derived_from<std::remove_cvref_t<T>, sf::Drawable>
                && std::derived_from<std::remove_cvref_t<T>, sf::Transformable>;

/**
 * \details From the given drawables, the function creates a texture that visually represents what
 *          they look like if drawn separatly, in order. The texture size covers the distance between
 *			the pixel at the leftmost/top edge of the leftmost/top drawable and the pixel at the
 *			rightmost/bottom edge of the rightmost/bottom drawable, not beginning at (0;0).
 * 
 * \tparam Ts: The types of the drawables. Must be derived from sf::Drawable and sf::Transformable.
 * \param[in] drawables: The drawables to create the texture from.
 * 
 * \note The drawables' origins are moved to 0;0, and their positions are changed; therefore the 
 *       drawables passed as arguments are very likely going to be modified.
 * \note If you create a texture from shapes, keep in mind that 'shapes' are drawn differently than
 * 		 'sprites' in SFML. Shapes use mathematical formulas to draw themselves, whereas sprites use
 *		 arrays of pixels (textures). While textures can be more detailed, they are also more pixelized.
 *		 Be aware that some may have artefacts in that case, especially if they are rotated, scaled, or
 *		 even if you change the origin of the new sprite (which has the returned texture applied to)
 *		 afterwards.
 * 
 * \return Returns a texture created from the given drawables such as sprites, cricleShape, convexShape...
 */
template<Drawable... Ts>
sf::Texture createTextureFromDrawables(Ts&&... drawables) noexcept
{
	// First, we need to calculate how much space the drawables will occupy in the render texture.

	// Before doing that, we must fix the origin: if a drawable's origin is centered (e.g., at its middle),
	// then adding its position to its size won't yield the correct bounding area —
	// because visually, part of the object extends in negative directions from its origin.
	// For example, a centered origin causes half the shape to be positioned in negative space,
	// which would lead to incorrect size and position computations.

	// Temporarily move drawables by their negative origin so that they align to (0,0)
	(std::forward<Ts>(drawables).move(-std::forward<Ts>(drawables).getOrigin()), ...);
	// Then reset their origin to (0, 0) to make further calculations easier
	(std::forward<Ts>(drawables).setOrigin(sf::Vector2f{ 0, 0 }), ...);

	// Compute the maximum extent in x and y: rightmost and bottommost edges
	sf::Vector2f maxSize{ 0, 0 };
	((maxSize.x = std::max(maxSize.x, std::forward<Ts>(drawables).getGlobalBounds().size.x + std::forward<Ts>(drawables).getGlobalBounds().position.x)), ...);
	((maxSize.y = std::max(maxSize.y, std::forward<Ts>(drawables).getGlobalBounds().size.y + std::forward<Ts>(drawables).getGlobalBounds().position.y)), ...);

	// Compute the minimum offset in x and y: leftmost and topmost edges
	sf::Vector2f offset{ maxSize }; // Will use std::min, therefore we use the maximum size as the initial offset.
	((offset.x = std::min(offset.x, std::forward<Ts>(drawables).getGlobalBounds().position.x)), ...);
	((offset.y = std::min(offset.y, std::forward<Ts>(drawables).getGlobalBounds().position.y)), ...);
	(std::forward<Ts>(drawables).move(-offset), ...); // Move all drawables so they align with (0,0) in the new texture space

	// Calculate the true size of the texture based on the maximum extent and offset.
	sf::Vector2u trueSize{ maxSize - offset }; 
	trueSize.x = ceil(trueSize.x); // Round to prevent artifacts due to subpixels
	trueSize.y = ceil(trueSize.y);

	sf::RenderTexture renderTexture{ static_cast<sf::Vector2u>(trueSize) };
	renderTexture.clear(sf::Color::Transparent);
	(renderTexture.draw(std::forward<Ts>(drawables)), ...);
	renderTexture.display();

	sf::Texture texture{ renderTexture.getTexture() };
	return texture;
}



std::optional<sf::Texture> loadTextureFromFile(std::ostringstream& errorMessage, std::string const& fileName, std::string const& path = "../res/") noexcept;


sf::Texture loadDefaultTexture(sf::Vector2f size) noexcept;

///////////////////////////////////////////////////////////////////////////////////////////////////
/// Helper functions
///////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////
/// Image loader functions
///////////////////////////////////////////////////////////////////////////////////////////////////

sf::Texture loadSolidRectangeShapeWithOutline(sf::Vector2u size, sf::Color fill = sf::Color{ 20, 20, 20 }, sf::Color outline = sf::Color{ 80, 80, 80 }, unsigned int thickness = 5) noexcept;

sf::Texture loadCheckBoxTexture(sf::Vector2u size);

///////////////////////////////////////////////////////////////////////////////////////////////////
/// Image loader functions
///////////////////////////////////////////////////////////////////////////////////////////////////

#endif //GUITEXTURESLOADER_HPP