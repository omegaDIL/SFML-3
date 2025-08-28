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
#include <unordered_set>
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



	class Slider
	{
	public:

		inline auto getCurrentValue() const noexcept { return m_curValue; }

	private:

		Slider(AdvancedInterface* agui, short internalIntervals = -1, UserFunction userFunction = nullptr, GrowthSliderFunction growthSliderFunction = nullptr) noexcept;
 
		void setCursor(float curPosY, SpriteWrapper& cursor, SpriteWrapper& background, TextWrapper* text) noexcept;

		AdvancedInterface* m_agui;

		float m_curValue;
		short m_internalIntervals; // number of intervals, min and max excluded. If equal to -1, no intervals

		UserFunction m_userFunction; // The function to call when the slider value is changed (e.g. to update the text displaying the current value).
		GrowthSliderFunction m_growthSliderFunction; // The function to apply to the value of the slider when it is changed.

	friend class AdvancedInterface;
	};

	class MultipleQuestionBoxes
	{
	public:

		
		[[nodiscard]] inline const std::unordered_set<unsigned short>& getChecked() const noexcept { return m_checked; };
		MultipleQuestionBoxes() noexcept = default;

	private:

		MultipleQuestionBoxes(unsigned short numberOfBoxes, bool multipleChoices, bool atLeastOne, unsigned short defaultCheckedBox) noexcept;
		MultipleQuestionBoxes(MultipleQuestionBoxes&&) noexcept = default;
		MultipleQuestionBoxes(MultipleQuestionBoxes const&) noexcept = default;
		MultipleQuestionBoxes& operator=(MultipleQuestionBoxes&&) noexcept = default;
		MultipleQuestionBoxes& operator=(MultipleQuestionBoxes const&) noexcept = default;

		void mqbPressed(unsigned short currentlyHovered) noexcept;

		void reverseCurrentHovered(unsigned short currentlyHovered) noexcept;

		unsigned short m_numberOfBoxes;
		bool m_multipleChoices;
		bool m_atLeastOne;

		std::unordered_set<unsigned short> m_checked;

	friend class AdvancedInterface;
	};

	//using MQB = MultipleQuestionBoxes;

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
	explicit AdvancedInterface(sf::RenderWindow* window, unsigned int relativeScalingDefinition = 1080) noexcept;

	AdvancedInterface() noexcept = delete;
	AdvancedInterface(AdvancedInterface const&) noexcept = delete;
	AdvancedInterface(AdvancedInterface&&) noexcept = default;
	AdvancedInterface& operator=(AdvancedInterface const&) noexcept = delete;
	AdvancedInterface& operator=(AdvancedInterface&&) noexcept = default;
	virtual ~AdvancedInterface() noexcept = default;


	void addSlider(std::string identifier, sf::Vector2f pos, unsigned int size = 300, short intervals = -1, UserFunction userFunction = nullptr, GrowthSliderFunction growthSliderFunction = nullptr, bool showValueWithText = true) noexcept;

	void removeSlider(const std::string& identifier) noexcept;

	[[nodiscard]] const Slider* const getSlider(const std::string& identifier) const noexcept;

	void addMQB(std::string identifier, sf::Vector2f posInit, sf::Vector2f posDelta, unsigned short numberOfBoxes, bool multipleChoices = true, bool atLeastOne = false, unsigned short defaultCheckedBox = 0) noexcept;

	void removeMQB(const std::string& identifier) noexcept;

	[[nodiscard]] MultipleQuestionBoxes* getMQB(const std::string& identifier) noexcept;


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
	static Item pressed(BasicInterface* activeGUI, sf::Vector2f cursorPos) noexcept;

private:

	std::unordered_map< std::string, Slider> m_sliders;
	std::unordered_map< std::string, MultipleQuestionBoxes> m_mqbs;

	static sf::Texture loadSolidRectange(sf::Vector2f scale, float outlineThickness) noexcept;
	static sf::Texture loadCheckBoxTexture(sf::Vector2f scale, float outlineThickness) noexcept;

	inline static const std::string sliderCursorPrefixeIdentifier{ "_sb_" };
	inline static const std::string sliderTextPrefixeIdentifier{ "_sb_" };

	inline static const std::string checkedMqbPrefixeIdentifier{ "_cb_" };
	inline static const std::string uncheckedMqbPrefixeIdentifier{ "_ub_" };
};

} // gui namespace

#endif //ADVANCEDINTERFACE_HPP