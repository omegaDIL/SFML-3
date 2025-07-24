/*******************************************************************
 * \file   GraphicalRessources.hpp, GraphicalRessources.cpp
 * \brief  Declare the entity wrappers for creating and managing graphical user interfaces.
 *
 * \author OmegaDIL.
 * \date   July 2025.
 *
 * \note These files depend on the SFML library.
 *********************************************************************/

#ifndef GRAPHICALRESSOURCES_HPP
#define GRAPHICALRESSOURCES_HPP

#include <SFML/Graphics.hpp>
#include <vector>
#include <list>
#include <unordered_map>
#include <memory>
#include <stdexcept>
#include <optional>
#include <sstream>
#include <cstdint>
#include <concepts>
#include <type_traits>
#include <cassert>

#ifndef NDEBUG 
#define ENSURE_VALID_PTR(ptr) \
	assert((ptr) && "ENSURE_VALID_PTR failed: pointer is null.")

#define ENSURE_NOT_OUT_OF_RANGE(index, size) \
	assert(((index) >= 0 && static_cast<size_t>((index)) < static_cast<size_t>((size))) && \
		   "ENSURE_NOT_OUT_OF_RANGE failed: index out of valid range.")
#else
#define ENSURE_VALID_PTR()
#define ENSURE_NOT_OUT_OF_RANGE(index, size)
#endif

 /**
  * \brief An exception thrown for graphical errors when loading ressources.
  *
  * \see `std::runtime_error`.
  */
struct LoadingGraphicalRessourceFailure : public std::runtime_error
{
public:

	/**
	 * \brief Initializes the exception with a message.
	 * \complexity O(1).
	 *
	 * \param[in] message: The error message.
	 */
	inline explicit LoadingGraphicalRessourceFailure(const std::string& message) : std::runtime_error{ message }
	{}


	/**
	 * \brief Returns the error message.
	 * \complexity O(1).
	 */
	inline virtual const char* what() const noexcept override
	{
		return std::runtime_error::what();
	}
};

namespace gui
{

///////////////////////////////////////////////////////////////////////////////////////////////////
/// A `sf::Transformable` wrapper.
///////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * \brief An enum that represents the alignment of a `sf::Transformable`.
 * For each axis, horizontal and vertical, there are three different positions: Top/Left, Center,
 * Bottom/Right. The "y axis" takes the first two bits, and the "x axis" takes the next two bits.
 * The Center value is 0b0000, which tells that both x and y is at the center since both of the two
 * pair of bits are set to 0. If you specify a "x axis" alignment, the "y axis" is set to be the
 * center. You can use the operator | to combine a "x axis" alignment with a "y axis" alignment. 
 * 
 * \note With 2 bits you have 4 possibilities, however this enum only uses 3 per pair. Therefore one
 *		 per pair has to use one more "possibility" in terms of binary representation. In this case, it
 *		 is left and top.
 *  
 * \see `operator|`, `computeNewOrigin`.
 */
enum class Alignment : uint8_t
{
	Center = 0, // Center is the default value for both x and y axis.

	Bottom = 1 << 0,
	Top = 1 << 1,

	Right = 1 << 2,
	Left = 1 << 3
};

/**
 * \brief Casts both operand to integers and apply the operator|.
 * \complexity O(1).
 *
 * \param[in] lhs: The first alignment that'll be casted.
 * \param[in] rhs: The second alignment to that'll be casted.
 *
 * \return The new alignment that was created with both alignments.
 *
 * \note If the alignment are not compatible, lhs is returned.
 */
Alignment operator|(Alignment lhs, Alignment rhs) noexcept;

/**
 * \brief Calculates the origin's coordinate given an alignment and a `sf::Transformable` bound.
 * \complexity O(1).
 * 
 * \param[in] bound: The bound of the `sf::Transformable` for which you want to compute the origin.
 * \param[in] alignment: The alignment of the `sf::Transformable`.
 * 
 * \return Returns the corresponding origin given an alignment and a bound of a `sf::Transformable`.
 * 
 * \see `Alignment`, `sf::Transformable::setOrigin`.
 */
sf::Vector2f computeNewOrigin(sf::FloatRect bound, Alignment alignment) noexcept;


/**
 * \brief Provides a basic wrapper for a `sf::Transformable`.
 * The wrapper basically uses composition with a `sf::Transformable` pointer (to avoid diamond
 * inheritance) to remove/add functions. First, it redefines the basic functions of `sf::Transformable`
 * like `move`, `scale`, `rotate`, `setPosition`... Except for `setOrigin`, because the wrapper
 * replaces it with `setAlignment` (as a pure virtual function) which should act as a `setOrigin`
 * function and should take into account the alignment. The wrapper also adds the function `resize`
 * that can be called each time the event window resize was triggered. Finally, a boolean that tells
 * whether or not the transformable should be drawn (as they are usually coupled with a `sf::Drawable`)
 * 
 * \note    This class is pure virtual. The functions are `setAlignment` and `setColor`.
 * \note    You should implement the getter functions for the `sf::Transformable`.
 * \warning Keep the transformable loaded as long as the corresponding instance is used - no nullptr
 *			checks are done. It is only a wrapper, and speed should remain as similar as a regular
 *			`sf::Transformable` instance. The program will assert otherwise.
 * 
 * \see `sf::Transformable`, `Alignment`.
 */
class TransformableWrapper
{
public:

