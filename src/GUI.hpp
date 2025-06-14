/*******************************************************************
 * @file   GUI.hpp
 * @brief  Declare the entities for creating and managing graphical user interfaces.
 *
 * @author OmegaDIL.
 * @date   July 2024.
 *
 * @note This file depends on the SFML library.
 *********************************************************************/

#ifndef GUI_HPP
#define GUI_HPP

#include <SFML/Graphics.hpp>
#include <string>
#include <unordered_map>
#include <vector>
#include <span>
#include <valarray>
#include <utility>
#include <optional>
#include <stdexcept>
#include <list>
#include <array>
#include <algorithm>
#include <initializer_list>
#include <unordered_set>
#include <memory>
#include <functional>
#include <sstream>
#include "Exceptions.hpp"
#include "GUITexturesLoader.hpp"


/**
 * @brief Must be a valid data types for the function << of std::basic_ostream.
 *
 * @param os: To check if it is valid with basic_ostream.
 * @tparam T: The type to check.
 */
template <typename T>
concept Ostreamable = requires(std::ostream & os, T t)
{
	{ os << t } -> std::same_as<std::ostream&>;
};

/**
 * @brief Manages items to create a basic GUI. You can display texts, sprites, shapes and a background.
 * @details All elements are fixed and can't be edited nor removed.
 *
 * @note This class stores UI componenents; it will use a considerable amount of memory. 
 * @warning Avoid deleting the `sf::RenderWindow` passed as an argument while this class is using it.
 * 
 * @code
 * sf::RenderWindow window{ ... };
 * FixedGraphicalInterface gui{ &window };
 * 
 * ...
 * 
 * gui.addText("Hello World", sf::Vector2f{ 100, 100 }, 12, windowSizeMin / 1080.f);
 * gui.addText("Hello", sf::Vector2f{ 300, 100 }, 12, windowSizeMin / 1080.f, sf::Color{ 255, 0, 0 });
 * 
 * sf::CircleShape shape{ ... };
 * ...
 * gui.addShape(&shape, false);
 * 
 * ...
 * 
 * window.clear();
 * gui.draw();
 * ...
 * window.display();
 * @endcode
 *
 * @see sf::RenderWindow, FixedGraphicalInterface::TextWrapper, DynamicGraphicalInterface.
 */
class FixedGraphicalInterface
{
protected: 
	//TODO: rename update ... by set ...

	/**
	 * @brief A wrapper for `sf::Text` that initializes the text, keeps the font, manages resizing.
	 *
	 * @note The font is common to all instances of this class.
	 * @warning Call TextWrapper::loadFont() to initialize properly the font, before creating any other
	 *          functions.
	 *
	 * @see FixedGraphicalInterface, sf::Text, create().
	 */
	class TextWrapper : private sf::Text
	{
	public:
		
		TextWrapper() noexcept = delete; 
		TextWrapper(TextWrapper const&) noexcept = default;
		TextWrapper(TextWrapper&&) noexcept = default;
		TextWrapper& operator=(TextWrapper const&) noexcept = default;
		TextWrapper& operator=(TextWrapper&&) noexcept = default;
		virtual ~TextWrapper() noexcept = default;

		/**
		 * @brief Initializes the text with the specified parameters.
		 * @complexity O(1).
		 *
		 * @tparam T: Type that can be streamed to `std::basic_ostream`.
		 * @param[in] content: The text label.
		 * @param[in] position: The position of the text.
		 * @param[in] characterSize: The size of the text characters.
		 * @param[in] scale: The scale of the text.
		 * @param[in] color: The color of the text.
		 * @param[in] rot: The rotation of the text.
		 */
		template<Ostreamable T>
		inline TextWrapper(T const& content, sf::Vector2f pos, unsigned int characterSize, float scale, sf::Color color, sf::Angle rot) noexcept
			: sf::Text{ m_font, "", characterSize }
		{
			setFillColor(color);
			setRotation(rot);
			setScale(sf::Vector2f{ scale, scale });
			updateContent(content);
			setPosition(pos);
		}


		/**
		 * @brief Updates the text content.
		 * @complexity O(1).
		 *
		 * @tparam T: Type that can be streamed to `std::basic_ostream`.
		 * @param[in] content: The new content for the text.
		 *
		 * @see updatePosition(), updateColor().
		 */
		template<Ostreamable T>
		inline void updateContent(T const& content) noexcept
		{
			std::ostringstream oss{}; // Convert the content to a string
			oss << content; // Assigning the content to the variable.

			setString(oss.str());
			setOrigin(getLocalBounds().getCenter()); // Recenters the text.
		}

		/**
		 * @brief Updates the position of the text.
		 * @complexity O(1).
		 *
		 * @param[in] pos: The new position for the text.
		 *
		 * @see sf::Text::setPosition().
		 */
		inline void updatePosition(sf::Vector2f pos) noexcept
		{
			setPosition(pos);
		}

		/**
		 * @brief Updates the rotation of the text.
		 * @complexity O(1).
		 *
		 * @param[in] rot: The new rotation for the text.
		 *
		 * @see sf::Text::setRotation.
		 */
		inline void updateRotation(sf::Angle rot) noexcept
		{
			setRotation(rot);
		}

