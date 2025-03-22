/*******************************************************************
 * @file   Interface.hpp
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
#include <stdexcept>
#include <array>
#include <algorithm>
#include <initializer_list>
#include <memory>
#include <functional>
#include <sstream>

extern sf::VideoMode windowSize;


//////////////////////////////////Interface classes//////////////////////////////////

/** @concept Ostreamable
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

/** @class Interface
 * @brief Manages a background for the window, creates a class for texts, and serves as a base class for other UI components.
 *
 * @note    The class uses the `windowSize` variable to adjust the proportions of its elements.
 * @warning Avoid deleting the `sf::RenderWindow` passed as an argument while this class is using it.
 *
 * @see sf::RenderWindow, InterfaceText.
 */
class Interface
{
public:

	/** @struct InterfaceText
	 * @brief A wrapper for `sf::Text` with additional custom properties.
	 *
	 * @note This class uses the `windowSize` variable to correctly adjust the proportions of the text.
	 * @note Call loadFont() once before creating any instance of this class.
	 *
	 * @see sf::Text, Interface
	 */
	struct InterfaceText : public sf::Text
	{
	public:

		InterfaceText() noexcept = default;
		InterfaceText(InterfaceText const&) noexcept = default;
		InterfaceText(InterfaceText&&) noexcept = default;
		virtual ~InterfaceText() noexcept = default;
		virtual InterfaceText& operator=(InterfaceText const&) noexcept = default;
		InterfaceText& operator=(InterfaceText&&) noexcept = default;

		/**
		 * @brief Initializes the text with the specified parameters, sets the color to white, and centers the origin.
		 * @complexity O(1).
		 *
		 * @param[in] string: The text string to set.
		 * @param[in] pos: The position to place the text.
		 * @param[in] characterSize: The size of the text characters.
		 *
		 * @see sf::Text, sf::Font, loadFont().
		 **/
		inline InterfaceText(std::string const& content, sf::Vector2f pos, unsigned int characterSize) noexcept
			: sf::Text{ *m_font, content, characterSize }
		{
			updatePos(pos);
		}

		/**
		 * @brief Loads the font used for the text.
		 * 
		 * @return An optional string containing an error message if the font could not be loaded.
		 * 
		 * @note The font is loaded only once for all instances of this class.
		 */
		static std::optional<std::string> loadFont() noexcept;


		/**
		 * @brief Updates the text content and repositions it accordingly.
		 * @complexity O(1).
		 *
		 * @param[in] content: The new content for the text.
		 * @tparam T: Type that can be streamed to `std::basic_ostream`.
		 */
		template<Ostreamable T>
		inline void updateContent(T const& content) noexcept
		{
			std::ostringstream oss{}; // Convert the content to a string
			oss << content; // Assigning the content to the variable.
			setString(oss.str());
			updatePos(getPosition()); // Updating position. If the new string is shorter/larger than the previous one, it was not centered.
		}

		/**
		 * @brief Refreshes the position and the size of the text after the window is resized
		 *
		 * @param[in] scalingFactor: The scaling factor between the previous size of the window and the new one.
		 */
		inline void resizingElements(sf::Vector2f scalingFactor) noexcept
		{
			updatePos(sf::Vector2f{ getPosition().x * scalingFactor.x, getPosition().y * scalingFactor.y });
		}

		/**
		 * @brief Accesses the base `sf::Text` object.
		 * @complexity O(1).
		 *
		 * @return A reference to the base `sf::Text` object.
		 */
		inline constexpr sf::Text const& getText() const noexcept
		{
			return *this;
		}

	private:

		/**
		 * @brief Updates the position and origin of the text. Useful when the text string changes.
		 * @complexity O(1).
		 *
		 * @param[in] pos: The new position for the text.
		 */
		inline void updatePos(sf::Vector2f pos) noexcept
		{
			sf::FloatRect const textBounds{ getLocalBounds() }; // Cache bounds to avoid multiple calls.
			setOrigin(sf::Vector2f{ textBounds.size.x / 2.f, textBounds.size.y / 2.f });

			float factor = std::min(windowSize.size.x, windowSize.size.y) / 720.f;
			setScale(sf::Vector2f{ factor, factor }); // The scaling factor must be the same.

			setPosition(pos); // Update the position according to the new origin.
		}

