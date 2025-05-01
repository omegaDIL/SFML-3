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
 * @see sf::RenderWindow, FixedGraphicalInterface::TextWrapper.
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
	 * @complexity O(1) : average case.
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
	 * @complexity O(1) : average case.
	 * @complexity O(N) : worst case, where N is the number of sprites/shapes already added (if vector resizes).
	 *
	 * @param[in] sprite: The sprite to add.
	 * @param[in] texture: The texture of the sprite.
	 * 
	 * @see addText(), addShape().
	 */
	void addSprite(sf::Sprite sprite, sf::Texture texture) noexcept;

	/**
	 * @brief Adds a sprite element to the interface.
	 * @complexity O(1) : average case.
	 * @complexity O(N) : worst case, where N is the number of sprites/shapes already added (if vector resizes).
	 *
	 * @param[in] shape: The shape to add.
	 * @param[in] smooth: True if the shape needs to be smoothed.
	 * 
	 * @note sf::Shape is converted to a sf::Sprite/sf::Texture pair.
	 * @note sf::Shape is left unchanged even though it is not const.
	 * 
	 * @see addText(), addSprite(), convertShapeToSprite().
	 */
	void addShape(sf::Shape* shape, bool smooth = true) noexcept;

	/** 
	 * @brief Renders the interface.
	 * @complexity O(N + K), where N is the number of texts, and K the number of sprites (+ shapes).
	 *
	 * @pre The window must (still) be valid.
	 * @post Safe to draw.
	 * @throw std::logic_error Basic exception guarrantee.
	 */
	virtual void draw() const;


	/**
	 * @brief Refreshes the transformables after the window is resized.
	 * @complexity O(N), where N is the number of graphical elements within interfaces associated with the resized window.
	 *
	 * @param[in] window: The window which was resized, and for which the interfaces will be resized.
	 * @param[in] scalingFactor: The scaling factor between the previous window's size and the new one.
	 * 
	 * @note Resizes only interfaces associated with the resized window.
	 */
	static void windowResized(sf::RenderWindow* window, sf::Vector2f scalingFactor) noexcept;

