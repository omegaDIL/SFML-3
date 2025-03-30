/*******************************************************************
 * @file   GUI.hpp
 * @brief  Declare the entities for creating and managing graphical user interfaces.
 *
 * @author OmegaDIL.
 * @date   July 2024.
 *
 * @note This file depends on the SFML library.
 * @note Most classes use the `windowSize` variable to adjust the proportions of its elements.
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
#include <stdexcept>
#include <array>
#include <algorithm>
#include <initializer_list>
#include <memory>
#include <functional>
#include <sstream>

extern sf::VideoMode windowSize;


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
 * @details All elements are fixed and can't be changed (except the background).
 *
 * @warning Avoid deleting the `sf::RenderWindow` passed as an argument while this class is using it.
 *
 * @see sf::RenderWindow, Interface::Text.
 */
class GraphicalFixedInterface
{
public:

	/**
	 * @brief A wrapper for `sf::Text` that initializes the text, keeps the font, manages resizing.
	 *
	 * @note The font is common to all instances of this class.
	 * @note Call create() to initialize properly the object. Don't call other members before it has
	 *		 been successfully executed (as it is useless).
	 *
	 * @see GraphicalFixedInterface, sf::Text, create().
	 */
	class Text
	{
	public:

		Text(Text const&) noexcept = default;
		Text(Text&&) noexcept = default;
		virtual ~Text() noexcept = default;
		virtual Text& operator=(Text const&) noexcept = default;
		Text& operator=(Text&&) noexcept = default;

		/** 
		 * @brief Default constructor.
		 * 
		 * @note Doesn't actually create the text object, it just prevents throwing exceptions if the user
		 *       calls other functions before create().
		 * 
		 * @see create().
		 */
		inline Text() noexcept : m_text{ std::make_unique<sf::Text>(m_font, "") }
		{}

		/**
		 * @brief Initializes the text with the specified parameters.
		 * @complexity O(1).
		 *
		 * @param[in] string: The text string to set.
		 * @param[in] pos: The position to place the text.
		 * @param[in] characterSize: The size of the text characters.
		 * @param[in] color: The color of the text.
		 * 
		 * @return An optional string that contains an error message if the font could not be loaded.
		 * 
		 * @note Because the font is common to all instances, it is loaded only once when the first object
		 * 		 is instantiated. Therefore, no error will occur after the first object.
		 * @note Call other members only after this function has been successfully executed (otherwise there is no effect).
		 **/
		[[nodiscard]] std::optional<std::string> create(std::string const& content, sf::Vector2f pos, unsigned int characterSize, sf::Color color = sf::Color{ 255, 255, 255 }) noexcept;


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
		void updateContent(T const& content) noexcept
		{
			std::ostringstream oss{}; // Convert the content to a string
			oss << content; // Assigning the content to the variable.

			m_text->setString(oss.str());
			updateTransformables(m_text->getPosition()); // recenter the text.
		}

		/**
		 * @brief Updates the position of the text.
		 * @complexity O(1).
		 * 
		 * @param[in] pos: The new position for the text.
		 * 
		 * @see updateContent(), updateColor().
		 */
		inline void updatePosition(sf::Vector2f pos) noexcept
		{
			m_text->setPosition(pos);
		}	

		/**
		 * @brief Updates the color of the text.
		 * @complexity O(1).
		 *
		 * @param[in] color: The new color for the text.
		 *
		 * @see updateContent(), updatePosition().
		 */
		inline void updateColor(sf::Color color) noexcept
		{
			m_text->setFillColor(color);
		}

		/**
		 * @brief Accesses the base `sf::Text` object.
		 * @complexity O(1).
		 *
		 * @return A reference to the base `sf::Text` object.
		 */
		inline sf::Text const& getText() const noexcept
		{
			return *m_text;
		}

	private:

		/**
		 * @brief Updates the position, the scale and the origin of the text.
		 * @details Is useful when the text string changes and is shorter or larger than the previous one.
		 * @complexity O(1).
		 *
		 * @param[in] pos: The new position for the text.
		 * 
		 * @note If the window is resized, the position passed should be the product of the current position
		 * 		 by the scaling factor between the previous window's size and the new one.
		 */
		void updateTransformables(sf::Vector2f pos) noexcept;

