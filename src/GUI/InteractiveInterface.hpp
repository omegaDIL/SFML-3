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
#include <functional>
#include <unordered_map>
#include <string>
#include <cstdint>

namespace gui
{



/**
 * \brief Manages an interface in which the user can add texts/sprites buttons or write.
 * 
 * 
 * \note This class stores UI componenents; it will use a considerable amount of memory.
 * \warning Avoid deleting the `sf::RenderWindow` passed as an argument while this class is using it.
 *			The progam will assert otherwise. 
 * 
 * \see `MutableInterface`.
 *
 * \code
 * 
 * \endcode
 */
class InteractiveInterface : public MutableInterface
{
public:  

	using ButtonFunction = std::function<void(InteractiveInterface*, std::string&)>;
	using WritableFunction = std::function<void(InteractiveInterface*, char32_t&, std::string&)>;

	struct Button
	{
		enum class When : std::uint8_t
		{
			hovered,
			pressed,
			unpressed,
			none
		} when;

		ButtonFunction function;		
		
		Button(ButtonFunction func = nullptr, When wh = When::none) noexcept
			: function{ std::move(func) }, when{ wh } {}
	};

	struct Item
	{		
		Item(InteractiveInterface* ptr, std::string const& id, Button* button) noexcept
			: identfier{ id }, igui{ ptr }, m_button{ button } {}

		InteractiveInterface* igui;
		std::string identfier;

	private:
		Button* m_button;

	friend class InteractiveInterface;
	};


	/**
	 * \brief Constructs the interface.
	 * \complexity O(1).
	 *
	 * \param[in,out] window: The window where the interface elements will be rendered.
	 * \param[in] relativeScaling: The scales of the `sf::Transformable`s are absolute and will make an
	 *			  item appear at a certain size. However, in smaller windows, the same scale will make a
	 *			  `sf::Transformable` appear larger compared to the window. Therefore a factor will be multiplied
	 *			  to the scales to adjust them. This factor is equal to 1.f with windows that have
	 *			  `relativeScalingDefinition` (this parameter) as their smallest in x or y axis. For example,
	 *			  with 1080: if your window is currently set to 1080 in x or y axis, then all scales are multiply
	 *			  by 1.f. But if the window is suddenly set to 2160, then all scales are multiply by 2.
	 *
	 * \note A default font is loaded when the first instance is called.
	 * \warning The program asserts if the window is not valid (nullptr or size 0).
	 * \warning The program asserts if the relativeScaling is set to 0.
	 *
	 * \pre A font should be named defaultFont.ttf in the assets folder.
	 * \post The font is loaded.
	 * \throw LoadingGraphicalRessourceFailure Strong exception guarrantee, but no text can use the
	 *		  default font and you'll have to load and use your own font.
	 */
	explicit InteractiveInterface(sf::RenderWindow* window, unsigned int relativeScalingDefinition = 1080);

	InteractiveInterface() noexcept = delete;
	InteractiveInterface(InteractiveInterface const&) noexcept = delete;
	InteractiveInterface(InteractiveInterface&&) noexcept = default;
	InteractiveInterface& operator=(InteractiveInterface const&) noexcept = delete;
	InteractiveInterface& operator=(InteractiveInterface&&) noexcept = default;
	virtual ~InteractiveInterface() noexcept = default;


	/**
	 * \brief Removes a text from the GUI. No effet if not there.
	 * \complexity O(1).
	 *
	 * \param[in] name: The identifier of the text.
	 *
	 * \see `removeDynamicSprite`.
	 */
	virtual void removeDynamicText(const std::string& identifier) noexcept override;
	
	/**
	 * \brief Removes a sprite from the GUI. No effet if not there.
	 * \complexity O(1).
	 *
	 * \param[in] name: The identifier of the sprite.
	 *
	 * \see `removeDynamicText`.
	 */
	virtual void removeDynamicSprite(const std::string& identifier) noexcept override;

	/**
	 * \brief Adds a button to the interface by turning a transformables into a button. 
	 * It can also replace an existing button. Can be added to both a sprite and a text if the identifiers
	 * match. Will each be removed independently when their according transformables are deleted.
	 * \complexity O(1).
	 *
	 * \param[in] identifier: The id of the text you want to turn into a button
	 * \param[in] function: The function executed when the button is pressed.
	 * 
	 * \note Not very suitable for perfomance critical code, nor complex functions that require a lot
	 *		 of arguments. 
	 * 
	 *  \see `InteractiveFunction`.
	 */
	void addInteractive(const std::string& identifier, ButtonFunction function = nullptr, Button::When when = Button::When::unpressed) noexcept;

