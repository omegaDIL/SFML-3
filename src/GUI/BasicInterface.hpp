/*******************************************************************
 * \file   BasicInterface.hpp, BasicInterface.cpp
 * \brief  Declare a basic graphical user interface that has rudimentary features.
 *
 * \author OmegaDIL.
 * \date   July 2025.
 *
 * \note These files depend on the SFML library.
 * \note All assertions are disabled in release mode. If broken, undefined behavior will occur.
 *********************************************************************/

#ifndef BASICINTERFACE_HPP
#define BASICINTERFACE_HPP

#include "GraphicalResources.hpp"
#include <SFML/Graphics.hpp>
#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>
#include <cassert>

#ifndef NDEBUG 
#define ENSURE_NOT_ZERO(value, message) \
	assert((value) != (0) && (message))

#define ENSURE_SFML_WINDOW_VALIDITY(window, message) \
	ENSURE_VALID_PTR((window), (message)); \
	ENSURE_NOT_ZERO((window->getSize().x), (message)); \
	ENSURE_NOT_ZERO((window->getSize().y), (message))

#else
#define ENSURE_NOT_ZERO(value, message)
#define ENSURE_SFML_WINDOW_VALIDITY(window, message)
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
 * sf::Vector2u windowSize{ 1000, 1000 };
 * sf::RenderWindow window{ sf::VideoMode{ windowSize }, "Template sfml 3" };
 * BGUI myInterface{ &window, 1080 }; // Create the interface with the window and the relative scaling definition.
 *
 * myInterface.addText("Welcome to the GUI!", { 500, 900 }, 48, sf::Color{ 255, 255, 255 }, "__default", gui::Alignment::Center, sf::Text::Bold | sf::Text::Underlined);
 * myInterface.addText("test1", { 500, 500 }, 32, sf::Color{ 255, 0, 255 }, "__default", gui::Alignment::Center, sf::Text::Italic | sf::Text::Underlined);
 *
 * sf::RectangleShape rect{ sf::Vector2f{ 200, 200 } };
 * myInterface.addSprite(gui::createTextureFromDrawables(rect), {500, 500});
 *
 * while (window.isOpen())
 * {
 *     while (const std::optional event = window.pollEvent())
 * 	   {
 * 	       if (event->is<sf::Event::Resized>())
 *		       BGUI::windowResized(&window, windowSize);
 *
 *		   if (event->is<sf::Event::Closed>() || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape))
 *			   window.close();
 *	   }
 *
 *	   window.clear();
 *	   myInterface.draw(); // Draw the interface.
 *	   window.display();
 *	}
 * \endcode
 */
class BasicInterface
{
public:

	/**
	 * \brief Constructs the basic graphical interface.
	 * \complexity O(1)
	 *
	 * \param[in,out] window A valid pointer to the SFML window where interface elements will be rendered.
	 * \param[in] relativeScalingDefinition A scaling baseline to ensure consistent visual proportions across
	 *			  various window sizes. Scales of `sf::Transformable` elements are adjusted based on the window size
	 *			  relative to this reference value:
	 *
	 *			  - If the smallest dimension (width or height) of the window equals `relativeScalingDefinition`,
	 *				no scaling is applied (scaling factor is 1.0).
	 *			  - If the window is smaller, the scaling factor becomes less than 1.0, making elements appear smaller.
	 *			  - If the window is larger, the scaling factor becomes greater than 1.0, making elements appear larger.
	 *
	 *			  For example, if `relativeScalingDefinition` is set to 1080:
	 *			  - A window of size 1920x1080 (16/9) → factor = 1.0
	 *			  - A window of size 540x960   (9/16) → factor = 0.5
	 *			  - A window of size 3840x2160 (16/9) → factor = 2.0
	 *			  - A window of size 7680x2160 (32/9) → factor = 2.0
	 *		
	 *			  If set to 0, no scaling is applied regardless of window size.
	 *
	 * \pre `window` must be a valid.
	 * \warning The program will assert otherwise.
	 */
	explicit BasicInterface(sf::RenderWindow* window, unsigned int relativeScalingDefinition = 1080) noexcept;