		/**
		 * @brief Updates the scale of the text.
		 * @complexity O(1).
		 *
		 * @param[in] scale: Adds the scale to the current one.
		 *
		 * @note Doesn't override the previous scale : uses scale() instead of setScale().
		 *
		 * @see sf::Text::scale.
		 */
		inline void updateScale(sf::Vector2f scale) noexcept
		{
			this->scale(scale);
		}

		/**
		 * @brief Updates the size of the text.
		 * @complexity O(1).
		 *
		 * @param[in] size: The size of the characters.
		 *
		 * @see sf::Text::scale().
		 */
		inline void updateCharacterSize(unsigned int size) noexcept
		{
			setCharacterSize(size);
			setOrigin(getLocalBounds().getCenter()); // Recenters the text.
		}

		/**
		 * @brief Updates the color of the text.
		 * @complexity O(1).
		 *
		 * @param[in] color: The new color for the text.
		 *
		 * @see sf::Text::setFillColor().
		 */
		inline void updateColor(sf::Color color) noexcept
		{
			setFillColor(color);
		}

		/**
		 * @brief Resizes the text when the window is resized.
		 * @complexity O(1).
		 *
		 * @param[in] scalingFactor: The scaling factor between the previous window's size and the new one.
		 * @param[in] scalingFactorMin: The scaling factor between the previous smallest window's size in x or y axis, and the new smallest one.
		 */
		inline void windowResized(sf::Vector2f scalingFactor, sf::Vector2f scalingFactorMin) noexcept
		{
			scale(scalingFactorMin);
			setPosition(sf::Vector2f{ getPosition().x * scalingFactor.x, getPosition().y * scalingFactor.y });
		}

		/**
		 * @brief Accesses the base `sf::Text` object.
		 * @complexity O(1).
		 *
		 * @return A reference to the base `sf::Text` object.
		 */
		[[nodiscard]] inline sf::Text const& getText() const noexcept
		{
			return *this;
		}


		/**
		 * @brief Creates the font if it is not already created.
		 * @complexity O(1).
		 *
		 * @return a string if failed.
		 */
		[[nodiscard]] static std::optional<std::string> loadFont() noexcept;

	private:

		static sf::Font m_font; // The font used for the texts.
	};

	class SpriteWrapper : private sf::Sprite
	{
	public:

		SpriteWrapper() noexcept = delete;
		SpriteWrapper(SpriteWrapper const&) noexcept = default;
		SpriteWrapper(SpriteWrapper&&) noexcept = default;
		SpriteWrapper& operator=(SpriteWrapper const&) noexcept = default;
		SpriteWrapper& operator=(SpriteWrapper&&) noexcept = default;
		virtual ~SpriteWrapper() noexcept = default;

		SpriteWrapper(sf::Texture* texture, sf::Vector2f pos, sf::Vector2f scale, sf::Angle rot, sf::Color color, const sf::IntRect& rectangle) noexcept;


		[[nodiscard]] inline bool checkIfSpriteHoldsTexture(sf::Texture* texture) const noexcept
		{
			return std::find(m_textures.begin(), m_textures.end(), texture) != m_textures.end();
		}

		[[nodiscard]] inline std::vector<sf::Texture*> const& getTextures() const noexcept
		{
			return m_textures;
		}

		void addTexture(sf::Texture* texture) noexcept;

		bool removeTexture(sf::Texture* texture) noexcept;

		void switchToNextTexture() noexcept;

		void switchToNextTexture(size_t i);



		/**
		 * @brief Updates the position of the sprite.
		 * @complexity O(1).
		 *
		 * @param[in] pos: The new position for the sprite.
		 *
		 * @see sf::Sprite::setPosition().
		 */
		inline void setPosition(sf::Vector2f pos) noexcept
		{
			sf::Sprite::setPosition(pos);
		}

		/**
		 * @brief Updates the rotation of the sprite.
		 * @complexity O(1).
		 *
		 * @param[in] rot: The new rotation for the sprite.
		 *
		 * @see sf::Sprite::setRotation.
		 */
		inline void setRotation(sf::Angle rot) noexcept
		{
			sf::Sprite::setRotation(rot);
		}

		/**
		 * @brief Scale of the sprite.
		 * @complexity O(1).
		 *
		 * @param[in] scale: The value to scale by.
		 *
		 * @see sf::Sprite::scale.
		 */
		inline void scale(sf::Vector2f scale) noexcept
		{
			sf::Sprite::scale(scale);
		}

		/**
		 * @brief Updates the color of the text.
		 * @complexity O(1).
		 *
		 * @param[in] color: The new color for the text.
		 *
		 * @see sf::Sprite::setColor().
		 */
		inline void setColor(sf::Color color) noexcept
		{
			sf::Sprite::setColor(color);
		}

		inline void setTextureRect(const sf::IntRect& rectangle) noexcept
		{
			sf::Sprite::setTextureRect(rectangle);
		}

