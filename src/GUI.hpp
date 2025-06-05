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
#include <array>
#include <algorithm>
#include <initializer_list>
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
	void addSprite(sf::Sprite sprite, std::shared_ptr<sf::Texture> texture) noexcept;

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


	/**
	 * @brief A wrapper for `sf::Text` that initializes the text, keeps the font, manages resizing.
	 *
	 * @note The font is common to all instances of this class.
	 * @warning Call TextWrapper::loadFont() to initialize properly the font, before creating any other
	 *          functions.
	 *
	 * @see FixedGraphicalInterface, sf::Text, create().
	 */
	struct TextWrapper : private sf::Text
	{
	public:

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
			: sf::Text{ m_font, "", characterSize}
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


	// Pointer to the window.
	mutable sf::RenderWindow* m_window; 
	
	using GraphicalElement = std::pair<sf::Sprite, std::shared_ptr<sf::Texture>>;
	// Collection of sprites in the interface.
	std::vector<GraphicalElement> m_sprites;

	// Collection of texts in the interface.
	std::vector<FixedGraphicalInterface::TextWrapper> m_texts; 


	// The relative path to the res folder used to load ressources. Default is "../res/". DO NOT CHANGE if not needed.
	static std::string const ressourcePath;

private:

	// Collection of all interfaces to perform the same operation (resizing for example). Stored by window.
	static std::unordered_multimap<sf::RenderWindow*, FixedGraphicalInterface*> allInterfaces;

	bool m_defaultBackground; // True if the default background is used.
};


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
	[[nodiscard]] FixedGraphicalInterface::TextWrapper& getDText(std::string const& identifier);

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
	[[nodiscard]] GraphicalElement* getDSprite(std::string const& identifier);

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
 * @warning Do not delete the `sf::RenderWindow` passed as an argument while this class is in use.
 *
 * @see DynamicGraphicalInterface.
 */
class UserInteractableGraphicalInterface : public DynamicGraphicalInterface
{
public:

	enum class InteractableItem
	{
		None,
		Button,
		Slider,
		//MultipleQuestionBox,
	};

	using IdentifierInteractableItem = std::pair<InteractableItem, std::string const*>; 


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
	inline explicit UserInteractableGraphicalInterface(sf::RenderWindow* window) noexcept
		: DynamicGraphicalInterface{ window }, m_buttons{}, m_sliders{}
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
	[[nodiscard]] std::function<void()>& getFunctionOfButton(std::string const& identifier);

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
	[[nodiscard]] UserInteractableGraphicalInterface::Slider& getSlider(std::string const& identifier);

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

	struct Slider
	{
		Slider() noexcept : m_mathFunction{ [](float x) { return x; } }, m_userFunction{ [](float x) { return x; } }, m_intervals{ -1 }
		{}
		Slider(Slider const&) noexcept = delete;
		Slider(Slider&&) noexcept = default;
		Slider& operator=(Slider const&) noexcept = delete;
		Slider& operator=(Slider&&) noexcept = default;
		~Slider() noexcept = default;

		Slider(std::function<float(float)> mathFunction, std::function<void(float)> functionOfChange, int interval) noexcept
			: m_mathFunction{ mathFunction }, m_userFunction{ functionOfChange }, m_intervals{ interval }
		{}

		std::function<float(float)> m_mathFunction; // The function to apply to the value of the slider when it is changed.
		std::function<void(float)> m_userFunction; // The function to call when the slider value is changed (e.g. to update the text displaying the current value).
		int m_intervals;
	};

	void changeValueSlider(std::string const& id, int mousePosY);

	std::unordered_map<std::string, std::function<void()>> m_buttons; // Collection of buttons in the interface.
	std::unordered_map<std::string, Slider> m_sliders; // Collection of sliders in the interface.


	static IdentifierInteractableItem m_hoveredElement; // The type of the button that is currently hovered.
};


//class WritableGraphicalInterface : public UserInteractableGraphicalInterface
//{};


using FGInterface = FixedGraphicalInterface;
using DGInterface = DynamicGraphicalInterface;
using IGInterface = UserInteractableGraphicalInterface;
//using WGInterface = WritableGraphicalInterface;
using GUI = IGInterface;

#endif // GUI_HPP






//bool addSlider(std::string const& id, sf::Vector2f pos, unsigned int length, float scale, float minValue = 0, float maxValue = 1, int intervalle = 0, bool displayCurrentValue = true) noexcept;
//
//double getValueSlider(std::string const& id) const;
//
//bool removeSlider(std::string const& identifier) noexcept;




//using MQB = MultipleQuestionBox;
//using DCM = DoubleCheckerMode;
//std::unordered_map<std::string, MQB> m_mqbs; // Collection of MQBs in the interface.
//std::unordered_map<std::string, DCM> m_dcms; // Collection of MQBs in the interface.
//class MultipleQuestionBox
//{
	//public:

	//	MultipleQuestionBox() noexcept = delete;
	//	MultipleQuestionBox(MultipleQuestionBox const&) noexcept = default;
	//	MultipleQuestionBox(MultipleQuestionBox&&) noexcept = default;
	//	~MultipleQuestionBox() noexcept = default;
	//	MultipleQuestionBox& operator=(MultipleQuestionBox const&) noexcept = default;
	//	MultipleQuestionBox& operator=(MultipleQuestionBox&&) noexcept = default;

	//	/**
	//	 * 
	//	 * \param box
	//	 * \param onlyOne: True if only one box can be selected.
	//	 */
	//	MultipleQuestionBox(std::vector<sf::Vector2f> pos, bool onlyOne) noexcept;


	//	void addBox(sf::Vector2f pos) noexcept;

	//	void removeBox(size_t index) noexcept;
	//	
	//	void mousePressed() noexcept;

	//	std::vector<bool> getValues() const noexcept;

	//	void reset(bool value = false) noexcept;


	//	bool onlyeOne; // True if only one box can be selected.

	//private:

	//	std::vector<std::pair<sf::Sprite, bool>> m_boxes; // The boxes of the MQB.
	//	sf::Texture m_textureBoxUnchecked; // The texture of the boxes.
	//	sf::Texture m_textureBoxChecked; // The texture of the boxes.
//};