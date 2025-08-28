/*******************************************************************
 * \file   GraphicalResources.hpp, GraphicalResources.cpp
 * \brief  Declare the entity wrappers for creating and managing graphical user interfaces.
 *
 * \author OmegaDIL.
 * \date   July 2025.
 *
 * \note These files depend on the SFML library.
 * \note All assertions are disabled in release mode. If broken, undefined behavior will occur.
 *********************************************************************/

#ifndef GRAPHICALRESOURCES_HPP
#define GRAPHICALRESOURCES_HPP

#include <SFML/Graphics.hpp>
#include <string>
#include <string_view>
#include <list>
#include <unordered_map>
#include <vector>
#include <memory>
#include <stdexcept>
#include <optional>
#include <sstream>
#include <cstdint>
#include <concepts>
#include <type_traits>

#ifndef NDEBUG 

#include <cassert>
#include <algorithm>

#define ENSURE_VALID_PTR(ptr, errorMessage) \
	assert((ptr) && (errorMessage))

#define ENSURE_NOT_OUT_OF_RANGE(index, size, errorMessage) \
	assert(((index) >= 0 && static_cast<size_t>((index)) < static_cast<size_t>((size))) && (errorMessage))

#else
#define ENSURE_VALID_PTR(ptr, errorMessage)
#define ENSURE_NOT_OUT_OF_RANGE(index, size, errorMessage)
#endif

namespace gui
{

 /**
  * \brief An exception for graphical errors when the loading fails.
  *
  * \see `std::runtime_error`.
  */
struct LoadingGraphicalResourceFailure : public std::runtime_error
{
public:

	/**
	 * \brief Initializes the exception with a message.
	 * \complexity O(1).
	 *
	 * \param[in] message The error message.
	 */
	inline explicit LoadingGraphicalResourceFailure(const std::string& message) : std::runtime_error{ message }
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


/**
 * \brief Functor that allows hashing of `std::string` and `std::string_view` without ambiguity.
 */
struct TransparentHash
{
	using is_transparent = void; // marks as transparent

	size_t operator()(std::string_view sv) const noexcept
	{
		return std::hash<std::string_view>{}(sv);
	}
	size_t operator()(const std::string& s) const noexcept
	{
		return std::hash<std::string_view>{}(s);
	}
};

/**
 * \brief Functor that allows equality comparison of `std::string` and `std::string_view` without ambiguity.
 */
struct TransparentEqual
{
	using is_transparent = void;

	bool operator()(std::string_view lhs, std::string_view rhs) const noexcept
	{
		return lhs == rhs;
	}
	bool operator()(const std::string& lhs, const std::string& rhs) const noexcept
	{
		return lhs == rhs;
	}
	bool operator()(std::string_view lhs, const std::string& rhs) const noexcept
	{
		return lhs == rhs;
	}
	bool operator()(const std::string& lhs, std::string_view rhs) const noexcept
	{
		return lhs == rhs;
	}
};


///////////////////////////////////////////////////////////////////////////////////////////////////
/// A `sf::Transformable` wrapper.
///////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * \brief Enum representing the alignment of a `sf::Transformable` object.
 * 
 * The alignment is defined separately for each axis—horizontal (x-axis) and vertical (y-axis)—with
 * three possible positions for each: Top/Left/Center, and Bottom/Right/Center. The alignment encoding
 * uses two bits per axis: the lower two bits for the y-axis, and the next two bits for the x-axis
 * The default value `Center` is encoded as `0b0000`, meaning both axes are centered (both bit pairs are 0).
 * If you specify an alignment for only one axis (e.g., Left or Top), the other axis defaults to Center.
 * You can combine horizontal and vertical alignments using the bitwise OR operator (`|`).
 *
 * \note While 2 bits provide 4 possible values, this enum only uses 3 per axis. As a result, one position
 *       per axis (Top and Left) uses the remaining binary pattern not shared with the others, and 
 *		 therefore are prefered if imcompatible alignment are set.
 *
 * \see `operator|`, `computeNewOrigin`
 */
enum class Alignment : uint8_t
{
	Center = 0,

	Bottom = 1 << 0,
	Top = 1 << 1,