	/**
	 * \brief Initializes the wrapper.
	 * \complexity O(1).
	 *
	 * \param[out] transformable: What `sf::Transformable` the wrapper is being used for.
	 * \param[in]  pos: The position of the `sf::Transformable`.
	 * \param[in]  scale: The scale of the `sf::Transformable`.
	 * \param[in]  rot: The rotation of the `sf::Transformable`.
	 * \param[in]  alignment: The alignment of the `sf::Transformable`.
	 * 
	 * \note The scale parameter should take into account the current size of the window. In a smaller
	 * 		 window, the same `sf::Transformable` will appear larger, and vice-versa.
	 * 
	 * \pre   The transformable pointer must not be nullptr.
	 * \post  The wrapper will use the right transformable.
	 * \throw std::invalid_argument basic exception guarantee: the instance is not usable.
	 */
	TransformableWrapper(sf::Transformable* transformable, sf::Vector2f pos, sf::Vector2f scale, sf::Angle rot = sf::degrees(0), Alignment alignment = Alignment::Center);
	
	TransformableWrapper() noexcept : hide{ false }, m_alignment{ Alignment::Center }, m_transformable{ nullptr } {}
	TransformableWrapper(TransformableWrapper const&) noexcept = default;
	TransformableWrapper(TransformableWrapper&&) noexcept = default;
	TransformableWrapper& operator=(TransformableWrapper const&) noexcept = default;
	TransformableWrapper& operator=(TransformableWrapper&&) noexcept = default;
	virtual ~TransformableWrapper() noexcept = default;


	/**
	 * \see Similar to the `sf::Transformable::move` function.
	 */
	void move(sf::Vector2f pos) noexcept;

	/**
	 * \see Similar to the `sf::Transformable::scale` function.
	 */
	void scale(sf::Vector2f scale) noexcept;

	/**
	 * \see Similar to the `sf::Transformable::rotate` function.
	 */
	void rotate(sf::Angle rot) noexcept;

	/**
	 * \see Similar to the `sf::Transformable::setPosition` function.
	 */
	void setPosition(sf::Vector2f pos) noexcept;

	/**
	 * \see Similar to the `sf::Transformable::setScale` function.
	 * 
	 * \note The scale parameter should take into account the current size of the window. In a smaller
	 * 		 window, the same `sf::Transformable` will appear larger, and vice-versa.
	 */
	void setScale(sf::Vector2f scale) noexcept;

	/**
	 * \see Similar to the `sf::Transformable::setRotation` function.
	 */
	void setRotation(sf::Angle angle) noexcept;

	/**
	 * \brief Sets the `sf::Transformable`'s origin given alignment.
	 * If the alignment is top/left, then the origin will be located at the top/left corner, which is
	 * (0;0) in SFML. If the alignment is center/right, then the origin should be located at the
	 * coordinate (size.x/2; size).
	 * \complexity O(1).
	 *
	 * \param[in] alignment The new alignment of the `sf::Transformable`.
	 *
	 * \code
	 * // Implementation should be like:
	 * m_alignment = alignment;
	 * setOrigin(computeNewOrigin(getLocalBounds(), m_alignment));
	 * \endcode
	 * 
	 * \see `sf::Transformable::setOrigin()`, `computeNewOrigin()`.
	 */
	virtual void setAlignment(Alignment alignment) noexcept = 0;

	/**
	 * \brief Apply a new color to the `sf::Transformable` object.
	 * 
	 * \param[in] color: The new color.
	 */
	virtual void setColor(sf::Color color) noexcept = 0;

	/**
	 * \brief Resizes and repositions the `sf::Transformable`, e.g., when the window is resized.
	 * Avoid distortion. If you actually want distortion, you can't use this function.
	 *
	 * \param[in] windowScaleFactor The per-axis scaling factor (x and y) of the window size.
	 * \param[in] relativeMinAxisScale The ratio between the new and old smallest window axis.
	 *            For example, if the window was resized from (1000, 500) to (750, 1000),
	 *            the scale factor is (0.75, 2), and the smallest axis ratio is 500 / 750 = 0.67
	 *            This helps to scale elements uniformly based on the smaller dimension.
	 */
	void resized(sf::Vector2f windowScaleFactor, float relativeMinAxisScale) noexcept;


	/// Tells if the element should be drawn.
	bool hide; 

protected:
	
	/**
	 * \brief Initializes the wrapper.
	 * \complexity O(1).
	 *
	 * \param[out] transformable: What `sf::Transformable` the wrapper is being used for.
	 * \param[in]  pos: The position of the `sf::Transformable`.
	 * \param[in]  scale: The scale of the `sf::Transformable`.
	 * \param[in]  rot: The rotation of the `sf::Transformable`.
	 * \param[in]  alignment: The alignment of the `sf::Transformable`.
	 *
	 * \note The scale parameter should take into account the current size of the window. In a smaller
	 * 		 window, the same `sf::Transformable` will appear larger, and vice-versa.
	 *
	 * \pre   The transformable pointer must not be nullptr.
	 * \post  The wrapper will use the right transformable.
	 * \throw std::invalid_argument strong exception guarantee: you can call this function again.
	 */
	void create(sf::Transformable* transformable, sf::Vector2f pos, sf::Vector2f scale, sf::Angle rot = sf::degrees(0), Alignment alignment = Alignment::Center);