		static std::unique_ptr<sf::Font> m_font; // The font used for the text.
	};


	/**
	 * @brief Constructs the interface.
	 * @complexity O(1).
	 *
	 * @param[in,out] window:     The window where the interface elements will be rendered.
	 * @param[in] backgroundPath: The background path of the interface. If empty, a black-grey background is created.
	 *
	 * @note The path must be relative to the media folder.
	 *
	 * @pre  The window must NOT be nullptr.
	 * @pre  The path must be valid or empty (the default background is created if empty).
	 * @post The interface will draw its elements on the right window.
	 * @post The background will be correclty loaded and displayed
	 * @throw std::invalid_argument if window is nullptr. Basic exception guarantee: the object cannot be used but is safe to destroy without memory leak.
	 * @throw std::invalid_argument if the path is neither valid nor empty. Strong exception guarantee: the object can be safely used but will have the default background.
	 */
	explicit Interface(sf::RenderWindow* window, std::string backgroundPath = "");

	Interface() noexcept = delete;
	Interface(Interface const&) noexcept = default;
	Interface(Interface&&) noexcept = default;
	virtual ~Interface() noexcept = default;
	Interface& operator=(Interface const&) noexcept = default;
	Interface& operator=(Interface&&) noexcept = default;

	/**
	 * @brief Updates the size/positioin of all elements in every interfaces.
	 *
	 * @param[in] scalingFactor: The scaling factor between the previous size of the window and the new one.
	 */
	static void windowResized(sf::Vector2f scalingFactor) noexcept;

	/** @pure
	 * @brief Renders the interface.
	 * @complexity O(1).
	 *
	 * @note Although this function is pure virtual, it has an implementation for drawing the background.
	 *
	 * @thorw std::logic_error if window is nullptr.
	 */
	inline virtual void draw() const = 0
	{
		if (!m_window)
			throw std::logic_error{ "Window is nullptr.\nOccured inside the draw function of Interface.\n" };

		m_window->draw(m_spriteBackground);
	}

protected:

	mutable sf::RenderWindow* m_window; // Pointer to the window.

	/**
	 * @brief Refreshes the position and the size of the elements after the window is resized
	 * @complexity O(1).
	 *
	 * @param[in] scalingFactor: The scaling factor between the previous size of the window and the new one.
	 */
	virtual void resizingElements(sf::Vector2f scalingFactor) noexcept;

	static std::vector<Interface*> m_everyMenu;

private:

	/*
	 * @brief Creates a texture for the background and gives the correct size. Can be used to update a texture after resizing.
	 * @complexity O(1).
	 */
	void creatingBackgroundTexture() noexcept;

	sf::Sprite  m_spriteBackground;  // Sprite  for the background.
	sf::Texture m_textureBackground; // Texture for the background.
	sf::Texture m_actualTextureBackground; // It contains a potential texture loaded from a file.
};


/** @class MenuInterface
 * @brief Manages buttons and texts on top of the `Interface` class.
 *
 * @note    The class uses the `windowSize` variable to adjust the proportions of its elements.
 * @warning Avoid deleting the `sf::RenderWindow` passed as an argument while this class is using it.
 *
 * @see Interface, sf::RenderWindow.
 */
class MenuInterface : public Interface
{
public:

	/**
	 * @brief Constructs the menu interface.
	 * @complexity O(1).
	 *
	 * @param[in,out] window:     The window where the interface elements will be rendered.
	 * @param[in] backgroundPath: The background path of the interface. If empty, a black-grey background is created.
	 *
	 * @note The path must be relative to the media folder.
	 *
	 * @pre  The window must NOT be nullptr.
	 * @pre  The path must be valid or empty (the default background is created if empty).
	 * @post The interface will draw its elements on the right window.
	 * @post The background will be correclty loaded and displayed
	 * @throw std::invalid_argument if window is nullptr. Basic exception guarantee: the object cannot be used but is safe to destroy without memory leak.
	 * @throw std::invalid_argument if the path is neither valid nor empty. Strong exception guarantee: the object can be safely used but will have the default background.
	 */
	explicit MenuInterface(sf::RenderWindow* window, std::string backgroundPath = "");