	Right = 1 << 2,
	Left = 1 << 3
};

/**
 * \brief Casts both operand to integers and apply the operator|.
 * \complexity O(1).
 *
 * \param[in] lhs The first alignment that'll be casted.
 * \param[in] rhs The second alignment to that'll be casted.
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
 * \param[in] bound The bound of the `sf::Transformable` for which you want to compute the origin.
 * \param[in] alignment The alignment of the `sf::Transformable`.
 * 
 * \return Returns the corresponding origin given an alignment and a bound of a `sf::Transformable`.
 * 
 * \see `Alignment`, `sf::Transformable::setOrigin`.
 */
sf::Vector2f computeNewOrigin(sf::FloatRect bound, Alignment alignment) noexcept;


/**
 * \brief Basic wrapper around a `sf::Transformable` using composition.
 * 
 * This class wraps a `sf::Transformable` via a raw pointer to avoid issues like diamond inheritance.
 * It redefines the basic transformation functions such as `move`, `scale`, `rotate`, `setPosition`, etc.,
 * with the notable exception of `setOrigin`, which is replaced by a pure virtual `setAlignment` method.
 * This method is intended to serve as a replacement for `setOrigin`, incorporating alignment logic.
 * Additionally, the class maintains a boolean flag indicating whether the transformable should be drawn—
 * useful when combined with `sf::Drawable`.
 *
 * \note This is a pure virtual class. The pure virtual methods are `setAlignment` and `setColor`.
 * \note Getter methods from `sf::Transformable` should be implemented in derived classes.
 * \warning The wrapped `sf::Transformable` must remain valid throughout the lifetime of this object.
 *          No `nullptr` checks are performed for performance reasons. Using an invalid pointer will
 *          trigger an assertion.
 *
 * \see `sf::Transformable`, `Alignment`
 */
class TransformableWrapper
{
public:
	
	TransformableWrapper(const TransformableWrapper&) noexcept = default;
	TransformableWrapper(TransformableWrapper&&) noexcept = default;
	TransformableWrapper& operator=(const TransformableWrapper&) noexcept = default;
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
	 * \complexity O(1).
	 *
	 * If the alignment is top/left, then the origin will be located at the top/left corner, which is
	 * (0;0) in SFML. If the alignment is center/right, then the origin will be located at the
	 * coordinate (size.x; size.y/2).
	 *
	 * \param[in] alignment The new alignment of the `sf::Transformable`.
	 *
	 * \code
	 * // Implementation should be like:
	 * m_alignment = alignment;
	 * transformable.setOrigin(computeNewOrigin(getLocalBounds(), m_alignment));
	 * \endcode
	 * 
	 * \see `sf::Transformable::setOrigin()`, `computeNewOrigin()`.
	 */
	virtual void setAlignment(Alignment alignment) noexcept = 0;

	/**
	 * \brief Apply a new color to the `sf::Transformable` object.
	 * 
	 * \param[in] color The new color.
	 */
	virtual void setColor(sf::Color color) noexcept = 0;


	/// Tells if the element should be drawn.
	bool hide; 

protected:
	
	inline TransformableWrapper() noexcept : hide{ true }, m_alignment{ Alignment::Center }, m_transformable{ nullptr } {}
	
	/**
	 * \brief Initializes the wrapper.
	 * \complexity O(1).
	 *
	 * \param[out] transformable What `sf::Transformable` the wrapper is being used for.
	 * \param[in]  pos The position of the `sf::Transformable`.
	 * \param[in]  scale The scale of the `sf::Transformable`.
	 * \param[in]  rot The rotation of the `sf::Transformable`.
	 * \param[in]  alignment The alignment of the `sf::Transformable`.
	 *
	 * \note The scale parameter should take into account the current size of the window. In a smaller
	 * 		 window, the same `sf::Transformable` will appear larger, and vice-versa.
	 */
	void create(sf::Transformable* transformable, sf::Vector2f pos, sf::Vector2f scale, sf::Angle rot = sf::degrees(0), Alignment alignment = Alignment::Center) noexcept;

	/// What `sf::Transformable` the wrapper is being used for.
	sf::Transformable* m_transformable; 	
	
	/// /// The current alignment of the `sf::Transformable`.
	Alignment m_alignment;
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
 * 
 * Rather than keeping a pointer to a font like `sf::Text` does, it actually stores multiple fonts
 * for the user to choose from. He can add/remove as much as he wants while keeping O(1) complexity.
 * The function `createFont` adds and loads a font that all instances can use, while `removeFont`
 * deletes it completely. Don't remove fonts that are being used by a text.
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
	 * \note The scale parameter should take into account the current size of the window. In a smaller
	 *       window, the same `sf::Text` will appear larger, and vice - versa.
	 * 
	 * \pre   You should have loaded a font with the name you gave, using the function `createFont`.
	 * \post  A font will be used.
	 * \throw std::invalid_argument basic exception guarantee: the instance is not usable.
	 * 
	 * \see `createFont`, `Ostreamable`.
	 */
	template<Ostreamable T>
	inline TextWrapper(const T& content, std::string_view fontName, unsigned int characterSize, sf::Vector2f pos, sf::Vector2f scale, sf::Color color = sf::Color::White, Alignment alignment = Alignment::Center, std::uint32_t style = 0, sf::Angle rot = sf::degrees(0))
		: TransformableWrapper{}, m_wrappedText{ s_defaultFont, "", characterSize }
	{
		create(&m_wrappedText, pos, scale, rot, alignment);

		if (!setFont(fontName))
			throw std::invalid_argument{ "Preconditition violated; the font " + std::string{ fontName } + " was not found when the constructor of TextWrapper was called" };

		setColor(color);
		setStyle(style);
		setContent(content); // Also computes the origin of the text with the correct alignment.
	}

