/*******************************************************************
 * \file   GUIWrappers.hpp
 * \brief  Declare the entity wrappers for creating and managing graphical user interfaces.
 *
 * \author OmegaDIL.
 * \date   July 2025.
 *
 * \note This file depends on the SFML library.
 *********************************************************************/

#ifndef GRAPHICALRESSOURCES_HPP
#define GRAPHICALRESSOURCES_HPP

#include <SFML/Graphics.hpp>
#include <memory>
#include <vector>
#include <list>
#include <unordered_map>
#include <stdexcept>
#include <cstdint>
#include <optional>


// namespace gui

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


///////////////////////////////////////////////////////////////////////////////////////////////////
/// A `sf::Transformable` Wrapper.
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
 *			`sf::Transformable`.
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
	 * \note Give the address of the correct `sf::Transformable` since it is not changeable after the
	 *       constructor, except in the move/copy assignment operators.
	 * 
	 * \pre   The transformable pointer must not be nullptr.
	 * \post  The wrapper will use the right transformable.
	 * \throw std::invalid_argument basic exception guarantee: the instance is not usable.
	 */
	TransformableWrapper(sf::Transformable* transformable, sf::Vector2f pos, sf::Vector2f scale, sf::Angle rot = sf::degrees(0), Alignment alignment = Alignment::Center);

	TransformableWrapper() noexcept = delete;
	TransformableWrapper(TransformableWrapper const&) noexcept = default;
	TransformableWrapper(TransformableWrapper&&) noexcept = default;
	TransformableWrapper& operator=(TransformableWrapper const&) noexcept = default;
	TransformableWrapper& operator=(TransformableWrapper&&) noexcept = default;
	virtual ~TransformableWrapper() noexcept = default;


	/**
	 * \see Similar to the `sf::Transformable::move` function.
	 */
	inline void move(sf::Vector2f pos) noexcept
	{
		m_transformable->move(pos);
	}

	/**
	 * \see Similar to the `sf::Transformable::scale` function.
	 */
	inline void scale(sf::Vector2f scale) noexcept
	{
		m_transformable->scale(scale);
	}

	/**
	 * \see Similar to the `sf::Transformable::rotate` function.
	 */
	inline void rotate(sf::Angle rot) noexcept
	{
		m_transformable->rotate(rot);
	}

	/**
	 * \see Similar to the `sf::Transformable::setPosition` function.
	 */
	inline void setPosition(sf::Vector2f pos) noexcept
	{
		m_transformable->setPosition(pos);
	}

	/**
	 * \see Similar to the `sf::Transformable::setScale` function.
	 * 
	 * \note The scale parameter should take into account the current size of the window. In a smaller
	 * 		 window, the same `sf::Transformable` will appear larger, and vice-versa.
	 */
	inline void setScale(sf::Vector2f scale) noexcept
	{
		m_transformable->setScale(scale);
	}

	/**
	 * \see Similar to the `sf::Transformable::setRotation` function.
	 */
	inline void setRotation(sf::Angle angle) noexcept
	{
		m_transformable->setRotation(angle);
	}

	/**
	 * \brief Sets the `sf::Transformable`'s origin given alignment.
	 * If the alignment is top/left, then the origin will be located at the top/left corner, which is
	 * (0;0) in sfml. If the alignment is center/right, then the origin should be located at the
	 * coordinate (size.x/2; size).
	 * \complexity O(1).
	 *
	 * \param[in] alignment The new alignment of the `sf::Transformable`.
	 * 
	 * \note It has an implementation even though it is a pure virtual function.
	 *
	 * \see `sf::Transformable::setOrigin()`, `computeNewOrigin()`.
	 */
	inline virtual void setAlignment(Alignment alignment) noexcept = 0
	{
		m_alignment = alignment;
	}

	/**
	 * \brief Apply a new color to the `sf::Transformable` object.
	 * 
	 * \param[in] color: The new color.
	 */
	inline virtual void setColor(sf::Color color) noexcept = 0;

	/**
	 * \brief Resizes and repositions the `sf::Transformable`, e.g., when the window is resized.
	 *
	 * \param[in] windowScaleFactor The per-axis scaling factor (x and y) of the window size.
	 * \param[in] relativeMinAxisScale The ratio between the new and old smallest window axis.
	 *            For example, if the window was resized from (1000, 300) to (900, 600),
	 *            the scale factor is (0.9, 2), and the smallest axis ratio is 600 / 300 = 2.
	 *            This helps to scale elements uniformly based on the smaller dimension.
	 */
	inline void resized(sf::Vector2f windowScaleFactor, float relativeMinAxisScale) noexcept
	{
		scale(sf::Vector2f{ relativeMinAxisScale, relativeMinAxisScale });
		setPosition(sf::Vector2f{ windowScaleFactor.x * m_transformable->getPosition().x, windowScaleFactor.x * m_transformable->getPosition().x });
	}


	/// Tells the interface if the element should be drawn.
	bool hide; 

