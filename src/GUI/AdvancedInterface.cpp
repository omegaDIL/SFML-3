#include "AdvancedInterface.hpp"

namespace gui
{

AdvancedInterface::AdvancedInterface(sf::RenderWindow* window, unsigned int relativeScalingDefinition)
	: InteractiveInterface{ window, relativeScalingDefinition }, m_sliders{}, m_mqbs{}
{
	if (SpriteWrapper::getTexture(mqbCheckedTextureName) != nullptr)
	{		
		sf::Vector2f boxSize{ 20, 20 };
		SpriteWrapper::createTexture(mqbCheckedTextureName, loadCheckBoxTexture(boxSize), SpriteWrapper::Reserved::No);
		SpriteWrapper::createTexture(mqbUncheckedTextureName, loadSolidRectange(boxSize), SpriteWrapper::Reserved::No);
		
		constexpr float goldenRatio{ 1.618f };
		SpriteWrapper::createTexture(sliderBackgroundTextureName, loadSolidRectange(sf::Vector2f{ 20 * goldenRatio, 20.f }), SpriteWrapper::Reserved::No);
		SpriteWrapper::createTexture(sliderCursorTextureName, loadSolidRectange(sf::Vector2f{ 20.f, 200.f }), SpriteWrapper::Reserved::No);
	}
}

void AdvancedInterface::addSlider(const std::string& identifier, sf::Vector2f pos, unsigned int size, short intervals, UserFunction userFunction, GrowthSliderFunction growthSliderFunction, bool showValueWithText, sf::Vector2f scale) noexcept
{
	if (getSlider(identifier) != nullptr)
		removeSlider(identifier);

	Slider newSlider{};
	newSlider.internalIntervals = intervals;
	newSlider.userFunction = userFunction;
	newSlider.growthSliderFunction = growthSliderFunction;
	m_sliders[identifier] = std::move(newSlider);

	scale.y *= size / 200.f;
	addDynamicSprite("_sb_" + identifier, sliderBackgroundTextureName, pos, scale);
	addDynamicSprite("_sc_" + identifier, sliderCursorTextureName, pos, sf::Vector2f{ scale.x, scale.x });
	addButton("_sb_" + identifier);

	if (showValueWithText)
	{
		sf::Vector2f posText{ getDynamicSprite("_sb_" + identifier)->getSprite().getGlobalBounds().position };
		addDynamicText("_ts_" + identifier, "", posText, 30u, sf::Color::White, "__default", Alignment::Right);
	}
}

void AdvancedInterface::removeSlider(const std::string& identifier) noexcept
{
	ENSURE_VALID_PTR(getSlider(identifier), "Slider was nullptr when removeSlider function was called in AdvancedInterface");

	m_sliders.erase(identifier);
	removeDynamicSprite("_sb_" + identifier);
	removeDynamicSprite("_sc_" + identifier);
	removeDynamicText("_ts_" + identifier); // Nothing happens if not there.
}

AdvancedInterface::Slider* AdvancedInterface::getSlider(const std::string& identifier) noexcept
{
	auto mapIterator{ m_sliders.find(identifier) };

	if (mapIterator == m_sliders.end())
		return nullptr;

	return &mapIterator->second;
}

void AdvancedInterface::addMQB(const std::string& identifier, unsigned short numberOfBoxes, sf::Vector2f posInit, sf::Vector2f posDelta, bool multipleChoices, bool atLeastOne, sf::Vector2f scale) noexcept
{
	if (getMQB(identifier) != nullptr)
		removeMQB(identifier);

	MQB newMqb{};
	newMqb.numberOfBoxes = numberOfBoxes;
	newMqb.multipleChoices = multipleChoices;
	newMqb.atLeastOne = atLeastOne;
	newMqb.currentlyHovered = 0;
	m_mqbs[identifier] = std::move(newMqb);

	for (unsigned int i{ 0 }; i < numberOfBoxes; ++i)
	{
		const sf::Vector2f curPos{ posInit.x + posDelta.x * i, posInit.y + posDelta.y * i }; // Calculate the position of the checkbox.
		const std::string identifier{ "_" + std::to_string(i+1) + "_" + identifier }; // Create the id for the checkbox.
	
		addDynamicSprite(identifier, mqbUncheckedTextureName, curPos, scale);
		getDynamicSprite(identifier)->addTexture(mqbCheckedTextureName);
		addButton(identifier);
	}
}

void AdvancedInterface::removeMQB(const std::string& identifier) noexcept
{
	auto mqbToRemove{ getMQB(identifier) };

	ENSURE_VALID_PTR(mqbToRemove, "MQB was nullptr when removeMQB function was called in AdvancedInterface");

	for (unsigned int i{ 0 }; i < mqbToRemove->numberOfBoxes; ++i)
	{
		const std::string identifier{ "_" + std::to_string(i) + "_" + identifier }; // Create the id for the checkbox.
		removeDynamicSprite(identifier);
	}

	m_mqbs.erase(identifier);
}

AdvancedInterface::MQB* AdvancedInterface::getMQB(const std::string& identifier) noexcept
{
	auto mapIterator{ m_mqbs.find(identifier) };

	if (mapIterator == m_mqbs.end())
		return nullptr;

	return &mapIterator->second;
}

InteractiveItem AdvancedInterface::updateHovered(BasicInterface* activeGUI, sf::Vector2i cursorPos) noexcept
{
	InteractiveInterface::updateHovered(activeGUI, cursorPos);

	if (s_hoveredItem.type != InteractiveInterface::ItemType::other)
		return s_hoveredItem;



	return s_hoveredItem;
}

InteractiveItem AdvancedInterface::pressed(BasicInterface* activeGUI) noexcept
{
	return s_hoveredItem;
}

InteractiveItem AdvancedInterface::unpressed(BasicInterface* activeGUI) noexcept
{
	return s_hoveredItem;
}

float AdvancedInterface::calculateValueOfSlider(const std::string& identifier) noexcept
{
	Slider* slider{ getSlider(identifier) }; // Throws an exception if not there.
	sf::Sprite const& cursor{ getDynamicSprite("_sc_" + identifier)->getSprite() };
	sf::Sprite const& background{ getDynamicSprite("_sb_" + identifier)->getSprite() };

	const float minPos{ background.getGlobalBounds().position.y };
	const float maxPos{ minPos + background.getGlobalBounds().size.y };
	const float curPos{ cursor.getPosition().y };

	return slider->growthSliderFunction(1 - (curPos - minPos) / (maxPos - minPos));
}

void AdvancedInterface::setCursorOfSlider(const std::string& identifier, unsigned int curPosY) noexcept
{
	Slider& slider{ m_sliders.at(identifier) }; // Throws an exception if not there.
	SpriteWrapper* cursor{ getDynamicSprite("_sc_" + identifier) };
	SpriteWrapper* background{ getDynamicSprite("_sb_" + identifier) };

	float minPos{ background->getSprite().getGlobalBounds().position.y };
	float maxPos{ minPos + background->getSprite().getGlobalBounds().size.y };
	float yPos{ static_cast<float>(curPosY) };

	if (yPos < minPos)
		yPos = minPos;
	else if (yPos > maxPos)
		yPos = maxPos;

	if (slider.internalIntervals >= 0)
	{
		float interval = 1.f / (slider.internalIntervals + 1); // The intervals exclude the min and max positions, so we add 1 to the number of intervals.
		float relativePos = (yPos - minPos) / (maxPos - minPos); // Between 0 and 1.
		float intervalPos = round(relativePos / interval) * interval; // Rounding to the nearest interval position.
		yPos = minPos + ((maxPos - minPos) * intervalPos); // Recalculating the position based on the intervals.
	}

	cursor->setPosition(sf::Vector2f{ cursor->getSprite().getPosition().x, yPos });
	float value{ slider.growthSliderFunction(1 - (yPos - minPos) / (maxPos - minPos)) }; // Get the value of the slider.

	if (m_dynamicTexts.find('_' + identifier) != m_dynamicTexts.end()) // If the slider has a text associated with it.
	{
		TextWrapper* text{ getDynamicText("_ts_" + identifier) };

		text->setPosition(sf::Vector2f{ text->getText().getPosition().x, yPos }); // Aligning the text with the cursor.
		text->setContent(value);
	}

	slider.userFunction(this, value); // Call the function associated with the slider.
}


sf::Texture AdvancedInterface::loadSolidRectange(sf::Vector2f scale, sf::Color fill, sf::Color outline, unsigned int thickness) noexcept
{
	sf::RectangleShape shape{ scale };
	shape.setFillColor(fill);
	shape.setOutlineColor(outline);
	shape.setOutlineThickness(std::min(scale.x, scale.y) / thickness);

	return createTextureFromDrawables(shape);
}

sf::Texture AdvancedInterface::loadCheckBoxTexture(sf::Vector2f scale) noexcept
{
	sf::Color fillColor{ 20, 20, 20 };
	sf::Color outlineColor{ 80, 80, 80 };

	auto texture{ loadSolidRectange(scale) };
	sf::Image image{ texture.copyToImage() };

	unsigned int checkThickness{ static_cast<unsigned int>(std::min(scale.x, scale.y) / 5) };
	for (unsigned int i{ 0 }; i < image.getSize().x; ++i)
	{
		for (unsigned int j{ 0 }; j < image.getSize().y; ++j)
		{
			if (std::abs(static_cast<int>(i) - static_cast<int>(j)) < checkThickness)
				image.setPixel(sf::Vector2u{ i, j }, outlineColor);
			else if (std::abs(static_cast<int>(image.getSize().x) - static_cast<int>(i) - static_cast<int>(j)) < checkThickness)
				image.setPixel(sf::Vector2u{ i, j }, outlineColor);
		}
	}

	if (texture.loadFromImage(image))
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





