/*******************************************************************
 * \file   GUI.hpp
 * \brief  Group all entities for creating and managing graphical user interfaces.
 *
 * \author OmegaDIL.
 * \date   July 2024.
 *
 * \note This file depends on the SFML library.
 *********************************************************************/

#ifndef GUI_HPP
#define GUI_HPP

#include "GUI/GraphicalRessources.hpp"
#include "GUI/BasicInterface.hpp"
#include "GUI/MutableInterface.hpp"
#include "GUI/InteractiveInterface.hpp"
#include "GUI/AdvancedInterface.hpp"

using BGUI = gui::BasicInterface;
using MGUI = gui::MutableInterface;
using IGUI = gui::InteractiveInterface; // Often enough for most apps



/** 
 * \brief Manages an interface in which the user can interact such as writing, pressing buttons...
 *
 * \note This class stores UI componenents; it will consume a considerable amount of memory.
 * \note This class does not add any new possibilities than MutableInterfaces, it only add
 * 		 prebuilt interactable elements such as buttons, sliders and multiple question boxes.
 * \warning Do not delete the `sf::RenderWindow` passed as an argument while this class is in use.
 *
 * \see MutableInterface.
 */
class UserInteractableGraphicalInterface : public gui::MutableInterface
{
private:

	struct Slider
	{

		Slider() noexcept = default; // No default constructor.
		Slider(Slider&&) noexcept = default; // No default constructor.
		Slider& operator=(Slider&&) noexcept = default; // No default constructor.

		inline Slider(int intervals, std::function<void(float)> const& userFunction, std::function<float(float)> const& mathFunction) noexcept
			: m_intervals{ intervals }, m_userFunction{ userFunction }, m_mathFunction{ mathFunction }
		{}

		int m_intervals;
		std::function<void(float)> m_userFunction; // The function to call when the slider value is changed (e.g. to update the text displaying the current value).
		std::function<float(float)> m_mathFunction; // The function to apply to the value of the slider when it is changed.
	};

	struct MultipleQuestionBoxes
	{
		//struct Box
		//{
		//	bool checked; // True if the box is checked, false otherwise.
		//	std::string name;
		//};

		MultipleQuestionBoxes() noexcept = default; // No default constructor.
		MultipleQuestionBoxes(MultipleQuestionBoxes&&) noexcept = default; // No default constructor.
		MultipleQuestionBoxes& operator=(MultipleQuestionBoxes&&) noexcept = default; // No default constructor.

		inline MultipleQuestionBoxes(bool multipleChoices, unsigned int numberOfBoxes, unsigned int checkedByDefault = 0) noexcept
			: m_multipleChoices{ multipleChoices }, m_numberOfBoxes{ numberOfBoxes }, m_boxes{}, m_currentlyHovered{ 0 }
		{
			m_boxes.resize(numberOfBoxes, false); // Initialize all boxes to unchecked.

			if (checkedByDefault-1 < numberOfBoxes && checkedByDefault-1 >= 0)
				m_boxes[checkedByDefault-1] = true; // Set the default checked box.
		}

		bool m_multipleChoices;
		unsigned int m_numberOfBoxes;

		std::vector<bool> m_boxes;
		unsigned int m_currentlyHovered; // 0 if no box is hovered, otherwise the index of the hovered box.
	};

	using MQB = MultipleQuestionBoxes;

public:

	enum class InteractableItem
	{
		None,
		Button,
		Slider,
		MQB
	};

	using IdentifierInteractableItem = std::pair<InteractableItem, std::string const*>; //TODO: *const*

	UserInteractableGraphicalInterface() noexcept = delete;
	UserInteractableGraphicalInterface(UserInteractableGraphicalInterface const&) noexcept = delete;
	UserInteractableGraphicalInterface(UserInteractableGraphicalInterface&&) noexcept = default;
	UserInteractableGraphicalInterface& operator=(UserInteractableGraphicalInterface const&) noexcept = delete;
	UserInteractableGraphicalInterface& operator=(UserInteractableGraphicalInterface&&) noexcept = default;
	virtual ~UserInteractableGraphicalInterface() noexcept = default;

	/**
	 * \brief Constructs the interface with a default background.
	 * \complexity O(1).
	 *
	 * \param[out] window: The window where the interface elements will be rendered.
	 *
	 * \see createBackground().
	 */
	inline explicit UserInteractableGraphicalInterface(sf::RenderWindow* window, std::string const& backgroundFileName = "") noexcept
		: MutableInterface{ window, backgroundFileName }, m_buttons{}, m_sliders{}
	{}

