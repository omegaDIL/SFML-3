#include "InteractiveInterface.hpp"

namespace gui
{

InteractiveInterface::InteractiveInterface(sf::RenderWindow* window, unsigned int relativeScalingDefinition)
	: MutableInterface{ window, relativeScalingDefinition }, m_interactiveTextButtons{}, m_interactiveSpriteButtons{}, m_writingText{ nullptr }, m_writingFunction{ nullptr }
{
	static constexpr std::string_view textureName{ "__plainGrey" }; 
	if (SpriteWrapper::getTexture(textureName) == nullptr) [[unlikely]]
	{
		sf::Image writingCursorImage{ sf::Vector2u{ 1u, 1u }, sf::Color{20, 20, 20} };
		sf::Texture writingCursorTexture{ std::move(writingCursorImage) };
		writingCursorTexture.setRepeated(true);

		SpriteWrapper::createTexture(std::string{ textureName }, std::move(writingCursorTexture), SpriteWrapper::Reserved::No);
	}

	addDynamicSprite(std::string{ writingCursorIdentifier }, textureName, { 0, 0 }, { 250.f, 50.f }, sf::IntRect{}, sf::degrees(0), Alignment::Left | Alignment::Bottom);
	getDynamicSprite(writingCursorIdentifier)->hide = true;
}

void InteractiveInterface::removeDynamicText(std::string_view identifier) noexcept
{
	const auto mapIterator{ m_dynamicTexts.find(identifier) };

	if (mapIterator == m_dynamicTexts.end())
		return; // No sprite with that identifier.

	const size_t index{ mapIterator->second };

	const TextWrapper const* text{ &m_texts[index] };
	if (s_hoveredItem.igui == this 
	&&  std::holds_alternative<TextWrapper*>(s_hoveredItem.item)
	&&  std::get<TextWrapper*>(s_hoveredItem.item) == text) [[unlikely]]
		s_hoveredItem = Item{};

	if (text == m_writingText) [[unlikely]]
	{
		m_writingText = nullptr;
		getDynamicSprite(writingCursorIdentifier)->hide = true;
	}

	MutableInterface::removeDynamicText(identifier);

	const size_t m_endTextInteractives{ m_interactiveTextButtons.size() };
	if (index < m_endTextInteractives) // Only for interactive texts.
	{
		swapElement(index, m_endTextInteractives - 1, m_texts, m_dynamicTexts, m_indexesForEachDynamicTexts);

		std::swap(m_interactiveTextButtons[index], m_interactiveTextButtons[m_endTextInteractives - 1]);
		m_interactiveTextButtons.pop_back(); // Remove the last element as it is now empty.
	}
}

void InteractiveInterface::removeDynamicSprite(std::string_view identifier) noexcept
{
	const auto mapIterator{ m_dynamicSprites.find(identifier) };

	if (mapIterator == m_dynamicSprites.end())
		return; // No sprite with that identifier.

	const size_t index{ mapIterator->second };

	const SpriteWrapper const* sprite{ &m_sprites[index] };
	if (s_hoveredItem.igui == this
	&&  std::holds_alternative<SpriteWrapper*>(s_hoveredItem.item)
	&&  std::get<SpriteWrapper*>(s_hoveredItem.item) == sprite) [[unlikely]]
		s_hoveredItem = Item{};

	MutableInterface::removeDynamicSprite(identifier);

	const size_t m_endSpriteInteractives{ m_interactiveSpriteButtons.size() };
	if (index < m_endSpriteInteractives) // Only for interactive sprites.
	{
		swapElement(index, m_endSpriteInteractives - 1, m_sprites, m_dynamicSprites, m_indexesForEachDynamicSprites);

		std::swap(m_interactiveSpriteButtons[index], m_interactiveSpriteButtons[m_endSpriteInteractives - 1]);
		m_interactiveSpriteButtons.pop_back(); // Remove the last element as it is now empty.
	}
}
 
void InteractiveInterface::addInteractive(const std::string& identifier, ButtonFunction function, Button::When when) noexcept
{
	if (function == nullptr) [[unlikely]]
		when = Button::When::none; // If no function is provided, the button will not be interactive.
	else if (when == Button::When::none) [[unlikely]]
		function = nullptr;

	const auto textIterator{ m_dynamicTexts.find(identifier) };
	const auto spriteIterator{ m_dynamicSprites.find(identifier) };

	if (textIterator != m_dynamicTexts.end() && textIterator->second >= m_interactiveTextButtons.size())
	{
		swapElement(textIterator->second, m_interactiveTextButtons.size(), m_texts, m_dynamicTexts, m_indexesForEachDynamicTexts);
		m_interactiveTextButtons.push_back(Button{ ((spriteIterator == m_dynamicSprites.end()) ? std::move(function) : function), when });
	}

	if (spriteIterator != m_dynamicSprites.end() && spriteIterator->second >= m_interactiveSpriteButtons.size())
	{
		swapElement(spriteIterator->second, m_interactiveSpriteButtons.size(), m_sprites, m_dynamicSprites, m_indexesForEachDynamicSprites);
		m_interactiveSpriteButtons.push_back(Button{ std::move(function), when }); // Some compilers might trigger a false positive warning for use of a moved-from object. 
	}
}

void InteractiveInterface::setWritingText(std::string_view identifier, WritableFunction function) noexcept
{
	if (m_writingText != nullptr && m_writingText->getText().getGlobalBounds().size.x == 0)
		m_writingText->setContent(emptinessWritingCharacters); // Ensure to avoid leaving the previous text empty and not clickable.
	
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

InteractiveInterface::Item InteractiveInterface::updateHovered(BasicInterface* activeGUI, sf::Vector2f cursorPos) noexcept
{
	ENSURE_VALID_PTR(activeGUI, "The gui was nullptr when the function updateHovered was called in InteractiveInterface");

	InteractiveInterface* igui{ dynamic_cast<InteractiveInterface*>(activeGUI) };

	if (igui == nullptr)
		return (s_hoveredItem = Item{});

	if (igui == s_hoveredItem.igui)
	{
		if (std::holds_alternative<TextWrapper*>(s_hoveredItem.item)
		&&	std::get<TextWrapper*>(s_hoveredItem.item)->getText().getGlobalBounds().contains(cursorPos))
			return s_hoveredItem; // No need to check again if the hovered item is the same.

		if (std::holds_alternative<SpriteWrapper*>(s_hoveredItem.item)
			&& std::get<SpriteWrapper*>(s_hoveredItem.item)->getSprite().getGlobalBounds().contains(cursorPos))
			return s_hoveredItem; // No need to check again if the hovered item is the same.
	}

	s_hoveredItem = Item{};

	const size_t m_endSpriteInteractives{ igui->m_interactiveSpriteButtons.size() };
	for (size_t i{ 0 }; i < m_endSpriteInteractives; ++i)
	{
		if (igui->m_sprites[i].getSprite().getGlobalBounds().contains(cursorPos))
		{
			s_hoveredItem = Item{ igui, &igui->m_sprites[i], &igui->m_interactiveSpriteButtons[i] };

			if (s_hoveredItem.m_button->when == InteractiveInterface::Button::When::hovered)
				s_hoveredItem.m_button->function(igui);
			
			break; // A text might be on top of the sprite, so we need to check it too.
		}
	}

	const size_t m_endTextInteractives{ igui->m_interactiveTextButtons.size() };
	for (size_t i{ 0 }; i < m_endTextInteractives; ++i)
	{
		if (igui->m_texts[i].getText().getGlobalBounds().contains(cursorPos))
		{
			s_hoveredItem = Item{ igui, &igui->m_texts[i], &igui->m_interactiveTextButtons[i] };

			if (s_hoveredItem.m_button->when == InteractiveInterface::Button::When::hovered)
				s_hoveredItem.m_button->function(igui);

			break; // A text might be on top of the sprite, so we need to check it too.
		}
	}

	return s_hoveredItem;
}

InteractiveInterface::Item InteractiveInterface::pressed(BasicInterface* activeGUI) noexcept
{
	ENSURE_VALID_PTR(activeGUI, "The gui was nullptr when the function pressed was called in InteractiveInterface");

	InteractiveInterface* igui{ dynamic_cast<InteractiveInterface*>(activeGUI) };

	if (igui != nullptr 
	&&  igui == s_hoveredItem.igui // This checks also ensure there is a hovered item.
	&&  s_hoveredItem.m_button->when == Button::When::pressed)
		s_hoveredItem.m_button->function(igui);

	return s_hoveredItem;
}

InteractiveInterface::Item InteractiveInterface::unpressed(BasicInterface* activeGUI) noexcept
{
	ENSURE_VALID_PTR(activeGUI, "The gui was nullptr when the function unpressed was called in InteractiveInterface");

	InteractiveInterface* igui{ dynamic_cast<InteractiveInterface*>(activeGUI) };

	if (igui != nullptr
		&& igui == s_hoveredItem.igui // This checks also ensure there is a hovered item.
		&& s_hoveredItem.m_button->when == Button::When::unpressed)
		s_hoveredItem.m_button->function(igui);

	return s_hoveredItem;
}

void InteractiveInterface::textEntered(BasicInterface* activeGUI, char32_t unicodeValue) noexcept
{
	ENSURE_VALID_PTR(activeGUI, "The gui was nullptr when the function textEntered was called in InteractiveInterface");

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
	//m_writingFunction(this, character, text);

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