	MenuInterface() noexcept = delete;
	MenuInterface(MenuInterface const&) noexcept = default;
	MenuInterface(MenuInterface&&) noexcept = default;
	virtual ~MenuInterface() noexcept = default;
	MenuInterface& operator=(MenuInterface const&) noexcept = default;
	MenuInterface& operator=(MenuInterface&&) noexcept = default;


	/**
	 * @brief Adds a text element to the interface.
	 * @complexity O(1).
	 *
	 * @param[in] content:       The text label.
	 * @param[in] position:      The position of the text.
	 * @param[in] characterSize: The size of the text characters.
	 * @tparam T: Type that can be streamed to `std::basic_ostream`.
	 *
	 * @see addButton
	 */
	template<Ostreamable T>
	inline void addText(T const& content, sf::Vector2f position, unsigned int characterSize) noexcept
	{
		m_texts.push_back(InterfaceText{ content, position, characterSize });
	}

	/**
	 * @brief Adds a button to the interface.
	 * @complexity O(1).
	 *
	 * @param[in] identifier:    The button identifier.
	 * @param[in] content:       The button label.
	 * @param[in] position:      The position of the button.
	 * @param[in] characterSize: The size of the button characters.
	 * @param[in] function:      The function to execute when this button is pressed
	 * @tparam T: Type that can be streamed to `std::basic_ostream`.
	 *
	 * @see	addText
	 */
	template<Ostreamable T>
	inline void addButton(size_t const identifier, T const& content, sf::Vector2f position, unsigned int characterSize, std::function<void()> function = []() {}) noexcept
	{
		m_buttons.emplace(identifier, std::pair{ InterfaceText{ content, position, characterSize }, function });
	}

	/**
	 * @brief Returns the content of the button pressed + executes its associated function.
	 * @complexity O(1).
	 *
	 * @return The button content that was pressed, or an empty string if no button was pressed.
	 *
	 * @note It assumes you already checked if the mouse was pressed.
	 */
	virtual inline std::string mousePressed() noexcept
	{
		if (m_pointingButton == 0)
			return "";

		m_buttons[m_pointingButton].second(); // Executing the function related to the button pressed
		return m_buttons[m_pointingButton].first.getText().getString();
	}

	/**
	 * @brief Detects if the mouse is hovering over a button to update its visual state.
	 * @complexity O(N), where N is the number of buttons.
	 *
	 * @note You should call this function when you change the menu to position the button's background accordingly.
	 */
	void mouseMoved();

	/**
	 * @brief Renders the menu interface.
	 * @complexity O(N), where N is the number of elements.
	 */
	virtual void draw() const override;

protected:

	/**
	 * @brief Refreshes the position and the size of the elements after the window is resized
	 * @complexity O(1).
	 *
	 * @param[in] scalingFactor: The scaling factor between the previous size of the window and the new one.
	 */
	virtual void resizingElements(sf::Vector2f scalingFactor) noexcept;

	// Collection of text elements in the interface.
	std::vector<InterfaceText> m_texts;
	// Collection of button elements in the interface. It maps them to the corresponding text elements.
	std::unordered_map<size_t, std::pair<InterfaceText, std::function<void()>>> m_buttons;

private:

	// Identifier of the button currently being hovered over by the mouse. 0 means there are no buttons. Shared among all instances.
	static size_t m_pointingButton;
	// The background sprite for the button being hovererd over by the mouse. Shared among all intsances.
	static sf::Sprite m_backgroundPointingButton;
};


/** @class DynamicMenuInterface
 * @brief  Manages an interface with dynamically changeable contents of elements (texts and buttons).
 *
 * @note	There is no "addDynamicButton" method because buttons are pre-defined and mapped with enum values in the `MenuInterface` class.
 * @note	Uses the `windowSize` variable to adjust element proportions.
 * @warning Do not delete the `sf::RenderWindow` passed as an argument while this class is in use.
 *
 * @see MenuInterface, sf::RenderWindow.
 */
