/*******************************************************************
 * \file   basicGUI.hpp
 * \brief  Declare a basic graphical user interface that has rudimentary features.
 *
 * \author OmegaDIL.
 * \date   July 2025.
 *
 * \note This file depends on the SFML library.
 *********************************************************************/

#ifndef BASICINTERFACE_HPP
#define BASICINTERFACE_HPP

#include "GraphicalRessources.hpp"
#include <SFML/Graphics.hpp>
#include <string>
#include <vector>
#include <unordered_map>
#include <cassert>

#ifndef NDEBUG 
#define ENSURE_NOT_ZERO(value) \
	assert((value) != 0 && "ENSURE_NOT_ZERO failed: value equals 0.")
#define ENSURE_SFML_WINDOW_VALIDITY(window) \
	ENSURE_VALID_PTR((window)); \
	ENSURE_NOT_ZERO((window->getSize().x)); \
	ENSURE_NOT_ZERO((window->getSize().y))
#else
#define ENSURE_NOT_ZERO()
#define ENSURE_SFML_WINDOW_VALIDITY()
#endif

namespace gui
{

/**
 * \brief Manages items to create a basic GUI. You can display texts, sprites, a background...
 * \details All elements are fixed and can't be edited nor removed.
 *
 * \note This class stores UI componenents; it will use a considerable amount of memory.
 * \warning Avoid deleting the `sf::RenderWindow` passed as an argument while this class is using it.
 *			The progam will assert otherwise.
 *
 *
 * \see `sf::RenderWindow`, `TextWrapper`, `SpriteWrapper`.
 *
 * \code
 * sf::RenderWindow window{ sf::VideoMode{ sf::Vector2u{ 2560, 1440 } }, "My project" };
 * BasicInterface mainInterface{ &window, 1440 };
 * 
 * mainInterface.addText("Welcome!!", { 1280, 500 }, sf::Color{ 255, 255, 255 }, 48, "__default", SpriteWrapper::Alignment::Center, sf::Style::Bold | sf::Style::Underlined);
 * mainInterface.addSprite("projectIcon.png", window.getView().getCenter(), sf::Vector{ 0.5f, 0.5f });
 * 
 * while (window.isOpen())
 * {
 *		while (const std::optional event = window.pollEvent())
 *		{
 *			if (event->is<sf::Event::Resized>())
 *				BasicInterface::windowResized(&window);
 * 
 * 			if (event->is<sf::Event::Closed>() || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape))
 *				window.close();
 *		}
 *
 *		window.clear();
 *		mainInterface.draw();
 *		window.display();
 *	}
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
	 * \param[in] relativeScaling: The scales of the `sf::Transformable`s are absolute and will make an
	 *			  item appear at a certain size. However, in smaller windows, the same scale will make a 
	 *			  `sf::Transformable` appear larger compared to the window. Therefore a factor will be multiplied
	 *			  to the scales to adjust them. This factor is equal to 1.f with windows that have `relativeScaling`
	 *            (this parameter) as their smallest in x or y axis. For example with 1080: if your
	 *			  window is currently set to 1080 in x or y axis, then all scales are multiply by 1.f. But if
	 *			  the window is suddenly set to 2160, then all scales are multiply by 2.
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
	virtual ~BasicInterface() noexcept; /// \complexity O(N) where N is the number of elements added


	/**
	 * \brief Adds a text element to the interface.
	 * \complexity amortized O(1).
	 *
	 * \param[in] content: What the `sf::Text` will display.
	 * \param[in] fontName: The name of the font that'll be used.
	 * \param[in] characterSize: The character size of the `sf::Text`.
	 * \param[in] pos: The position of the `sf::Text`.
	 * \param[in] color: The color of the `sf::Text`.
	 * \param[in] alignment: The alignment of the `sf::Text`.
	 * \param[in] style: The style of the `sf::Text` (regular, italic, underlined...).
	 * \param[in] scale: The scale of the `sf::Text`.
	 * \param[in] rot: The rotation of the `sf::Text`.
	 *
	 * \note Some styles may not be available with your font.
	 *
	 * \pre   A texture should have been loaded with the name you gave.
	 * \post  The correct font will be used.
	 * \throw std::invalid_argument strong exception guarantee: nothing happens.
	 * 
	 * \see `TextWrapper`, `addSprite`.
	 */
	template<Ostreamable T>
	inline void addText(const T& content, sf::Vector2f pos, sf::Color color = sf::Color::White, unsigned int characterSize = 30u, const std::string& fontName = "__default", Alignment alignment = Alignment::Center, sf::Text::Style style = sf::Text::Style::Regular, sf::Vector2f scale = sf::Vector2f{ 1, 1 }, sf::Angle rot = sf::degrees(0))
	{
		ENSURE_SFML_WINDOW_VALIDITY(m_window);
		float relativeScalingValue{ std::min(m_window->getSize().x / relativeScaling, m_window->getSize().y / relativeScaling) };
		
		TextWrapper newText{ content, fontName, characterSize, pos, scale * relativeScalingValue, color, alignment, style, rot };
		m_texts.push_back(std::move(newText));
	}