		/**
		 * @brief Loads the font for texts
		 *
		 * @return a string that contains an error message if the font could not be loaded.
		 */
		std::optional<std::string> loadFont() noexcept;
		

		std::unique_ptr<sf::Text> m_text; // The text object.

		static sf::Font m_font; // The font used for the texts.

	friend class GraphicalFixedInterface;
	};


	GraphicalFixedInterface() noexcept = delete;
	GraphicalFixedInterface(GraphicalFixedInterface const&) noexcept = default;
	GraphicalFixedInterface(GraphicalFixedInterface&&) noexcept = default;
	virtual ~GraphicalFixedInterface() noexcept = default;
	GraphicalFixedInterface& operator=(GraphicalFixedInterface const&) noexcept = default;
	GraphicalFixedInterface& operator=(GraphicalFixedInterface&&) noexcept = default;

	/**
	 * @brief Constructs The interface with a default background.
	 * @complexity O(1).
	 * 
	 * @param[out] window: The window where the interface elements will be rendered. 
	 * 
	 * @see createBackground().
	 */
	inline explicit GraphicalFixedInterface(sf::RenderWindow* window) noexcept
		: m_window{ window }, m_spriteBackground{ nullptr }, m_textureBackground{}, m_texts{}, m_sprites{}
	{
		allInterfaces.push_back(this); // Add this interface to the collection.

		loadDefaultBackground();
	}


	/**
	 * @brief Loads a background from a file.
	 * @complexity O(1).
	 * 
	 * @param[in] fileName: The relative path to the background image.
	 * 
	 * @return The error message if the background could not be loaded.
	 *
	 */
	std::optional<std::string> createBackground(std::string const& fileName);

	/**
	 * @brief Adds a text element to the interface.
	 * @complexity O(1).
	 *
	 * @param[in] content: The text label.
	 * @param[in] position: The position of the text.
	 * @param[in] characterSize: The size of the text characters.
	 * @param[in] color: The color of the text.
	 * 
	 * @throw LoadingUIRessourceFailure if the font was not load yet and could not be loaded.
	 * 		  Strong exception guarantee.
	 * 
	 * @see addSprite(), addShape().
	 */
	void addText(std::string const& content, sf::Vector2f position, unsigned int characterSize, sf::Color color);

	/**
	 * @brief Adds a text element to the interface.
	 * @complexity O(1) : average case.
	 * @complexity O(N) : worst case, where N is the number of texts already added.
	 * 
	 * @param[in] text: The text to add.
	 * 
	 * @see addSprite(), addShape().
	 */
	void addText(GraphicalFixedInterface::Text text) noexcept;

	/**
	 * @brief Adds a sprite element to the interface.
	 * @complexity O(N) where N is the number of shapes + sprites already added.
	 *
	 * @param[in] sprite: The shape to add.
	 * @param[in] texture: The texture of the shape.
	 * 
	 * @see addText(), addShape().
	 */
	void addSprite(sf::Sprite sprite, sf::Texture texture) noexcept;

	/**
	 * @brief Adds a shape element to the interface.
	 * @complexity O(N) where N is the number of shapes + sprites already added.
	 *
	 * @param[in] shape: The shape to add.
	 * @param[in] smooth: True if the shape needs to be smoothed.
	 * 
	 * @note No effect if the shape is nullptr.
	 * @note The function creates a copy of the shape so you can destroy it
	 * @note The shape is still valid and unchanged.
	 * @note The shape is converted into a sf::Texture and a sf::Sprite by sf::RenderTexture, so please
	 * 		 keep in mind that it is a slow action.
	 * @note The transformables applied are kept and ARE USED AS DEFAULT VALUES.
	 * 
	 * @see addText(), addSprite().
	 */
	void addShape(sf::Shape* shape, bool smooth = true) noexcept;

	/** 
	 * @brief Renders the interface.
	 * @complexity O(N + K), where N is the number of texts, and K the number of shapes.
	 *
	 * @pre The window must have been valid when passed to the constructor and still exists.
	 * @post The elements are drawn on the window.
	 * @throw std::logic_error if window is nullptr.
	 */
	virtual void draw() const;