class DynamicMenuInterface : public MenuInterface
{
public:

	/**
	 * @brief Constructs the dynamic menu interface.
	 * @complexity O(1).
	 *
	 * @param[in,out] window:     The window where the interface elements will be rendered.
	 * @param[in] backgroundPath: The background path of the interface. If empty, a black-grey background is created.
	 *
	 * @note The path must be relative to the media folder.
	 *
	 * @pre  The window must NOT be nullptr.
	 * @pre  The path must be valid or empty (the default background is created if empty).
	 * @post The interface will draw its elements on the right window.
	 * @post The background will be correclty loaded and displayed
	 * @throw std::invalid_argument if window is nullptr. Basic exception guarantee: the object cannot be used but is safe to destroy without memory leak.
	 * @throw std::invalid_argument if the path is neither valid nor empty. Strong exception guarantee: the object can be safely used but will have the default background.
	 */
	explicit DynamicMenuInterface(sf::RenderWindow* window, std::string backgroundPath = "") noexcept;

	DynamicMenuInterface() noexcept = delete;
	DynamicMenuInterface(DynamicMenuInterface const&) noexcept = default;
	DynamicMenuInterface(DynamicMenuInterface&&) noexcept = default;
	virtual ~DynamicMenuInterface() noexcept = default;
	DynamicMenuInterface& operator=(DynamicMenuInterface const&) noexcept = default;
	DynamicMenuInterface& operator=(DynamicMenuInterface&&) noexcept = default;


	/**
	 * @brief Adds a dynamic text to the interface.
	 *
	 * @param[in] identifier:     The text identifier.
	 * @param[in] defaultContent: The text label.
	 * @param[in] position:       The position of the text.
	 * @param[in] characterSize:  The size of the text characters.
	 * @tparam T: Type that can be streamed to `std::basic_ostream`.
	 *
	 * @see addText, addButton
	 */
	template<Ostreamable T>
	inline void addDynamicText(std::string const& identifier, T const& defaultContent, sf::Vector2f position, unsigned int characterSize) noexcept
	{
		m_dynamicTexts.emplace(identifier, InterfaceText{ defaultContent, position, characterSize });
	}

	/**
	 * @brief Updates the content of a button.
	 * @complexity O(1).
	 *
	 * @param[in] identifier:  The button's identifier whose content is being updated.
	 * @param[in] content:	   The new value for the button.
	 * @param[in] newFunction: True if we want to replace the old value of the funcion.
	 * @param[in] function:	   The function to execute when this button is pressed.
	 * @tparam T: Type that can be streamed to `std::basic_ostream`.
	 *
	 * @note If the button is not found in the interface, this function does nothing.
	 */
	template<Ostreamable T>
	void setNewValueButton(size_t const identifier, T const& content, bool newFunction = false, std::function<void()> function = []() {}) noexcept
	{
		auto const dynamicButtonElem{ m_buttons.find(identifier) };

		if (dynamicButtonElem != m_buttons.end())
		{
			dynamicButtonElem->second.first.updateContent(content);
			if (newFunction)
				dynamicButtonElem->second.second = function;
		}
	}

	/**
	 * @brief Updates the content of a dynamic text.
	 * @complexity O(1).
	 *
	 * @param[in] identifier: The dynamic text's identifier whose content is being updated.
	 * @param[in] content:    The new value for the dynamic text.
	 * @tparam T: Type that can be streamed to `std::basic_ostream`.
	 *
	 * @note If the dynamic text is not found in the interface, this function does nothing.
	 */
	template<Ostreamable T>
	void setNewValueText(std::string const& identifier, T const& content) noexcept
	{
		auto const dynamicTextElem{ m_dynamicTexts.find(identifier) };

		if (dynamicTextElem != m_dynamicTexts.end())
			dynamicTextElem->second.updateContent(content);
	}

	/**
	 * @brief Sets a background on the button affiliated with the current values of the dynamic elements.
	 * @complexity O(1).
	 *
	 * @param[in] identifier: The button's identifiers.
	 *
	 * @note If the button is not found in the interface, this function does nothing.
	 */
	void setAffiliatedButton(size_t identifier) noexcept;