	/**
	 * \brief Adds a button to the interface by turning a text into a button.
	 * \complexity O(1).
	 *
	 * \param[in] id: The id of the text you want to turn into a button
	 * \param[in] function: The function executed when the button is pressed.
	 *
	 * \note No effect if no text with the id, or there's already a button with the same id.
	 * \note The button id is the same as the text id.
	 * \note You can still access the text turned with the function getDText(), and remove it with removeDText().
	 */
	bool addButton(std::string const& identifier, std::function<void()> function = [](){}) noexcept;

	bool addSlider(std::string const& identifier, sf::Vector2u size, sf::Vector2f pos, sf::Vector2f scale, std::function<float(float)> mathFunction = [](float x){return x;}, std::function<void(float)> changeFunction = [](float x){}, int interval = -1, bool showValueWithText = true) noexcept;

	bool addMQB(std::string const& identifier, sf::Vector2f posInit, sf::Vector2f posDelta, sf::Vector2f scale, unsigned int numberOfBoxes, bool multipleChoices = false, unsigned int defaultChecked = 0) noexcept; // TODO: Mettre en diff au lieu de pos2

	/**
	 * \param[in] identifier: The id of the button you want to check.
	 * \complexity O(1).
	 *
	 * \return True if it exists.
	 */
	[[nodiscard]] inline bool doesButtonExist(std::string const& identifier) const noexcept
	{
		return m_buttons.find(identifier) != m_buttons.end();
	}

	/**
	 * \param[in] identifier: The id of the slider you want to check.
	 * \complexity O(1).
	 *
	 * \return True if it exists.
	 */
	[[nodiscard]] inline bool doesSliderExist(std::string const& identifier) const noexcept
	{
		return m_sliders.find(identifier) != m_sliders.end();
	}

	/**
	 * \param[in] identifier: The id of the slider you want to check.
	 * \complexity O(1).
	 *
	 * \return True if it exists.
	 */
	[[nodiscard]] inline bool doesMQBExist(std::string const& identifier) const noexcept
	{
		return m_mqbs.find(identifier) != m_mqbs.end();
	}

	/**
	 * \brief Returns the function associated with the button.
	 * \complexity O(1).
	 * 
	 * \param[in] identifier: the id of the button you want to access.
	 * 
	 * \return A reference to the function lambda you want to access.
	 * 
	 * \pre Be sure that you added a button with this id.
	 * \post The appropriate lambda is returned.
	 * \throw std::out_of_range if id not there.
	 */
	[[nodiscard]] inline std::function<void()>& getFunctionOfButton(std::string const& identifier);

	/**
	 * \brief Returns the state of the boxes (checked: true, unchecked: false).
	 * \complexity O(1).
	 *
	 * \param[in] identifier: the id of the mqb you want to access.
	 *
	 * \return A vector of booleans representing the state of the boxes.
	 *
	 * \pre Be sure that you added a mqb with this id.
	 * \post The appropriate lambda is returned.
	 * \throw std::out_of_range if id not there.
	 */
	[[nodiscard]] inline std::vector<bool> const& getStateOfMQB(std::string const& identifier);


	/**
	 * \brief Returns the current value associated with the slider.
	 * \complexity O(1).
	 *
	 * \param[in] identifier: the id of the slider you want to access.
	 *
	 * \return The current value of the slider.
	 *
	 * \pre Be sure that you added a slider with this id.
	 * \post The appropriate value is returned.
	 * \throw std::out_of_range if id not there.
	 */
	[[nodiscard]] float getValueOfSlider(std::string const& identifier);

	/**
	 * \brief Returns the slider object associated with the slider.
	 * \details It allows you to access the slider's properties: its functions and intervals.
	 * \complexity O(1).
	 *
	 * \param[in] identifier: the id of the slider you want to access.
	 *
	 * \return The current value of the slider.
	 *
	 * \pre Be sure that you added a slider with this id.
	 * \post The appropriate value is returned.
	 * \throw std::out_of_range if id not there.
	 */
	[[nodiscard]] inline UserInteractableGraphicalInterface::Slider& getSlider(std::string const& identifier);