		/**
		 * @brief Resizes the text when the window is resized.
		 * @complexity O(1).
		 *
		 * @param[in] scalingFactor: The scaling factor between the previous window's size and the new one.
		 * @param[in] scalingFactorMin: The scaling factor between the previous smallest window's size in x or y axis, and the new smallest one.
		 */
		inline void windowResized(sf::Vector2f scalingFactor, sf::Vector2f scalingFactorMin) noexcept
		{
			scale(scalingFactorMin);
			setPosition(sf::Vector2f{ getPosition().x * scalingFactor.x, getPosition().y * scalingFactor.y });
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

	private:

		size_t m_curTextureIndex{};
		std::vector<sf::Texture*> m_textures; // The textures used for the sprites. They are not copied, but shared.
	};

public:

	FixedGraphicalInterface() noexcept = delete;
	FixedGraphicalInterface(FixedGraphicalInterface const&) noexcept = delete;
	FixedGraphicalInterface(FixedGraphicalInterface&& other) noexcept;
	FixedGraphicalInterface& operator=(FixedGraphicalInterface const&) noexcept = delete;
	FixedGraphicalInterface& operator=(FixedGraphicalInterface&& other) noexcept;
	virtual ~FixedGraphicalInterface() noexcept;

	/**
	 * @brief Constructs the interface with a default background.
	 * @complexity O(1).
	 * 
	 * @param[in,out] window: The window where the interface elements will be rendered. 
	 * @param[in] backgroundFileName: The file name of the background.
	 * 
	 * @pre The window must be valid, and have a valid size.
	 * @post The instance is safe to use and the background is set with the appropriate size.
	 * @throw std::logic_error Basic exception guarrantee.
	 * 
	 * @pre The font should be named font.ttf and located at where ressourcePath points to.
	 * @post The font is loaded.
	 * @throw LoadingGUIRessourceFailure Strong exception guarrantee, but no text can be displayed.
	 * 
	 * @pre The path of the background should be valid within the res folder.
	 * @post Your background is loaded.
	 * @throw LoadingGUIRessourceFailure Strong exception guarrantee, but the default background is used.
	 * 
	 * @note If you don't want a background, just pass an empty string and no exceptions will be thrown.
	 * @note The font is loaded is common to all texts, and therefore, it is loaded once (when the
	 *		 first gui instance is created).
	 */
	explicit FixedGraphicalInterface(sf::RenderWindow* window, std::string const& backgroundFileName = "");


	/**
	 * @brief Adds a text element to the interface.
	 * @complexity O(1) : best case.
	 * @complexity O(N) : worst case, where N is the number of texts already added (if vector resizes).
	 * 
	 * @tparam T: Type that can be streamed to `std::basic_ostream`.
	 * @param[in] content: The text label.
	 * @param[in] position: The position of the text.
	 * @param[in] characterSize: The size of the text characters.
	 * @param[in] scale: The scale of the text.
	 * @param[in] color: The color of the text.
	 * @param[in] rot: The rotation of the text.
	 * 
	 * @note The scale is the same for both x and y axis.
	 * 
	 * @see FixedGraphicalInterface::TextWrapper, addSprite(), addShape().
	 */
	template<Ostreamable T>
	void addText(T const& content, sf::Vector2f position, unsigned int characterSize, float scale, sf::Color color = sf::Color{ 255, 255, 255 }, sf::Angle rot = sf::degrees(0)) noexcept
	{
		TextWrapper newText{ content, position, characterSize, scale, color, rot };
		m_texts.push_back(std::move(newText));
	}

	/**
	 * @brief Adds a sprite element to the interface.
	 * @complexity O(1) : best case.
	 * @complexity O(N) : worst case, where N is the number of sprites/shapes already added (if vector resizes).
	 * 
	 * @param[in] sprite: The sprite to add.
	 * @param[in] texture: The texture of the sprite.
	 * 
	 * @note The texture is not copied and therefore has to point somewhere that remains valid as long
	 *		 as the sprite is used. You could (and should) either point to a newly dynamically allocated
	 *		 texture, or to a texture that is already stored in the interface.
	 * 
	 * @see addText(), addShape().
	 */
	void addSprite(sf::Sprite sprite, sf::Texture texture) noexcept;

	/**
	 * @brief Adds a sprite element to the interface.
	 * @complexity O(1) : best case.
	 * @complexity O(N) : worst case, where N is the number of sprites/shapes already added (if vector resizes).
	 *
	 * @param[in] shape: The shape to add.
	 * @param[in] smooth: True if the shape needs to be smoothed.
	 * 
	 * @note sf::Shape is converted to a sf::Sprite/sf::Texture pair.
	 * @note sf::Shape is left unchanged even though it is not qualified with const.
	 * 
	 * @see addText(), addSprite(), convertShapeToSprite().
	 */
	void addShape(sf::Shape& shape, bool smooth = true) noexcept;

	/** 
	 * @brief Renders the interface.
	 * @complexity O(N), where N is the number of graphical elements.
	 *
	 * @pre The window must be valid.
	 * @post Safe to draw.
	 * @throw std::logic_error Basic exception guarrantee.
	 */
	virtual void draw() const;


