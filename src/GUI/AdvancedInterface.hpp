/*******************************************************************
 * \file   AdvancedInterface.hpp, AdvancedInterface.cpp
 * \brief  Declare a gui with a slider, and multiple question boxes.
 *
 * \author OmegaDIL.
 * \date   July 2025.
 *
 * \note These files depend on the SFML library.
 *********************************************************************/

#ifndef ADVANCEDINTERFACE_HPP
#define ADVANCEDINTERFACE_HPP

#include "InteractiveInterface.hpp"
#include <SFML/Graphics.hpp>
#include <unordered_map>
#include <string>
#include <functional>
#include <unordered_set>

namespace gui
{



class AdvancedInterface : public InteractiveInterface
{
public:

	struct ItemType
	{
		inline static const uint8_t slider{ 3 };
		inline static const uint8_t mqb{ 4 };
	};

	using UserFunction = std::function<void(AdvancedInterface*, float)>;
	using GrowthSliderFunction = std::function<float(float x)>;



	struct Slider
	{
	private:

		float curValue;

	public:

		inline auto getCurrentValue() const noexcept { return curValue; }

		short internalIntervals; // number of intervals, min and max excluded. If equal to -1, no intervals
		UserFunction userFunction; // The function to call when the slider value is changed (e.g. to update the text displaying the current value).
		GrowthSliderFunction growthSliderFunction; // The function to apply to the value of the slider when it is changed.
		
	friend class AdvancedInterface;
	};

	struct MultipleQuestionBoxes
	{
	private:

		unsigned short numberOfBoxes;
		unsigned short currentlyHovered;
		std::unordered_set<unsigned short> checked;

	public:

		bool multipleChoices;
		bool atLeastOne;

		inline auto getNumberOfBoxes() const noexcept { return numberOfBoxes; }
		inline auto getCurrentlyHovered() const noexcept { return currentlyHovered; }
		inline auto getChecked() const noexcept { return checked; }

	friend class AdvancedInterface;
	};

	using MQB = MultipleQuestionBoxes;

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
	explicit AdvancedInterface(sf::RenderWindow* window, unsigned int relativeScalingDefinition = 1080);

	AdvancedInterface() noexcept = delete;
	AdvancedInterface(AdvancedInterface const&) noexcept = delete;
	AdvancedInterface(AdvancedInterface&&) noexcept = default;
	AdvancedInterface& operator=(AdvancedInterface const&) noexcept = delete;
	AdvancedInterface& operator=(AdvancedInterface&&) noexcept = default;
	virtual ~AdvancedInterface() noexcept = default;


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
	void addSlider(const std::string& identifier, sf::Vector2f pos, unsigned int size = 200, short intervals = -1, UserFunction userFunction = nullptr, GrowthSliderFunction growthSliderFunction = nullptr, bool showValueWithText = true, sf::Vector2f scale = sf::Vector2f{ 1.f, 1.f }) noexcept;

	void removeSlider(const std::string& identifier) noexcept;

	[[nodiscard]] Slider* getSlider(const std::string& identifier) noexcept;

	void addMQB(std::string const& identifier, unsigned short numberOfBoxes, sf::Vector2f posInit, sf::Vector2f posDelta, bool multipleChoices = true, bool atLeastOne = false, sf::Vector2f scale = sf::Vector2f{ 1.f, 1.f }) noexcept; // TODO: Mettre en diff au lieu de pos2

	void removeMQB(const std::string& identifier) noexcept;

	[[nodiscard]] MQB* getMQB(const std::string& identifier) noexcept;


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
	static InteractiveItem updateHovered(BasicInterface* activeGUI, sf::Vector2i cursorPos) noexcept;

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
	static InteractiveItem pressed(BasicInterface* activeGUI) noexcept;

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
	static InteractiveItem unpressed(BasicInterface* activeGUI) noexcept;

private:

	float calculateValueOfSlider(const std::string& identifier) noexcept;

	void setCursorOfSlider(const std::string& identifier, unsigned int curPosY) noexcept;

	std::unordered_map< std::string, Slider> m_sliders;
	std::unordered_map< std::string, MQB> m_mqbs;


	static sf::Texture loadSolidRectange(sf::Vector2f scale, sf::Color fill = sf::Color{ 20, 20, 20 }, sf::Color outline = sf::Color{ 80, 80, 80 }, unsigned int thickness = 5) noexcept;
	static sf::Texture loadCheckBoxTexture(sf::Vector2f scale) noexcept;

	inline static const std::string mqbCheckedTextureName{ "__cb" };
	inline static const std::string mqbUncheckedTextureName{ "__ub" };
	inline static const std::string sliderBackgroundTextureName{ "__sb" };
	inline static const std::string sliderCursorTextureName{ "__sc" };
};

} // gui namespace

#endif //ADVANCEDINTERFACE_HPP