	/**
	 * @complexity O(1).
	 *
	 * @return The content of the button related to the current values of the dynamic elements.
	 */
	inline std::string getAffiliatedButton() noexcept
	{
		return m_buttons[m_affiliatedIdButtonOfDynamicElements].first.getText().getString();
	}

	/**
	 * @brief Draws the interface.
	 * @complexity O(N), where N is the number of elements.
	 */
	virtual void draw() const override;

protected:

	/**
	 * @brief Refreshes the position and the size of the elements after the window is resized
	 * @complexity O(1).
	 *
	 * @param[in] scalingFactor: The scaling factor between the previous size of the window and the new one.
	 */
	virtual void resizingElements(sf::Vector2f scalingFactor) noexcept;

	// Collection of dynamic text elements in the interface.
	std::unordered_map<std::string, InterfaceText> m_dynamicTexts;

private:

	// Identifier of the last pressed button that changed the values of the dynamic elements. 0 means there are no buttons.
	size_t m_affiliatedIdButtonOfDynamicElements;
	// Background sprite for `m_affiliatedButton`. 
	sf::Sprite m_backgroundAffiliatedButton;
};


/** @class UserWritableMenuInterface
 * @brief  Manages an interface in which the user can write.
 *
 * @note	Uses the `windowSize` variable to adjust element proportions.
 * @warning Do not delete the `sf::RenderWindow` passed as an argument while this class is in use.
 *
 * @see DynamicMenuInterface, sf::RenderWindow.
 */
class UserWritableMenuInterface : public DynamicMenuInterface
{
public:

	/**
	 * @brief Constructs a dynamic menu interface in which the user is able to write to change the content of one text.
	 *
	 * @param[in,out] window:     The window where the interface elements will be rendered.
	 * @param[in] backgroundPath: The background path of the interface. If empty, a black-grey background is created.
	 *
	 * @note The path must be relative to the media folder.
	 *
	 * @pre  The window must NOT be nullptr.
	 * @pre  The path must be valid or empty (the default background is created if empty).
	 * @post The interface will draw its elements on the right window.
	 * @post The background will be correclty loaded and displayed
	 * @throw std::invalid_argument if window is nullptr. Basic exception guarantee: the object cannot be used but is safe to destroy without memory leak.
	 * @throw std::invalid_argument if the path is neither valid nor empty. Strong exception guarantee: the object can be safely used but will have the default background.
	 */
	explicit UserWritableMenuInterface(sf::RenderWindow* window, std::string backgroundPath = "") noexcept;

	UserWritableMenuInterface() noexcept = delete;
	UserWritableMenuInterface(UserWritableMenuInterface const&) noexcept = default;
	UserWritableMenuInterface(UserWritableMenuInterface&&) noexcept = default;
	virtual ~UserWritableMenuInterface() noexcept = default;
	UserWritableMenuInterface& operator=(UserWritableMenuInterface const&) noexcept = default;
	UserWritableMenuInterface& operator=(UserWritableMenuInterface&&) noexcept = default;


	/**
	 * @brief Adds a dynamic text to the interface.
	 *
	 * @param[in] defaultContent: The text label.
	 * @param[in] position:       The position of the text.
	 * @param[in] characterSize:  The size of the text characters.
	 * @tparam T: Type that can be streamed to `std::basic_ostream`.
	 *
	 * @see addText, addButton
	 */
	template<Ostreamable T>
	inline void addEditableText(T const& defaultContent, sf::Vector2f position, unsigned int characterSize, bool onlyNumbers = false) noexcept
	{
		m_editableTexts.push_back(std::pair{ InterfaceText{ defaultContent, position, characterSize }, onlyNumbers });
	}

	/**
	 * @brief Returns the content of the button pressed + executes its associated function.
	 * @complexity O(1).
	 *
	 * @return The button content that was pressed, or `None` if no button was pressed.
	 */
	virtual std::string mousePressed() noexcept final;

	/**
	 * @brief Changes the content of the dynamicText.
	 * @complexity O(1).
	 *
	 * @param[in] character: The unicode character of the input to insert.
	 *
	 * @return Returns the input of the text only if char is a return character. Otherwise, an empty string.
	 *
	 * @note Depending on the font you load, all characters might not be dsiplayed well.
	 */
	std::string userTyping(int character) noexcept;