	/// The current alignment of the `sf::Transformable`.
	Alignment m_alignment; 

private:

	/// What `sf::Transformable` the wrapper is being used for.
	sf::Transformable* m_transformable; 
};

///////////////////////////////////////////////////////////////////////////////////////////////////
/// A `sf::Transformable` wrapper.
///////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////
/// A `sf::Text` wrapper.
///////////////////////////////////////////////////////////////////////////////////////////////////

/// The type must be streamable to `std::basic_ostream`.
template <typename T>
concept Ostreamable = requires(std::ostream & os, T t)
{
	{ os << t } -> std::same_as<std::ostream&>;
};

/**
 * \brief A wrapper for `sf::Text` that simplifies its use.
 * Rather than keeping a pointer to a font like `sf::Text` does, it actually stores multiple fonts
 * for the user to choose from. He can add/remove as much as he wants while keeping O(1) complexity.
 * 
 * \see `sf::Text`, `sf::Font`, `TransformableWrapper`.
 */
class TextWrapper final : public TransformableWrapper
{
public:

	/**
	 * \brief Initializes the wrapper.
	 * \complexity O(1).
	 *
	 * \param[in] content: What the `sf::Text` will display.
	 * \param[in] fontName: The name of the font that'll be used.
	 * \param[in] characterSize: The character size of the `sf::Text`.
	 * \param[in] pos: The position of the `sf::Text`.
	 * \param[in] scale: The scale of the `sf::Text`.
	 * \param[in] color: The color of the `sf::Text`.
	 * \param[in] alignment: The alignment of the `sf::Text`.
	 * \param[in] style: The style of the `sf::Text` (regular, italic, underlined...).
	 * \param[in] rot: The rotation of the `sf::Text`.
	 * 
	 * \note The scale parameter should take into account the current size of the window. In a smaller
	 *       window, the same `sf::Text` will appear larger, and vice - versa.
	 * \note Some styles may not be available with your font.
	 * 
	 * \pre   You should have loaded a font with the name you gave, using the function `createFont`.
	 * \post  The correct font will be used.
	 * \throw std::invalid_argument basic exception guarantee: the instance is not usable.
	 * 
	 * \see `createFont`, `Ostreamable`.
	 */
	template<Ostreamable T>
	inline TextWrapper(const T& content, const std::string& fontName, unsigned int characterSize, sf::Vector2f pos, sf::Vector2f scale, sf::Color color = sf::Color::White, Alignment alignment = Alignment::Center, sf::Text::Style style = sf::Text::Style::Regular, sf::Angle rot = sf::degrees(0))
		: TransformableWrapper{}, m_wrappedText{ nullptr }
	{
		sf::Font* usedFont{ getFont(fontName) };
		if (usedFont == nullptr)
			throw std::invalid_argument{ "This name is not affiliate with any font" };
		
		m_wrappedText = std::make_unique<sf::Text>(*usedFont, "", characterSize);
		this->create(m_wrappedText.get(), pos, scale, rot, alignment);

		setColor(color);
		setStyle(style);
		setContent(content); // Also computes the origin of the text with the correct alignment.
	}

	TextWrapper() noexcept = delete;
	TextWrapper(TextWrapper const&) noexcept = default;
	TextWrapper(TextWrapper&&) noexcept = default;
	TextWrapper& operator=(TextWrapper const&) noexcept = default;
	TextWrapper& operator=(TextWrapper&&) noexcept = default;
	virtual ~TextWrapper() noexcept = default;


	/**
	 * \brief Updates the text content.
	 * \complexity O(1).
	 *
	 * \param[in] content: The new content for the text.
	 *
	 * \see `sf::Text::setString`, `Ostreamable`.
	 */
	template<Ostreamable T>
	void setContent(const T& content) noexcept
	{
		std::ostringstream oss{}; // Convert the content to a string.
		oss << content; // Assigning the content to the variable.

		m_wrappedText->setString(oss.str());
		m_wrappedText->setOrigin(computeNewOrigin(m_wrappedText->getLocalBounds(), m_alignment));
	}

	/**
	 * \brief Sets a new font for the text, only if the resource exists.
	 * \complexity O(1).
	 *
	 * \param[in] name: The name of the font that'll be used.
	 *
	 * \return `true` if it was set, false `otherwise`.
	 *
	 * \note This function is designed not to throw, to support scenarios where the user tries multiple
	 *		 font names until one is successfully found and set. This is useful when font loading may have
	 *		 failed earlier, and fallback attempts are expected behavior.
	 *
	 * \see `sf::Text::setFont`, `createFont`, `loadFontFromFile`.
	 */
	bool setFont(const std::string& name) noexcept;

	/**
	 * \see `sf::Text::setCharacterSize`.
	 */
	void setCharacterSize(unsigned int size) noexcept;

	/**
	 * \see `sf::Text::setFillColor`.
	 */
	virtual void setColor(sf::Color color) noexcept final;

	/**
	 * \see `sf::Text::setStyle`.
	 * 
	 * \note Some styles may not be available with your font.
	 */
	void setStyle(sf::Text::Style style) noexcept;