	TextWrapper() noexcept = delete;
	TextWrapper(const TextWrapper&) noexcept;
	TextWrapper(TextWrapper&&) noexcept;
	TextWrapper& operator=(const TextWrapper&) noexcept;
	TextWrapper& operator=(TextWrapper&&) noexcept;
	virtual ~TextWrapper() noexcept = default;


	/**
	 * \brief Updates the text content.
	 * \complexity O(1).
	 *
	 * \param[in] content The new content for the text.
	 *
	 * \see `sf::Text::setString`, `Ostreamable`.
	 */
	template<Ostreamable T>
	inline void setContent(const T& content) noexcept
	{
		std::ostringstream oss{}; // Convert the content to a string.
		oss << content; // Assigning the content to the variable.

		m_wrappedText.setString(oss.str());
		m_wrappedText.setOrigin(computeNewOrigin(m_wrappedText.getLocalBounds(), m_alignment));
	}

	/**
	 * \brief Sets a new font for the text, only if the resource exists.
	 * \complexity O(1).
	 *
	 * \param[in] name The name of the font that'll be used.
	 *
	 * \return `true` if it was set, false `otherwise`.
	 *
	 * \note This function is designed not to throw, to support scenarios where the user tries multiple
	 *		 font until one is successfully found and set. This is useful when font loading may have failed
	 *		 earlier, and fallback attempts are expected behavior.
	 *
	 * \see `sf::Text::setFont`, `createFont`, `loadFontFromFile`.
	 */
	bool setFont(std::string_view name) noexcept;

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
	 */
	void setStyle(std::uint32_t style) noexcept;

	/**
	 * \brief Sets the `sf::Transformable`'s origin given alignment.
	 * \complexity O(1).
	 *
	 * If the alignment is top/left, then the origin will be located at the top/left corner, which is
	 * (0;0) in SFML. If the alignment is center/right, then the origin will be located at the
	 * coordinate (size.x; size.y/2).
	 *
	 * \param[in] alignment The new alignment of the `sf::Transformable`.
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
		return m_wrappedText;
	}


	/**
	 * \brief Loads a font from file and registers it under a given name for shared use across instances.
	 * \complexity Amortized O(1).
	 *
	 * This function loads a font from the specified file and stores it under the provided alias.
	 * Fonts are stored in a shared internal list to prevent reallocation and duplication.
	 * If a font with the same name already exists, the function does nothing—allowing safe repeated calls.
	 *
	 * \param[in] name The alias under which the font will be stored.
	 * \param[in] fileName The path to the font file within the assets folder.
	 * 
	 * \note Do not start a font name with an underscore.
	 *
	 * \pre `fileName` must refer to a valid font file in the assets directory.
	 * \post The font is available for use via the alias `name`.
	 * \throw LoadingGraphicalResourceFailure Strong exception guarantee: no state is modified on failure.
	 *
	 * \see `loadFontFromFile`, `setFont`
	 */
	static void createFont(std::string name, std::string_view fileName);

	/**
	 * \brief Loads a font and registers it under a given name for shared use across instances.
	 * \complexity Amortized O(1).
	 *
	 * This function loads a font from the specified file and stores it under the provided alias.
	 * Fonts are stored in a shared internal list to prevent reallocation and duplication.
	 * If a font with the same name already exists, the function does nothing—allowing safe repeated calls.
	 *
	 * \param[in] name The alias under which the font will be stored.
	 * \param[in] fileName The path to the font file within the assets folder.
	 *
	 * \note Do not start a font name with an underscore.
	 *
	 * \see `loadFontFromFile`, `setFont`
	 */
	static void createFont(std::string name, sf::Font font) noexcept;
	
	/**
	 * \brief Removes the font from the wrapper with the given name.
	 * \complexity Amortized O(1).
	 *
	 * If no font with exist under that name, the function does nothing—allowing safe repeated calls.
	 *
	 * \param[in] name The alias under which the font was stored.
	 *
	 * \pre No text should currently use the font you remove.
	 * \post The removal of the font is safe.
	 * \warning The program may crash at any point otherwise (likely when the text is drawn).
	 * 
	 * \see `createFont`, `getFont`.
	 */
	static void removeFont(std::string_view name) noexcept;