protected:

	/**
	 * @details The pointers to the textures become invalid when adding or removing an element
	 *          to the sprite vector.
	 * @complexity O(N), where N is the size of 'm_sprites'
	 */
	void resetTextureForSprites() noexcept;

	/**
	 * @brief Loads/Resizes the background image and resizes it to fit the window.
	 * @details Loads the default background first, and then loads the custom background (if needed, that
	 * 			is to say if the file name is not empty). If it fails, it uses the default background.
	 * @complexity O(1).
	 * 
	 * @param[in] fileName The file name of the background.
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
		inline TextWrapper(T const& content, sf::Vector2f pos, unsigned int characterSize, float scale, sf::Color color = sf::Color{ 255, 255, 255 }, sf::Angle rot = sf::degrees(0)) noexcept
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
		 * @param[in] scalingFactorMin: The scaling factor between the previous smallest window's size in x or y axis, and the new one.
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
	
	using GraphicalElement = std::pair<sf::Sprite, sf::Texture>;
	// Collection of sprites in the interface.
	std::vector<GraphicalElement> m_sprites;

	// Collection of texts in the interface.
	std::vector<FixedGraphicalInterface::TextWrapper> m_texts; 


	// The relative path to the res folder used to load ressources. Default is "../res/". DO NOT CHANGE if not needed.
	static std::string const ressourcePath;

private:

	// Collection of all interfaces to perform the same operation (resizing for example). Stored by window.
	static std::unordered_multimap<sf::RenderWindow*, FixedGraphicalInterface*> allInterfaces;

	bool m_defaultBackground{ true }; // True if the default background is used.
};


/**
 * @brief  Manages an interface with changeable contents of elements (texts and shapes).
 *
 * @note This class stores UI componenents; it will consume a considerable amount of memory.
 * @note The background is dynamic and changeable with the getDSprite() function using the identifier
 *       "background". Note that is not removable.
 * @warning Do not delete the `sf::RenderWindow` passed as an argument while this class is in use.
 *
 * @code
 * sf::RenderWindow window{ ... };
 * DynamicGraphicalInterface gui{ &window };
 *
 * ...
 *
 * gui.addDynamicText("text 1", "Hello", sf::Vector2f{ 300, 100 }, 12, windowSizeMin / 1080.f, sf::Color{ 255, 0, 0 });
 * gui.addDynamicText("text 2", "Hallo", sf::Vector2f{ 100, 100 }, 12, windowSizeMin / 1080.f, sf::Color{ 255, 0, 0 });
 * gui.addDynamicSprite("sprite", std::move(player), std::move(texturePlayer));
 * 
 * gui.getDText("text 1").updateContent("Hello World");
 *
 * ...
 *
 * window.clear();
 * gui.draw();
 * ...
 * window.display();
 * @endcode
 *
 * @see FixedGraphicalInterface.
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
		: FixedGraphicalInterface{ window, backgroundFileName }, m_dynamicTextsIds{}, m_dynamicSpritesIds{}
	{
		m_dynamicSpritesIds.emplace("background", 0); // The background is the first element in the vector.
	}


	/**
	 * @brief Adds a dynamic text to the interface.
	 * @complexity O(1) : average case.
	 * @complexity O(N) : worst case, where N is the number of texts already added (if vector resizes).
	 * 
	 * @tparam T: Type that can be streamed to `std::basic_ostream`.
	 * @param[in] identifier:     The text identifier.
	 * @param[in] content:		  The text label.
	 * @param[in] position:       The position of the text.
	 * @param[in] characterSize:  The size of the text characters.
	 * @param[in] color:		  The color of the text.
	 * 
	 * @return True if added.
	 *
	 * @note No effect if a text has the same id.
	 * @note The identifiers must be unique between texts.
	 * @note The ids between one text and one sprite/shape can be the same.
	 * 
	 * @see addText().
	 */
	template<Ostreamable T>
	bool addDynamicText(std::string const& identifier, T const& content, sf::Vector2f position, unsigned int characterSize, float scale, sf::Color color = sf::Color{ 255, 255, 255 }, sf::Angle rot = sf::degrees(0))
	{
		if (m_dynamicTextsIds.find(identifier) != m_dynamicTextsIds.end())
			return false;

		addText(content, position, characterSize, scale, color, rot);
		m_dynamicTextsIds[identifier] = m_texts.size() - 1;

		return true;
	}

	/**
	 * @brief Adds a dynamic sprite element to the interface.
	 * @complexity O(1) : average case.
	 * @complexity O(N) : worst case, where N is the number of sprites/shapes already added (if vector resizes).
	 *
	 * @param[in] identifier: The text identifier.
	 * @param[in] sprite: The sprite to add.
	 * @param[in] texture: The texture of the sprite.
	 *
	 * @return True if added.
	 * 
	 * @note No effect if a sprite/shape has the same id.
	 * @note The identifiers must be unique between shapes/sprites.
	 * @note The ids between one text and one sprite/shape can be the same.
	 *
	 * @see addSprite().
	 */
	bool addDynamicSprite(std::string const& identifier, sf::Sprite sprite, sf::Texture texture) noexcept;

	/**
	 * @brief Adds a dynamic shape element to the interface.
	 * @complexity O(1) : average case.
	 * @complexity O(N) : worst case, where N is the number of sprites/shapes already added (if vector resizes).
	 *
	 * @param[in] identifier: The text identifier.
	 * @param[in] shape: The shape to add.
	 * @param[in] smooth: True if the shape needs to be smoothed.
	 *
	 * @return True if added.
	 *
	 * @note No effect if a sprite/shape has the same id.
	 * @note The identifiers must be unique between shapes/sprites.
	 * @note The ids between one text and one sprite/shape can be the same.
	 *
	 * @see addShape().
	 */
	bool addDynamicShape(std::string const& identifier, sf::Shape* shape, bool smooth = true) noexcept;

	/**
	 * @param[in] identifier: The id of the text you want to check.
	 * @complexity O(1).
	 * 
	 * @return True if it exists.
	 */
	[[nodiscard]] inline bool doesDTextExist(std::string const& identifier) const noexcept
	{
		return m_dynamicTextsIds.find(identifier) != m_dynamicTextsIds.end();
	}

	/**
	 * @param[in] identifier: The id of the sprite you want to check.
	 * @complexity O(1).
	 * 
	 * @return True if it exists.
	 */
	[[nodiscard]] inline bool doesDSpriteExist(std::string const& identifier) const noexcept
	{
		return m_dynamicSpritesIds.find(identifier) != m_dynamicSpritesIds.end();
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
	 * @throw std::out_of_range if id not there.
	 */
	[[nodiscard]] FixedGraphicalInterface::TextWrapper& getDText(std::string const& identifier);

	/**
	 * @brief Allows you to change some attributes of a dyanmic sprite.
	 * @complexity O(1).
	 *
	 * @param[in] identifier: The id of the sprite you want to access.
	 *
	 * @return A reference to the sprite you want to access.
	 *
	 * @pre Be sure that you added the sprite/shape with this id.
	 * @post The appropriate sprite/texture is returned.
	 * @throw std::out_of_range if id not there.
	 */
	[[nodiscard]] std::pair<sf::Sprite*, sf::Texture*> getDSprite(std::string const& identifier);

	/**
	 * @brief Removes a dynamic text.
	 * @complexity O(N), where N is the number of texts (static + dynamic).
	 *
	 * @param[in] identifier: The id of the text you want to remove.
	 * 
	 * @return True if removed.
	 *
	 * @note No effect if not there.
	 */
	virtual bool removeDText(std::string const& identifier) noexcept;

	/**
	 * @brief Removes a sprite
	 * @complexity O(N), where N is the number of sprites/shapes (static + dynamic).
	 *
	 * @param[in] identifier: The id of the sprite you want to remove.
	 * 
	 * @return True if removed.
	 *
	 * @note No effect if not there.
	 * @note You can't remove the background : it has no effect.
	 */
	bool removeDSprite(std::string const& identifier) noexcept;

protected:

	std::unordered_map<std::string, size_t> m_dynamicTextsIds; // All index of dynamic texts in the interface.
	std::unordered_map<std::string, size_t> m_dynamicSpritesIds; // All index of dynamic sprites in the interface.
};