	/**
	 * \brief Sets the `sf::Text`'s origin given alignment.
	 * If the alignment is top/left, then the origin will be located at the top/left corner, which is
	 * (0;0) in SFML. If the alignment is center/right, then the origin should be located at the
	 * coordinate (size.x/2; size).
	 * \complexity O(1).
	 *
	 * \param[in] alignment The new alignment of the `sf::Text`.
	 *
	 * \see `sf::Transformable::setOrigin()`, `computeNewOrigin()`.
	 */
	virtual void setAlignment(Alignment alignment) noexcept final;

	/**
	 * \brief Accesses the wrapped `sf::Text` object.
	 * \complexity O(1).
	 *
	 * \return A reference to the wrapped `sf::Text` object.
	 */
	[[nodiscard]] inline const sf::Text& getText() const noexcept
	{
		return *m_wrappedText;
	}


	/**
	 * \brief Creates a font into the wrapper with a name, which can be used by all instances.
	 * You can also use this function to replace an existing font by another one.
	 * \complexity O(1).
	 *
	 * \param[in] name: The alias of a font.
	 * \param[in] fileName: The file within the assets folder that contains the font.
	 *
	 * \note The font will be loaded immediately.
	 * \warning Do not replace a font which is currently in use.
	 *
	 * \pre The file name should be a correct path to a font within the assets folder.
	 * \post The font will be loaded.
	 * \throw LoadingGraphicalRessourceFailure strong exception guarantee: nothing happens.
	 *
	 * \see `loadFontFromFile`, `setFont`.
	 */
	static void createFont(const std::string& name, const std::string& fileName);

	/**
	 * \brief Creates a font into the wrapper with a name, which can be used by all instances.
	 * You can also use this function to replace an existing font by another one.
	 * \complexity O(1).
	 *
	 * \param[in] name: The alias of a font.
	 * \param[in] font: The font you want to load (using std::move is recommended).
	 * 
	 * \warning Do not replace a font which is currently in use.
	 * 
	 * \see `loadFontFromFile`, `setFont`.
	 */
	static void createFont(const std::string& name, sf::Font font) noexcept;
	
	/**
	 * \brief Removes the font from the wrapper with the name given. No effet if not there.
	 * \complexity O(1).
	 *
	 * \param[in] name: The alias of a font.
	 * 
	 * \warning No text should currently use the font you remove.
	 */
	static void removeFont(const std::string& name) noexcept;

	/**
	 * \brief Returns a font ptr, or nullptr if it does not exist.
	 * \complexity O(1).
	 *
	 * \param[in] name: The alias of a font.
	 *
	 * \return The address of the font.
	 */
	[[nodiscard]] static sf::Font* const getFont(const std::string& name) noexcept;

private:

	/// What `sf::Text` the wrapper is being used for.
	std::unique_ptr<sf::Text> m_wrappedText;


