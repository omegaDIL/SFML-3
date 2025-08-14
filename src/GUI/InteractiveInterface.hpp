/*******************************************************************
 * \file   InteractiveInterface.hpp, InteractiveInterface.cpp
 * \brief  Declare a gui in which the user can interact with, e.g. writing or pressing stuff.
 *
 * \author OmegaDIL.
 * \date   July 2025.
 *
 * \note These files depend on the SFML library.
 *********************************************************************/

#ifndef INTERACTIVEINTERFACE_HPP
#define INTERACTIVEINTERFACE_HPP

#include "MutableInterface.hpp"
#include <SFML/Graphics.hpp>
#include <string>
#include <string_view>
#include <functional>
#include <unordered_map>
#include <cstdint>	

namespace gui
{

/**
 * \brief  Manages an interface with changeable contents: texts and shapes.
 * 
 * Interactive elements are designed to respond to mouse hover events. When an element is hovered,
 * the `updateHovered` function returns a pointer to it. 
 * Buttons are a special type of interactive element with an attached function. This function can be
 * triggered when the button is hovered, pressed, or released.
 * For writing texts, you can access `exitWritingCharacter` which tells what character stops the 
 * writing (enter by default), or `emptinessWritingCharacters` which enters a string if the writing 
 * text is empty when the user exits it ("0" by default).
 *
 * A code example is provided at the end of the file.
 *
 * \note This class stores UI componenents; it will use a considerable amount of memory.
 * \note Each mutable elements might consume a little more memory than their fixed counterparts.
 * \warning Avoid deleting the `sf::RenderWindow` passed as an argument while this class is using it.
 *			The progam will assert otherwise. 
 * 
 * \see `BasicInterface`.
 */
class InteractiveInterface : public MutableInterface
{
public:  

	using ButtonFunction = std::function<void(InteractiveInterface*)>;
	using WritableFunction = std::function<void(InteractiveInterface*, char32_t, std::string&)>;

	/**
	 * \brief Represents an interactive that has a function attached to it.
	 */
	struct Button
	{
		ButtonFunction function; // What function to execute

		enum class When : std::uint8_t
		{
			hovered,
			pressed,
			unpressed,
			none
		} when; // When to execute the function

		Button(ButtonFunction func = nullptr, When wh = When::none) noexcept
			: function{ std::move(func) }, when{ wh } {}
	};

	/**
	 * \brief Represents the interactive currently hovered.
	 */
	struct Item
	{			
	private:

		Button* m_button; // The pointer to the interactive's button.

	public:

		InteractiveInterface* igui; // The gui that owns the interactive element.
		std::string identifier; // The item that is hovered, either a text or a sprite.
		
		enum class Type : std::uint8_t
		{
			Text,
			Sprite,
			None
		} type; // The type of the transformable; text or sprite.

		Item(InteractiveInterface* iguiPtr, std::string id, Type tp, Button* button) noexcept
			: igui{ iguiPtr }, identifier{ id }, type{ tp }, m_button{ button } {}

		Item() noexcept 
			: igui{ nullptr }, identifier{ "" }, type{ Type::None }, m_button{nullptr} {}

		Item(const Item& item) noexcept = default;
		Item(Item&& item) noexcept = default;
		Item& operator=(const Item& item) noexcept = default;
		Item& operator=(Item&& item) noexcept = default;
		~Item() noexcept = default;

	friend class InteractiveInterface;
	};


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
	explicit InteractiveInterface(sf::RenderWindow* window, unsigned int relativeScalingDefinition = 1080) noexcept;

	InteractiveInterface() noexcept = delete;
	InteractiveInterface(const InteractiveInterface&) noexcept = delete;
	InteractiveInterface(InteractiveInterface&&) noexcept = default;
	InteractiveInterface& operator=(const InteractiveInterface&) noexcept = delete;
	InteractiveInterface& operator=(InteractiveInterface&&) noexcept = default;
	virtual ~InteractiveInterface() noexcept;


	/**
	 * \brief Removes a text from the GUI. No effet if not there.
	 * \complexity O(1).
	 *
	 * \param[in] name: The identifier of the text.
	 *
	 * \see `removeDynamicSprite`.
	 */
	virtual void removeDynamicText(std::string_view identifier) noexcept override;
	
	/**
	 * \brief Removes a sprite from the GUI. No effet if not there.
	 * \complexity O(1).
	 *
	 * \param[in] name: The identifier of the sprite.
	 *
	 * \see `removeDynamicText`.
	 */
	virtual void removeDynamicSprite(std::string_view identifier) noexcept override;