	/**
	 * \brief Returns a ptr to the button's function, or nullptr if it does not exist.
	 * \complexity O(1).
	 *
	 * \param[in] identifier: The identifier of the button.
	 *
	 * \return The function ptr.
	 * 
	 * \see `addButton`.
	 */
	[[nodiscard]] ButtonFunction* getInteractive(const std::string& identifier) noexcept;

	/**
	 * \brief Sets the text that will be edited once the user types a character.
	 * \complexity O(1).
	 * 
	 * \param[in] identifier: The dynamic text that will be the writable text.
	 * \param[in] function: A function that can modify the entry, and/or what was already set. The
	 *			  function takes the current interactive interface as a ptr, the new character (char32_t), and
	 *			  the whole string (std::string) before adding that character to that string. The string and
	 *			  the character are taken by reference (not const) so you are able to edit them. Once the
	 *			  function is called, the character is added.
	 *
	 * \see `WritableFunction`.
	 */
	void setWritingText(const std::string& identifier, WritableFunction function = nullptr) noexcept;

	inline void noWriting() noexcept
	{
		updateWritingText(exitWritingCharacter);
	}

	/**
	 * \brief Returns the text that is being written on.
	 * \complexity O(1).
	 *
	 * \return a pointer to that text, or nullptr.
	 */
	[[nodiscard]] inline TextWrapper* getWritingText() noexcept
	{
		return m_writingText;
	}


	/**
	 * \brief Updates the hovered element when the mouse mouve, if the gui is interactive.
	 * \details You might not want to call this function every time the mouse moved. You should
	 * 			only call it when you want to update the hovered element. For example you could want to keep
	 *			the same hovered element as long as the mouse is pressed.
	 * \complexity O(1), if the interactive element is the same as previously.
	 * \complexity O(N), otherwise; where N is the number of interactable elements in your active interface.
	 *
	 * \param[out] activeGUI: The GUI to update. No effect if not interactive
	 * \param[in]  cursorPos: The position of the mouse/cursor/touch event WITHIN the window.
	 *
	 * \return The gui address + id + type of the element that is currently hovered.
	 * 
	 * \warning Asserts if activeGUI is nullptr.
	 */
	static Item updateHovered(BasicInterface* activeGUI, sf::Vector2i cursorPos) noexcept;

	/**
	 * \brief Tells the active GUI that the cursor is pressed.
	 * \complexity O(1).
	 *
	 * \param[out] activeGUI: The GUI to update. No effect if not interactive
	 *
	 * \return The gui address + id + type of the element that is currently hovered.
	 *
	 * \warning Asserts if activeGUI is nullptr.
	 */
	static Item pressed(BasicInterface* activeGUI) noexcept;

	/**
	 * \brief Tells the active GUI that the cursor is released.
	 * \complexity O(1).
	 *
	 * \param[out] activeGUI: The GUI to update. No effect if not interactive
	 *
	 * \return The gui address + id + type of the element that is currently hovered.
	 * 
	 * \warning Asserts if activeGUI is nullptr.
	 */
	static Item unpressed(BasicInterface* activeGUI) noexcept;

	/**
	 * \brief Enters a character into the writing text. Can remove last character if backspace is entered,
	 *		  reset the writing text if the exit character is entered.
	 * \complexity O(1).
	 * 
	 * \param[out] activeGUI: The current interface that might be interactive.
	 * \param[in]  unicodeValue: The unicode value of the character entered.
	 * 
	 * \warning Asserts if activeGUI is nullptr.
	 */
	static void textEntered(BasicInterface* activeGUI, char32_t unicodeValue) noexcept;

	inline static char32_t exitWritingCharacter{ 0x001B }; // Set by default on escape character
	inline static std::string emptinessWritingCharacters{ "0" }; // Set by default on escape character

protected:
	
	inline static Item s_hoveredItem{ nullptr, "", nullptr };

private:
	
	/**
	 * \brief Enters a character into the writing text. Can remove last character if backspace is entered,
	 *		  reset the writing text if the exit character is entered.
	 * \complexity O(1).
	 *
	 * \param[in] character: The unicode value of the character entered.
	 */
	void updateWritingText(char32_t character) noexcept;


	size_t m_endTextInteractives;
	size_t m_endSpriteInteractives;

	std::unordered_map<size_t, Button> m_interactiveTexts; 
	std::unordered_map<size_t, Button> m_interactiveSprites; 

	TextWrapper* m_writingText;
	WritableFunction m_writingFunction;


	inline static const std::string writingCursorIdentifier{ "__wc" }; // Use constexpr
};

} // gui namespace

#endif //INTERACTIVEINTERFACE_HPP