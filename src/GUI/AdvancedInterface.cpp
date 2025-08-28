#include "AdvancedInterface.hpp"

namespace gui
{

AdvancedInterface::Slider::Slider(AdvancedInterface* agui, short internalIntervals, UserFunction userFunction, GrowthSliderFunction growthSliderFunction) noexcept
	: m_agui{ agui }, m_internalIntervals{ internalIntervals }, m_userFunction{ userFunction }, m_growthSliderFunction{ growthSliderFunction }
{
	ENSURE_VALID_PTR(m_agui, "Precondition violated; the interface given for the slider was nullptr in the constructor of slider");

	static const std::function<float(float)> defaultGrowth{ [](float x) -> float { return x; } };
	if (m_growthSliderFunction == nullptr)
		m_growthSliderFunction =  defaultGrowth;

	m_curValue = m_growthSliderFunction(0.5f);
}

void AdvancedInterface::Slider::setCursor(float curPosY, SpriteWrapper& cursor, SpriteWrapper& background, TextWrapper* text) noexcept
{
	float minPos{ background.getSprite().getGlobalBounds().position.y };
	float maxPos{ minPos + background.getSprite().getGlobalBounds().size.y };
	float yPos{ static_cast<float>(curPosY) };

	if (yPos < minPos)
		yPos = minPos;
	else if (yPos > maxPos)
		yPos = maxPos;

	if (m_internalIntervals >= 0)
	{
		float interval = 1.f / (m_internalIntervals + 1); // The intervals exclude the min and max positions, so we add 1 to the number of intervals.
		float relativePos = (yPos - minPos) / (maxPos - minPos); // Between 0 and 1.
		float intervalPos = round(relativePos / interval) * interval; // Rounding to the nearest interval position.
		yPos = minPos + ((maxPos - minPos) * intervalPos); // Recalculating the position based on the intervals.
	}

	cursor.setPosition(sf::Vector2f{ cursor.getSprite().getPosition().x, yPos });
	m_curValue = m_growthSliderFunction(1 - (yPos - minPos) / (maxPos - minPos)); // Get the value of the slider.

	if (text != nullptr) // If the slider has a text associated with it.
	{
		text->setPosition(sf::Vector2f{ text->getText().getPosition().x, yPos }); // Aligning the text with the cursor.
		text->setContent(m_curValue);
	}

	if (m_userFunction != nullptr)
		m_userFunction(m_agui, m_curValue); // Call the function associated with the slider.
}

AdvancedInterface::MultipleQuestionBoxes::MultipleQuestionBoxes(unsigned short numberOfBoxes, bool multipleChoices, bool atLeastOne, unsigned short defaultCheckedBox) noexcept
	: m_numberOfBoxes{ numberOfBoxes }, m_multipleChoices{ multipleChoices }, m_atLeastOne{ atLeastOne }, m_checked{}
{
	ENSURE_NOT_ZERO(numberOfBoxes, "Precondition violated; the number of boxes is equal to 0 in the constructor of MQB");
	assert(numberOfBoxes != 1 || atLeastOne != true && "Precondition violated; the mqb is useless as it has only one box which can't be unchecked due to the variable atLeastOne being true in the constructor of MQB");
	assert(atLeastOne != true || defaultCheckedBox != 0 && "Preconditon violated; the mqb can't be completely unchecked and yet it has no default box checked in the constructor of MQB");
	assert(defaultCheckedBox <= numberOfBoxes && "Precondtion violated; the default checked box is greater than the number of total box");

	if (defaultCheckedBox != 0)
		m_checked.insert(defaultCheckedBox);
}

void AdvancedInterface::MultipleQuestionBoxes::mqbPressed(unsigned short currentlyHovered) noexcept   
{
	bool isChecked{ m_checked.find(currentlyHovered) != m_checked.end() };

	if (m_multipleChoices == false)
		m_checked.clear();

	if (!m_atLeastOne || !isChecked)
		reverseCurrentHovered(currentlyHovered);
}

void AdvancedInterface::MultipleQuestionBoxes::reverseCurrentHovered(unsigned short currentlyHovered) noexcept
{
	auto iterator{ m_checked.find(currentlyHovered) };

	if (iterator == m_checked.end())
		m_checked.insert(currentlyHovered);
	else if (!m_atLeastOne || m_checked.size() != 1)
		m_checked.erase(iterator);
}


AdvancedInterface::AdvancedInterface(sf::RenderWindow* window, unsigned int relativeScalingDefinition) noexcept
	: InteractiveInterface{ window, relativeScalingDefinition }, m_sliders{}, m_mqbs{}
{}

void AdvancedInterface::addSlider(std::string identifier, sf::Vector2f pos, unsigned int size, short intervals, UserFunction userFunction, GrowthSliderFunction growthSliderFunction, bool showValueWithText) noexcept
{
	static constexpr std::string_view sliderBackgroundTextureName{ "__sb" };
	static constexpr std::string_view sliderCursorTextureName{ "__sc" };

	if (SpriteWrapper::getTexture(sliderBackgroundTextureName) == nullptr)
	{
		constexpr float goldenRatio{ 1.618f };
		constexpr unsigned int size{ 30 };
		static constexpr float outlineThickness{ -5.f };
		SpriteWrapper::createTexture(std::string{ sliderBackgroundTextureName }, loadSolidRectange(sf::Vector2f{ size, 10 * size },			 outlineThickness), SpriteWrapper::Reserved::No);
		SpriteWrapper::createTexture(std::string{ sliderCursorTextureName },	 loadSolidRectange(sf::Vector2f{ size * goldenRatio, size }, outlineThickness), SpriteWrapper::Reserved::No);
	}

	if (m_sliders.find(identifier) != m_sliders.end()) 
		return;
	
	addDynamicSprite(identifier, sliderBackgroundTextureName, pos, sf::Vector2f{1.f, size/ 300.f});
	addDynamicSprite(sliderCursorPrefixeIdentifier + identifier, sliderCursorTextureName, pos);
	addInteractive(identifier);

	if (showValueWithText)
	{
		sf::Vector2f posText{ getDynamicSprite(sliderCursorPrefixeIdentifier + identifier)->getSprite().getGlobalBounds().position };
		addDynamicText(sliderTextPrefixeIdentifier + identifier, "", posText, 30, sf::Color::White, "__default", Alignment::Right);
	}

	m_sliders.insert(std::make_pair(identifier, Slider{ this, intervals, std::move(userFunction), std::move(growthSliderFunction)}));
	m_sliders.at(identifier).setCursor(0.f, *getDynamicSprite(sliderCursorPrefixeIdentifier + identifier), *getDynamicSprite(identifier), getDynamicText(sliderTextPrefixeIdentifier + identifier));
} 

void AdvancedInterface::removeSlider(const std::string& identifier) noexcept 
{
	if (m_sliders.find(identifier) == m_sliders.end())
		return;

	m_sliders.erase(identifier);
	removeDynamicSprite(sliderCursorPrefixeIdentifier + identifier);
	removeDynamicSprite(identifier);
	removeDynamicText(sliderTextPrefixeIdentifier + identifier); // Nothing happens if not there.
}

const AdvancedInterface::Slider* const AdvancedInterface::getSlider(const std::string& identifier) const noexcept
{
	auto itSlider{ m_sliders.find(identifier) };

	if (itSlider == m_sliders.end())
	return nullptr;

	return &itSlider->second;
}

void AdvancedInterface::addMQB(std::string identifier, sf::Vector2f posInit, sf::Vector2f posDelta, unsigned short numberOfBoxes, bool multipleChoices, bool atLeastOne, unsigned short defaultCheckedBox) noexcept
{
	static constexpr std::string_view uncheckedMqbTextureName{ "__ub" };
	static constexpr std::string_view checkedMqbTextureName{ "__cb" };

	if (SpriteWrapper::getTexture(uncheckedMqbTextureName) == nullptr)
	{
		static constexpr sf::Vector2f boxSize{ 25, 25 };
		static constexpr float outlineThickness{ -3.f };
		SpriteWrapper::createTexture(std::string{ uncheckedMqbTextureName }, loadSolidRectange(boxSize,   outlineThickness), SpriteWrapper::Reserved::No);
		SpriteWrapper::createTexture(std::string{ checkedMqbTextureName },   loadCheckBoxTexture(boxSize, outlineThickness), SpriteWrapper::Reserved::No);
	}

	if (getMQB(identifier) != nullptr)
		return;

	m_mqbs.insert(std::make_pair(identifier, MultipleQuestionBoxes{ numberOfBoxes, multipleChoices, atLeastOne, defaultCheckedBox }));

	for (unsigned int i{ 0 }; i < numberOfBoxes; ++i)
	{
		const sf::Vector2f curPos{ posInit.x + posDelta.x * i, posInit.y + posDelta.y * i }; // Calculate the position of the checkbox.
		const std::string identifierForEachBox{ "_" + std::to_string(i+1) + "_" + identifier }; // Create the id for the checkbox.
	
		addDynamicSprite(identifierForEachBox, uncheckedMqbTextureName, curPos);
		getDynamicSprite(identifierForEachBox)->addTexture(checkedMqbTextureName);
		addInteractive(identifierForEachBox, [&hover = this->m_mqbs[identifier], i](InteractiveInterface*) { hover.mqbPressed(i+1); }, Button::When::pressed);

		if (i + 1 == defaultCheckedBox) 
			getDynamicSprite(identifierForEachBox)->switchToNextTexture();
	}
}

void AdvancedInterface::removeMQB(const std::string& identifier) noexcept
{
	auto* mqb{ getMQB(identifier) };

	if (mqb == nullptr)
		return;

	for (unsigned int i{ 0 }; i < mqb->m_numberOfBoxes; ++i)
	{
		const std::string identifierForEachBox{ "_" + std::to_string(i) + "_" + identifier }; // Create the id for the checkbox.
		removeDynamicSprite(identifierForEachBox);
	}

	m_mqbs.erase(identifier);
}

AdvancedInterface::MultipleQuestionBoxes* AdvancedInterface::getMQB(const std::string& identifier) noexcept
{
	auto mapIterator{ m_mqbs.find(identifier) };

	if (mapIterator == m_mqbs.end())
		return nullptr;

	return &mapIterator->second;
}

InteractiveInterface::Item AdvancedInterface::pressed(BasicInterface* activeGUI, sf::Vector2f cursorPos) noexcept
{
	AdvancedInterface* agui{dynamic_cast<AdvancedInterface*>(activeGUI)};
	if (agui == nullptr || s_hoveredItem.identifier == "" || s_hoveredItem.igui != agui)
		return s_hoveredItem;

	auto sliderIterator{ agui->m_sliders.find(s_hoveredItem.identifier) };
	if (sliderIterator == agui->m_sliders.end())
		return s_hoveredItem;

	std::string const& identifier{ sliderIterator->first };
	sliderIterator->second.setCursor(cursorPos.y, *agui->getDynamicSprite(sliderCursorPrefixeIdentifier + identifier), *agui->getDynamicSprite(identifier), agui->getDynamicText(sliderTextPrefixeIdentifier + identifier));

	return s_hoveredItem;
}


sf::Texture AdvancedInterface::loadSolidRectange(sf::Vector2f scale, float outlineThickness) noexcept
{
	static constexpr sf::Color fillColor{ 20, 20, 20 };
	static constexpr sf::Color outlineColor{ 80, 80, 80 };

	sf::RectangleShape shape{ scale };
	shape.setFillColor(fillColor);
	shape.setOutlineColor(outlineColor);
	shape.setOutlineThickness(outlineThickness);

	return createTextureFromDrawables(shape);
}

sf::Texture AdvancedInterface::loadCheckBoxTexture(sf::Vector2f scale, float outlineThickness) noexcept
{
	static constexpr sf::Color outlineColor{ 80, 80, 80 };
	
	auto texture{ loadSolidRectange(scale, outlineThickness) };
	sf::Image image{ texture.copyToImage() };

	for (unsigned int i{ 0 }; i < image.getSize().x; ++i)
	{
		for (unsigned int j{ 0 }; j < image.getSize().y; ++j)
		{
			if (std::abs(static_cast<int>(i) - static_cast<int>(j)) < std::abs(outlineThickness))
				image.setPixel(sf::Vector2u{ i, j }, outlineColor);
			else if (std::abs(static_cast<int>(image.getSize().x) - static_cast<int>(i) - static_cast<int>(j)) < std::abs(outlineThickness))
				image.setPixel(sf::Vector2u{ i, j }, outlineColor);
		}
	}

	if (!texture.loadFromImage(image))
		texture = sf::Texture{};
	return texture;
}

}



