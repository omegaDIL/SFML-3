/*******************************************************************
 * @file   GUIWrappers.hpp
 * @brief  Declare the entity wrappers for creating and managing graphical user interfaces.
 *
 * @author OmegaDIL.
 * @date   July 2025.
 *
 * @note This file is used for GUI
 * @note This file depends on the SFML library.
 *********************************************************************/

#ifndef GUIWRAPPERS_HPP
#define GUIWRAPPERS_HPP

#include "SFML/Graphics.hpp"
#include <vector>
#include <list>
#include <unordered_map>
#include <cstdint>
#include <stdexcept>


 /**
  * @brief Exception thrown for graphical errors when loading ressources.
  *
  * @see std::runtime_error.
  */
struct LoadingGraphicalRessourceFailure : public std::runtime_error
{
public:

	/**
	 * @brief Initializes the exception with a message.
	 * @complexity O(1)
	 *
	 * @param[in] message: The error message.
	 */
	inline explicit LoadingGraphicalRessourceFailure(const std::string& message) : std::runtime_error{ message }
	{}


	/**
	 * @complexity O(1).
	 *
	 * @return the error message.
	 */
	inline virtual const char* what() const noexcept override
	{
		return std::runtime_error::what();
	}
};


///////////////////////////////////////////////////////////////////////////////////////////////////
/// Pure Virtual Wrapper
///////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * @brief An enum that represents the alignment of a sf::Transformable
 * @details For each axis, horizontal and vertical, there are three different positions: Top/Left,
 *          Center, Bottom/Right. The "y axis" takes the first two bits, and the "x axis" takes the
 *			next two bits. The Center value is 0b0000, which tells that both x and y is at the center since
 *			both of the two pair of bits are set to 0. If you specify a "x axis" alignment, the "y axis"
 *			is set to be the center. You can use the operator | to combine a "x axis" alignment with a
 *			"y axis" alignment. 
 * 
 * @note With 2 bits you have 4 possibilities, however this enum only uses 3 per pair. Therefore one
 *		 per pair has to use one more "possibility" in terms of binary representation. In this case, it
 *		 is left and top.
 *  
 * @see operator|
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
 * @brief Casts both operand to integers and apply the operator |
 *
 * @param[in] lhs: The first alignment that'll be casted
 * @param[in] rhs: The second alignment to that'll be casted
 *
 * @return The new alignment that was created with both alignments.
 *
 * @note If the alignment are not compatible, lhs is returned
 */
Alignment operator|(Alignment lhs, Alignment rhs) noexcept;

/**
 * @param[in] bound: The bound of the sf::Transformable you want to compute the origin
 * @param[in] alignment: The alignment of the 
 * 
 * @return Returns the corresponding origin given an alignment and a bound of a sf::Transformable.
 */
sf::Vector2f computeNewOrigin(sf::FloatRect bound, Alignment alignment) noexcept;


/**
 * @brief Provides basic functions to define for derived classes, with attributes to use.
 * 
 * @note To avoid diamond inheritance, the class is not derived from a sf::Transformable, rather
 * 		 it adds pure virtual functions that would be defined in a sf::Transformable to be implemented
 *		 in derived wrapper classes.
 * 
 * @see sf::Transformable, SpriteWrapper, TextWrapper.
 */
struct TransformableWrapper
{
public:

	/**
	 * @brief Initializes the Wrapper with an alignment and sets the hideness to false
	 * 
	 * @param[in] align: The alignment for the element
	 */
	explicit TransformableWrapper(Alignment alignment = Alignment::Center) noexcept
		: m_alignment{ alignment }, hide{ false }
	{}

	TransformableWrapper(TransformableWrapper const&) noexcept = default;
	TransformableWrapper(TransformableWrapper&&) noexcept = default;
	TransformableWrapper& operator=(TransformableWrapper const&) noexcept = default;
	TransformableWrapper& operator=(TransformableWrapper&&) noexcept = default;
	virtual ~TransformableWrapper() noexcept = default;


	/**
	 * @brief Scales the element.
	 * 
	 * @param[in] scale: The scale to multiply to the current one.
	 * 
	 * @see sf::Transformable::scale()
	 */
	inline virtual void scale(sf::Vector2f scale) noexcept = 0;

	/**
	 * @brief Sets the position of the element.
	 *
	 * @param[in] pos: The new position of the element.
	 * 
	 * @see sf::Transformable::setPosition()
	 */
	inline virtual void setPosition(sf::Vector2f pos) noexcept = 0;

	/**
	 * @brief Sets the rotation of the element.
	 *
	 * @param[in] angle: The new rotation of the element.
	 * 
	 * @see sf::Transformable::setRotation()
	 */
	inline virtual void setRotation(sf::Angle angle) noexcept = 0;

	/**
	 * @brief Sets the alignment of the element.
	 *
	 * @param[in] angle: The new alignment of the element.
	 *
	 * @see sf::Transformable::setOrigin(), computeNewOrigin() 
	 */
	inline virtual void setAlignment(Alignment alignment) noexcept = 0;

	/**
	 * @brief Resizes and repositions the element, e.g., when the window is resized.
	 *
	 * @param[in] windowScaleFactor The per-axis scaling factor (x and y) of the window size.
	 * @param[in] relativeMinAxisScale The ratio between the new and old smallest window axis.
	 *             For example, if the window was resized from (1000, 300) to (900, 600),
	 *             the scale factor is (0.9, 2), and the smallest axis ratio is 600 / 300 = 2.
	 *             This helps to scale elements uniformly based on the smaller dimension.
	 */
	inline virtual void resized(sf::Vector2f windowScaleFactor, float relativeMinAxisScale) noexcept = 0;


	bool hide; // Tells the interface if the element should be drawn.

protected:
	