	/**
	 * @brief Refreshes the transformables after the window is resized.
	 *
	 * @param[in] scalingFactor: The scaling factor between the previous window's size and the new one.
	 * 
	 * @note Be aware of windowSize: it should have the new size.
	 */
	static void windowResized(sf::Vector2f scalingFactor) noexcept;


	// The relative path to the media folder used to load ressources.
	static std::string mediaPath;

protected:

	// Pointer to the window.
	mutable sf::RenderWindow* m_window; 

	// Collection of all interfaces to perform the same operation (resizing for example).
	static std::vector<GraphicalFixedInterface*> allInterfaces;

private:

	/**
	 * @brief Loads the default background.
	 */
	void loadDefaultBackground() noexcept;

	/**
	 * @brief Refreshes the position and the size of the elements after the window is resized
	 * @complexity O(N + K).
	 *
	 * @param[in] scalingFactor: The scaling factor between the previous size of the window and the new one.
	 */
	void resizeElements(sf::Vector2f scalingFactor) noexcept;


	std::unique_ptr<sf::Sprite> m_spriteBackground;  // Sprite  for the background.
	sf::Texture m_textureBackground; // Texture for the background.

	std::vector<Text> m_texts; // Collection of text elements in the interface.
	std::vector<std::pair<sf::Sprite, sf::Texture>> m_sprites; // Collection of sprites in the interface.
};

using GIText = GraphicalFixedInterface::Text;
using GInterface = GraphicalFixedInterface;