	inline BasicInterface() noexcept : m_window{ nullptr }, m_texts{}, m_sprites{}, m_relativeScalingDefinition{ 1080 } {}
	BasicInterface(const BasicInterface&) noexcept = delete;
	BasicInterface(BasicInterface&& other) noexcept;
	BasicInterface& operator=(const BasicInterface&) noexcept = delete;
	BasicInterface& operator=(BasicInterface&& other) noexcept;
	virtual ~BasicInterface() noexcept; /// \complexity O(N) where N is the number of elements added


	/**
	 * \brief Adds a text.
	 * \complexity Amortized O(1).
	 *
	 * \param[in] content What the `sf::Text` will display.
	 * \param[in] fontName The name of the font that'll be used.
	 * \param[in] characterSize The character size of the `sf::Text`.
	 * \param[in] pos The position of the `sf::Text`.
	 * \param[in] scale The scale of the `sf::Text`.
	 * \param[in] color The color of the `sf::Text`.
	 * \param[in] alignment The alignment of the `sf::Text`.
	 * \param[in] style The style of the `sf::Text` (regular, italic, underlined...).
	 * \param[in] rot The rotation of the `sf::Text`.
	 *
	 * \note When this function is called for the first time, and until it is loaded successfully, it
	 *		 will try to load the default font under the name `__default` from the path `assets/defaultFont.ttf`.
	 * 
	 * \pre A font file named `defaultFont.ttf` must exist in the `assets/` folder.
	 * \post The default font is loaded and available for use.
	 * \throw LoadingGraphicalRessourceFailure Strong exception guarantee: nothing happens.
	 * 
	 * \pre   A font must have been loaded with the name you gave, either using the function `createFont`
	 *		  or being the default font under the name __default. 
	 * \post  A font will be used.
	 * \throw std::invalid_argument Strong exception guarantee: nothing happens.
	 *
	 * \see `createFont`, `Ostreamable`.
	 */
	template<Ostreamable T>
	inline void addText(const T& content, sf::Vector2f pos, unsigned int characterSize = 30u, sf::Color color = sf::Color::White, std::string_view fontName = s_defaultFontName, Alignment alignment = Alignment::Center, std::uint32_t style = 0, sf::Vector2f scale = sf::Vector2f{1, 1}, sf::Angle rot = sf::degrees(0))
	{
		ENSURE_SFML_WINDOW_VALIDITY(m_window, "The window is invalid in the function addText of BasicInterface");

		// Loads the default font.
		static constexpr std::string_view defaultFontPath{ "defaultFont.ttf" };
		if (TextWrapper::getFont(s_defaultFontName) == nullptr) [[unlikely]] // Does not exist yet.
			TextWrapper::createFont(std::string{ s_defaultFontName }, defaultFontPath); // Throws an exception if loading fails.

		float relativeScalingValue{ 1.f };
		if (m_relativeScalingDefinition != 0) [[likely]]
			relativeScalingValue *= std::min(m_window->getSize().x, m_window->getSize().y) / static_cast<float>(m_relativeScalingDefinition);

		TextWrapper newText{ content, fontName, characterSize, pos, scale * relativeScalingValue, color, alignment, style, rot };
		m_texts.push_back(std::move(newText));
	}

