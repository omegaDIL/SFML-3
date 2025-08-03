/*******************************************************************
 * \file   BasicInterface.hpp, BasicInterface.cpp
 * \brief  Declare a basic graphical user interface that has rudimentary features.
 *
 * \author OmegaDIL.
 * \date   July 2025.
 *
 * \note These files depend on the SFML library.
 *********************************************************************/

#ifndef BASICINTERFACE_HPP
#define BASICINTERFACE_HPP

#include "GraphicalResources.hpp"
#include <SFML/Graphics.hpp>
#include <string>
#include <vector>
#include <unordered_map>
#include <cassert>

#ifndef NDEBUG 
#define ENSURE_NOT_ZERO(value) \
	assert((value) != 0 && "ENSURE_NOT_ZERO failed: value equals 0.")
#define ENSURE_SFML_WINDOW_VALIDITY(window) \
	ENSURE_VALID_PTR((window), ""); \
	ENSURE_NOT_ZERO((window->getSize().x)); \
	ENSURE_NOT_ZERO((window->getSize().y))
#else
#define ENSURE_NOT_ZERO()
#define ENSURE_SFML_WINDOW_VALIDITY()
#endif

namespace gui
{

/**
 * \brief Manages items to create a basic GUI. You can display texts and sprites.
 * \details All elements are fixed and can't be edited nor removed.
 *
 * \note This class stores UI componenents; it will use a considerable amount of memory.
 * \warning Avoid deleting the `sf::RenderWindow` passed as an argument while this class is using it.
 *			The progam will assert otherwise.
 *
 * \see `sf::RenderWindow`, `TextWrapper`, `SpriteWrapper`.
 *
 * \code
 * sf::Vector2u size{ 2560, 1440 };
 * sf::RenderWindow window{ sf::VideoMode{ size }, "My project" };
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
 *				BasicInterface::windowResized(&window, size);
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
	 *			  to the scales to adjust them. This factor is equal to 1.f with windows that have 
	 *			  `relativeScalingDefinition` (this parameter) as their smallest in x or y axis. For example,
	 *			  with 1080: if your window is currently set to 1080 in x or y axis, then all scales are multiply
	 *			  by 1.f. But if the window is suddenly set to 2160, then all scales are multiply by 2.
	 * 
	 * \note A default font is loaded when the first instance is called (named __default).
	 * \warning The program asserts if the window is not valid (nullptr or size 0).
	 * \warning The program asserts if the relativeScaling is set to 0.
	 *
	 * \pre A font should be named defaultFont.ttf in the assets folder.
	 * \post The font is loaded.
	 * \throw LoadingGraphicalRessourceFailure Strong exception guarrantee, but no text can use the
	 *		  default font and you'll have to load and use your own font.
	 */
	explicit BasicInterface(sf::RenderWindow* window, unsigned int relativeScalingDefinition = 1080);

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
	 * \pre   A font should have been loaded with the name you gave.
	 * \post  The correct font will be used.
	 * \throw std::invalid_argument strong exception guarantee: nothing happens.
	 * 
	 * \see `TextWrapper`, `addSprite`.
	 */
	template<Ostreamable T>
	void addText(const T& content, sf::Vector2f pos, sf::Color color = sf::Color::White, unsigned int characterSize = 30u, const std::string& fontName = "__default", Alignment alignment = Alignment::Center, std::uint32_t style = 0, sf::Vector2f scale = sf::Vector2f{ 1, 1 }, sf::Angle rot = sf::degrees(0))
	{
		ENSURE_SFML_WINDOW_VALIDITY(m_window);

		float relativeScalingValue{ static_cast<float>(std::min(m_window->getSize().x, m_window->getSize().y)) / m_relativeScalingDefinition };
		
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
	 * \brief Renders the interface. Texts are drawn above sprites.
	 * \complexity O(N), where N is the number of graphical elements.
	 * 
	 * \warning The program assert if the window is not valid (nullptr or size 0)
	 * 
	 * \see `sf::Drawable::draw()`.
	 */
	void draw() const noexcept;


	/**
	 * \brief Updates the window, views, and the window' interfaces drawables after resizement.
	 * The max size for the window is the screen size. Everything is scaled without distortion.
	 * \complexity O(N), where N is the number of graphical elements within all interfaces associated to the resized window.
	 *
	 * \param[in,out] window: The window which was resized, and for which the interfaces will be resized.
	 * \param[in,out] previousSize: The window's size before resizement.
	 * \param[out] views: All views for that window you want to resize accordingly.
	 * 
	 * \note The current view of the window is resized and applied back. However it is copy, therefore
	 *		 you need to give all views in the parameter even the one that is currently applied at the
	 *		 moment of the call. If you have no views, leave that parameter empty.
	 * \note You don't need to call this function if you want to keep the same window'size througout the
	 *		 program. Just apply the previous size back to the window.
	 * \warning The program asserts if the window is not valid (nullptr or size 0).
	 * \warning The program asserts if previous size is set to 0.
	 */
	template<typename... Ts> requires (std::same_as<Ts, sf::View*> && ...)
	inline static void windowResized(sf::RenderWindow* resizedWindow, sf::Vector2u& previousSize, Ts... views) noexcept
	{
		ENSURE_SFML_WINDOW_VALIDITY(resizedWindow);
		ENSURE_NOT_ZERO(previousSize.x);
		ENSURE_NOT_ZERO(previousSize.y);

		const sf::Vector2u maxSize{ sf::VideoMode::getDesktopMode().size };
		sf::Vector2u newSize{ resizedWindow->getSize() };

		if (newSize.x > maxSize.x) // Not larger than the current window
			newSize.x = maxSize.x;
		if (newSize.y > maxSize.y)
			newSize.y = maxSize.y;
		
		// Updates current view and the others.
		const sf::Vector2f scaleFactor{ newSize.x / static_cast<float>(previousSize.x), newSize.y / static_cast<float>(previousSize.y) };
		sf::View view{ resizedWindow->getView() };
		view.setSize(sf::Vector2f{ view.getSize().x * scaleFactor.x, view.getSize().y * scaleFactor.y});
		(views->setSize(sf::Vector2f{ views.getSize().x * scaleFactor.x, views.getSize().y * scaleFactor.y }), ...);

		// Update drawables.
		const float relativeMinAxisScale{ static_cast<float>(std::min(newSize.x, newSize.y)) / std::min(previousSize.x, previousSize.y) };
		proportionKeeper(resizedWindow, relativeMinAxisScale);

		// Update window.
		previousSize = newSize; // Updates previous size.
		resizedWindow->setView(view);
		resizedWindow->setSize(newSize);
	}

protected:

	/// Pointer to the window.
	mutable sf::RenderWindow* m_window;
	/// Collection of texts in the interface.
	std::vector<TextWrapper> m_texts;
	/// Collection of sprites in the interface.
	std::vector<SpriteWrapper> m_sprites;

	/// All scales are multiply by a factor one if the min axis definition is this value. 
	/// Otherwise there is a larger factor for larger windows, and vice-versa.
	unsigned int m_relativeScalingDefinition;

private:

	/**
	 * \brief Scales and repositions all window' interfaces drawables after resizement.
	 *
	 * \param[in] window: The window which was resized, and for which the interfaces will be resized.
	 * \param[in] relativeMinAxisScale The ratio between the new and old smallest window axis.
	 *            For example, if the window was resized from (1000, 500) to (750, 1000),
	 *            the scale factor is (0.75, 2), and the smallest axis ratio is 500 / 750 = 0.67
	 *            This helps to scale elements uniformly based on the smaller dimension.
	 *
	 * \warning The program asserts if the window is nullptr.
	 * \warning The program asserts if relativeMinAxisScale is set to 0.
	 */
	static void proportionKeeper(sf::RenderWindow* resizedWindow, float relativeMinAxisScale) noexcept;


	/// Collection of all interfaces to perform resizing. Stored by window.
	inline static std::unordered_multimap<sf::RenderWindow*, BasicInterface*> s_allInterfaces{};
};


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
 * \note This function accounts for the drawables' transformables such as rotation, position...
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
	sf::Vector2f trueSize{ maxSize - offset };
	trueSize.x = ceilf(trueSize.x); // Round to prevent artifacts due to subpixels
	trueSize.y = ceilf(trueSize.y);

	sf::RenderTexture renderTexture{ static_cast<sf::Vector2u>(trueSize) };
	renderTexture.clear(sf::Color::Transparent);
	(renderTexture.draw(std::forward<Ts>(drawables)), ...);
	renderTexture.display();

	sf::Texture texture{ renderTexture.getTexture() };
	texture.setSmooth(true);
	return texture;
}

} // gui namespace

#endif // BASICINTERFACE_HPP