	/**
	 * \brief Adds a sprite element to the interface.
	 * \complexity amortized O(1).
	 *
	 * \param[in] textureName: The alias of the texture.
	 * \param[in] pos: The position of the `sf::Sprite`.
	 * \param[in] scale: The scale of the `sf::Sprite`.
	 * \param[in] rect: The display part of the texture. If size is 0, it is set to the whole texture size.
	 * \param[in] rot: The rotation of the `sf::Sprite`.
	 * \param[in] alignment: The alignment of the `sf::Sprite`.
	 * \param[in] color: The color of the `sf::Sprite` that will be multiply by the texture color.
	 *
	 * \pre   A texture should have loaded with the name you gave.
	 * \pre   It should not be already reserved.
	 * \post  The correct texture will be used.
	 * \throw std::invalid_argument strong exception guarantee: nothing happens.
	 *
	 * \see `SpriteWrapper`, `addText`.
	 */
	void addSprite(const std::string& textureName, sf::Vector2f pos, sf::Vector2f scale = sf::Vector2f{ 1.f, 1.f }, sf::IntRect rect = sf::IntRect{}, sf::Angle rot = sf::degrees(0), Alignment alignment = Alignment::Center, sf::Color color = sf::Color::White);
	
	/**
	 * \brief Adds a sprite element to the interface.
	 * \complexity amortized O(1).
	 *
	 * \param[in] texture: The texture of the sprite.
	 * \param[in] pos: The position of the `sf::Sprite`.
	 * \param[in] scale: The scale of the `sf::Sprite`.
	 * \param[in] rect: The display part of the texture. If size is 0, it is set to the whole texture size.
	 * \param[in] rot: The rotation of the `sf::Sprite`.
	 * \param[in] alignment: The alignment of the `sf::Sprite`.
	 * \param[in] color: The color of the `sf::Sprite` that will be multiply by the texture color.
	 *
	 * \see `SpriteWrapper`, `addText`.
	 */
	void addSprite(sf::Texture texture, sf::Vector2f pos, sf::Vector2f scale = sf::Vector2f{ 1.f, 1.f }, sf::IntRect rect = sf::IntRect{}, sf::Angle rot = sf::degrees(0), Alignment alignment = Alignment::Center, sf::Color color = sf::Color::White) noexcept;

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

protected:

	/// Pointer to the window.
	mutable sf::RenderWindow* m_window;
	/// Collection of texts in the interface.
	std::vector<TextWrapper> m_texts;
	/// Collection of sprites in the interface.
	std::vector<SpriteWrapper> m_sprites;

	/// All scales are multiply by a factor one if the min axis definition is this value. 
	/// Otherwise larger factor for larger window, and vice-versa.
	unsigned int m_relativeScaling;

private:

	/// Collection of all interfaces to perform resizing. Stored by window.
	static std::unordered_multimap<sf::RenderWindow*, BasicInterface*> s_allInterfaces;
};


/**
 * @brief Handles the changes due to a resize of the window with its GUIs.
 *
 * @param[in,out] window: the window.
 */
void handleEventResize(sf::RenderWindow* window) noexcept;

} // gui namespace

#endif // BASICINTERFACE_HPP