protected:
	
	/// The current alignment of the `sf::Transformable`.
	Alignment m_alignment; 

private:

	/// What `sf::Transformable` the wrapper is being used for.
	sf::Transformable* m_transformable; 
};

///////////////////////////////////////////////////////////////////////////////////////////////////
/// A `sf::Transformable` Wrapper.
///////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////
/// A `sf::Text` Wrapper.
///////////////////////////////////////////////////////////////////////////////////////////////////

/// The type must be streamable to `std::basic_ostream`.
template <typename T>
concept Ostreamable = requires(std::ostream & os, T t)
{
	{ os << t } -> std::same_as<std::ostream&>;
};

/**
 * \brief A wrapper for `sf::Text` that simplifies its use.
 * The `TextWrapper` adds a constructor which allows initializing the class with more parameters
 * than the one that is available in `sf::Text`. Furthermore, rather than keeping a pointer to a
 * font like `sf::Text` does, it actually stores multiple fonts for the user to use. He can load/unload
 * as much as he wants while keeping O(1) complexity. Inherits from `TransformableWrapper` as well
 * in order to have the alignment, resize functions and hide attribute.
 *
 * \see `sf::Text`, `sf::Font`, `TransformableWrapper`.
 */
class TextWrapper : private sf::Text, public TransformableWrapper
{
public:

	using TransformableWrapper::move;
	using TransformableWrapper::scale;
	using TransformableWrapper::rotate;
	using TransformableWrapper::setPosition;
	using TransformableWrapper::setScale;
	using TransformableWrapper::setRotation;
	using TransformableWrapper::resized;