	/**
	 * @brief Refreshes the transformables after the window is resized.
	 * @complexity O(N), where N is the number of graphical elements within all interfaces associated with the resized window.
	 *
	 * @param[in] window: The window which was resized, and for which the interfaces will be resized.
	 * @param[in] scalingFactor: The scaling factor between the previous window's size and the new one.
	 * 
	 * @note Resizes only interfaces associated with the resized window.
	 */
	static void windowResized(sf::RenderWindow* window, sf::Vector2f scalingFactor) noexcept;

protected:

	/**
	 * @details The pointers to the textures can become invalid when adding or removing an element
	 *          to the sprite vector.
	 * @complexity O(N), where N is the number of sprites/shapes.
	 */
	void resetTextureForSprites() noexcept;

	/**
	 * @brief Loads/Resizes the background image and resizes it to fit the window.
	 * @details Loads the default background first, and then loads the custom background (if needed, that
	 * 			is to say if the file name is not empty). If it fails, it uses the default background.
	 * @complexity O(1).
	 * 
	 * @param[in] fileName: The file name of the background.
	 * 
	 * @return A string containing an error message if the loading failed.
	 */
	std::optional<std::string> loadBackground(std::string const& backgroundFileName) noexcept;



	// Pointer to the window.
	mutable sf::RenderWindow* m_window; 
	
	using GraphicalElement = std::pair<sf::Sprite, std::shared_ptr<sf::Texture>>;
	
	static std::list<sf::Texture> s_textures; // All textures used in the interface. They are not copied, but shared.
	// Collection of sprites in the interface.
	std::vector<SpriteWrapper> m_sprites;

	// Collection of texts in the interface.
	std::vector<FixedGraphicalInterface::TextWrapper> m_texts; 


	// The relative path to the res folder used to load ressources. Default is "../res/". DO NOT CHANGE if not needed.
	static std::string const ressourcePath;

private:

	// Collection of all interfaces to perform the same operation (resizing for example). Stored by window.
	static std::unordered_multimap<sf::RenderWindow*, FixedGraphicalInterface*> allInterfaces;

	bool m_defaultBackground; // True if the default background is used.
};
//TODO: add s_ prefixe

/**
 * @brief  Manages an interface with changeable contents of elements (texts and shapes).
 *
 * @note This class stores UI componenents; it will consume a considerable amount of memory.
 * @note The background is dynamic and changeable with the getDSprite() function using the identifier
 *       "_background".
 * @note You should not put an underscore before dynamic elements' identifiers, as they are reserved
 *       for the class.
 * @note Ids must be unique within the text vector, and the shape/sprite vector. However, they can 
 * 		 be the same between a text and a sprite/shape.
 * @warning Do not delete the `sf::RenderWindow` passed as an argument while this class is in use.
 *
 * @code
 * sf::RenderWindow window{ ... };
 * DynamicGraphicalInterface gui{ &window };
 *
 * ...
 *
 * gui.addDynamicText("text 1", "Hello", sf::Vector2f{ 300, 100 }, 12, windowSizeMin / 1080.f, sf::Color{ 255, 0, 0 });
 * gui.addDynamicText("text 2", "Hallo", sf::Vector2f{ 100, 100 }, 14, windowSizeMin / 1080.f, sf::Color{ 255, 0, 0 });
 * gui.addText("text 3", "Bonjour", sf::Vector2f{ 200, 200 }, 14, windowSizeMin / 1080.f, sf::Color{ 0, 255, 0 }); // Not dynamic.
 * gui.addDynamicSprite("sprite", std::move(player), std::move(texturePlayer));
 * 
 * gui.getDText("text 1").updateContent("Hello World");
 * auto charSize = gui.getDText("text 2").getText().getCharacterSize(); // 14.
 *
 * ...
 *
 * window.clear();
 * gui.draw(); // First to be drawn.
 * ...
 * window.display();
 * @endcode
 *
 * @see FixedGraphicalInterface, UserInteractableGraphicalInterface.
 */
class DynamicGraphicalInterface : public FixedGraphicalInterface
{
public:

	DynamicGraphicalInterface() noexcept = delete;
	DynamicGraphicalInterface(DynamicGraphicalInterface const&) noexcept = delete;
	DynamicGraphicalInterface(DynamicGraphicalInterface&&) noexcept = default;
	DynamicGraphicalInterface& operator=(DynamicGraphicalInterface const&) noexcept = delete;
	DynamicGraphicalInterface& operator=(DynamicGraphicalInterface&&) noexcept = default;
	virtual ~DynamicGraphicalInterface() noexcept = default;

	/**
	 * @brief Constructs the interface with a default background.
	 * @complexity O(1).
	 *
	 * @param[out] window: The window where the interface elements will be rendered.
	 */
	explicit DynamicGraphicalInterface(sf::RenderWindow* window, std::string const& backgroundFileName = "")
		: FixedGraphicalInterface{ window, backgroundFileName }, m_dynamicTextIds{}, m_dynamicSpriteIds{}
	{
		m_dynamicSpriteIds.emplace("_background", 0); // The background is the first element in the vector.
	}