	/**
	 * \brief Returns a font ptr, or nullptr if it does not exist.
	 * \complexity O(1).
	 *
	 * \param[in] name The alias under which the font was stored.
	 *
	 * \return The address of the font.
	 * 
	 * \see `createFont`.
	 */
	[[nodiscard]] static sf::Font* getFont(std::string_view name) noexcept;

private:

	/// What `sf::Text` the wrapper is being used for.
	sf::Text m_wrappedText;

	/// Contains all loaded fonts
	inline static std::list<sf::Font> s_allFonts{};
	/// Allows to find fonts with a name in O(1) time complexity.
	inline static std::unordered_map<std::string, std::list<sf::Font>::iterator, TransparentHash, TransparentEqual> s_accessToFonts{};
	
	/// A default font that is used to initialize the `sf::Text` before setting its actual font.
	inline static const sf::Font s_defaultFont{}; 
};


/**
 * \brief Loads a font from a file.
 * \complexity O(1).
 * 
 * \param[out] errorMessage Will add the error message to this stream if the loading fails.
 * \param[in]  fileName The name of the file.
 * \param[in]  path The path to this file (assets file by default).
 * 
 * \return a sf::Font if the loading was successful, std::nullopt otherwise.
 * 
 * \note The complete path is "path + fileName" therefore it does not matter if the fileName
 *		 variable contains also a part of the path, as long as the complete path is valid.
 * \note A message is added to the stream only if the function returns std::nullopt.
 * 
 * \see `TextWrapper`, `sf::Font::openFromFile`.
 */
[[nodiscard]] std::optional<sf::Font> loadFontFromFile(std::ostringstream& errorMessage, std::string_view fileName, std::string_view path = "../assets/") noexcept;

///////////////////////////////////////////////////////////////////////////////////////////////////
/// A `sf::Text` wrapper.
///////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////
/// A `sf::Sprite` wrapper.
///////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * \brief A wrapper around `sf::Sprite` that simplifies texture management.
 *
 * This class facilitates the use of `sf::Texture` by managing all texture resources internally.
 * Two types of textures are supported:
 * - **Shared textures**: Can be used by multiple sprite instances.
 * - **Reserved textures**: Owned exclusively by a single sprite instance.
 *
 * To use a texture, you must first call `createTexture` to load it into the global store. Then, use
 * `addTexture` on each sprite instance that will use that texture. A reserved texture is claimed by
 * the first instance that adds it using `addTexture`. When reserved, `addTexture` must only be called 
 * if the texture was reserved by its respective sprite instance.
 *
 * Sprites can have multiple textures. Each instance maintains a "texture vector" that stores all its
 * associated textures, optionally using different `sf::IntRect` subregions of the same image. The order
 * in which textures are added is preserved.
 *
 * Resource Management:
 * - Use `createTexture` / `removeTexture` to control the global texture store.
 * - Use `loadTexture` / `unloadTexture` to manage memory without removing references.
 * - `removeTexture` completely deletes a shared texture once no sprite references it.
 * - `unloadTexture` releases GPU memory but keeps texture pointers in texture vectors valid.
 * - Reserved textures cannot be manually removed with `removeTexture`; they are automatically
 *   cleaned up when their owning sprite instance is destroyed. However, they can still be unloaded.
 * 
 * A code example is provided at the end of the file.
 *
 * \note Reserved textures may consume slightly more memory due to exclusive ownership.
 *
 * \see `sf::Sprite`, `sf::Texture`, `TransformableWrapper`
 */
class SpriteWrapper final : public TransformableWrapper
{
public:

	/**
	 * \brief Initializes the wrapper.
	 * \complexity O(1).
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
	 * \note The scale parameter should take into account the current size of the window. In a smaller
	 *       window, the same `sf::Text` will appear larger, and vice - versa.
	 * \note The constructor accounts for the reserve state of the texture.
	 * 
	 * \pre `fileName` must refer to a valid texture file in the assets directory.
	 * \post The texture is available for use via the alias `name`.
	 * \throw invalid_argument Basic exception guarantee: the instance is not usable.
	 *
	 * \see `createTexture`.
	 */
	SpriteWrapper(std::string_view textureName, sf::Vector2f pos, sf::Vector2f scale, sf::IntRect rect = sf::IntRect{}, sf::Angle rot = sf::degrees(0), Alignment alignment = Alignment::Center, sf::Color color = sf::Color::White);