	/**
	 * \brief Turns an existing transformable into an interactive element.
	 * \complexity O(1)
	 *
	 * If both a sprite and a text share the same identifier, the interactive status
	 * (and button behavior) is applied to both. They act as two independent buttons:
	 * deleting one does not affect the other.
	 *
	 * If both transformables are already interactive, nothing happens. However, if
	 * only one is interactive (e.g., because the other was added later), the status
	 * is still applied to the non-interactive one.
	 * 
	 * If neither a sprite or a text has the corresponding identifier, the function does nothing for
	 * fallback behavior.
	 *
	 * \param[in] identifier The ID of the text to turn into a button.
	 * \param[in] function   The function executed when the button is pressed.
	 * \param[in] when		 When to execute the lambda above.
	 *
	 * \note Creating a button is not recommended for performance-critical code or for complex functions
	 *		 requiring many arguments. Check for the return value of `updateHovered` instead.
	 * \note If either the function is set to nullptr, or when to none, the interactive will not be
	 *		 a button.
	 * 
	 * \warning May invalidate any pointers of any TransformableWrapper in this gui.
	 */
	void addInteractive(const std::string& identifier, ButtonFunction function = nullptr, Button::When when = Button::When::unpressed) noexcept;

	/**
	 * \brief Sets the dynamic text that will be edited when the user types a character.
	 * \complexity O(1).
	 *
	 * \param[in] identifier The identifier of the writable text to edit.
	 * \param[in] function   Optional callback invoked after a character is entered
	 *                       (not on deletion). The callback receives:
	 *                       - A pointer to the current interface
	 *                       - A reference to the full text
	 *                       - The entered character as a char32_t
	 *						 E.g. You can make a function that accepts numerical values only by using pop_back on the
	 *						 text reference if it doesn't satisfy the condition.
	 * 
	 * \note Does not suit well for texts that are rotated. Consider resetting it to 0 degrees.
	 *
	 * \see `WritableFunction`.
	 */
	void setWritingText(std::string_view identifier, WritableFunction function = nullptr) noexcept;

	/**
	 * \brief Returns the text that is being written on.
	 * \complexity O(1).
	 *
	 * \return a pointer to that text, or nullptr.
	 */
	[[nodiscard]] inline TextWrapper* getWritingText() noexcept
	{
		return getDynamicText(m_writingTextIdentifier);
	}


	/**
	 * \brief Updates the hovered element when the mouse mouve, if the gui is interactive.	 
	 * \complexity O(1), if the interactive element is the same as previously.
	 * \complexity O(N), otherwise; where N is the number of interactable elements in your active interface.
	 * 
	 * You may choose not to update the hovered element on every mouse move—for example,
	 * only when the mouse button is not pressed as well.
	 * If `when` is set to `hovered`, the element's associated function is executed
	 * each time the hovered element changes.
	 *
	 * \param[out] activeGUI: The current GUI. No effect if not interactive
	 * \param[in]  cursorPos: The position of the cursor/touch event WITHIN the window's view.
	 *
	 * \return The item that is currently hovered.
	 * 
	 * \warning Asserts if activeGUI is nullptr.
	 */
	static Item updateHovered(BasicInterface* activeGUI, sf::Vector2f cursorPos) noexcept;

	/**
	 * \brief Tells the active GUI that the cursor is pressed.
	 * \complexity O(1).
	 *
	 * \param[out] activeGUI: The current GUI. No effect if not interactive
	 *
	 * \return The item that is currently hovered.
	 *
	 * \note You do not need to call this function if no buttons are executed when pressed.
	 * \warning Asserts if activeGUI is nullptr.
	 */
	static Item pressed(BasicInterface* activeGUI) noexcept;

	/**
	 * \brief Tells the active GUI that the cursor is released.
	 * \complexity O(1).
	 * 
	 * \param[out] activeGUI: The current GUI. No effect if not interactive
	 *
	 * \return The item that is currently hovered.
	 *
	 * \note You do not need to call this function if no buttons are executed when unpressed.
	 * \warning Asserts if activeGUI is nullptr.
	 */
	static Item unpressed(BasicInterface* activeGUI) noexcept;

	/**
	 * \brief Enters a character into the writing text. Can remove last character if backspace is entered,
	 *		  reset the writing text if the exit character is entered.
	 * \complexity O(1).
	 * 
	 * \param[out] activeGUI: The current interface that might be interactive.
	 * \param[in]  character: The unicode value of the character entered.
	 * 
	 * \note You do not need to call this function if no writing is performed.
	 * \warning Asserts if activeGUI is nullptr.
	 */
	static void textEntered(BasicInterface* activeGUI, char32_t character) noexcept;