	/**
	 * \brief Initializes the wrapper.
	 * \complexity O(1).
	 *
	 * \param[in] content: What the text will display.
	 * \param[in] fontName: The name of the font that'll be used.
	 * \param[in] characterSize: The character size of the text.
	 * \param[in] pos: The position of the text.
	 * \param[in] scale: The scale of the text.
	 * \param[in] color: The color of the text.
	 * \param[in] alignment: The alignment of the text.
	 * \param[in] style: The style of the text (regular, italic, underlined...).
	 * \param[in] rot: The rotation of the text.
	 * 
	 * \note The scale parameter should take into account the current size of the window.In a smaller
	 *       window, the same `sf::Text` will appear larger, and vice - versa.
	 * 
	 * \pre   You should have loaded a font with the name you gave as argument, using the function
	 *		 `loadFontIntoWrapper`.
	 * \post  The correct font will be used.
	 * \throw std::invalid_argument basic exception guarantee: the instance is not usable.
	 */
	template<Ostreamable T>
	TextWrapper(T const& content, std::string const& fontName, unsigned int characterSize, sf::Vector2f pos, sf::Vector2f scale, sf::Color color = sf::Color::White, Alignment alignment = Alignment::Center, sf::Text::Style style = sf::Text::Style::Regular, sf::Angle rot = sf::degrees(0))
		: sf::Text{ getFontForConstructor(name), "", characterSize}, TransformableWrapper{ this, pos, scale, rot, alignment }
	{
		setFillColor(color);
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
	 * \see `sf::Text::setString`.
	 */
	template<Ostreamable T>
	inline void setContent(T const& content) noexcept
	{
		std::ostringstream oss{}; // Convert the content to a string
		oss << content; // Assigning the content to the variable.

		setString(oss.str());
		setOrigin(computeNewOrigin(getLocalBounds(), m_alignment));
	}

	/**
	 * \brief Sets a new font for the text.
	 * \complexity O(1).
	 *
	 * \param[in] name: The name of the font that'll be used.
	 * 
	 * \pre   You should have loaded a font with the name you gave as argument, using the function
	 *		 `loadFontIntoWrapper`.
	 * \post  The correct font will be used.
	 * \throw std::invalid_argument strong exception guarantee: the instance remains the same as before
	 *		  the call.
	 * 
	 * \see `sf::Text::setFont`.
	 */
	void setFont(std::string const& name);

	/**
	 * \see `sf::Text::setCharacterSize`.
	 */
	inline void setCharacterSize(unsigned int size) noexcept
	{
		sf::Text::setCharacterSize(size);
		setOrigin(computeNewOrigin(getLocalBounds(), m_alignment));
	}

	/**
	 * \see `sf::Text::setFillColor`.
	 */
	inline virtual void setColor(sf::Color color) noexcept final
	{
		setFillColor(color);
	}

	/**
	 * \see `sf::Text::setStyle`.
	 */
	inline void setStyle(sf::Text::Style style) noexcept
	{
		sf::Text::setStyle(style);
	}

	/**
	 * \see `sf::TransformableWrapper::setAlignment`.
	 */
	inline virtual void setAlignment(Alignment alignment) noexcept final
	{
		TransformableWrapper::setAlignment(alignment);
		setOrigin(computeNewOrigin(getLocalBounds(), m_alignment));
	}

	/**
	 * \brief Accesses the base `sf::Text` object.
	 * \complexity O(1).
	 *
	 * \return A reference to the base `sf::Text` object.
	 */
	[[nodiscard]] inline sf::Text const& getText() const noexcept
	{
		return *this;
	}


	/**
	 * \brief Loads a font into the wrapper with the corresponding name, and can be used by all instances.
	 * \complexity O(1).
	 *
	 * \param[in] name: The alias of the font.
	 * \param[in] font: The font you want to load (using std::move is recommended).
	 * 
	 * \note You can also use this function to replace an existing font by another one.
	 * 
	 * \return `true` if you replaced an existing font and `false` if the font was added without
	 *		   replacement.
	 */
	static bool loadFontIntoWrapper(std::string const& name, sf::Font font) noexcept;

	/**
	 * \brief Unoads a font from the wrapper with the corresponding name.
	 * \complexity O(1).
	 *
	 * \param[in] name: The alias of the font.
	 * 
	 * \note No font should currently use the font you remove.
	 *
	 * \pre   A font must exist with this name.
	 * \post  The correct font will be removed.
	 * \throw std::invalid_argument strong exception guarantee: nothing happen.
	 */
	static void unloadFontFromWrapper(std::string const& name);
	
	/**
	 * \brief Checks if a font exists.
	 * \complexity O(1).
	 *
	 * \param[in] name: The alias of the font.
	 *
	 * \return `true` if the font exists.
	 */
	[[nodiscard]] inline static bool doesFontexist(std::string const& name) noexcept
	{
		return s_accessToFonts.find(name) != s_accessToFonts.end();
	}

private:

	/**
	 * @brief Returns a font in order to construct the `sf::Text`
	 * 
	 * \param[in] name: The alias of the font.
	 * 
	 * \pre   A font must exist with this name.
	 * \post  The correct font will be returned.
	 * \throw std::invalid_argument strong exception guarantee: nothing happen.
	 * 
	 * \return A reference to the font for the constructor.
	 */
	static sf::Font& getFontForConstructor(std::string const& name);


	/// Contains all loaded fonts
	static std::list<sf::Font> s_fonts; 
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
 * \note The complete path is "path + fileName" therefore it does not matter if the fileName
 *		 variable contains also a part of the path, as long as the complete path is valid.
 * \note A message is added to the stream only if the function returns std::nullopt.
 * 
 * \return a sf::Font if the loading was successful, std::nullopt otherwise.
 * 
 * \see `TextWrapper`, `sf::Font::openFromFile`.
 */
std::optional<sf::Font> loadFontFromFile(std::ostringstream& errorMessage, std::string const& fileName, std::string const& path = "../assets/") noexcept;

///////////////////////////////////////////////////////////////////////////////////////////////////
/// A `sf::Text` Wrapper.
///////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////
/// A `sf::Sprite` Wrapper.
///////////////////////////////////////////////////////////////////////////////////////////////////


struct TextureHolder
{
	std::unique_ptr<sf::Texture> texture;
	std::string fileName;
};

struct TextureInfo
{
	TextureHolder* texture;
	sf::IntRect displayedTexturePart;
};


/**
 * \brief A wrapper for `sf::Sprite` that initializes the sprite, keeps the textures, manages resizing.
 * \details A
 *
 * \see sf::Sprite, TransformableWrapper, TextWrapper.
 */
class SpriteWrapper final : private sf::Sprite, public TransformableWrapper
{
public:

	using TransformableWrapper::move;
	using TransformableWrapper::scale;
	using TransformableWrapper::rotate;
	using TransformableWrapper::setPosition;
	using TransformableWrapper::setScale;
	using TransformableWrapper::setRotation;
	using TransformableWrapper::resized;


	SpriteWrapper(sf::Texture texture, sf::Vector2f pos, sf::Vector2f scale, sf::Angle rot = sf::degrees(0), Alignment alignment = Alignment::Center, sf::Color color = sf::Color::White) noexcept;

	SpriteWrapper(std::string const& texture, sf::Vector2f pos, sf::Vector2f scale, sf::Angle rot = sf::degrees(0), Alignment alignment = Alignment::Center, sf::Color color = sf::Color::White);

	SpriteWrapper() noexcept = delete;
	SpriteWrapper(SpriteWrapper const&) noexcept = default;
	SpriteWrapper(SpriteWrapper&&) noexcept = default;
	SpriteWrapper& operator=(SpriteWrapper const&) noexcept = default;
	SpriteWrapper& operator=(SpriteWrapper&&) noexcept = default;
	virtual ~SpriteWrapper() noexcept = default;


	inline virtual void setColor(sf::Color color) noexcept final
	{
		sf::Sprite::setColor(color);
	}

	inline virtual void setAlignment(Alignment alignment) noexcept final
	{
		TransformableWrapper::setAlignment(alignment);
		setOrigin(computeNewOrigin(getLocalBounds(), m_alignment));
	}


	inline void addTexture(sf::Texture texture) noexcept
	{
		m_uniqueTextures.push_back(std::move(texture)); // Add the texture to the vector of unique textures.
		m_textures.push_back(&m_uniqueTextures.back()); // Add the texture to the vector of textures.
	}

	inline void addTexture(std::string const& sharedTexture)
	{
		m_textures.push_back(&(*s_accessingSharedTexture.at(sharedTexture))); // Add the texture to the vector of textures.
	}

	inline void switchToNextTexture() noexcept
	{
		m_curTextureIndex = (m_curTextureIndex + 1) % m_textures.size(); // Switch to the next texture in the vector.
		setTexture(*m_textures[m_curTextureIndex].texture); // Set the texture of the sprite to the next texture.
		setTextureRect(m_textures[m_curTextureIndex].rectangle);
	}

	inline void switchToNextTexture(size_t i)
	{
		if (i >= m_textures.size())
			throw std::out_of_range{ "Index out of range for the sprites vector." };

		m_curTextureIndex = i; // Switch to the texture at index i.
		setTexture(*m_textures[m_curTextureIndex].texture); // Set the texture of the sprite to the texture at index i.
		setTextureRect(m_textures[m_curTextureIndex].rectangle);
	}

	[[nodiscard]] inline size_t getCurrentTextureIndex() const noexcept
	{
		return m_curTextureIndex; // Returns the current texture index.
	}

	[[nodiscard]] inline sf::Texture* getCurrentTexture() /*const*/ noexcept
	{	// TODO: return const pointer instead of pointer.
		return m_textures[m_curTextureIndex].texture; // Returns the current texture of the sprite.
	}

	/**
	 * \brief Accesses the base `sf::Sprite` object.
	 * \complexity O(1).
	 *
	 * \return A reference to the base `sf::Sprite` object.
	 */
	[[nodiscard]] inline sf::Sprite const& getSprite() const noexcept
	{
		return *this;
	}


	static void addSharedTexture(std::string const& identifier, sf::Texture textures) noexcept;

	static void removeSharedTexture(std::string const& identifier) noexcept;

	[[nodiscard]] static inline sf::Texture& getSharedTexture(std::string const& identifier)
	{
		return *s_accessingSharedTexture.at(identifier);
	}

	[[nodiscard]] static inline bool checkIfTextureExists(std::string const& identifier) noexcept
	{
		return s_accessingSharedTexture.find(identifier) != s_accessingSharedTexture.end();
	}

private:

	size_t m_curTextureIndex; // The current texture used
	std::vector<TextureInfo> m_textures; // Textures that are used by this sprite, including unique and shared textures.
	std::list<TextureHolder> m_uniqueTextures; // Textures that are only used by this sprite.

	static std::list<TextureHolder> s_sharedTextures;
	static std::unordered_map<std::string, std::list<TextureHolder>::iterator> s_accessingSharedTexture; // Maps identifiers to textures for quick access.

};

///////////////////////////////////////////////////////////////////////////////////////////////////
/// A `sf::Sprite` Wrapper.
///////////////////////////////////////////////////////////////////////////////////////////////////

#endif //GRAPHICALRESSOURCES_HPP