	/**
	 * @brief Draws the interface.
	 * @complexity O(N), where N is the number of elements.
	 */
	void draw() const noexcept override;

private:

	/**
	 * @brief Refreshes the position and the size of the elements after the window is resized
	 * @complexity O(1).
	 *
	 * @param[in] scalingFactor: The scaling factor between the previous size of the window and the new one.
	 */
	virtual void resizingElements(sf::Vector2f scalingFactor) noexcept;

	// The texts that the user can modify. The bool attibute determines if only numbers can be written. 
	std::vector<std::pair<InterfaceText, bool>> m_editableTexts;

	// The current text to change.
	InterfaceText* m_currentText;
	// In case the new value is not valid, we reinstaure the previous one.
	std::string m_previousValue;
	// True if itg must have only letters.
	bool m_currentTextOnlyNumbers;

	// Acts as a sprite to represent the cursor
	sf::RectangleShape m_cursor;
};


class GameInterface : public Interface
{
public:

	/**
	 * @brief Constructs the interface.
	 * @complexity O(1).
	 *
	 * @param[in,out] window:     The window where the interface elements will be rendered.
	 * @param[in] rules: The rules that the game'll use.
	 * @param[in] backgroundPath: The background path of the interface. If empty, a black-grey background is created.
	 *
	 * @note The path must be relative to the media folder.
	 *
	 * @pre  The window must NOT be nullptr.
	 * @pre  The path must be valid or empty (the default background is created if empty).
	 * @post The interface will draw its elements on the right window.
	 * @post The background will be correclty loaded and displayed
	 * @throw std::invalid_argument if window is nullptr. Basic exception guarantee: the object cannot be used but is safe to destroy without memory leak.
	 * @throw std::invalid_argument if the path is neither valid nor empty. Strong exception guarantee: the object can be safely used but will have the default background.
	 */
	explicit GameInterface(sf::RenderWindow* window, RulesGame const* const rules, std::string backgroundPath = "");

	GameInterface() noexcept = delete;
	GameInterface(GameInterface const&) noexcept = default;
	GameInterface(GameInterface&&) noexcept = default;
	virtual ~GameInterface() noexcept = default;
	GameInterface& operator=(GameInterface const&) noexcept = default;
	GameInterface& operator=(GameInterface&&) noexcept = default;


	/**
	 * @brief When the game is starting, call this function to prepare the interface.
	 * @complexity O(1).
	 *
	 * @param[in] score: The score that'll be updated throughout the game.
	 */
	inline void starting(size_t* const score) noexcept
	{
		m_score = score;

		m_scoreText.updateContent("");
		m_chronoText.updateContent("");

		m_chronoTimer.restart();
		m_chronoPoints.restart();
	}

	/**
	 * @brief Refreshes the position and the size of the elements after the window is resized
	 * @complexity O(1).
	 *
	 * @param[in] scalingFactor: The scaling factor between the previous size of the window and the new one.
	 */
	virtual void resizingElements(sf::Vector2f scalingFactor) noexcept;

	/**
	 * @brief Draws the interface and adds the points related to elapsed time.
	 * @complexity O(1).
	 */
	virtual void draw() const noexcept final;

private:

	/**
	 * @brief Creates the texture for power-ups and hearts
	 * @complexity O(1).
	 */
	void createTexture() noexcept;


	InterfaceText m_scoreText; // The text that displays the score gained.
	mutable size_t* m_score; // Used to add points as time passes.
	RulesGame const* m_currentRules; // The rules of the game

	mutable InterfaceText m_chronoText; // The text that displays the time elapsed.
	sf::Clock m_chronoTimer; // Tha actual timer since the beginning.
	mutable sf::Clock m_chronoPoints; // The timer to know when we add points to the player.

	std::vector<sf::Sprite> m_hearts; // The sprites that represent the heart.
	std::vector<sf::Sprite> m_power_ups; // The sprites that represent the power ups.
	std::unordered_map<std::string, std::shared_ptr<sf::Texture>> m_textures; // All textures.
};