	SpriteWrapper() noexcept = delete;
	SpriteWrapper(const SpriteWrapper&) noexcept = delete; // For reserved texture.
	SpriteWrapper(SpriteWrapper&&) noexcept;
	SpriteWrapper& operator=(const SpriteWrapper&) noexcept = delete; // For reserved texture.
	SpriteWrapper& operator=(SpriteWrapper&&) noexcept;
	virtual ~SpriteWrapper() noexcept; /// \complexity O(N) where N is the number of reserved texture to deallocate.


	/**
	 * \see `sf::Sprite::setColor`.
	 */
	virtual void setColor(sf::Color color) noexcept final;

	/**
	 * \brief Sets the `sf::Transformable`'s origin given alignment.
	 * \complexity O(1).
	 *
	 * If the alignment is top/left, then the origin will be located at the top/left corner, which is
	 * (0;0) in SFML. If the alignment is center/right, then the origin will be located at the
	 * coordinate (size.x; size.y/2).
	 *
	 * \param[in] alignment The new alignment of the `sf::Transformable`.
	 *
	 * \note Keeping the origin of a small sprite at Top/Left is recommended. The way SFML handles drawing
	 *		 sprites make their textures, sometimes, larger at certain lines/pixels and smaller at other
	 *		 places. For some reasons, this problem does not occur when the origin is at 0;0. So unless, you
	 *		 want to rotate the sprite by a different alignment, or the texture is big enough to not see the
	 *		 difference, set the alignment to Top/Left. If you still want to give positions as if the center
	 *		 was the origin, you can use the `spriteWrapped.setPosition` function and then use that line: 
	 *		 `spriteWrapped.move(-computeNewOrigin(spriteWrapped.getSprite().getLocalBounds(), Alignment::Center))`
	 *	
	 * \see `sf::Transformable::setOrigin()`, `computeNewOrigin()`.
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
		return m_wrappedSprite;
	}

	/**
	 * \brief Switches the currently displayed texture of the sprite to another one in the texture vector.
	 * \complexity O(1).
	 * 
	 * This function updates the sprite’s currently active texture and sub-rectangle by applying the given
	 * `offset` to the current texture index. The texture vector is treated as circular: the resulting index
	 * wraps around using modulo if it goes out of bounds.
	 * 
	 * For example:
	 * - `switchToNextTexture(1)` moves to the next texture in the list.
	 * - `switchToNextTexture(-1)` moves to the previous one.
	 * - Any positive or negative offset can be used depending on how textures were added (e.g. to switch
	 *   animation frames, resolutions, visual states, etc.).
	 *
	 * \param[in] offset The relative offset from the current texture index. Can be positive or negative.
	 *
	 * \note The function will load the next texture if it was not previously loaded. That being said,
	 *		 if you want to avoid a sudden loss of fps (especially for large texture), consider load it
	 *		 before
	 * \note The previous texture is not unloaded.
	 * 
	 * \pre If loading, the file name should be a correct path to a texture within the assets folder.
	 * \post The texture will be loaded.
	 * \throw LoadingGraphicalResourceFailure strong exception guarantee: nothing happens.
	 *
	 * \see `switchToTexture`, `getCurrentTextureIndex`, `addTexture`, `loadTexture`.
	 */
	void switchToNextTexture(long long indexOffset = 1);