	/// Contains all loaded fonts
	static std::list<sf::Font> s_allFonts; 
	/// Allows to find fonts with a name in O(1) time complexity.
	static std::unordered_map<std::string, std::list<sf::Font>::iterator> s_accessToFonts;
};


/**
 * \brief Loads a font from a file.
 * \complexity O(1).
 * 
 * \param[out] errorMessage: Will add the error message to this stream if the loading fails
 * \param[in]  fileName: The name of the file.
 * \param[in]  path: The path to this file (assets file by default).
 * 
 * \return a sf::Font if the loading was successful, std::nullopt otherwise.
 * 
 * \note The complete path is "path + fileName" therefore it does not matter if the fileName
 *		 variable contains also a part of the path, as long as the complete path is valid.
 * \note A message is added to the stream only if the function returns std::nullopt.
 * 
 * \see `TextWrapper`, `sf::Font::openFromFile`.
 */
std::optional<sf::Font> loadFontFromFile(std::ostringstream& errorMessage, std::string const& fileName, std::string const& path = "../assets/") noexcept;

///////////////////////////////////////////////////////////////////////////////////////////////////
/// A `sf::Text` wrapper.
///////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////
/// A `sf::Sprite` wrapper.
///////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * \brief Used within the sprite wrapper to represent a texture.
 * The pointer to the texture allows the user to have it set to nullptr to free memory whenever he
 * wants to reduce memory usage. The file name contains the name of the file where the texture is
 * stored, within the assets folder.
 * 
 * \note You should not unload a texture that has no file name attached to it, as it might be not
 *		 loadable after.
 * 
 * \see `SpriteWrapper`, `sf::Texture`. 
 */
struct TextureHolder
{
	std::unique_ptr<sf::Texture> actualTexture;
	std::string fileName;
};

/**
 * \brief Represents a ressource for a sprite to use, with a texture and a rectangle. 
 * Does not own the texture.
 * 
 * \see `SpriteWrapper`, `TextureHolder`.
 */
struct TextureInfo
{
	TextureHolder* texture;
	sf::IntRect displayedTexturePart;
};

/**
 * \brief A wrapper for `sf::Sprite` that simplifies its use.
 * The wrapper simplifies the use of `sf::Texture` as it stores all of them. We can distinguish two
 * types of textures: shared and reserved; shared textures can be set to multiple sprites while
 * reserved ones are owned by one instance. You have to call the function `createTexture` to put
 * textures in the wrapper in both case, and then `addTexture` for each instance that uses that
 * texture (only once for reserved texture!). Moreover, as one sprite can also have multiple textures,
 * the 'texture vector' contains access to them. Call the function `addTexture` to add more textures,
 * or the same texture with different `sf::IntRect`s. The 'texture vector' is kept in order as
 * textures are added. Keep in mind the difference between the functions `create`/`remove` and 
 * `load`/`unload`. The function `remove` completely deletes a texture when it has no chance to be
 * used again (e.g. when no sprite instance can access it from their 'texture vector'). The function
 * `unload` only frees memory for a texture, but all ptr in any 'texture vector' of all instances
 * will still be valid. Reserved texture aren't removable using the function `removeTexture`, but
 * removed when the destructor of that sprite instance is called. However, they can be unloaded.
 * 
 * \note Reserved texture may use a small additional amount of memory.
 *
 * \see `sf::Sprite`, `sf::Texture`, `TransformableWrapper`.
 * 
 * \code Simple sprites: buttons.
 * std::string button{ "buttonTex" };
 * SpriteWrapper::createTexture(button, "button.jpg", Reserved::No, true); // Loads it immediately
 * 
 * sf::Vector2f scale{ std::min(windowSize.x, windowSize.y) / 1080, 0 };
 * scale.y = scale.x; // Scale the buttons for different screen definition.
 * SpriteWrapper launchGame  { button, { 200, 200 }, scale, sf::degrees(0), Alignment::Top | Alignment::Left };
 * SpriteWrapper settingsGame{ button, { 400, 200 }, scale, sf::degrees(0), Alignment::Top | Alignment::Left };
 * SpriteWrapper closeGame   { button, { 600, 200 }, scale, sf::degrees(0), Alignment::Top | Alignment::Left };
 * \endcode
 * 
 * \code main character sprite
 * sf::Vector2f scale{ std::min(windowSize.x, windowSize.y) / 1080, 0 };
 * scale.y = scale.x; // Scale the buttons for different screen definition.
 * 
 * // No sense for these textures to be applied to other sprite so they are reserved
 * SpriteWrapper::createTexture("hero run1080", "1080/running_sprite.png", Reserved::Yes, true); // true means they are loaded
 * SpriteWrapper::createTexture("hero run2160", "2160/running_sprite.png", Reserved::Yes); // false by default.
 * SpriteWrapper::createTexture("hero attack1080", "1080/attacking_sprite.png", Reserved::Yes, true);
 * SpriteWrapper::createTexture("hero attack2160", "2160/attacking_sprite.png"); // Reserved and false by default.
 * 
 * // Applies the texture to the sprite while claiming the reserve state.
 * SpriteWrapper player{ "hero run1080", sf::IntRect{ {}, {50, 50} }, { 960, 540 }, scale };
 * player.addTexture("hero run1080", sf::IntRec{ {50, 50}, {50, 50} }); // First intRect was added in the constructor.
 * player.addTexture("hero run2160, sf::IntRect{ {}, {100, 100} }, sf::IntRec{ {100, 100}, {100, 100} }); // 4k texture are larger
 * player.addTexture("hero attack1080", sf::IntRec{ {0, 0}, {50, 50} }, sf::IntRec{ {50, 50}, {50, 50} });
 * player.addTexture("hero attack2160, sf::IntRect{ {}, {100, 100} }, sf::IntRec{ {100, 100}, {100, 100} });
 * 
 * // Let's assume we're in the game loop
 * if (player.getSprite().getScale().x == 1.8) // Accounts for screen definition and other scaling reasons (e.g. gameplay, zooming).
 * {	// Let's say 1.8 times is too pixeled so we load the 4k textures. 
 *		SpriteWrapper::loadTexture("hero run2160"); // using another thread is recommended.
 *		SpriteWrapper::loadTexture("hero attack2160");
 * 
 *		player.setScale(0.5f, 0.5f); // 4K is 2 times larger on each axis
 *		player.switchToNextTexture(2); // 4k textures were added 2 index after 1080p, regardless of which textures is currently set.
 * 
 *		SpriteWrapper::unloadTexture("hero run1080"); // using another thread is recommended.
 *		SpriteWrapper::unloadTexture("hero attack1080");
 * }	// The same would go for smaller scaled textures to reduce memery usage
 * 
 * if (isRunning) // Running animation is played
 * {
 *		if(DidEnoughTimeElapse) // boolean flag to animate the textures
 *		{	// Two textures (intRect here) for the running animation: if first is displayed then switch to next one, otherwise previous one.
 *			int nextAnimatingRunningTexture{ (player.getCurrentTextureIndex() % 2 == 0) ? 1 : -1  };
 *			player.switchToNextTexture(nextAnimatingRunningTexture);
 *		}
 * }
 * \endcode
 */
class SpriteWrapper final : public TransformableWrapper
{
public:

	/**
	 * \brief Initializes the wrapper.
	 * \complexity O(1).
	 * 
	 * \param[in] textureName: The alias of the texture.
	 * \param[in] rect: The display part of the texture. If size is 0, it is set to the whole texture size.
	 * \param[in] pos: The position of the `sf::Sprite`.
	 * \param[in] scale: The scale of the `sf::Sprite`.
	 * \param[in] rot: The rotation of the `sf::Sprite`.
	 * \param[in] alignment: The alignment of the `sf::Sprite`.
	 * \param[in] color: The color of the `sf::Sprite` that will be multiply by the texture color.
	 * 
	 * \note The scale parameter should take into account the current size of the window. In a smaller
	 *       window, the same `sf::Text` will appear larger, and vice - versa.
	 * \note The constructor accounts for the reserve state of the texture.
	 * 
	 * \pre   You should have loaded a texture with the name you gave, using the function `createTexture`.
	 * \pre   It should not be already reserved.
	 * \post  The correct texture will be used.
	 * \throw std::invalid_argument basic exception guarantee: the instance is not usable.
	 * 
	 * \see `createTexture`.
	 */
	SpriteWrapper(const std::string& textureName, sf::IntRect rect, sf::Vector2f pos, sf::Vector2f scale, sf::Angle rot = sf::degrees(0), Alignment alignment = Alignment::Center, sf::Color color = sf::Color::White);

	/**
	 * \brief Initializes the wrapper.
	 * \complexity O(1).
	 *
	 * \param[in] textureName: The alias of the texture. The rect is the whole texture.
	 * \param[in] pos: The position of the `sf::Sprite`.
	 * \param[in] scale: The scale of the `sf::Sprite`.
	 * \param[in] rot: The rotation of the `sf::Sprite`.
	 * \param[in] alignment: The alignment of the `sf::Sprite`.
	 * \param[in] color: The color of the `sf::Sprite` that will be multiply by the texture color.
	 *
	 * \note The scale parameter should take into account the current size of the window. In a smaller
	 *       window, the same `sf::Text` will appear larger, and vice - versa.
	 * \note The constructor accounts for the reserve state of the texture.
	 *
	 * \pre   You should have loaded a texture with the name you gave, using the function `createTexture`.
	 * \pre   It should not be already reserved.
	 * \post  The correct texture will be used.
	 * \throw std::invalid_argument basic exception guarantee: the instance is not usable.
	 *
	 * \see `createTexture`.
	 */
	SpriteWrapper(const std::string& textureName, sf::Vector2f pos, sf::Vector2f scale, sf::Angle rot = sf::degrees(0), Alignment alignment = Alignment::Center, sf::Color color = sf::Color::White);

	SpriteWrapper() noexcept = delete;
	SpriteWrapper(SpriteWrapper const&) noexcept = delete; // For reserved texture.
	SpriteWrapper(SpriteWrapper&&) noexcept = default;
	SpriteWrapper& operator=(SpriteWrapper const&) noexcept = delete; // For reserved texture.
	SpriteWrapper& operator=(SpriteWrapper&&) noexcept = default;
	virtual ~SpriteWrapper() noexcept; /// \complexity O(N) where N is the number of reserved texture to deallocate.


	/**
	 * \see `sf::Sprite::setColor`.
	 */
	virtual void setColor(sf::Color color) noexcept final;

	/**
	 * \brief Sets the `sf::Sprite`'s origin given alignment.
	 * If the alignment is top/left, then the origin will be located at the top/left corner, which is
	 * (0;0) in SFML. If the alignment is center/right, then the origin should be located at the
	 * coordinate (size.x/2; size).
	 * \complexity O(1).
	 *
	 * \param[in] alignment The new alignment of the `sf::Sprite`.
	 *
	 * \note Keeping the origin of a small sprite at Top/Left is recommended. The way SFML handles drawing
	 *		 sprites make their textures, sometimes, larger at certain lines/pixels and smaller at other
	 *		 places. For some reason, this problem does not occur when the origin is at 0;0. So unless, you
	 *		 want to rotate the sprite by a different alignment, or the texture is big enough to not see the
	 *		 difference, set the alignment to Top/Left. If you still want to give positions as if the center
	 *		 was the origin, you can use the `spriteWrapped.setPosition` function and then use that line: 
	 *		 `spriteWrapped.move(-computeNewOrigin(spriteWrapped.getSprite().getLocalBounds(), Alignment::Center))`
	 *		 
	 * \see `sf::Transformable::setOrigin`, `computeNewOrigin`, `Alignment`.
	 */
	virtual void setAlignment(Alignment alignment) noexcept final;

	/**
	 * \brief Accesses the wrapped `sf::Sprite` object.
	 * \complexity O(1).
	 *
	 * \return A reference to the wrapped `sf::Sprite` object.
	 */
	[[nodiscard]] inline const sf::Sprite& getSprite() const noexcept
	{
		return *m_wrappedSprite;
	}

	/**
	 * \brief Applies the texture/its rect to the sprite by looking at the next index.
	 * Similar to texture = cur + offset. Loop If the overall index is out of range. If it is 2 past
	 * the last texture, then the texture applied is the texture indexed at 2 in the texture vector. 
	 * \complexity O(1).
	 * 
	 * @param[in] indexOffset: The next index. By default 1 : index = curIndex + 1. Can be negative.
	 * 
	 * \note The function will load the next texture if it was not previously loaded. That being said,
	 *		 if you want to avoid a sudden loss of fps (especially for large texture), consider load it
	 *		 before
	 * \note The previous texture is not unloaded. 
	 *
	 * \pre If loading, the file name should be a correct path to a texture within the assets folder.
	 * \post The texture will be loaded.
	 * \throw LoadingGraphicalRessourceFailure strong exception guarantee: nothing happens.
	 */
	void switchToNextTexture(long long indexOffset = 1);

	/**
	 * \see Same as switchToNextTexture but you specify the index.
	 * 
	 * \warning If the index is out if range, the program will assert.
	 */
	void switchToTexture(size_t index);

