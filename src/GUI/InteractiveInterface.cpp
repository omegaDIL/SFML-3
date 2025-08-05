#include "InteractiveInterface.hpp"

namespace gui
{

InteractiveInterface::InteractiveInterface(sf::RenderWindow* window, unsigned int relativeScalingDefinition)
	: MutableInterface{ window, m_relativeScalingDefinition }, m_endTextInteractives{ 0 }, m_endSpriteInteractives{ 0 }, m_interactiveTexts{}, m_interactiveSprites{}, m_writingText{ nullptr }, m_writingFunction{ nullptr }
{
	std::string textureName{ "__plainGrey" }; 
	if (SpriteWrapper::getTexture(textureName) == nullptr) [[unlikely]]
	{
		sf::Image writingCursorImage{ sf::Vector2u{ 1u, 1u }, sf::Color{20, 20, 20} };
		sf::Texture writingCursorTexture{ std::move(writingCursorImage) };
		writingCursorTexture.setRepeated(true);

		SpriteWrapper::createTexture(textureName, std::move(writingCursorTexture), SpriteWrapper::Reserved::No);
	}

	addDynamicSprite(writingCursorIdentifier, textureName, { 0, 0 }, { 250.f, 50.f }, sf::IntRect{}, sf::degrees(0), Alignment::Left | Alignment::Bottom);
	getDynamicSprite(writingCursorIdentifier)->hide = true;
}

void InteractiveInterface::removeDynamicText(const std::string& identifier) noexcept
{
	const auto mapIterator{ m_dynamicTexts.find(identifier) };

	if (mapIterator == m_dynamicTexts.end())
		return; // No sprite with that identifier.

	size_t index{ mapIterator->second };

	if (&m_texts[index] == m_writingText)
	{
		m_writingText = nullptr;
		getDynamicSprite(writingCursorIdentifier)->hide = true;
	} 

	MutableInterface::removeDynamicSprite(identifier);

	if (index < m_endTextInteractives)
	{
		swapElement(index, m_endTextInteractives - 1, m_texts, m_dynamicTexts, m_indexesForEachDynamicTexts);

		std::swap(m_interactiveTexts[index], m_interactiveTexts[m_endTextInteractives - 1]);
		m_interactiveTexts.erase(m_endTextInteractives - 1); // Remove the last element as it is now empty.

		--m_endTextInteractives;
	}
}

void InteractiveInterface::removeDynamicSprite(const std::string& identifier) noexcept
{
	const auto mapIterator{ m_dynamicSprites.find(identifier) };

	if (mapIterator == m_dynamicSprites.end())
		return; // No sprite with that identifier.

	size_t index{ mapIterator->second };

	MutableInterface::removeDynamicSprite(identifier);

	if (index < m_endSpriteInteractives)
	{
		swapElement(index, m_endSpriteInteractives - 1, m_sprites, m_dynamicSprites, m_indexesForEachDynamicSprites);

		std::swap(m_interactiveSprites[index], m_interactiveSprites[m_endSpriteInteractives - 1]);
		m_interactiveSprites.erase(m_endSpriteInteractives - 1); // Remove the last element as it is now empty.

		--m_endSpriteInteractives;
	}
}
 
void InteractiveInterface::addInteractive(const std::string& identifier, ButtonFunction function, Button::When when) noexcept
{
	const auto sprite{ getDynamicSprite(identifier) };
	const auto text{ getDynamicText(identifier) };

	if (text != nullptr)
	{
		swapElement(m_dynamicTexts[identifier], m_endTextInteractives++, m_texts, m_dynamicTexts, m_indexesForEachDynamicTexts);
		m_interactiveTexts[m_endTextInteractives] = Button{ (sprite == nullptr) ? std::move(function) : function, when };
	}

	if (sprite != nullptr)
	{
		swapElement(m_dynamicSprites[identifier], m_endSpriteInteractives++, m_sprites, m_dynamicSprites, m_indexesForEachDynamicSprites);
		m_interactiveSprites[m_endSpriteInteractives] = Button{ std::move(function), when };
	}
}
	
InteractiveInterface::ButtonFunction* InteractiveInterface::getButton(const std::string& identifier) noexcept
{
	const auto mapIteratorTextIndexes{ m_dynamicTexts.find(identifier) };
	if  (mapIteratorTextIndexes != m_dynamicTexts.end())
	{
		const auto mapIteratorTextButton{ m_interactiveTexts.find(mapIteratorTextIndexes->second) };
		if (mapIteratorTextButton != m_interactiveTexts.end())
			return &mapIteratorTextButton->second.function;
	}

	const auto mapIteratorSpriteIndexes{ m_dynamicSprites.find(identifier) };
	if (mapIteratorSpriteIndexes != m_dynamicSprites.end())
	{
		auto mapIteratorSpriteButton{ m_interactiveSprites.find(mapIteratorSpriteIndexes->second) };
		if (mapIteratorSpriteButton != m_interactiveSprites.end())
			return &mapIteratorSpriteButton->second.function;
	}

	return nullptr;
}

void InteractiveInterface::setWritingText(const std::string& identifier, WritableFunction function) noexcept
{
	if (m_writingText != nullptr && m_writingText->getText().getGlobalBounds().size.x == 0)
		m_writingText->setContent("0"); // Ensure to avoid leaving the previous text empty and not clickable.
	
	// Actually updates the writing text.
	m_writingText = getDynamicText(identifier);
	m_writingFunction = function;
	ENSURE_VALID_PTR(m_writingText, "The identifier was not found resulting in writingText being nullptr when setWritingText was called in InteractiveInterface");

	// Updates the cursor.
	SpriteWrapper* cursor{ getDynamicSprite(writingCursorIdentifier) };
	sf::FloatRect rect{ m_writingText->getText().getGlobalBounds() };
	cursor->setPosition(sf::Vector2f{ rect.position.x + rect.size.x, rect.position.y + rect.size.y });
	cursor->hide = false;
}

InteractiveInterface::Item InteractiveInterface::updateHovered(BasicInterface* activeGUI, sf::Vector2i cursorPos) noexcept
{
	ENSURE_VALID_PTR(activeGUI, "The gui was nullptr when updateHovered was called in InteractiveInterface");

	InteractiveInterface* igui{ dynamic_cast<InteractiveInterface*>(activeGUI) };
	s_hoveredItem = Item{ nullptr, "", nullptr };

	if (igui == nullptr)
		return s_hoveredItem;

	sf::Vector2f cursorPosCasted{ igui->m_window->mapPixelToCoords(cursorPos) };

	for (size_t i{ 0 }; i < igui->m_endSpriteInteractives; ++i) 
	{
		if (igui->m_sprites[i].getSprite().getGlobalBounds().contains(cursorPosCasted))
		{
			s_hoveredItem = Item{ igui, igui->m_indexesForEachDynamicSprites[i]->first, &igui->m_interactiveSprites[i] };

			if (s_hoveredItem.m_button->when == InteractiveInterface::Button::When::hovered)
				s_hoveredItem.m_button->function(igui, s_hoveredItem.identfier);
			
			break; // A text might be on top of the sprite, so we need to check it too.
		}
	}

	for (size_t i{ 0 }; i < igui->m_endTextInteractives; ++i)
	{
		if (igui->m_texts[i].getText().getGlobalBounds().contains(cursorPosCasted))
		{
			s_hoveredItem = Item{ igui, igui->m_indexesForEachDynamicSprites[i]->first, &igui->m_interactiveSprites[i] };

			if (s_hoveredItem.m_button->when == InteractiveInterface::Button::When::hovered)
				s_hoveredItem.m_button->function(igui, s_hoveredItem.identfier);

			break; // A text might be on top of the sprite, so we need to check it too.
		}
	}

	return s_hoveredItem;
}

InteractiveInterface::Item InteractiveInterface::pressed(BasicInterface* activeGUI) noexcept
{
	ENSURE_VALID_PTR(activeGUI, "The gui was nullptr when pressed was called in InteractiveInterface");

	InteractiveInterface* igui{ dynamic_cast<InteractiveInterface*>(activeGUI) };

	// if active is not interactive 
	// Or if there is no hovered item
	// Or if the hovered item is not part of the active gui
	if (igui == nullptr || s_hoveredItem.identfier == "" || s_hoveredItem.igui != igui)
		return s_hoveredItem;

	if (s_hoveredItem.m_button->when == Button::When::pressed)
		s_hoveredItem.m_button->function(igui, s_hoveredItem.identfier); // Ensure the sprite is created.

	return s_hoveredItem;
}

InteractiveInterface::Item InteractiveInterface::unpressed(BasicInterface* activeGUI) noexcept
{
	ENSURE_VALID_PTR(activeGUI, "The gui was nullptr when unpressed was called in InteractiveInterface");

	InteractiveInterface* igui{ dynamic_cast<InteractiveInterface*>(activeGUI) };

	// if active is not interactive 
	// Or if there is no hovered item
	// Or if the hovered item is not part of the active gui
	if (igui == nullptr || s_hoveredItem.identfier == "" || s_hoveredItem.igui != igui)
		return s_hoveredItem;
	
	if (s_hoveredItem.m_button->when == Button::When::unpressed)
		s_hoveredItem.m_button->function(igui, s_hoveredItem.identfier); // Ensure the sprite is created.
	
	return s_hoveredItem;
}

void InteractiveInterface::textEntered(BasicInterface* activeGUI, char32_t unicodeValue) noexcept
{
	ENSURE_VALID_PTR(activeGUI, "The gui was nullptr when textEntered was called in InteractiveInterface");

	InteractiveInterface* gui{ dynamic_cast<InteractiveInterface*>(activeGUI) };

	if (gui == nullptr || gui->m_writingText == nullptr)
		return;

	gui->updateWritingText(unicodeValue);
}

void InteractiveInterface::updateWritingText(char32_t character) noexcept
{
	if (character == exitWritingCharacter)
	{
		if (m_writingText->getText().getGlobalBounds().size.x == 0)
			m_writingText->setContent("0"); // Ensure to avoid leaving the previous text empty and not clickable.
		m_writingText = nullptr;
		getDynamicSprite(writingCursorIdentifier)->hide = true;
		return;
	}

	std::string text{ m_writingText->getText().getString() };
	m_writingFunction(this, character, text);

	static constexpr char32_t backspaceCharacter{ 0x0008 };
	if (!text.empty() && character == backspaceCharacter)
		text.pop_back();
	else [[likely]]
		text.push_back(character);
	m_writingText->setContent(text);

	SpriteWrapper* cursor{ getDynamicSprite(writingCursorIdentifier) };
	sf::FloatRect rect{ m_writingText->getText().getGlobalBounds() };
	cursor->setPosition(sf::Vector2f{ rect.position.x + rect.size.x, rect.position.y + rect.size.y });
}

} // gui namespace