	/**
	 * \see Same as switchToNextTexture but you specify the index.
	 * 
	 * \pre The index must not be out of range
	 * \warning If the index is out if range, the program will assert.
	 * 
	 * \see `switchToNextTexture`.
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
	 * \brief Adds a texture (with one or more sub-rectangles) to this instance's texture vector.
	 * 
	 * \complexity In release mode, Amortized O(1).
	 * \complexity In debug mode, Amortized O(1) for shared and non claimed reserved textures.
	 * \complexity In debug mode, O(N) for claimed reserved textures where N is the number of reserved
	 *			   textures claimed by this instance.
	 * 
	 * 
	 * For each `sf::IntRect` provided, a pair of (`sf::Texture*`, `sf::IntRect`) is added to the internal
	 * texture vector. This allows switching between different sub-regions (e.g., animation frames) or entirely
	 * different textures using `switchToNextTexture`.
	 * 
	 * Textures can be either shared or reserved:
	 * - **Shared textures** may be reused across multiple instances.
	 * - **Reserved textures** are exclusive to a single instance and may only be added here if the current
	 *   instance is the owner.
	 * 
	 * The first added pair (texture + rect) will be at index 0, the next at index 1, and so on.
	 * You must manually track texture indices if needed.
	 *
	 * \param name The alias of the texture (must have been registered via `createTexture`).
	 * \param rects One or more `sf::IntRect`s to define which sub-regions of the texture to add. If any
	 *				rects are equal to `sf::IntRect{}` (default constructor), they are set to the whole texture
	 *				size.
	 *
	 * \return `true` if the texture(s) were successfully added to the texture vector, `false` if the texture
	 *         could not be found or used.
	 *
	 * \note Once added, the order and content of the texture vector cannot be modified.
	 * \note This function is designed not to throw if the texture was not found, to support scenarios
	 *		 where the user tries multiple textures names until one is successfully found and set. This is
	 *		 useful when texture loading may have failed earlier, and fallback attempts are expected behavior.
	 *
	 * \pre For reserved textures, they must not be claimed by another instance.
	 * \post The texture(s) will be appended to this instance’s texture vector.
	 * \warning The program asserts if the texture is reserved, claimed, but not by this instance.
	 *
	 * \see `createTexture`, `loadTexture`, `unloadTexture`, `switchToNextTexture`
	 */
	template<typename... Ts> requires (std::same_as<Ts, sf::IntRect> && ...)
	inline bool addTexture(std::string_view name, Ts... rects)
	{
		auto mapAccessIterator{ s_accessToTextures.find(name) };
		if (mapAccessIterator == s_accessToTextures.end()) [[unlikely]]
			return false; // Texture not there.

#ifndef NDEBUG
		auto mapUniqueIterator{ s_allUniqueTextures.find(&*mapAccessIterator->second) };
		if (mapUniqueIterator != s_allUniqueTextures.end() // is reserved.
		&&  mapUniqueIterator->second == true // has already been claimed by an instance...
		&&  std::find(m_uniqueTextures.begin(), m_uniqueTextures.end(), name) == m_uniqueTextures.end()) [[unlikely]] // ...but not by this one.
			assert(!"Precondition violated; The reserved texture was not available anymore for this sprite instance when addTexture was called in SpriteWrapper");
#endif // NDEBUG

		TextureHolder* texture{ &*mapAccessIterator->second };
		(m_textures.push_back(TextureInfo{ texture, rects }), ...);

		if (mapUniqueIterator != s_allUniqueTextures.end() && mapUniqueIterator->second == false) // For reserved texture.
		{
			mapUniqueIterator->second = true;  // Mark it as true, meaning the reserve state was claimed.
			m_uniqueTextures.push_back(std::string{ name });
		}

		return true;
	}

	/**
	 * \see Similar to the `addTexture` template overloaded function, with an IntRect that covers the
	 *		whole texture since it has a size 0.
	 */
	inline bool addTexture(std::string_view name)
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
	 * \brief Creates a texture from a file and registers it under a given name.
	 * \complexity Amortized O(1).
	 * 
	 * This function loads a texture from the specified file and stores it under the provided alias.
	 * Textures are stored in a shared internal list to prevent reallocation and duplication.
	 * If a textrue with the same name already exists, the function does nothing—allowing safe repeated 
	 * calls.
	 *
	 * \param[in] name The alias under which the texture will be stored.
	 * \param[in] fileName The path to the texture file within the assets folder.
	 * \param[in] shared: `No` if the texture is reserved.
	 * \param[in] loadImmediately: `true` if you want the texture to be loaded from the file when the
	 *							   function is called. if so, may throw an exception if failed.
	 *
	 * \note If loadImmediately is set to `true`, It is recommended to call this from a separate thread
	 *		 for large textures or to avoid frame drops.
	 * \note Do not start a texture name with an underscore.
	 *
	 * \pre `fileName` must refer to a valid texture file in the assets directory.
	 * \post The texture is available for use via the alias `name`.
	 * \throw LoadingGraphicalResourceFailure Strong exception guarantee: no state is modified on failure.
	 *
	 * \see `loadTextureFromFile`, `addTexture`.
	 */
	static void createTexture(std::string name, std::string fileName, Reserved shared = Reserved::Yes, bool loadImmediately = false);

	/**
	 * \brief Creates a texture from a file and registers it under a given name.
	 * \complexity Amortized O(1).
	 *
	 * This function loads a texture from the specified file and stores it under the provided alias.
	 * Textures are stored in a shared internal list to prevent reallocation and duplication.
	 * If a textrue with the same name already exists, the function does nothing—allowing safe repeated
	 * calls.
	 *
	 * \param[in] name The alias under which the texture will be stored.
	 * \param[in] fileName The path to the texture file within the assets folder.
	 * \param[in] shared: `No` if the texture is reserved.
	 *
	 * \note Since no file is given, unloading the `sf::Texture` is not possible. Nothing happens it
	 *		 is tried.
	 * \note Do not start a texture name with an underscore.
	 *
	 * \see `loadTextureFromFile`, `addTexture`.
	 */
	static void createTexture(std::string name, sf::Texture texture, Reserved shared = Reserved::Yes) noexcept;
	
