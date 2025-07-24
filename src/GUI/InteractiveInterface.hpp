/*******************************************************************
 * \file   InteractiveInterface.hpp, InteractiveInterface.cpp
 * \brief  Declare a gui in which the user can interact with, e.g. writing or pressing stuff.
 *
 * \author OmegaDIL.
 * \date   July 2025.
 *
 * \note These files depend on the SFML library.
 *********************************************************************/

#ifndef InteractiveInterface_HPP
#define InteractiveInterface_HPP

#include "MutableInterface.hpp"
#include <SFML/Graphics.hpp>
#include <functional>
#include <string>
#include <cstdint>

namespace gui
{

/**
 * \brief Manages an interface in which the user can add texts/sprites buttons or write.
 * 
 * 
 * \note Whereas `MutableInterface` adds new possibilities like modifing items, this class adds common and
 *		 pre-made features on top on `MutableInterface`. Everything added by this class could be rebuilt
 *		 from that base class.
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

	struct InteractiveType
	{
		inline static const uint8_t None{ 0 };
		inline static const uint8_t Button{ 1 };
	};

	struct InteractiveItem
	{
		InteractiveInterface* gui;
		uint8_t type;
		std::string* identfier;
	};

	using InteractiveFunction = std::function<void(InteractiveInterface*)>;
	using WritableFunction = std::function<void(InteractiveInterface*, char32_t&, std::string&)>;


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
	inline explicit InteractiveInterface(sf::RenderWindow* window, unsigned int relativeScalingDefinition = 1080)
		: MutableInterface{ window, m_relativeScalingDefinition }
	{}

	InteractiveInterface() noexcept = delete;
	InteractiveInterface(InteractiveInterface const&) noexcept = delete;
	InteractiveInterface(InteractiveInterface&&) noexcept = default;
	InteractiveInterface& operator=(InteractiveInterface const&) noexcept = delete;
	InteractiveInterface& operator=(InteractiveInterface&&) noexcept = default;
	virtual ~InteractiveInterface() noexcept = default;


	virtual void removeDynamicText(const std::string& identifier) noexcept override;
	
	virtual void removeDynamicSprite(const std::string& identifier) noexcept override;

	/**
	 * \brief Adds a button to the interface by turning a drawable into a button. Can also replace it.
	 * If no text nor sprite has that identifier, the button will nervertheless be added, at which point 
	 * it will have an effect and become active. If both a sprite and a text have the same identifier,
	 * the button is added to both of them, and will be deleted when the first of them is.
	 * \complexity O(1).
	 *
	 * \param[in] identifier: The id of the text you want to turn into a button
	 * \param[in] function: The function executed when the button is pressed.
	 * 
	 * \note Not very suitable for perfomance critical code, nor complex functions that require a lot
	 *		 of arguments. 
	 * \warning Asserts if the identifier is empty.
	 */
	void addButton(std::string const& identifier, InteractiveFunction function = [](InteractiveInterface*){}) noexcept;

	/**
	 * \brief Returns a ptr to the button's function, or nullptr if it does not exist.
	 * \complexity O(1).
	 *
	 * \param[in] identifier: The identifier of the button.
	 *
	 * \return The function ptr.
	 *
	 * \warning Asserts if the identifier is empty.
	 */
	[[nodiscard]] InteractiveFunction* const getButton(std::string const& identifier) noexcept;

	void setWritingText(const std::string& identifier, WritableFunction function = [](InteractiveInterface*, char32_t&, std::string&){}) noexcept;

	[[nodiscard]] TextWrapper* const getWritingText() const noexcept;

	[[nodiscard]] WritableFunction* const getWritingFunction() const noexcept;


	static void textEntered(BasicInterface* activeGUI, char32_t unicodeValue) noexcept;

	/**
	 * \brief Updates the hovered element when the mouse mouve if the gui is interactive.
	 * \details You might not want to call this function every time the mouse moved. You should
	 * 			only call it when you want to update the hovered element. For example you could want to keep
	 *			the same hovered element as long as the mouse is pressed.
	 * \complexity O(1), if the interactive element is the same as previously.
	 * \complexity O(N), where N is the number of interactable elements in your active interface.
	 *
	 * \param[out] activeGUI: The GUI tu update.
	 *
	 * \return The type + the id of the element that is currently hovered.
	 */
	static InteractiveItem updateHovered(BasicInterface* activeGUI) noexcept;

	/**
	 * \brief Tells the GUI that the mouse is pressed.
	 * \complexity O(1).
	 *
	 * \param[out] activeGUI: The GUI to update.
	 *
	 * \return The type + the id of the element that is currently hovered. Empty
	 *
	 * \note No effect if the active GUI is not an interactable GUI or nothing is hovered.
	 * \note You should not call this function when the mouse pressed EVENT is triggered. Instead, you
	 * 		 should call it as long as the mouse is pressed and if there's an event (the first frame it/
	 *		 is pressed/the mouse moved).
	 */
	static InteractiveItem mousePressed(BasicInterface* activeGUI) noexcept;

	/**
	 * \brief Tells the active GUI that the mouse is released.
	 * \complexity O(1).
	 *
	 * \param[out] activeGUI: The active GUI to update.
	 *
	 * \return The type + the id of the element that is currently hovered.
	 *
	 * \note No effect if the active GUI is not an interactable GUI or nothing is hovered.
	 * \note You should call this function when the mouse released event is triggered.
	 */
	static InteractiveItem mouseUnpressed(BasicInterface* activeGUI) noexcept;

protected:

	std::vector<TextWrapper*> m_frequentInteractiveTexts; // Collection of texts ptr that are usually searched.
	std::vector<SpriteWrapper*> m_frequentInteractiveSprites; // Collection of sprites ptr that are usually searched.


	static InteractiveItem s_hoveredItem;

private:

	std::unordered_map<std::string, InteractiveFunction> m_buttons; // Collection of buttons in the interface.

	TextWrapper* m_writingText;
	WritableFunction m_writingFunction;
};

} // gui namespace

#endif //InteractiveInterface_HPP