/** 
 * @brief Manages an interface in which the user can interact such as writing, pressing buttons...
 *
 * @note This class stores UI componenents; it will consume a considerable amount of memory.
 * @warning Do not delete the `sf::RenderWindow` passed as an argument while this class is in use.
 *
 * @see DynamicGraphicalInterface, Slider, sf::RenderWindow.
 */
class UserInteractableGraphicalInterface : public DynamicGraphicalInterface
{
public:

	enum class InteractableItem
	{
		None,
		Button,
		Slider,
		MultipleQuestionBox,
		DoubleCheckerMode
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
		: DynamicGraphicalInterface{ window }, m_buttons{}, m_sliders{}, m_hoveredElement{ std::make_pair(InteractableItem::None, nullptr) }
	{}


	/**
	 * @brief Adds a button to the interface by turning a text into a button.
	 *
	 * @param[in] id: The id of the text you want to turn into a button
	 * @param[in] function: The function executed when the button is pressed.
	 *
	 * @note No effect if no text with the id, or there's already a button with the same id.
	 * @note The button id is the same as the text id.
	 * @note You can still access the text turned with the function getDText().
	 * @warning Don't remove the button using removeDText(), use removeButton() instead.
	 */
	bool addButton(std::string const& id, std::function<void()> function = [](){}) noexcept;

	[[nodiscard]] std::function<void()>& getFunctionOfButton(std::string const& identifier);

	/**
	 * @brief Removes a button
	 *
	 * @param[in] identifier: The id of the button you want to remove.
	 *
	 * @note No effect if not there.
	 */
	bool removeButton(std::string const& identifier) noexcept;