	/**
	 * \brief Removes a shared texture from the wrapper with the given name. 
	 * \complexity Amortized O(1).
	 *
	 * If no texture with exist under that name, the function does nothing—allowing safe repeated calls.
	 *
	 * \param[in] name The alias under which the texture was stored.
	 *
	 * \note It is recommended to call this from a separate thread for large textures or to avoid frame drops.
	 *
	 * \pre No sprite should currently use the texture you remove, or use it after with its 
	 *		"texture vector".
	 * \post The removal of the texture is safe.
	 * \warning The program may crash at any point otherwise (likely when the sprite is drawn).
	 * 
	 * \pre The texture can't be reserved.
	 * \post The removal of the texture is possible.
	 * \warning Asserts if the texture is reserved.
	 */
	static void removeTexture(std::string_view name) noexcept;

	/**
	 * \brief Returns a texture ptr, or nullptr if it does not exist.
	 * \complexity O(1).
	 *
	 * \param[in] name The alias under which the texture was stored.
	 *
	 * \return The address of the texture.
	 */
	[[nodiscard]] static sf::Texture* getTexture(std::string_view name) noexcept;

	/**
	 * \brief Loads a previously registered texture into the graphical memory (e.g., VRAM).
	 * \complexity O(1)
	 *
	 * This function attempts to load the texture associated with the given alias into GPU memory.
	 * If the texture is already loaded, the function does nothing and returns `true`.
	 *
	 * \param[in] name The alias of a texture (must have been created using `createTexture`).
	 * \param[in] failingImpliesRemoval If set to `true`, a loading failure will automatically trigger
	 *             the complete removal of the texture from the wrapper (equivalent to `removeTexture`).
	 *             This only applies to *actual loading failures*, not when:
	 *             - The alias does not exist.
	 *             - The texture is already loaded.
	 *
	 * \return `true` if the texture was successfully loaded or was already loaded.
	 *         `false` if the texture could not be loaded.
	 *
	 * \note `failingImpliesRemoval` is ignored for reserved texture.
	 * \note It is recommended to call this from a separate thread for large textures or to avoid frame drops.
	 * \note This function is designed not to throw if the texture was not found, to support scenarios
	 *		 where the user tries multiple textures names until one is successfully found and set. This is
	 *		 useful when texture loading may have failed earlier, and fallback attempts are expected behavior.
	 * 
	 * \pre The texture alias must exist and refer to a valid file path within the assets folder.
	 * \post The texture will be loaded into memory if not already loaded.
	 * \throw LoadingGraphicalResourceFailure (strong exception guarantee) only if the file path is valid but the texture fails to load.
	 *
	 * \see `unloadTexture`, `createTexture`, `addTexture`, `removeTexture`
	 */
	static bool loadTexture(std::string_view name, bool failingImpliesRemoval = false);
	
	/**
	 * \brief Unloads (without removing) an existing texture from GPU memory (VRAM).
	 * \complexity O(1)
	 *
	 * This function frees the graphical memory used by the texture associated with the given alias,
	 * without deleting the texture's metadata or references. It allows the texture to be reloaded
	 * later using `loadTexture`, as long as a valid file path was originally set.
	 *
	 * \param[in] name The alias of the texture to unload.
	 *
	 * \return `true` if the texture was successfully unloaded or was already unloaded.
	 *         `false` if the texture could not be found or if its file path was not set.
	 *
	 * \note It is recommended to call this from a separate thread for large textures or to avoid frame drops.
	 * \note This function is designed not to throw if the texture was not found, to support scenarios
	 *		 where the user tries multiple textures names until one is successfully found and set. This is
	 *		 useful when texture loading may have failed earlier, and fallback attempts are expected behavior.
	 * \warning No sprite instance should currently use the texture when it is unloaded.
	 *          However, it is safe for the texture to remain in any instance's texture vector,
	 *          as pointers will remain valid and can be reloaded transparently.
	 *
	 * \see `loadTexture`, `removeTexture`
	 */
	static bool unloadTexture(std::string_view name) noexcept;

private:

	/**
	 * \brief Used within the sprite wrapper list to represent a texture.
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
	 * \brief Represents a resource for a sprite to use, with a texture and a rectangle.
	 * Does not own the texture.
	 *
	 * \see `SpriteWrapper`, `TextureHolder`.
	 */
	struct TextureInfo
	{
		TextureHolder* texture;
		sf::IntRect displayedTexturePart;
	};


	/// What `sf::Sprite` the wrapper is being used for.
	sf::Sprite m_wrappedSprite;

	/// The current index within the texture vector.
	size_t m_curTextureIndex; 
	/// All textures used by the sprite.
	std::vector<TextureInfo> m_textures;

	/// Contains the name of all reserved textures used by this sprite.
	std::vector<std::string> m_uniqueTextures;