	Alignment m_alignment; // The current alignment of the element: where is the origin of the sf::Transformable.
};

///////////////////////////////////////////////////////////////////////////////////////////////////
/// Pure Virtual Wrapper
///////////////////////////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////////////////////////
/// Sprite Wrapper
///////////////////////////////////////////////////////////////////////////////////////////////////

struct TextureInfo
{
	/**
	 * @brief Stores a texture pointer with a rectangle
	 * 
	 * @param[in,out] tex: The texture
	 * @param[in] rect: the rectangle for the texture
	 */
	TextureInfo(sf::Texture* tex, sf::IntRect rect = sf::IntRect{})
		: texture{ tex }
	{
		if (rect.size == sf::Vector2i{})
			rectangle.size = static_cast<sf::Vector2i>(texture->getSize());
	}

	sf::Texture* texture;
	sf::IntRect rectangle; 
};

/**
 * @brief A wrapper for `sf::Sprite` that initializes the sprite, keeps the textures, manages resizing.
 * @details A 
 *
 * @see sf::Sprite, TransformableWrapper, TextWrapper.
 */
class SpriteWrapper final : private sf::Sprite, public TransformableWrapper 
{
public:

	SpriteWrapper(std::string const& sharedTexture, sf::Vector2f pos, sf::Vector2f scale, Alignment alignment = Alignment::Center, sf::Angle rot = sf::degrees(0), sf::Color color = sf::Color::White);

	SpriteWrapper(sf::Texture texture, sf::Vector2f pos, sf::Vector2f scale, sf::Angle rot = sf::degrees(0),  Alignment alignment = Alignment::Center, sf::Color color = sf::Color::White) noexcept;

	SpriteWrapper() noexcept = delete;
	SpriteWrapper(SpriteWrapper const&) noexcept = default;
	SpriteWrapper(SpriteWrapper&&) noexcept = default;
	SpriteWrapper& operator=(SpriteWrapper const&) noexcept = default;
	SpriteWrapper& operator=(SpriteWrapper&&) noexcept = default;
	virtual ~SpriteWrapper() noexcept = default;


	/**
	 * @brief Scales the sprite.
	 *
	 * @param[in] scale: Multiplies the current scale to the scale passed as argument.
	 *
	 * @see sf::Transformable::scale()
	 */
	inline virtual void scale(sf::Vector2f scale) noexcept
	{
		sf::Sprite::scale(scale);
	}

	/**
	 * @brief Sets the position of the element.
	 *
	 * @param[in] pos: The new position of the sprite.
	 *
	 * @see sf::Transformable::setPosition()
	 */
	inline virtual void setPosition(sf::Vector2f pos) noexcept
	{
		sf::Sprite::setPosition(pos);
	}

	/**
	 * @brief Sets the rotation of the element.
	 *
	 * @param[in] angle: The new rotation of the sprite.
	 *
	 * @see sf::Transformable::setRotation()
	 */
	inline virtual void setRotation(sf::Angle angle) noexcept
	{
		sf::Sprite::setRotation(angle);
	}

	/**
	 * @brief Sets the color of the sprite that'll be multiplied to the texture's color.
	 *
	 * @param[in] color: The new color for the sprite.
	 *
	 * @see sf::Transformable::setRotation()
	 */
	inline virtual void setColor(sf::Color color) noexcept
	{
		sf::Sprite::setColor(color);
	}

	/**
	 * @brief Sets the alignment of the sprite.
	 *
	 * @param[in] angle: The new alignment of the sprite.
	 *
	 * @see sf::Transformable::setOrigin(), computeNewOrigin()
	 */
	inline virtual void setAlignment(Alignment alignment) noexcept
	{
		sf::Sprite::setOrigin(computeNewOrigin(getLocalBounds(), alignment));
	}

	/**
	 * @brief Resizes and repositions the sprite, e.g., when the window is resized.
	 *
	 * @param[in] windowScaleFactor The per-axis scaling factor (x and y) of the window size.
	 * @param[in] relativeMinAxisScale The ratio between the new and old smallest window axis.
	 *             For example, if the window was resized from (1000, 300) to (900, 600),
	 *             the scale factor is (0.9, 2), and the smallest axis ratio is 600 / 300 = 2.
	 *             This helps to scale elements uniformly based on the smaller dimension.
	 */
	inline virtual void resized(sf::Vector2f windowScaleFactor, float relativeMinAxisScale) noexcept override
	{
		sf::Sprite::scale(sf::Vector2f{ relativeMinAxisScale, relativeMinAxisScale });
		sf::Sprite::setPosition(sf::Vector2f{ windowScaleFactor.x * getPosition().x, windowScaleFactor.x * getPosition().x });
	}

	/**
	 * @brief Accesses the base `sf::Sprite` object.
	 * @complexity O(1).
	 *
	 * @return A reference to the base `sf::Sprite` object.
	 */
	[[nodiscard]] inline sf::Sprite const& getSprite() const noexcept
	{
		return *this;
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

	static std::list<sf::Texture> s_sharedTextures;
	static std::unordered_map<std::string, std::list<sf::Texture>::iterator> s_accessingSharedTexture; // Maps identifiers to textures for quick access.

	std::list<sf::Texture> m_uniqueTextures; // Textures that are only used by this sprite.
	std::vector<TextureInfo> m_textures; // Textures that are used by this sprite, including unique and shared textures.
	size_t m_curTextureIndex; // The current texture used
};

///////////////////////////////////////////////////////////////////////////////////////////////////
/// Sprite Wrapper
///////////////////////////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////////////////////////
/// Text Wrapper
///////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////
/// Text Wrapper
///////////////////////////////////////////////////////////////////////////////////////////////////

#endif //GUIWRAPPERS_HPP