	bool addSlider(std::string const& id, sf::Vector2f pos, unsigned int length, float scale, int minValue = 0, int maxValue = 1, int intervalle = 0, bool displayCurrentValue = true) noexcept;

	double getValueSlider(std::string const& id) const;

	bool removeSlider(std::string const& identifier) noexcept;

	/** 
	 * @brief Detects if the mouse is hovering over a button to update its visual state.
	 * @complexity O(N), where N is the number of buttons.
	 *
	 * @note You should call this function when you change the menu to position the button's background 
	 *       accordingly.
	 * 
	 * @pre The window must have been valid when passed to the constructor and still exists.
	 * @post The elements are drawn on the window.
	 * @throw std::logic_error if window is nullptr.
	 */
	IdentifierInteractableItem mouseMoved();

	IdentifierInteractableItem mousePressed() noexcept;

	IdentifierInteractableItem mouseUnpressed() noexcept;


	/**
	 * @complexity O(1).
	 * 
	 * @return The id of the button that is currently hovered.
	 */
	[[nodiscard]] inline IdentifierInteractableItem getHoveredInteractableItem() const noexcept
	{
		return m_hoveredElement;
	}

	/**
	 * @brief Renders the interface.
	 * @complexity O(N + K + M), where N is the number of texts, K the number of shapes, and M the number
	 *			   of sliders.
	 *
	 * @pre The window must have been valid when passed to the constructor and still exists.
	 * @post The elements are drawn on the window.
	 * @throw std::logic_error if window is nullptr.
	 */
	virtual void draw() const override;

private:

	class Slider
	{
	public:

		Slider() noexcept : m_maxValue{ 1 }, m_minValue{ 0 }, m_intervalles { 0 } {}
		Slider(Slider const&) noexcept = delete;
		Slider& operator=(Slider const&) noexcept = delete;
		~Slider() noexcept = default;

		Slider(sf::RenderWindow* window, sf::Vector2f pos, sf::Vector2u size, float scale, int minValue = 0, int maxValue = 1, int intervalle = 0, bool displayCurrentValue = true) noexcept;
		
		Slider(Slider&&) noexcept;

		Slider& operator=(Slider&&) noexcept;


		/**
		 * .
		 *
		 * \return
		 */
		void changeValue(float mousePosY) noexcept;

		/**
		 * .
		 *
		 * \return
		 */
		double getValue() const noexcept;

		/**
		 * .
		 * 
		 * \param relativePos
		 * \return 
		 */
		double setCursor(float relativePosY = 0.0f) noexcept;

		void draw() const noexcept;


		//TODO: add a function to resize when event is triggered.

	private:

		mutable sf::RenderWindow* m_window;

		sf::Texture m_textureBackgroundSlider; // The texture of the background.
		sf::Texture m_textureCursorSlider; // The texture of the slider

		std::unique_ptr<sf::Sprite> m_backgroundSlider; // The background of the slider.
		std::unique_ptr<sf::Sprite> m_cursorSlider; // The slider itself.

		std::unique_ptr<TextWrapper> m_currentValueText;

		int m_maxValue;
		int m_minValue;
		int m_intervalles;

	friend UserInteractableGraphicalInterface; //TODO: revoir friend.
	};

	class MultipleQuestionBox
	{
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
	};


	using Button = std::pair<size_t, std::function<void()>>; // The first is the element's index and the second is the text's index.
	using MQB = MultipleQuestionBox;
	//using DCM = DoubleCheckerMode;

	std::unordered_map<std::string, Button> m_buttons; // Collection of buttons in the interface.
	std::unordered_map<std::string, Slider> m_sliders; // Collection of sliders in the interface.
	//std::unordered_map<std::string, MQB> m_mqbs; // Collection of MQBs in the interface.

	IdentifierInteractableItem m_hoveredElement; // The type of the button that is currently hovered.
};


using FGInterface = FixedGraphicalInterface;
using DGInterface = DynamicGraphicalInterface;
using IGInterface = UserInteractableGraphicalInterface;
using GUI = IGInterface;

#endif // GUI_HPP