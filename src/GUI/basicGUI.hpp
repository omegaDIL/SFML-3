/*******************************************************************
 * \file   basicGUI.hpp
 * \brief  Declare a basic graphical user interface that has rudimentary features.
 *
 * \author OmegaDIL.
 * \date   July 2025.
 *
 * \note This file depends on the SFML library.
 *********************************************************************/

#ifndef BASICGUI_HPP
#define BASICGUI_HPP

#include "GraphicalRessources.hpp"
#include <SFML/Graphics.hpp>
#include <string>
#include <vector>
#include <unordered_map>
#include <cassert>

#ifndef NDEBUG 
#define ENSURE_NOT_ZERO(value) \
	assert((value) != 0 && "ENSURE_NOT_ZERO failed: value equals 0.")
#else
#define ENSURE_NOT_ZERO()
#endif


//namespace gui

/**
 * \brief Manages items to create a basic GUI. You can display texts, sprites, a background...
 * \details All elements are fixed and can't be edited nor removed.
 *
 * \note This class stores UI componenents; it will use a considerable amount of memory.
 * \warning Avoid deleting the `sf::RenderWindow` passed as an argument while this class is using it.
 *
 * \see `sf::RenderWindow`, `TextWrapper`, `SpriteWrapper`.
 *
 * \code
 * 
 * \endcode
 */
class BasicInterface
{
public:

	/**
	 * \brief Constructs the interface.
	 * \complexity O(1).
	 *
	 * \param[in,out] window: The window where the interface elements will be rendered.
	 * \param[in] relativeScaling: 
	 *
	 * \note A default font is loaded when the first instance is called
	 * \warning The program assert if the window is not valid (nullptr or size 0)
	 *
	 * \pre A font should be named defaultFont.ttf in the assets folder.
	 * \post The font is loaded.
	 * \throw LoadingGraphicalRessourceFailure Strong exception guarrantee, but no text can use the
	 *		  default font and you'll have to load and use your own font.
	 */
	explicit BasicInterface(sf::RenderWindow* window, unsigned int relativeScaling = 1080);

	BasicInterface() noexcept = delete;
	BasicInterface(BasicInterface const&) noexcept = delete;
	BasicInterface(BasicInterface&& other) noexcept;
	BasicInterface& operator=(BasicInterface const&) noexcept = delete;
	BasicInterface& operator=(BasicInterface&& other) noexcept;
	virtual ~BasicInterface() noexcept;


	/**
	 * \brief Adds a text element to the interface.
	 * \complexity amortized O(1).
	 *
	 * \tparam T: Type that can be streamed to `std::basic_ostream`.
	 * \param[in] content: The text label.
	 * \param[in] position: The position of the text.
	 * \param[in] characterSize: The size of the text characters.
	 * \param[in] scale: The scale of the text.
	 * \param[in] color: The color of the text.
	 * \param[in] rot: The rotation of the text.
	 *
	 * \see TextWrapper, addSprite().
	 */
	template<Ostreamable T>
	void addText(const T& content, unsigned int characterSize, sf::Vector2f position, sf::Color color = sf::Color{ 255, 255, 255 }, sf::Vector2f scale = sf::Vector2f{ 1.f, 1.f }, sf::Angle rot = sf::degrees(0)) noexcept
	{
		TextWrapper newText{ content, characterSize, position, scale, color };
		m_texts.push_back(std::move(newText));
	}

	/**
	 * \brief Adds a sprite element to the interface.
	 * \complexity amortized O(1).
	 *
	 * \param[in] sprite: The sprite to add.
	 * \param[in] texture: The texture of the sprite.
	 *
	 * \see addText(), addShape().
	 */
	void addSprite(const std::string& texture, sf::Vector2f pos, sf::Vector2f scale, sf::Angle rot = sf::degrees(0), sf::IntRect rectangle = sf::IntRect{ sf::Vector2i{}, sf::Vector2i{} }, sf::Color color = sf::Color::White);

	/**
	 * \brief Adds a sprite element to the interface.
	 * \complexity amortized O(1).
	 *
	 * \param[in] sprite: The sprite to add.
	 * \param[in] texture: The texture of the sprite.
	 *
	 * \see addText(), addShape().
	 */
	void addSprite(sf::Texture texture, sf::Vector2f pos, sf::Vector2f scale, sf::Angle rot = sf::degrees(0), sf::IntRect rectangle = sf::IntRect{ sf::Vector2i{}, sf::Vector2i{} }, sf::Color color = sf::Color::White) noexcept;

	/**
	 * \brief Renders the interface.
	 * \complexity O(N), where N is the number of graphical elements.
	 * 
	 * \warning The program assert if the window is not valid (nullptr or size 0)
	 * 
	 * \see `sf::Drawable::draw()`.
	 */
	void draw() const noexcept;


	/**
	 * \brief Refreshes the transformables after the window is resized.
	 * Resizes only interfaces displayed on the resized window.
	 * \complexity O(N), where N is the number of graphical elements within all interfaces associated with the resized window.
	 *
	 * \param[in] window: The window which was resized, and for which the interfaces will be resized.
	 */
	static void windowResized(sf::RenderWindow* window) noexcept;


	unsigned int relativeScaling;

protected:

	// Pointer to the window.
	mutable sf::RenderWindow* m_window;

	// Collection of sprites in the interface.
	std::vector<SpriteWrapper> m_sprites;

	// Collection of texts in the interface.
	std::vector<TextWrapper> m_texts;

private:

	// Collection of all interfaces to perform resizing. Stored by window.
	static std::unordered_multimap<sf::RenderWindow*, BasicInterface*> allInterfaces;
};

#endif // BASICGUI_HPP