	/**
	 * @brief Adds a dynamic text element to the interface.
	 * @complexity O(1) : best case.
	 * @complexity O(N) : worst case, where N is the number of fixed AND dynamic texts already added
	 *  			      (if vector resizes).
	 *
	 * @tparam T: Type that can be streamed to `std::basic_ostream`.
	 * @param[in] content: The text label.
	 * @param[in] position: The position of the text.
	 * @param[in] characterSize: The size of the text characters.
	 * @param[in] scale: The scale of the text.
	 * @param[in] color: The color of the text.
	 * @param[in] rot: The rotation of the text.
	 * 
	 * @return True if added.
	 * 
	 * @note The scale is the same for both x and y axis.
	 * 
	 * @see addText().
	 */
	template<Ostreamable T>
	bool addDynamicText(std::string const& identifier, T const& content, sf::Vector2f position, unsigned int characterSize, float scale, sf::Color color = sf::Color{ 255, 255, 255 }, sf::Angle rot = sf::degrees(0))
	{
		if (m_dynamicTextIds.find(identifier) != m_dynamicTextIds.end() || identifier.empty())
			return false;

		addText(content, position, characterSize, scale, color, rot);
		m_dynamicTextIds[identifier] = m_texts.size() - 1;

		return true;
	}

	/**
	 * @brief Adds a dynamic sprite element to the interface.
	 * @complexity O(1) : best case.
	 * @complexity O(N) : worst case, where N is the number of fixed AND dynamic sprites/shapes already
	 *				      added (if vector resizes).
	 *
	 * @param[in] identifier: The sprite identifier.
	 * @param[in] sprite: The sprite to add.
	 * @param[in] texture: The texture of the sprite.
	 *
	 * @return True if added.
	 * 
	 * @note The texture is not copied and therefore has to point somewhere that remains valid as long
	 *		 as the sprite is used. You could (and should) either point to a newly dynamically allocated
	 *		 texture, or to a texture that is already stored in the interface.
	 *
	 * @see addSprite(), addDynamicShape().
	 */
	bool addDynamicSprite(std::string const& identifier, sf::Sprite sprite, std::shared_ptr<sf::Texture> texture) noexcept;

	/**
	 * @brief Adds a dynamic shape element to the interface.
	 * @complexity O(1) : best case.
	 * @complexity O(N) : worst case, where N is the number of fixed AND dynamic sprites/shapes already
	 *				      added (if vector resizes).
	 *
	 * @param[in] identifier: The text identifier.
	 * @param[in] shape: The shape to add.
	 * @param[in] smooth: True if the shape needs to be smoothed.
	 *
	 * @return True if added.
	 *
	 * @note sf::Shape is converted to a sf::Sprite/sf::Texture pair.
	 * @note sf::Shape is left unchanged even though it is not qualified with const.
	 *
	 * @see addShape(), addDynamicSprite().
	 */
	bool addDynamicShape(std::string const& identifier, sf::Shape& shape, bool smooth = true) noexcept;

	/**
	 * @param[in] identifier: The id of the dynamic text you want to check.
	 * @complexity O(1).
	 * 
	 * @return True if it exists.
	 */
	[[nodiscard]] inline bool doesDTextExist(std::string const& identifier) const noexcept
	{
		return m_dynamicTextIds.find(identifier) != m_dynamicTextIds.end();
	}

	/**
	 * @param[in] identifier: The id of the dynamic sprite you want to check.
	 * @complexity O(1).
	 * 
	 * @return True if it exists.
	 */
	[[nodiscard]] inline bool doesDSpriteExist(std::string const& identifier) const noexcept
	{
		return m_dynamicSpriteIds.find(identifier) != m_dynamicSpriteIds.end();
	}

	/**
	 * @brief Allows you to change some attributes of a dynamic text.
	 * @complexity O(1).
     * 
	 * @param[in] identifier: The id of the text you want to access.
	 * 
	 * @return A reference to the text you want to access.
	 * 
	 * @pre Be sure that you added a text with this id.
	 * @post The appropriate text is returned.
	 * @throw std::out_of_range Strong exception guarrantee.
	 */
	[[nodiscard]] inline FixedGraphicalInterface::TextWrapper& getDText(std::string const& identifier);

	/**
	 * @brief Allows you to change some attributes of a dynamic sprite.
	 * @complexity O(1).
	 *
	 * @param[in] identifier: The id of the sprite you want to access.
	 *
	 * @return A reference to the sprite, with its texture, you want to access.
	 *
	 * @pre Be sure that you added the sprite/shape with this id.
	 * @post The appropriate sprite/texture is returned.
	 * @throw std::out_of_range Strong exception guarrantee.
	 */
	[[nodiscard]] inline GraphicalElement* getDSprite(std::string const& identifier);

	/**
	 * @brief Removes a dynamic text element.
	 * @complexity O(N) : where N is the number of fixed AND dynamic texts already added.
	 *
	 * @param[in] identifier: The id of the text you want to remove.
	 * 
	 * @return True if removed.
	 *
	 * @note No effect if not there.
	 */
	virtual bool removeDText(std::string const& identifier) noexcept;