//////////////////////////////////Interface classes//////////////////////////////////


//////////////////////////////////Helper functions//////////////////////////////////

/**
 * @brief Creates a gradient texture
 * @complexity O(N), where N is the size.
 *
 * @param[in] size:  The size of the texture.
 * @param[in] red:   The red color of the texture.
 * @param[in] green: The green color of the texture.
 * @param[in] blue:  The blue color of the texture.
 *
 * @return The texture.
 *
 * @note Give the size as if the texture is used in a 720*720 window. Will be adjusted inside this function.
 */
std::unique_ptr<sf::Texture> createGradientTexture(sf::Vector2u size, sf::Uint8 red, sf::Uint8 green, sf::Uint8 blue) noexcept;

/**
 * @brief Initialise a sprite with a texture and set the origin at the center.
 *
 * @param[out] sprite:   The sprite to set.
 * @param[in]  texture:  Its texture.
 * @param[in]  position: Its position.
  */
inline void initializeSprite(sf::Sprite& sprite, sf::Texture const& texture, sf::Vector2f position = sf::Vector2f{ 0, 0 }) noexcept
{
	sprite.setTexture(texture);
	sf::FloatRect const textureOrigin{ sprite.getGlobalBounds() }; // Cache the bounds to avoid multiple calls.
	sprite.setOrigin(textureOrigin.width / 2.f, textureOrigin.height / 2.f);
	sprite.setPosition(position);
}

//////////////////////////////////Helper functions//////////////////////////////////


//////////////////////////////////Factory & managing functions//////////////////////////////////

/**
 * @brief   Creates some texts/buttons for a main interface.
 * @details The texts are the title and the credits. The buttons are play, score, and close. The window will close if close is pressed.
 * @complexity constant O(1).
 *
 * @param[out] menu: The interface in which the texts/buttons will be created.
 */ 
void createElemForMainInterface(MenuInterface& menu, sf::RenderWindow& window) noexcept;

/**
 * @brief   Creates some texts/buttons for a score interface.
 * @details The buttons are all the difficulties, a button back, one to set the difficulty, and one to
 *			see the rules of the difficulty. The other elements are 10 dynamic text to store the best
 *			scores, a text that stores the average,	and one to see the internat current difficulty of
 *			the game.
 * @complexity constant O(1).
 *
 * @param[out] menu: The interface in which the texts/buttons will be created.
 */
void createElemForScoreInterface(DynamicMenuInterface& menu, std::span<size_t> const& easyScore, std::span<size_t> const& mediumScore, std::span<size_t> const& hardScore, std::span<size_t> const& competitionScore, std::span<size_t> const& custom1Score, std::span<size_t> const& custom2Score, std::span<size_t> const& custom3Score) noexcept;

/**
 * @brief   Manages the texts and buttons of the score interface for a selected difficulty.
 * @details Displays the top 10 scores for the selected difficulty, updates the background
 *          button to indicate the current difficulty, and modifies the rules button to either
 *			display or customize the rules.
 *
 * @param[out] menu:       The interface to manage.
 * @param[in]  difficulty: The button corresponding to the selected difficulty.
 * @param[in]  scores:     All the scores, from which the relevant ones will be displayed on the score interface.
 *
 * @pre  The button must be associated with a difficulty.
 * @post The see button will have the appropriate content.
 */
void managingScoreInterfaceButtonPressed(DynamicMenuInterface& menu, size_t difficulty, std::span<size_t const> const score) noexcept;

/**
 * @brief   Creates some texts/buttons for a user writable interface.
 * @details TODO
 * @complexity constant O(1).
 *
 * @param[out] menu: The interface in which the texts/buttons will be created.
 */
void createElemForCustomDiffInterface(UserWritableMenuInterface& menu, std::span<size_t> const& easyScore, std::span<size_t> const& mediumScore, std::span<size_t> const& hardScore, std::span<size_t> const& competitionScore, std::span<size_t> const& custom1Score, std::span<size_t> const& custom2Score, std::span<size_t> const& custom3Score) noexcept;

//////////////////////////////////Factory & managing functions//////////////////////////////////

#endif // GUI_HPP