	/**
	 * \brief Removes a dynamic text, and the button associated (if it exists).
	 * \complexity O(N), where N is the number of texts (static + dynamic) (if vector resizes).
	 *
	 * \param[in] identifier: The id of the text you want to remove.
	 *
	 * \return True if removed.
	 *
	 * \note No effect if not there.
	 */
	virtual bool removeDText(std::string const& identifier) noexcept override;

	bool removeSlider(std::string const& identifier) noexcept;

	bool removeMQB(std::string const& identifier) noexcept;

	/**
	 * \brief Updates the hovered element.
	 * \details You (might not) want to call this function every time the mouse moved. You should
	 * 			rather call it when the mouse mouve AND you want to update the hovered element. It may be
	 *			each time the mouse move. However, if you have a slider and want to let the user update the
	 *			slider as long as he keeps the mouse pressed, then you should call this function while the
	 *			mouse is unpressed.
	 * \complexity O(N), where N is the number of interactable elements in your active interface.
	 *
	 * \param[out] activeGUI: The active GUI tu update.
	 *
	 * \return The type + the id of the element that is currently hovered.
	 *
	 * \note No effect if the active GUI is not an interactable GUI.
	 */
	static IdentifierInteractableItem mouseMoved(BasicInterface* activeGUI) noexcept;

	/**
	 * \brief Tells the active GUI that the mouse is pressed.
	 * \complexity O(1).
	 *
	 * \param[out] activeGUI: The active GUI to update.
	 *
	 * \return The type + the id of the element that is currently hovered.
	 *
	 * \note No effect if the active GUI is not an interactable GUI or nothing is hovered.
	 * \note You should not call this function when the mouse pressed EVENT is triggered. Instead, you
	 * 		 should call it as long as the mouse is pressed and if there's an event (the first frame it/
	 *		 is pressed/the mouse moved).
	 *
	 * \code
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
	 * \endcode
	 */
	static IdentifierInteractableItem mousePressed(BasicInterface* activeGUI) noexcept;

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
	static IdentifierInteractableItem mouseUnpressed(BasicInterface* activeGUI) noexcept;

	/**
	 * \complexity O(1).
	 *
	 * \return The type + the id of the element that is currently hovered.
	 */
	[[nodiscard]] static inline IdentifierInteractableItem getHoveredInteractableItem() noexcept
	{
		return m_hoveredElement;
	}

private:

	void changeValueSlider(std::string const& id, int mousePosY);


	std::unordered_map<std::string, std::function<void()>> m_buttons; // Collection of buttons in the interface.
	std::unordered_map<std::string, Slider> m_sliders; // Collection of sliders in the interface.
	std::unordered_map<std::string, MultipleQuestionBoxes> m_mqbs; // Collection of sliders in the interface.

	static UserInteractableGraphicalInterface* m_interfaceWithHoveredElement;
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

	explicit WritableGraphicalInterface(sf::RenderWindow* window, std::string const& backgroundFileName = "", std::function<void(char32_t&, std::string&)> function = [](char32_t&, std::string&) {}); // TODO: retirer non parametre des autres lambda

	void setCurrentlyEditedText(std::string const& identifier) noexcept;

	static std::string textEntered(BasicInterface* curInterface, char32_t unicodeValue) noexcept;

	[[nodiscard]] inline bool isCurrentlyEditing() const noexcept
	{
		return !m_currentlyEditedText.empty();
	}

	[[nodiscard]] inline sf::String const& getCurrentlyEditingText()
	{
		return getDText(m_currentlyEditedText).getText().getString(); // Throw std::out_of_range if not there.
	}

	[[nodiscard]] inline std::function<void(char32_t&, std::string&)>& getFunctionOfEditing() 
	{
		return m_textEnteredFunction;
	}

	inline virtual bool removeDText(std::string const& identifier) noexcept override
	{
		if (identifier == m_currentlyEditedText)
			m_currentlyEditedText.clear(); // Clear the currently edited text if it is removed.

		return UserInteractableGraphicalInterface::removeDText(identifier);
	}

private:

	void applyNewText(gui::TextWrapper* text, std::string& newText) noexcept;


	std::string m_currentlyEditedText;
	std::function<void(char32_t&, std::string&)> m_textEnteredFunction; // Pour chaque texte
};

using IGInterface = UserInteractableGraphicalInterface;
using WGInterface = WritableGraphicalInterface;
//TODO: mettre les using en haut

#endif // GUI_HPP