	/**
	 * \brief Returns the index of the texture set to this sprite, within the texture vector.
	 * \complexity O(1).
	 * 
	 * \return The index within vector.
	 */
	[[nodiscard]] inline size_t getCurrentTextureIndex() const noexcept
	{
		return m_curTextureIndex;
	}

	/**
	 * \brief Adds a texture to the texture vector.
	 * For each `sf::IntRect` given, a pair of `sf::Texture` ptr + `sf::IntRect` is added to the
	 * instance. Switching between different textures or intrect is done the same way with the function
	 * `switchToNextTexture`. Don't be afraid to add a lot of textures, just ptr/ints are stored. Reserved
	 * textures are restricted to a single instance, but this function can still be called with a reserved
	 * texture if it belongs to the current instance.
	 * \complexity O(1) For shared textures or non claimed reserved textures.
	 * \complexity O(N) For claimed reserved textures. N is the number of unique reserved texture already
	 *					claimed by this instance.
	 * 
	 * \param name: The alias of a texture.
	 * \param rects: All intrects you want to add. If size is 0, it is set to the whole texture size.
	 * 
	 * \return `true` if added to the texture vector, `false` otherwise.
	 * 
	 * \note The first pair (texture + rect) is added as index 0, the second one as index 1, and so on...
	 *       You keep track of your indexes.
	 * \note You can't edit the order/indexes and which texture was set to the sprite once added.
	 * \note This function is designed not to throw if the texture was not found, to support scenarios
	 *		 where the user tries multiple textures names until one is successfully found and set. This is
	 *		 useful when texture loading may have failed earlier, and fallback attempts are expected behavior.
	 * 
	 * \pre For reserved textures, it must not have been reserved yet.
	 * \post The texture will be added.
	 * \throw std::invalid_argument strong exception guarantee: nothing happens.
	 * 
	 * \see `createTexture`, `loadTexture`, `unloadTexture`, `switchToNextTexture`.
	 */
	template<typename... Ts> requires (std::same_as<Ts, sf::IntRect> && ...)
	bool addTexture(std::string name, Ts... rects)
	{
		auto mapAccessIterator{ s_accessToTextures.find(name) };
		if (mapAccessIterator == s_accessToTextures.end()) [[unlikely]]
			return false; // Texture not there.

		auto mapUniqueIterator{ s_allUniqueTextures.find(mapAccessIterator->second) };
		if (mapUniqueIterator != s_allUniqueTextures.end() // is reserved.
		&&  mapUniqueIterator->second == true // has already been claimed by an instance...
		&&  std::find(m_uniqueTextures.begin(), m_uniqueTextures.end(), name) == m_uniqueTextures.end()) [[unlikely]] // ...but not by this one.
			throw std::invalid_argument{ "The reserved texture was not available anymore for this sprite instance" };

		TextureHolder* texture{ &*mapAccessIterator->second };

		(m_textures.push_back(TextureInfo{ texture, rects }), ...);

		if (mapUniqueIterator->second == false) // For reserved texture.
		{
			mapUniqueIterator->second = true;  // Mark it as true, meaning the reserve state was claimed.
			m_uniqueTextures.push_back(name);
		}

		return true;
	}

	/**
	 * \see Similar to the `addTexture` template overloaded function, with an IntRect that covers the
	 *		whole texture since it has a size 0.
	 */
	inline bool addTexture(std::string name)
	{
		return addTexture(name, sf::IntRect{});
	}


	/**
	 * \brief Tells whether or not a texture should be reserved to a specific instance.
	 * `Yes` if you want the texture to be availabe for a single sprite, preventing
	 *  any other instances to use it.
	 */
	enum class Reserved : uint8_t { Yes, No };

	/**
	 * \brief Creates a texture into the wrapper with a name, which could be used by all instances.
	 * You can also use this function to replace an existing texture by another one.
	 * \complexity amor O(1).
	 *
	 * \param[in] name: The alias of a texture.
	 * \param[in] fileName: The file within the assets folder that contains the texture.
	 * \param[in] shared: `Yes` if the first instance to use the texture reserves it. 
	 * \param[in] loadImmediately: `true` if you want the texture to be loaded from the file when the
	 *							   function is called. if so, may throw an exception if failed.
	 *
	 * \note If you replace a texture, the reserved state is not updated. A reserved texture belongs to
	 *		 its sprite instance and you can't "take it away" once specified, only replace it.
	 * \warning Do not replace a texture which is currently set to a `sf::Sprite`.
	 *
	 * \pre The file name should be a correct path to a texture within the assets folder.
	 * \post The texture will be loaded.
	 * \throw LoadingGraphicalRessourceFailure strong exception guarantee: nothing happens.
	 *
	 * \see `loadTextureFromFile`, `addTexture`.
	 */
	static void createTexture(const std::string& name, const std::string& fileName, Reserved shared = Reserved::Yes, bool loadImmediately = false);
	