///** @class MenuInterface
// * @brief Manages buttons and texts on top of the `Interface` class.
// *
// * @note    The class uses the `windowSize` variable to adjust the proportions of its elements.
// * @warning Avoid deleting the `sf::RenderWindow` passed as an argument while this class is using it.
// *
// * @see Interface, sf::RenderWindow.
// */
//class MenuInterface : public Interface
//{
//public:
//
//	/**
//	 * @brief Constructs the menu interface.
//	 * @complexity O(1).
//	 *
//	 * @param[in,out] window:     The window where the interface elements will be rendered.
//	 * @param[in] backgroundPath: The background path of the interface. If empty, a black-grey background is created.
//	 *
//	 * @note The path must be relative to the media folder.
//	 *
//	 * @pre  The window must NOT be nullptr.
//	 * @pre  The path must be valid or empty (the default background is created if empty).
//	 * @post The interface will draw its elements on the right window.
//	 * @post The background will be correclty loaded and displayed
//	 * @throw std::invalid_argument if window is nullptr. Basic exception guarantee: the object cannot be used but is safe to destroy without memory leak.
//	 * @throw std::invalid_argument if the path is neither valid nor empty. Strong exception guarantee: the object can be safely used but will have the default background.
//	 */
//	explicit MenuInterface(sf::RenderWindow* window, std::string backgroundPath = "");
//
//	MenuInterface() noexcept = delete;
//	MenuInterface(MenuInterface const&) noexcept = default;
//	MenuInterface(MenuInterface&&) noexcept = default;
//	virtual ~MenuInterface() noexcept = default;
//	MenuInterface& operator=(MenuInterface const&) noexcept = default;
//	MenuInterface& operator=(MenuInterface&&) noexcept = default;
//
//
//	/**
//	 * @brief Adds a text element to the interface.
//	 * @complexity O(1).
//	 *
//	 * @param[in] content:       The text label.
//	 * @param[in] position:      The position of the text.
//	 * @param[in] characterSize: The size of the text characters.
//	 * @tparam T: Type that can be streamed to `std::basic_ostream`.
//	 *
//	 * @see addButton
//	 */
//	template<Ostreamable T>
//	inline void addText(T const& content, sf::Vector2f position, unsigned int characterSize) noexcept
//	{
//		m_texts.push_back(Text{ content, position, characterSize });
//	}
//
//	/**
//	 * @brief Adds a button to the interface.
//	 * @complexity O(1).
//	 *
//	 * @param[in] identifier:    The button identifier.
//	 * @param[in] content:       The button label.
//	 * @param[in] position:      The position of the button.
//	 * @param[in] characterSize: The size of the button characters.
//	 * @param[in] function:      The function to execute when this button is pressed
//	 * @tparam T: Type that can be streamed to `std::basic_ostream`.
//	 *
//	 * @see	addText
//	 */
//	template<Ostreamable T>
//	inline void addButton(size_t const identifier, T const& content, sf::Vector2f position, unsigned int characterSize, std::function<void()> function = []() {}) noexcept
//	{
//		//m_buttons.emplace(identifier, std::make_pair(Text{ content, position, characterSize }, function));
//	}
//
//	/**
//	 * @brief Returns the content of the button pressed + executes its associated function.
//	 * @complexity O(1).
//	 *
//	 * @return The button content that was pressed, or an empty string if no button was pressed.
//	 *
//	 * @note It assumes you already checked if the mouse was pressed.
//	 */
//	virtual inline std::string mousePressed() noexcept
//	{
//		if (m_pointingButton == 0)
//			return "";
//		return "";
//		//m_buttons[m_pointingButton].second(); // Executing the function related to the button pressed
//		//return m_buttons[m_pointingButton].first.getText().getString();
//	}
//
//	/**
//	 * @brief Detects if the mouse is hovering over a button to update its visual state.
//	 * @complexity O(N), where N is the number of buttons.
//	 *
//	 * @note You should call this function when you change the menu to position the button's background accordingly.
//	 */
//	void mouseMoved();
//
//	/**
//	 * @brief Renders the menu interface.
//	 * @complexity O(N), where N is the number of elements.
//	 */
//	virtual void draw() const override;
//
//protected:
//
//	/**
//	 * @brief Refreshes the position and the size of the elements after the window is resized
//	 * @complexity O(1).
//	 *
//	 * @param[in] scalingFactor: The scaling factor between the previous size of the window and the new one.
//	 */
//	virtual void resizingElements(sf::Vector2f scalingFactor) noexcept;
//
//	// Collection of text elements in the interface.
//	std::vector<Text> m_texts;
//	// Collection of button elements in the interface. It maps them to the corresponding text elements.
//	//std::unordered_map<size_t, std::pair<Text, std::function<void()>>> m_buttons;
//
//private:
//
//	// Identifier of the button currently being hovered over by the mouse. 0 means there are no buttons. Shared among all instances.
//	static size_t m_pointingButton;
//	// The background sprite for the button being hovererd over by the mouse. Shared among all intsances.
//	static std::unique_ptr<sf::Sprite> m_backgroundPointingButton;
//};
//
//
///** @class DynamicMenuInterface
// * @brief  Manages an interface with dynamically changeable contents of elements (texts and buttons).
// *
// * @note	There is no "addDynamicButton" method because buttons are pre-defined and mapped with enum values in the `MenuInterface` class.
// * @note	Uses the `windowSize` variable to adjust element proportions.
// * @warning Do not delete the `sf::RenderWindow` passed as an argument while this class is in use.
// *
// * @see MenuInterface, sf::RenderWindow.
// */
//class DynamicMenuInterface : public MenuInterface
//{
//public:
//
//	/**
//	 * @brief Constructs the dynamic menu interface.
//	 * @complexity O(1).
//	 *
//	 * @param[in,out] window:     The window where the interface elements will be rendered.
//	 * @param[in] backgroundPath: The background path of the interface. If empty, a black-grey background is created.
//	 *
//	 * @note The path must be relative to the media folder.
//	 *
//	 * @pre  The window must NOT be nullptr.
//	 * @pre  The path must be valid or empty (the default background is created if empty).
//	 * @post The interface will draw its elements on the right window.
//	 * @post The background will be correclty loaded and displayed
//	 * @throw std::invalid_argument if window is nullptr. Basic exception guarantee: the object cannot be used but is safe to destroy without memory leak.
//	 * @throw std::invalid_argument if the path is neither valid nor empty. Strong exception guarantee: the object can be safely used but will have the default background.
//	 */
//	explicit DynamicMenuInterface(sf::RenderWindow* window, std::string backgroundPath = "") noexcept;
//
//	DynamicMenuInterface() noexcept = delete;
//	DynamicMenuInterface(DynamicMenuInterface const&) noexcept = default;
//	DynamicMenuInterface(DynamicMenuInterface&&) noexcept = default;
//	virtual ~DynamicMenuInterface() noexcept = default;
//	DynamicMenuInterface& operator=(DynamicMenuInterface const&) noexcept = default;
//	DynamicMenuInterface& operator=(DynamicMenuInterface&&) noexcept = default;
//
//
//	/**
//	 * @brief Adds a dynamic text to the interface.
//	 *
//	 * @param[in] identifier:     The text identifier.
//	 * @param[in] defaultContent: The text label.
//	 * @param[in] position:       The position of the text.
//	 * @param[in] characterSize:  The size of the text characters.
//	 * @tparam T: Type that can be streamed to `std::basic_ostream`.
//	 *
//	 * @see addText, addButton
//	 */
//	template<Ostreamable T>
//	inline void addDynamicText(std::string const& identifier, T const& defaultContent, sf::Vector2f position, unsigned int characterSize) noexcept
//	{
//		m_dynamicTexts.emplace(identifier, Text{ defaultContent, position, characterSize });
//	}
//
//	/**
//	 * @brief Updates the content of a button.
//	 * @complexity O(1).
//	 *
//	 * @param[in] identifier:  The button's identifier whose content is being updated.
//	 * @param[in] content:	   The new value for the button.
//	 * @param[in] newFunction: True if we want to replace the old value of the funcion.
//	 * @param[in] function:	   The function to execute when this button is pressed.
//	 * @tparam T: Type that can be streamed to `std::basic_ostream`.
//	 *
//	 * @note If the button is not found in the interface, this function does nothing.
//	 */
//	template<Ostreamable T>
//	void setNewValueButton(size_t const identifier, T const& content, bool newFunction = false, std::function<void()> function = []() {}) noexcept
//	{
//		//auto const dynamicButtonElem{ m_buttons.find(identifier) };
//
//		//if (dynamicButtonElem == m_buttons.end())
//		//	return;
//		//
//		//dynamicButtonElem->second.first.updateContent(content);
//		//if (newFunction)
//		//	dynamicButtonElem->second.second = function;
//	}
//
//	/**
//	 * @brief Updates the content of a dynamic text.
//	 * @complexity O(1).
//	 *
//	 * @param[in] identifier: The dynamic text's identifier whose content is being updated.
//	 * @param[in] content:    The new value for the dynamic text.
//	 * @tparam T: Type that can be streamed to `std::basic_ostream`.
//	 *
//	 * @note If the dynamic text is not found in the interface, this function does nothing.
//	 */
//	template<Ostreamable T>
//	void setNewValueText(std::string const& identifier, T const& content) noexcept
//	{
//		auto const dynamicTextElem{ m_dynamicTexts.find(identifier) };
//
//		if (dynamicTextElem != m_dynamicTexts.end())
//			dynamicTextElem->second.updateContent(content);
//	}
//
//	/**
//	 * @brief Sets a background on the button affiliated with the current values of the dynamic elements.
//	 * @complexity O(1).
//	 *
//	 * @param[in] identifier: The button's identifiers.
//	 *
//	 * @note If the button is not found in the interface, this function does nothing.
//	 */
//	void setAffiliatedButton(size_t identifier) noexcept;
//
//	/**
//	 * @complexity O(1).
//	 *
//	 * @return The content of the button related to the current values of the dynamic elements.
//	 */
//	inline std::string getAffiliatedButton() noexcept
//	{
//		//return m_buttons[m_affiliatedIdButtonOfDynamicElements].first.getText().getString();
//		return "None";
//	}
//
//	/**
//	 * @brief Draws the interface.
//	 * @complexity O(N), where N is the number of elements.
//	 */
//	virtual void draw() const override;
//
//protected:
//
//	/**
//	 * @brief Refreshes the position and the size of the elements after the window is resized
//	 * @complexity O(1).
//	 *
//	 * @param[in] scalingFactor: The scaling factor between the previous size of the window and the new one.
//	 */
//	virtual void resizingElements(sf::Vector2f scalingFactor) noexcept;
//
//	// Collection of dynamic text elements in the interface.
//	std::unordered_map<std::string, Text> m_dynamicTexts;
//
//private:
//
//	// Identifier of the last pressed button that changed the values of the dynamic elements. 0 means there are no buttons.
//	size_t m_affiliatedIdButtonOfDynamicElements;
//	// Background sprite for `m_affiliatedButton`. 
//	std::unique_ptr<sf::Sprite> m_backgroundAffiliatedButton;
//};
//
//
///** @class UserWritableMenuInterface
// * @brief  Manages an interface in which the user can write.
// *
// * @note	Uses the `windowSize` variable to adjust element proportions.
// * @warning Do not delete the `sf::RenderWindow` passed as an argument while this class is in use.
// *
// * @see DynamicMenuInterface, sf::RenderWindow.
// */
//class UserWritableMenuInterface : public DynamicMenuInterface
//{
//public:
//
//	/**
//	 * @brief Constructs a dynamic menu interface in which the user is able to write to change the content of one text.
//	 *
//	 * @param[in,out] window:     The window where the interface elements will be rendered.
//	 * @param[in] backgroundPath: The background path of the interface. If empty, a black-grey background is created.
//	 *
//	 * @note The path must be relative to the media folder.
//	 *
//	 * @pre  The window must NOT be nullptr.
//	 * @pre  The path must be valid or empty (the default background is created if empty).
//	 * @post The interface will draw its elements on the right window.
//	 * @post The background will be correclty loaded and displayed
//	 * @throw std::invalid_argument if window is nullptr. Basic exception guarantee: the object cannot be used but is safe to destroy without memory leak.
//	 * @throw std::invalid_argument if the path is neither valid nor empty. Strong exception guarantee: the object can be safely used but will have the default background.
//	 */
//	explicit UserWritableMenuInterface(sf::RenderWindow* window, std::string backgroundPath = "") noexcept;
//
//	UserWritableMenuInterface() noexcept = delete;
//	UserWritableMenuInterface(UserWritableMenuInterface const&) noexcept = default;
//	UserWritableMenuInterface(UserWritableMenuInterface&&) noexcept = default;
//	virtual ~UserWritableMenuInterface() noexcept = default;
//	UserWritableMenuInterface& operator=(UserWritableMenuInterface const&) noexcept = default;
//	UserWritableMenuInterface& operator=(UserWritableMenuInterface&&) noexcept = default;
//
//
//	/**
//	 * @brief Adds a dynamic text to the interface.
//	 *
//	 * @param[in] defaultContent: The text label.
//	 * @param[in] position:       The position of the text.
//	 * @param[in] characterSize:  The size of the text characters.
//	 * @tparam T: Type that can be streamed to `std::basic_ostream`.
//	 *
//	 * @see addText, addButton
//	 */
//	template<Ostreamable T>
//	inline void addEditableText(T const& defaultContent, sf::Vector2f position, unsigned int characterSize, bool onlyNumbers = false) noexcept
//	{
//		m_editableTexts.push_back(std::pair{ Text{ defaultContent, position, characterSize }, onlyNumbers });
//	}
//
//	/**
//	 * @brief Returns the content of the button pressed + executes its associated function.
//	 * @complexity O(1).
//	 *
//	 * @return The button content that was pressed, or `None` if no button was pressed.
//	 */
//	virtual std::string mousePressed() noexcept final;
//
//	/**
//	 * @brief Changes the content of the dynamicText.
//	 * @complexity O(1).
//	 *
//	 * @param[in] character: The unicode character of the input to insert.
//	 *
//	 * @return Returns the input of the text only if char is a return character. Otherwise, an empty string.
//	 *
//	 * @note Depending on the font you load, all characters might not be dsiplayed well.
//	 */
//	std::string userTyping(int character) noexcept;
//
//	/**
//	 * @brief Draws the interface.
//	 * @complexity O(N), where N is the number of elements.
//	 */
//	void draw() const noexcept override;
//
//private:
//
//	/**
//	 * @brief Refreshes the position and the size of the elements after the window is resized
//	 * @complexity O(1).
//	 *
//	 * @param[in] scalingFactor: The scaling factor between the previous size of the window and the new one.
//	 */
//	virtual void resizingElements(sf::Vector2f scalingFactor) noexcept;
//
//	// The texts that the user can modify. The bool attibute determines if only numbers can be written. 
//	std::vector<std::pair<Text, bool>> m_editableTexts;
//
//	// The current text to change.
//	Text* m_currentText;
//	// In case the new value is not valid, we reinstaure the previous one.
//	std::string m_previousValue;
//	// True if itg must have only letters.
//	bool m_currentTextOnlyNumbers;
//
//	// Acts as a sprite to represent the cursor
//	sf::RectangleShape m_cursor;
//};

