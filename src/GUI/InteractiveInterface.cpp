#include "InteractiveInterface.hpp"

namespace gui
{

InteractiveInterface::InteractiveInterface(sf::RenderWindow* window, unsigned int relativeScalingDefinition)
	: MutableInterface{ window, m_relativeScalingDefinition }, m_interactivesTexts{}, m_interactivesSprites{}, m_writingText{ nullptr }, m_writingFunction{ nullptr }
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
	if (getDynamicText(identifier) == m_writingText)
	{
		m_writingText = nullptr;
		getDynamicSprite(writingCursorIdentifier)->hide = true;
	}

	if (m_interactivesTexts.find(identifier) != m_interactivesTexts.end())
		m_interactivesTexts.erase(identifier);
	 
	MutableInterface::removeDynamicText(identifier);
}

void InteractiveInterface::removeDynamicSprite(const std::string& identifier) noexcept
{
	if (m_interactivesSprites.find(identifier) != m_interactivesSprites.end())
		m_interactivesSprites.erase(identifier);

	MutableInterface::removeDynamicSprite(identifier);
}

void InteractiveInterface::addButton(const std::string& identifier, ButtonFunction function) noexcept
{
	if (getDynamicSprite(identifier) != nullptr)
		m_interactivesSprites[identifier] = function;

	if (getDynamicText(identifier) != nullptr)
		m_interactivesTexts[identifier] = std::move(function);
}
	
InteractiveInterface::ButtonFunction* InteractiveInterface::getButton(const std::string& identifier) noexcept
{
	auto mapIteratorTexts{ m_interactivesTexts.find(identifier) };
	if (mapIteratorTexts != m_interactivesTexts.end())
		return &mapIteratorTexts->second;

	auto mapIteratorSprites{ m_interactivesSprites.find(identifier) };
	if (mapIteratorSprites != m_interactivesSprites.end())
		return &mapIteratorSprites->second;

	return nullptr;
}

void InteractiveInterface::setWritingText(const std::string& identifier, WritableFunction function) noexcept
{
	if (m_writingText != nullptr && m_writingText->getText().getGlobalBounds().size.x == 0)
		m_writingText->setContent("0"); // Ensure to avoid leaving the previous text empty and not clickable.
	
	// Actually updates the writing text.
	m_writingText = getDynamicText(identifier);
	m_writingFunction = function;
	ENSURE_VALID_PTR(m_writingText);

	// Updates the cursor.
	SpriteWrapper* cursor{ getDynamicSprite(writingCursorIdentifier) };
	sf::FloatRect rect{ m_writingText->getText().getGlobalBounds() };
	cursor->setPosition(sf::Vector2f{ rect.position.x + rect.size.x, rect.position.y + rect.size.y });
	cursor->hide = false;
}

InteractiveItem InteractiveInterface::updateHovered(BasicInterface* activeGUI, sf::Vector2u cursorPos) noexcept
{
	ENSURE_VALID_PTR(activeGUI);

	auto setHovered = [](InteractiveInterface* gui, std::string const* id, uint8_t type) -> void
	{
		s_hoveredItem.gui = gui;
		s_hoveredItem.identfier = id;
		s_hoveredItem.type = 0;
	};

	setHovered(nullptr, nullptr, ItemType::none);

	InteractiveInterface* gui{ dynamic_cast<InteractiveInterface*>(activeGUI) };

	if (gui == nullptr)
		return s_hoveredItem;

	sf::Vector2f cursorPosCasted{ static_cast<sf::Vector2f>(cursorPos) };
	
	for (auto& button : gui->m_interactivesSprites)
	{
		const std::string identifier{ button.first };

		if (gui->getDynamicSprite(identifier)->getSprite().getGlobalBounds().contains(cursorPosCasted))
		{
			setHovered(gui, &button.first, ItemType::button);
			return s_hoveredItem;
		}
	}

	for (auto& button : gui->m_interactivesTexts)
	{
		const std::string identifier{ button.first };

		if (gui->getDynamicText(identifier)->getText().getGlobalBounds().contains(cursorPosCasted))
		{
			setHovered(gui, &button.first, ItemType::button);
			return s_hoveredItem;
		}
	}

	return s_hoveredItem;
}

InteractiveItem InteractiveInterface::unpressed(BasicInterface* activeGUI) noexcept
{
	ENSURE_VALID_PTR(activeGUI);

	InteractiveInterface* gui{ dynamic_cast<InteractiveInterface*>(activeGUI) };

	// if active is not interactive 
	// Or if the hovered item is not part of the active gui
	if (gui == nullptr || s_hoveredItem.gui != gui)
		return s_hoveredItem;
	
	if (s_hoveredItem.type == ItemType::button)
	{
		auto function{ (*gui->getButton(*s_hoveredItem.identfier)) };
		if (function != nullptr)
			function(gui);
	}

	return s_hoveredItem;
}

void InteractiveInterface::textEntered(BasicInterface* activeGUI, char32_t unicodeValue) noexcept
{
	ENSURE_VALID_PTR(activeGUI);

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