/*





	for (auto& slider : isDerived->m_sliders)
	{
		if (isDerived->getDSprite("__sb" + slider.first).getSprite().getGlobalBounds().contains(mousePos))
		{
			m_hoveredElement = std::make_pair(InteractableItem::Slider, &slider.first);
			return m_hoveredElement;
		}
	}

	for (auto& mqb : isDerived->m_mqbs)
	{
		size_t index{ isDerived->m_dynamicSprites.find("__0_" + mqb.first)->second };
		//std::string id{ "__" + std::to_string(i) + "_" + identifier }; // Create the id for the checkbox.


		for (int i{ 0 }; i < mqb.second.m_numberOfBoxes; ++i)
		{
			if (isDerived->m_sprites[index + i].getSprite().getGlobalBounds().contains(mousePos))
			{
				mqb.second.m_currentlyHovered = i+1; // Update the currently hovered box index.
				m_hoveredElement = std::make_pair(InteractableItem::MQB, &mqb.first);
				return m_hoveredElement;
			}
		}
	}




	auto* mqb{ &isDerived->m_mqbs.at(identifier) };
		if (mqb->m_multipleChoices == true)
		{
			unsigned int currentBox{ mqb->m_currentlyHovered - 1 }; // Get the index of the currently hovered box.
			mqb->m_boxes[currentBox] = !mqb->m_boxes[currentBox];

			std::string id{ "__" + std::to_string(currentBox) + "_" + identifier }; // Create the id for the checkbox.
			isDerived->getDSprite(id).switchToNextTexture();
		}
		else
		{
			std::fill(mqb->m_boxes.begin(), mqb->m_boxes.end(), false); // Uncheck all boxes.
			mqb->m_boxes[mqb->m_currentlyHovered - 1] = true; // Check the currently hovered box.

			for (unsigned int i{ 0 }; i < mqb->m_numberOfBoxes; ++i)
			{
				std::string id{ "__" + std::to_string(i) + "_" + identifier }; // Create the id for the checkbox.
				int textureNumber{ (i == mqb->m_currentlyHovered - 1) ? 1 : 0 }; // 0 is unchecked texture, 1 is checked texture.

				isDerived->getDSprite(id).switchToNextTexture(textureNumber);
			}
		}
*/