//TODO: Implement this class
//class GameInterface : public Interface
//{
//public:
//
//	/**
//	 * @brief Constructs the interface.
//	 * @complexity O(1).
//	 *
//	 * @param[in,out] window:     The window where the interface elements will be rendered.
//	 * @param[in] rules: The rules that the game'll use.
//	 * @param[in] backgroundPath: The background path of the interface. If empty, a black-grey background is created.
//	 *
//	 * @note The path must be relative to the media folder.
//	 *
//	 * @pre  The window must NOT be nullptr.
//	 * @pre  The path must be valid or empty (the default background is created if empty).
//	 * @post The interface will draw its elements on the right window.
//	 * @post The background will be correclty loaded and displayed
//	 * @throw std::invalid_argument if window is nullptr. Basic exception guarantee: the object cannot be used but is safe to destroy without memory leak.
//	 * @throw std::invalid_argument if the path is neither valid nor empty. Strong exception guarantee: the object can be safely used but will have the default background.
//	 */
//	explicit GameInterface(sf::RenderWindow* window, RulesGame const* const rules, std::string backgroundPath = "");
//
//	GameInterface() noexcept = delete;
//	GameInterface(GameInterface const&) noexcept = default;
//	GameInterface(GameInterface&&) noexcept = default;
//	virtual ~GameInterface() noexcept = default;
//	GameInterface& operator=(GameInterface const&) noexcept = default;
//	GameInterface& operator=(GameInterface&&) noexcept = default;
//
//
//	/**
//	 * @brief When the game is starting, call this function to prepare the interface.
//	 * @complexity O(1).
//	 *
//	 * @param[in] score: The score that'll be updated throughout the game.
//	 */
//	inline void starting(size_t* const score) noexcept
//	{
//		m_score = score;
//
//		m_scoreText.updateContent("");
//		m_chronoText.updateContent("");
//
//		m_chronoTimer.restart();
//		m_chronoPoints.restart();
//	}
//
//	/**
//	 * @brief Refreshes the position and the size of the elements after the window is resized
//	 * @complexity O(1).
//	 *
//	 * @param[in] scalingFactor: The scaling factor between the previous size of the window and the new one.
//	 */
//	virtual void resizingElements(sf::Vector2f scalingFactor) noexcept;
//
//	/**
//	 * @brief Draws the interface and adds the points related to elapsed time.
//	 * @complexity O(1).
//	 */
//	virtual void draw() const noexcept final;
//
//private:
//
//	/**
//	 * @brief Creates the texture for power-ups and hearts
//	 * @complexity O(1).
//	 */
//	void createTexture() noexcept;
//
//
//	Text m_scoreText; // The text that displays the score gained.
//	mutable size_t* m_score; // Used to add points as time passes.
//	RulesGame const* m_currentRules; // The rules of the game
//
//	mutable Text m_chronoText; // The text that displays the time elapsed.
//	sf::Clock m_chronoTimer; // Tha actual timer since the beginning.
//	mutable sf::Clock m_chronoPoints; // The timer to know when we add points to the player.
//
//	std::vector<sf::Sprite> m_hearts; // The sprites that represent the heart.
//	std::vector<sf::Sprite> m_power_ups; // The sprites that represent the power ups.
//	std::unordered_map<std::string, std::shared_ptr<sf::Texture>> m_textures; // All textures.
//};