	/**
	 * \brief Creates a texture into the wrapper with a name, which could be used by all instances.
	 * You can also use this function to replace an existing texture by another one.
	 * \complexity amor O(1).
	 *
	 * \param[in] name: The alias of a texture.
	 * \param[in] fileName: The file within the assets folder that contains the texture.
	 * \param[in] shared: `Yes` if the first instance to use the texture reserves it.
	 *
	 * \note Because only a `sf::Texture` was given without a file name, the texture can't be unloaded.
	 * \note If you replace a texture, the reserved state is not updated. A reserved texture belongs to
	 *		 its sprite instance and you can't "take it away" once specified, only replace it.
	 * \warning Do not replace a texture which is currently set to a `sf::Sprite`.
	 *
	 * \see `loadTextureFromFile`, `addTexture`.
	 */
	static void createTexture(const std::string& name, sf::Texture texture, Reserved shared = Reserved::Yes) noexcept;
	
	/**
	 * \brief Removes the texture from the wrapper with the name given. No effet if not there. 
	 * You can remove a loaded or unloaded texture.
	 * \complexity O(1).
	 *
	 * \param[in] name: The alias of a texture.
	 *
	 * \warning The texture should not be within any instances' texture vector.
	 * 
	 * \see `unloadTexture`.
	 */
	static void removeTexture(const std::string& name) noexcept;

	/**
	 * \brief Returns a textureHolder ptr, or nullptr if it does not exist.
	 * \complexity O(1).
	 *
	 * \param[in] name: The alias of a texture.
	 *
	 * \return The address of the texture.
	 *
	 * \see `TextureHolder`.
	 */
	[[nodiscard]] static TextureHolder* const getTexture(const std::string& name) noexcept;

	/**
	 * \brief Loads an existing texture from a file into the graphical ram. 
	 * \complexity O(1).
	 * 
	 * \param[in] name: The alias of a texture.
	 * \param[in] failingImpliesRemoval: By setting this to `true`, if the loading fails, the texture
	 *			  will automatically be totally removed from the wrapper (see `removeTexture`). ONLY includes
	 *			  LOADING FAILURE, not if the texture path was not provided/the texture not found/already loaded.
	 * 
	 * \return `true` if loading was successful/already loaded, `false` otherwise.
	 * 
	 * \note Using another thread is recommended for heavy textures.
	 * \note This function is designed not to throw if the texture was not found, to support scenarios
	 *		 where the user tries multiple textures names until one is successfully found and set. This is
	 *		 useful when texture loading may have failed earlier, and fallback attempts are expected behavior.
	 * 
	 * \pre The file name should be a correct path to a texture within the assets folder.
	 * \post The texture will be loaded.
	 * \throw LoadingGraphicalRessourceFailure strong exception guarantee: nothing happens.
	 * 
	 * \see `unloadTexture`, `createTexture`, `addTexture`.
	 */
	static bool loadTexture(const std::string& name, bool failingImpliesRemoval = false);
	
	/**
	 * \brief Unloads (without removing it) an existing texture from the graphical ram.
	 * No effect if the texture name was not found or the file name was not provided, since it would be
	 * then impossible to load it again.
	 * \complexity O(1).
	 *
	 * \param[in] name: The alias of a texture.
	 *
	 * \return `true` if unloading was successful/already unloaded, `false` otherwise.
	 *
	 * \note This function is designed not to throw if the texture was not found, to support scenarios
	 *		 where the user tries multiple textures names until one is successfully found and set. This is
	 *		 useful when texture loading may have failed earlier, and fallback attempts are expected behavior.
	 * \warning No texture should not be set to any sprite instances, but can be within any texture vector.
	 * 
	 * \see `loadTexture`, `removeTexture`.
	 */
	static bool unloadTexture(const std::string& name) noexcept;

private:

	/// What `sf::Sprite` the wrapper is being used for.
	std::unique_ptr<sf::Sprite> m_wrappedSprite;

	/// The current index within the texture vector.
	size_t m_curTextureIndex; 
	/// All textures used by the sprite.
	std::vector<TextureInfo> m_textures;
	/// Contains the name of all textures used only by this sprite.
	std::vector<std::string> m_uniqueTextures;

	/// Contains all textures, whether they are used or not/loaded or not.
	static std::list<TextureHolder> s_allTextures;
	/// Maps identifiers to textures for quick access.
	static std::unordered_map<std::string, std::list<TextureHolder>::iterator> s_accessToTextures; 
	/// Textures that can be used just once by a single instance.
	static std::unordered_map<std::list<TextureHolder>::iterator, bool> s_allUniqueTextures;
};


/**
 * \brief Loads a texture from a file.
 * \complexity O(1).
 *
 * \param[out] errorMessage: Will add the error message to this stream if the loading fails
 * \param[in]  fileName: The name of the file.
 * \param[in]  path: The path to this file (assets file by default).
 *
 * \return a sf::Texture if the loading was successful, std::nullopt otherwise.
 * 
 * \note The complete path is "path + fileName" therefore it does not matter if the fileName
 *		 variable contains also a part of the path, as long as the complete path is valid.
 * \note A message is added to the stream only if the function returns std::nullopt.
 * 
 * \see `SpriteWrapper`, `sf::Sprite::loadFromFile`.
 */
std::optional<sf::Texture> loadTextureFromFile(std::ostringstream& errorMessage, std::string const& fileName, std::string const& path = "../assets/") noexcept;

///////////////////////////////////////////////////////////////////////////////////////////////////
/// A `sf::Sprite` wrapper.
///////////////////////////////////////////////////////////////////////////////////////////////////

} // gui namespace

#endif //GRAPHICALRESSOURCES_HPP