	/**
	 * @brief Removes a dynamic sprite element.
	 * @complexity O(N) : where N is the number of fixed AND dynamic sprites/shapes already added.
	 *
	 * @param[in] identifier: The id of the sprite you want to remove.
	 * 
	 * @return True if removed.
	 *
	 * @note No effect if not there.
	 * @note You can't remove the background : it has no effect.
	 * @note Since shapes are converted to sprites, you can remove them with this function.
	 */
	bool removeDSprite(std::string const& identifier) noexcept;

protected:

	std::unordered_map<std::string, size_t> m_dynamicTextIds; // All indexes of dynamic texts in the interface.
	std::unordered_map<std::string, size_t> m_dynamicSpriteIds; // All indexes of dynamic sprites/shapes in the interface.
};


/** 
 * @brief Manages an interface in which the user can interact such as writing, pressing buttons...
 *
 * @note This class stores UI componenents; it will consume a considerable amount of memory.
 * @note This class does not add any new possibilities than DynamicGraphicalInterfaces, it only add
 * 		 prebuilt interactable elements such as buttons, sliders and multiple question boxes.
 * @warning Do not delete the `sf::RenderWindow` passed as an argument while this class is in use.
 *
 * @see DynamicGraphicalInterface.
 */
class UserInteractableGraphicalInterface : public DynamicGraphicalInterface
{
private:

	struct Slider
	{
		Slider() noexcept : m_mathFunction{ [](float x) { return x; } }, m_userFunction{ [](float x) { return x; } }, m_intervals{ -1 } {}
		Slider(Slider const&) noexcept = delete;
		Slider(Slider&&) noexcept = default;
		Slider& operator=(Slider const&) noexcept = delete;
		Slider& operator=(Slider&&) noexcept = default;
		~Slider() noexcept = default;

		inline Slider(std::function<float(float)> mathFunction, std::function<void(float)> functionOfChange, int interval) noexcept
			: m_mathFunction{ mathFunction }, m_userFunction{ functionOfChange }, m_intervals{ interval }
		{}

		std::function<float(float)> m_mathFunction; // The function to apply to the value of the slider when it is changed.
		std::function<void(float)> m_userFunction; // The function to call when the slider value is changed (e.g. to update the text displaying the current value).
		int m_intervals;
	};

	struct MultipleQuestionBox
	{
		MultipleQuestionBox() noexcept : m_multipleChoices{ false }, m_numberOfBoxes{ 0 }, m_boxes{}, m_currentlyHovered{ 0 } {}
		MultipleQuestionBox(MultipleQuestionBox const&) noexcept = delete;
		MultipleQuestionBox(MultipleQuestionBox&&) noexcept = default;
		MultipleQuestionBox& operator=(MultipleQuestionBox const&) noexcept = delete;
		MultipleQuestionBox& operator=(MultipleQuestionBox&&) noexcept = default;
		~MultipleQuestionBox() noexcept = default;

		inline MultipleQuestionBox(bool multipleChoices, unsigned int numberOfBoxes, unsigned int checkedByDefault = 0) noexcept
			: m_multipleChoices{ multipleChoices }, m_numberOfBoxes{ numberOfBoxes }, m_boxes{}, m_currentlyHovered{ 0 }
		{
			m_boxes.resize(numberOfBoxes, false); // Initialize all boxes to unchecked.

			if (checkedByDefault-1 < numberOfBoxes && checkedByDefault-1 >= 0)
				m_boxes[checkedByDefault-1] = true; // Set the default checked box.
		}

		//TODO: faire du statique plus souvent
		static std::shared_ptr<sf::Texture> m_uncheckedTexture; // The texture used for the checkboxes.
		static std::shared_ptr<sf::Texture> m_checkedTexture; // The texture used for the checkboxes.
		bool m_multipleChoices;
		unsigned int m_numberOfBoxes;

		std::vector<bool> m_boxes;
		unsigned int m_currentlyHovered; // 0 if no box is hovered, otherwise the index of the hovered box.
	};

	enum class InteractableItem
	{
		None,
		Button,
		Slider,
		MQB
	};

	using IdentifierInteractableItem = std::pair<InteractableItem, std::string const*>;
	using MQB = MultipleQuestionBox;

public:

	UserInteractableGraphicalInterface() noexcept = delete;
	UserInteractableGraphicalInterface(UserInteractableGraphicalInterface const&) noexcept = delete;
	UserInteractableGraphicalInterface(UserInteractableGraphicalInterface&&) noexcept = default;
	UserInteractableGraphicalInterface& operator=(UserInteractableGraphicalInterface const&) noexcept = delete;
	UserInteractableGraphicalInterface& operator=(UserInteractableGraphicalInterface&&) noexcept = default;
	virtual ~UserInteractableGraphicalInterface() noexcept = default;

	/**
	 * @brief Constructs the interface with a default background.
	 * @complexity O(1).
	 *
	 * @param[out] window: The window where the interface elements will be rendered.
	 *
	 * @see createBackground().
	 */
	inline explicit UserInteractableGraphicalInterface(sf::RenderWindow* window, std::string const& backgroundFileName = "") noexcept
		: DynamicGraphicalInterface{ window, backgroundFileName }, m_buttons{}, m_sliders{}
	{}