	/**
	 * \brief Adds a sprite.
	 * \complexity Amortized O(1).
	 *
	 * \param[in] textureName: The alias of the texture.
	 * \param[in] pos: The position of the `sf::Sprite`.
	 * \param[in] scale: The scale of the `sf::Sprite`.
	 * \param[in] rect: The display part of the texture. If the rect is equal to `sf::IntRect{}` (the
	 *					default constructor), it is set to the whole texture size.
	 * \param[in] rot: The rotation of the `sf::Sprite`.
	 * \param[in] alignment: The alignment of the `sf::Sprite`.
	 * \param[in] color: The color of the `sf::Sprite` that will be multiply by the texture color.
	 *
	 * \pre `fileName` must refer to a valid texture file in the assets directory.
	 * \post The texture is available for use via the alias `name`.
	 * \throw invalid_argument Strong exception guarantee: nothing happens.
	 *
	 * \see `createTexture`.
	 */
	void addSprite(std::string_view textureName, sf::Vector2f pos, sf::Vector2f scale = sf::Vector2f{ 1.f, 1.f }, sf::IntRect rect = sf::IntRect{}, sf::Angle rot = sf::degrees(0), Alignment alignment = Alignment::Center, sf::Color color = sf::Color::White);
	
	/**
	 * \see Similar to `addSprite`, but adds a reserved texture as well, which allows the function to
	 *		be noexcept.
	 */
	void addSprite(sf::Texture texture, sf::Vector2f pos, sf::Vector2f scale = sf::Vector2f{ 1.f, 1.f }, sf::IntRect rect = sf::IntRect{}, sf::Angle rot = sf::degrees(0), Alignment alignment = Alignment::Center, sf::Color color = sf::Color::White) noexcept;

	/**
	 * \brief Renders the interface. Texts are drawn above sprites.
	 * \complexity O(N), where N is the number of graphical elements.
	 * 
	 * \see `sf::Drawable::draw()`.
	 */
	void draw() const noexcept;