//////////////////////////////////Interface classes//////////////////////////////////


//////////////////////////////////Helper functions//////////////////////////////////

///**
// * @brief Creates a gradient texture
// * @complexity O(N), where N is the size.
// *
// * @param[in] size:  The size of the texture.
// * @param[in] red:   The red color of the texture.
// * @param[in] green: The green color of the texture.
// * @param[in] blue:  The blue color of the texture.
// *
// * @return The texture.
// *
// * @note Give the size as if the texture is used in a 720*720 window. Will be adjusted inside this function.
// */
//std::unique_ptr<sf::Texture> createGradientTexture(sf::Vector2u size, uint8_t red, uint8_t green, uint8_t blue) noexcept;
//
///**
// * @brief Initialise a sprite with a texture and set the origin at the center.
// *
// * @param[out] sprite:   The sprite to set.
// * @param[in]  texture:  Its texture.
// * @param[in]  position: Its position.
//  */
//inline void initializeSprite(sf::Sprite& sprite, sf::Vector2f position = sf::Vector2f{ 0, 0 }) noexcept
//{
//	sf::FloatRect const textureOrigin{ sprite.getGlobalBounds() }; // Cache the bounds to avoid multiple calls.
//	sprite.setOrigin(sf::Vector2f{ textureOrigin.size.x / 2.f, textureOrigin.size.y / 2.f });
//	sprite.setPosition(position);
//}
//
////////////////////////////////////Helper functions//////////////////////////////////
//
//
////////////////////////////////////Factory & managing functions//////////////////////////////////
//
///**
// * @brief   Creates some texts/buttons for a main interface.
// * @details The texts are the title and the credits. The buttons are play, score, and close. The window will close if close is pressed.
// * @complexity constant O(1).
// *
// * @param[out] menu: The interface in which the texts/buttons will be created.
// */ 
//void createElemForMainInterface(MenuInterface& menu, sf::RenderWindow& window) noexcept;
//
///**
// * @brief   Creates some texts/buttons for a score interface.
// * @details The buttons are all the difficulties, a button back, one to set the difficulty, and one to
// *			see the rules of the difficulty. The other elements are 10 dynamic text to store the best
// *			scores, a text that stores the average,	and one to see the internat current difficulty of
// *			the game.
// * @complexity constant O(1).
// *
// * @param[out] menu: The interface in which the texts/buttons will be created.
// */
//void createElemForScoreInterface(DynamicMenuInterface& menu, std::span<size_t> const& easyScore, std::span<size_t> const& mediumScore, std::span<size_t> const& hardScore, std::span<size_t> const& competitionScore, std::span<size_t> const& custom1Score, std::span<size_t> const& custom2Score, std::span<size_t> const& custom3Score) noexcept;
//
///**
// * @brief   Manages the texts and buttons of the score interface for a selected difficulty.
// * @details Displays the top 10 scores for the selected difficulty, updates the background
// *          button to indicate the current difficulty, and modifies the rules button to either
// *			display or customize the rules.
// *
// * @param[out] menu:       The interface to manage.
// * @param[in]  difficulty: The button corresponding to the selected difficulty.
// * @param[in]  scores:     All the scores, from which the relevant ones will be displayed on the score interface.
// *
// * @pre  The button must be associated with a difficulty.
// * @post The see button will have the appropriate content.
// */
//void managingScoreInterfaceButtonPressed(DynamicMenuInterface& menu, size_t difficulty, std::span<size_t const> const score) noexcept;
//
///**
// * @brief   Creates some texts/buttons for a user writable interface.
// * @details TODO
// * @complexity constant O(1).
// *
// * @param[out] menu: The interface in which the texts/buttons will be created.
// */
//void createElemForCustomDiffInterface(UserWritableMenuInterface& menu, std::span<size_t> const& easyScore, std::span<size_t> const& mediumScore, std::span<size_t> const& hardScore, std::span<size_t> const& competitionScore, std::span<size_t> const& custom1Score, std::span<size_t> const& custom2Score, std::span<size_t> const& custom3Score) noexcept;

//////////////////////////////////Factory & managing functions//////////////////////////////////

#endif // GUI_HPP