	/**
	 * @brief Adds a button to the interface by turning a text into a button.
	 * @complexity O(1).
	 *
	 * @param[in] id: The id of the text you want to turn into a button
	 * @param[in] function: The function executed when the button is pressed.
	 *
	 * @note No effect if no text with the id, or there's already a button with the same id.
	 * @note The button id is the same as the text id.
	 * @note You can still access the text turned with the function getDText(), and remove it with removeDText().
	 */
	bool addButton(std::string const& id, std::function<void()> function = [](){}) noexcept;

	bool addSlider(std::string const& id, sf::Vector2u size, sf::Vector2f pos, std::function<float(float)> mathFunction = [](float x){return x;}, std::function<void(float)> changeFunction = [](float x){}, int interval = -1, bool showValueWithText = true) noexcept;

	bool addMQB(std::string const& id, bool multipleChoices, unsigned int numberOfBoxes, sf::Vector2f pos1, sf::Vector2f pos2, unsigned int defaultChecked = 1) noexcept; // TODO: Mettre en diff au lieu de pos2

	/**
	 * @param[in] identifier: The id of the button you want to check.
	 * @complexity O(1).
	 *
	 * @return True if it exists.
	 */
	[[nodiscard]] inline bool doesButtonExist(std::string const& identifier) const noexcept
	{
		return m_buttons.find(identifier) != m_buttons.end();
	}

	/**
	 * @param[in] identifier: The id of the slider you want to check.
	 * @complexity O(1).
	 *
	 * @return True if it exists.
	 */
	[[nodiscard]] inline bool doesSliderExist(std::string const& identifier) const noexcept
	{
		return m_sliders.find(identifier) != m_sliders.end();
	}

	/**
	 * @param[in] identifier: The id of the slider you want to check.
	 * @complexity O(1).
	 *
	 * @return True if it exists.
	 */
	[[nodiscard]] inline bool doesMQBExist(std::string const& identifier) const noexcept
	{
		return m_mqbs.find(identifier) != m_mqbs.end();
	}

	/**
	 * @brief Returns the function associated with the button.
	 * @complexity O(1).
	 * 
	 * @param[in] identifier: the id of the button you want to access.
	 * 
	 * @return A reference to the function lambda you want to access.
	 * 
	 * @pre Be sure that you added a button with this id.
	 * @post The appropriate lambda is returned.
	 * @throw std::out_of_range if id not there.
	 */
	[[nodiscard]] inline std::function<void()>& getFunctionOfButton(std::string const& identifier);

	/**
		 * @brief Returns the state of the boxes (checked: true, unchecked: false).
		 * @complexity O(1).
		 *
		 * @param[in] identifier: the id of the mqb you want to access.
		 *
		 * @return A vector of booleans representing the state of the boxes.
		 *
		 * @pre Be sure that you added a mqb with this id.
		 * @post The appropriate lambda is returned.
		 * @throw std::out_of_range if id not there.
		 */
	[[nodiscard]] inline std::vector<bool> const& getStateOfMQB(std::string const& identifier);


	/**
	 * @brief Returns the current value associated with the slider.
	 * @complexity O(1).
	 *
	 * @param[in] identifier: the id of the slider you want to access.
	 *
	 * @return The current value of the slider.
	 *
	 * @pre Be sure that you added a slider with this id.
	 * @post The appropriate value is returned.
	 * @throw std::out_of_range if id not there.
	 */
	[[nodiscard]] float getValueOfSlider(std::string const& identifier);

	/**
	 * @brief Returns the slider object associated with the slider.
	 * @details It allows you to access the slider's properties: its functions and intervals.
	 * @complexity O(1).
	 *
	 * @param[in] identifier: the id of the slider you want to access.
	 *
	 * @return The current value of the slider.
	 *
	 * @pre Be sure that you added a slider with this id.
	 * @post The appropriate value is returned.
	 * @throw std::out_of_range if id not there.
	 */
	[[nodiscard]] inline UserInteractableGraphicalInterface::Slider& getSlider(std::string const& identifier);

	/**
	 * @brief Removes a dynamic text, and the button associated (if it exists).
	 * @complexity O(N), where N is the number of texts (static + dynamic) (if vector resizes).
	 *
	 * @param[in] identifier: The id of the text you want to remove.
	 *
	 * @return True if removed.
	 *
	 * @note No effect if not there.
	 */
	virtual bool removeDText(std::string const& identifier) noexcept final;

	bool removeSlider(std::string const& identifier) noexcept;

	bool removeMQB(std::string const& identifier) noexcept;

	/**
	 * @brief Updates the hovered element.
	 * @details You (might not) want to call this function every time the mouse moved. You should
	 * 			rather call it when the mouse mouve AND you want to update the hovered element. It may be
	 *			each time the mouse move. However, if you have a slider and want to let the user update the
	 *			slider as long as he keeps the mouse pressed, then you should call this function while the
	 *			mouse is unpressed.
	 * @complexity O(N), where N is the number of interactable elements in your active interface.
	 *
	 * @param[out] activeGUI: The active GUI tu update.
	 *
	 * @return The type + the id of the element that is currently hovered.
	 *
	 * @note No effect if the active GUI is not an interactable GUI.
	 */
	static IdentifierInteractableItem mouseMoved(FixedGraphicalInterface* activeGUI) noexcept;