	/**
	 * \brief Handles window rescaling and updates views/interfaces' drawables accordingly.
	 * \complexity O(N + M), where N is the number of graphical elements in all interfaces associated
	 *						 with the resized window (if their 'relativeScalingDefinition's were not set to 0). And M
	 *						 is the number of views passed as arguments.
	 *
	 * This function should be called after a window has been resized. It updates the current view
	 * of the window, rescales interface elements according to their respective `relativeScalingDefinition`,
	 * and adjusts the provided views. If an interface has a scaling definition of `0`, it will not be
	 * modifies.
	 * The scales and positions/centers are modified for both transformables and views, but without distortion.
	 * The resized window is always constrained to the screen resolution, and larger than 480 px on each axis.
	 *
	 * \param[in,out] resizedWindow A valid pointer to the window that was resized.
	 * \param[in,out] previousSize The window's size before resizing; updated after the call.
	 * \param[out] views A variadic list of pointers to `sf::View` objects that should be resized
	 *                   consistently with the window. Must include the currently applied view as well.
	 *
	 * \note The function internally resizes and re-applies the current view of the window. However,
	 *       since the view is copied internally, all relevant views (including the one currently in use)
	 *       must be passed explicitly as arguments.
	 * \note If your application keeps a fixed window size, you do not need to call this function.
	 *       Simply restore the previous window size if needed.
	 *
	 * \pre `resizedWindow` must be a valid window.
	 * \pre `previousSize` must represent a valid size.
	 * \warning The program will assert otherwise.
	 */
	template<typename... Ts> requires (std::same_as<Ts, sf::View*> && ...)
	inline static void windowResized(sf::RenderWindow* resizedWindow, sf::Vector2u& previousSize, Ts... views) noexcept
	{
		ENSURE_SFML_WINDOW_VALIDITY(resizedWindow, "Precondition violated; The window is invalid in the function windowResized of BasicInterface");
		ENSURE_NOT_ZERO(previousSize.x, "Precondition violated; The previous size is invalid in the function windowResized of BasicInterface");
		ENSURE_NOT_ZERO(previousSize.y, "Precondition violated; The previous size is invalid in the function windowResized of BasicInterface");

		const sf::Vector2u maxSize{ sf::VideoMode::getDesktopMode().size };
		sf::Vector2u newSize{ resizedWindow->getSize() };

		if (newSize.x < 480) // Not larger than the current window
			newSize.x = 480;
		if (newSize.y < 480)
			newSize.y = 480;

		if (newSize.x > maxSize.x) // Not larger than the current window
			newSize.x = maxSize.x;
		if (newSize.y > maxSize.y)
			newSize.y = maxSize.y;
		
		// Updates current view and the others.
		const sf::Vector2f scaleFactor{ newSize.x / static_cast<float>(previousSize.x), newSize.y / static_cast<float>(previousSize.y) };
		sf::View view{ resizedWindow->getView() };
		view.setSize(sf::Vector2f{ view.getSize().x * scaleFactor.x, view.getSize().y * scaleFactor.y});
		view.setCenter(sf::Vector2f{ view.getCenter().x * scaleFactor.x, view.getCenter().y * scaleFactor.y});
		(views->setSize(sf::Vector2f{ views->getSize().x * scaleFactor.x, views->getSize().y * scaleFactor.y }), ...);
		(views->setCenter(sf::Vector2f{ views->getCenter().x * scaleFactor.x, views->getCenter().y * scaleFactor.y }), ...);

		// Update drawables.
		const float relativeMinAxisScale{ static_cast<float>(std::min(newSize.x, newSize.y)) / std::min(previousSize.x, previousSize.y) };
		proportionKeeper(resizedWindow, scaleFactor, relativeMinAxisScale);

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

	/// All scales are multiplied by a factor one if the min axis (between x and y) is the same as this
	/// value. Otherwise the factor is adjusted to ensure same visual proportions across different window sizes. 
	unsigned int m_relativeScalingDefinition;


	/// The name of the default font.
	inline static constexpr std::string_view s_defaultFontName{ "__default" };

private:

	/**
	 * \brief Modifies the window' interfaces drawables after resizement.
	 * \complexity O(N), where N is the number of graphical elements in all interfaces associated with
	 *					 the resized window (if their 'relativeScalingDefinition's were not set to 0).
	 * 
	 * Rescales and repositions all interface elements associated with the resized window, if their 
	 * relativeScalingDefinition isn't 0.
	 * 
	 * \param[in] window: The window which was resized, and for which the interfaces will be resized.
	 * \param[in] scaleFactor The factor by which the window was resized, in both x and y axes.
	 * \param[in] relativeMinAxisScale The ratio between the new and old smallest window axis.
	 *            For example, if the window was resized from (1000, 500) to (750, 1000),
	 *            the scale factor is (0.75, 2), and the smallest axis ratio is 750 / 500 = 1.5.
	 *            This helps to scale elements uniformly based on the smaller dimension.
	 *
	 * \pre `resizedWindow` must be a valid window.
	 * \pre `relativeMinAxisScale` must represent a valid proportion (not 0).
	 * \pre `scaleFactor` must not be equal to 0.
	 * \warning The program will assert otherwise.
	 */
	static void proportionKeeper(sf::RenderWindow* resizedWindow, sf::Vector2f scaleFactor, float relativeMinAxisScale) noexcept;


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
 * \complexity O(N), where N is the number of drawables passed as arguments.
 *  
 * From the given drawables, the function creates a texture that visually represents what
 * they look like if drawn separatly, in order. The texture size covers the distance between
 * the pixel at the leftmost/top edge of the leftmost/top drawable and the pixel at the
 * rightmost/bottom edge of the rightmost/bottom drawable, not beginning at (0;0). It accounts for
 * the drawables' transformables such as rotation, position...
 * 
 * \param[in] drawables: The drawables to create the texture from.
 *
 * \return Returns a texture created from the given drawables such as sprites, cricleShape, convexShape...
 *
 * \note The drawables' origins are moved to 0;0, and their positions are adjusted; therefore the
 *       drawables passed as arguments are very likely going to be modified.
 * \note If you create a texture from shapes, keep in mind that shapes are drawn differently than
 *		 sprites in SFML. Shapes use mathematical formulas to draw themselves, whereas sprites use arrays
 *		 of pixels (textures). While textures can be more detailed, they are also more pixelized. Be aware
 *		 that some sprites may have artefacts, which shapes usually don't have. For some reasons, this
 *	     seems to not be the case if the origins of such sprites are located at 0,0.
 * \note Consider generating a mipmap.
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