	/// Contains all textures, whether they are used or not/loaded or not.
	inline static std::list<TextureHolder> s_allTextures{};
	/// Maps identifiers to textures for quick access.
	inline static std::unordered_map < std::string, std::list<TextureHolder>::iterator, TransparentHash, TransparentEqual> s_accessToTextures{};
	/// Textures that can be used just once by a single instance.
	inline static std::unordered_map<TextureHolder*, bool> s_allUniqueTextures{};

	/// A default texture that is used to initialize the `sf::Sprite` before setting its actual texture.
	inline static const sf::Texture s_defaultTexture{}; 
};

/**
 * Here are two use of SpriteWrapper. One is simple; the other is slightly more complicated, yet very complete.
 * 
 * \code Simple sprites: buttons.
 * const std::string button{ "buttonTex" };
 * gui::SpriteWrapper::createTexture(button, "button.jpg", gui::SpriteWrapper::Reserved::No, true); // Loads it immediately
 * 
 * sf::Vector2f scale{ std::min(windowSize.x, windowSize.y) / 1080, 0 };
 * scale.y = scale.x; // Scale the buttons for different screen definition.
 * gui::SpriteWrapper launchGame  { button, { 200, 200 }, scale, sf::degrees(0), gui::Alignment::Top | gui::Alignment::Left };
 * gui::SpriteWrapper settingsGame{ button, { 400, 200 }, scale, sf::degrees(0), gui::Alignment::Top | gui::Alignment::Left };
 * gui::SpriteWrapper closeGame   { button, { 600, 200 }, scale, sf::degrees(0), gui::Alignment::Top | gui::Alignment::Left };
 * \endcode
 * 
 * \code main character sprite
 * sf::Vector2f scale{ std::min(windowSize.x, windowSize.y) / 1080, 0 };
 * scale.y = scale.x; // Scale the buttons for different screen definition.
 * 
 * // No sense for these textures to be applied to other sprites  so they are reserved
 * gui::SpriteWrapper::createTexture("hero run1080", "1080/running_sprite.png", gui::SpriteWrapper::Reserved::Yes, true); // true means they are loaded
 * gui::SpriteWrapper::createTexture("hero run2160", "2160/running_sprite.png", gui::SpriteWrapper::Reserved::Yes); // false by default.
 * gui::SpriteWrapper::createTexture("hero attack1080", "1080/attacking_sprite.png", gui::SpriteWrapper::Reserved::Yes, true);
 * gui::SpriteWrapper::createTexture("hero attack2160", "2160/attacking_sprite.png"); // Reserved and false by default.
 * 
 * // Applies the texture to the sprite while claiming the reserve state.
 * gui::SpriteWrapper player{ "hero run1080", { 960, 540 }, scale, sf::IntRect{ {}, {50, 50} } };
 * player.addTexture("hero run1080", sf::IntRect{ {50, 0}, {50, 50} }); // First intRect was added in the constructor.
 * player.addTexture("hero run2160", sf::IntRect{ {}, {100, 100} }, sf::IntRect{ {100, 0}, {100, 100} }); // 4k texture are larger
 * player.addTexture("hero attack1080", sf::IntRect{ {0, 0}, {50, 50} }, sf::IntRect{ {50, 0}, {50, 50} });
 * player.addTexture("hero attack2160", sf::IntRect{ {}, {100, 100} }, sf::IntRect{ {100, 0}, {100, 100} });
 * 
 * // Let's assume we're in the game loop
 * if (player.getSprite().getScale().x == 1.8) // Accounts for screen definition and other scaling reasons (e.g. gameplay, zooming).
 * {	// Let's say 1.8 times is too pixeled so we load the 4k textures. 
 *		gui::SpriteWrapper::loadTexture("hero run2160"); // using another thread is recommended.
 *		gui::SpriteWrapper::loadTexture("hero attack2160");
 * 
 *		player.setScale(sf::Vector2f{ 0.5f, 0.5f }); // 4K is 2 times larger on each axis
 *		player.switchToNextTexture(2); // 4k textures were added 2 index after 1080p, regardless of which textures is currently set.
 * 
 *		gui::SpriteWrapper::unloadTexture("hero run1080"); // using another thread is recommended.
 *		gui::SpriteWrapper::unloadTexture("hero attack1080");
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
[[nodiscard]] std::optional<sf::Texture> loadTextureFromFile(std::ostringstream& errorMessage, std::string_view fileName, std::string_view path = "../assets/") noexcept;

///////////////////////////////////////////////////////////////////////////////////////////////////
/// A `sf::Sprite` wrapper.
///////////////////////////////////////////////////////////////////////////////////////////////////

} // gui namespace

#endif //GRAPHICALRESOURCES_HPP