	/**
	 * @brief Tells the active GUI that the mouse is pressed.
	 * @complexity O(1).
	 *
	 * @param[out] activeGUI: The active GUI to update.
	 *
	 * @return The type + the id of the element that is currently hovered.
	 *
	 * @note No effect if the active GUI is not an interactable GUI or nothing is hovered.
	 * @note You should not call this function when the mouse pressed EVENT is triggered. Instead, you
	 * 		 should call it as long as the mouse is pressed and if there's an event (the first frame it/
	 *		 is pressed/the mouse moved).
	 *
	 * @code
	 * while (const std::optional event = window.pollEvent())
	 * {
	 *		if (event->is<sf::Event::Closed>())
	 *			window.close();
	 *
	 *		...
	 *
	 *		// When the mouse is pressed + if there's an event (which is entering the loop).
	 *		if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left))
	 *			IGInterface::mousePressed(interface);
	 * }
	 * @endcode
	 */
	static IdentifierInteractableItem mousePressed(FixedGraphicalInterface* activeGUI) noexcept;

	/**
	 * @brief Tells the active GUI that the mouse is released.
	 * @complexity O(1).
	 *
	 * @param[out] activeGUI: The active GUI to update.
	 *
	 * @return The type + the id of the element that is currently hovered.
	 *
	 * @note No effect if the active GUI is not an interactable GUI or nothing is hovered.
	 * @note You should call this function when the mouse released event is triggered.
	 */
	static IdentifierInteractableItem mouseUnpressed(FixedGraphicalInterface* activeGUI) noexcept;

	/**
	 * @complexity O(1).
	 *
	 * @return The type + the id of the element that is currently hovered.
	 */
	[[nodiscard]] static inline IdentifierInteractableItem getHoveredInteractableItem() noexcept
	{
		return m_hoveredElement;
	}

private:

	void changeValueSlider(std::string const& id, int mousePosY);


	std::unordered_map<std::string, std::function<void()>> m_buttons; // Collection of buttons in the interface.
	std::unordered_map<std::string, Slider> m_sliders; // Collection of sliders in the interface.
	std::unordered_map<std::string, MultipleQuestionBox> m_mqbs; // Collection of sliders in the interface.


	static IdentifierInteractableItem m_hoveredElement; // The type of the button that is currently hovered.
};


class WritableGraphicalInterface : public UserInteractableGraphicalInterface
{
public:

	WritableGraphicalInterface() noexcept = delete;
	WritableGraphicalInterface(WritableGraphicalInterface const&) noexcept = delete;
	WritableGraphicalInterface(WritableGraphicalInterface&&) noexcept = default;
	WritableGraphicalInterface& operator=(WritableGraphicalInterface const&) noexcept = delete;
	WritableGraphicalInterface& operator=(WritableGraphicalInterface&&) noexcept = default;
	virtual ~WritableGraphicalInterface() noexcept = default;

	inline WritableGraphicalInterface(sf::RenderWindow* window, std::string const& backgroundFileName = "", std::function<void(char32_t&, std::string&)> function = [](char32_t&, std::string&){}) // TODO: retirer non parametre des autres lambda
		: UserInteractableGraphicalInterface{ window, backgroundFileName }, m_textEnteredFunction{ function }
	{
		sf::RectangleShape cursorEditing{ sf::Vector2f{ 1.f, 1.f } };

		cursorEditing.setSize(sf::Vector2f{ 2.f, 20.f });
		cursorEditing.setOrigin(cursorEditing.getLocalBounds().getCenter());
		cursorEditing.setPosition(sf::Vector2f{ window->getView().getSize().x * 2, 0.f}); // Hide it
		cursorEditing.setFillColor(sf::Color{ 80, 80, 80 });
		addDynamicShape("__cursorEditing", cursorEditing, false); // Add the cursor to the interface.
	}


	bool setCurrentlyEditedText(std::string const& identifier) noexcept;

	static std::string textEntered(FixedGraphicalInterface* curInterface, char32_t unicodeValue) noexcept;

	[[nodiscard]] inline bool isCurrentlyEditing() const noexcept
	{
		return !m_currentlyEditedText.empty();
	}

	[[nodiscard]] inline sf::String const& getCurrentlyEditingText() const
	{
		return m_texts[m_dynamicTextIds.at(m_currentlyEditedText)].getText().getString(); // Throw std::out_of_range if not there.
	}

	[[nodiscard]] inline std::function<void(char32_t&, std::string&)>& getFunctionOfEditing() 
	{
		return m_textEnteredFunction;
	}

private:

	void applyNewText(TextWrapper* text, std::string& newText) noexcept;


	static std::string m_currentlyEditedText; // Collection of texts in the interface. (overrides the one from FixedGraphicalInterface)
	std::function<void(char32_t&, std::string&)> m_textEnteredFunction;
};


using FGInterface = FixedGraphicalInterface;
using DGInterface = DynamicGraphicalInterface;
using IGInterface = UserInteractableGraphicalInterface;
using WGInterface = WritableGraphicalInterface;
using GUI = WGInterface;
//TODO: mettre les using en haut

#endif // GUI_HPP