	inline static char32_t exitWritingCharacter{ 0x000D }; // Set by default on escape character
	inline static std::string emptinessWritingCharacters{ "0" }; // Set by default on escape character

protected:
	 
	inline static Item s_hoveredItem{}; // The current item that is hovered.

private:

	std::vector<Button> m_interactiveTextButtons; // Contains the buttons for texts
	std::vector<Button> m_interactiveSpriteButtons; // Contains the buttons for texts

	std::string m_writingTextIdentifier; // The identifier of the writing text. Empty otherwise.
	WritableFunction m_writingFunction; // The writing function

	inline static constexpr std::string_view writingCursorIdentifier{ "__wc" }; // The identifier of the writing cursor sprite
};

/**
 * \code
 * sf::Vector2u windowSize{ 1000, 1000 };
 * sf::RenderWindow window{ sf::VideoMode{ windowSize }, "Template sfml 3" };
 * IGUI mainInterface{ &window, 1080 };
 * IGUI otherInterface{ &window, 1080 };
 * BGUI* curInterface{ &mainInterface };
 *
 *
 * mainInterface.addText("Hi!!\nWelcome to my GUI", sf::Vector2f{ 200, 150 }, 48, sf::Color{255, 255, 255}, "__default", gui::Alignment::Left);
 *
 * mainInterface.addDynamicText("text1", "entry", { 500, 400 });
 * mainInterface.addInteractive("text1", [](IGUI* igui) {igui->setWritingText("text1"); });
 *
 * mainInterface.addDynamicText("text2", "entry", {500, 500});
 * mainInterface.addInteractive("text2", [](IGUI* igui) {igui->setWritingText("text2"); });
 *
 * mainInterface.addDynamicText("other", "switch", { 500, 800 });
 * mainInterface.addInteractive("other", [&otherInterface, &curInterface](IGUI*) mutable {curInterface = &otherInterface; });
 *
 *
 * sf::RectangleShape rect{ { 50, 50 } };
 * otherInterface.addDynamicSprite("colorChanger", gui::createTextureFromDrawables(rect), sf::Vector2f{500, 850});
 * otherInterface.addInteractive("colorChanger");
 *
 * otherInterface.addDynamicText("main", "switch", { 500, 500 });
 * otherInterface.addInteractive("main", [&mainInterface, &curInterface](IGUI*) mutable {curInterface = &mainInterface; });
 *
 *
 * IGUI::Item curItem{};
 * while (window.isOpen())
 * { 
 *     while (const std::optional event = window.pollEvent())
 *     { 
 *	       if (event->is<sf::Event::MouseMoved>() && !sf::Mouse::isButtonPressed(sf::Mouse::Button::Left))
 *		       curItem = IGUI::updateHovered(curInterface, window.mapPixelToCoords(event->getIf<sf::Event::MouseMoved>()->position));
 *
 *		   if (event->is<sf::Event::MouseButtonPressed>() && sf::Mouse::isButtonPressed(sf::Mouse::Button::Left))
 *			   IGUI::pressed(curInterface); // Is not necessary since no interactables have a pressed button.
 *
 *		   if (event->is<sf::Event::MouseButtonReleased>() && !sf::Mouse::isButtonPressed(sf::Mouse::Button::Left))
 *			   IGUI::unpressed(curInterface);
 *
 *		   if (event->is<sf::Event::TextEntered>())
 *			   IGUI::textEntered(curInterface, event->getIf<sf::Event::TextEntered>()->unicode);
 *
 *		   if (event->is<sf::Event::Resized>())
 *			   BGUI::windowResized(&window, windowSize);
 *
 *		   if (event->is<sf::Event::Closed>() || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape))
 *			   window.close();
 *	   }
 *
 *	   // If you ignore this feature, you could delete the variable curItem entirely.
 *	   // However, this is less limited since you can use more arguments, watch for more events...
 *	   // Moreover, this is better for perfomance-critical functions because storing a function in a
 *	   // `std::function` impacts the fps.
 *	   if (curItem.igui == &otherInterface && curItem.identifier == "colorChanger")
 *	       otherInterface.getDynamicSprite("colorChanger")->rotate(sf::degrees(1));
 *
 *	   window.clear();
 *	   curInterface->draw();
 *	   window.display();
 * }
 * \endcode
 */

} // gui namespace

#endif //INTERACTIVEINTERFACE_HPP