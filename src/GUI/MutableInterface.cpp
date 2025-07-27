#include "MutableInterface.hpp"

namespace gui
{

void MutableInterface::addDynamicSprite(const std::string& identifier, const std::string& textureName, sf::Vector2f pos, sf::Vector2f scale, sf::IntRect rect, sf::Angle rot, Alignment alignment, sf::Color color)
{
	ENSURE_STRING_NOT_EMPTY(identifier);

	if (m_dynamicSprites.find(identifier) != m_dynamicSprites.end())
		removeDynamicText(identifier);

	addSprite(textureName, pos, scale, rect, rot, alignment, color);
	m_dynamicSprites[identifier] = m_sprites.size() - 1;
	m_indexesForEachDynamicSprites[m_sprites.size() - 1] = m_dynamicSprites.find(identifier); // Add the index to the vector of indexes for dynamic sprites.
}

void MutableInterface::addDynamicSprite(const std::string& identifier, sf::Texture texture, sf::Vector2f pos, sf::Vector2f scale = sf::Vector2f{ 1.f, 1.f }, sf::IntRect rect = sf::IntRect{}, sf::Angle rot = sf::degrees(0), Alignment alignment = Alignment::Center, sf::Color color = sf::Color::White)
{
	ENSURE_STRING_NOT_EMPTY(identifier);

	if (m_dynamicSprites.find(identifier) != m_dynamicSprites.end())
		removeDynamicText(identifier);

	addSprite(texture, pos, scale, rect, rot, alignment, color);
	m_dynamicSprites[identifier] = m_sprites.size() - 1;
	m_indexesForEachDynamicSprites[m_sprites.size() - 1] = m_dynamicSprites.find(identifier); // Add the index to the vector of indexes for dynamic sprites.
}

void MutableInterface::removeDynamicText(const std::string& identifier) noexcept
{
	ENSURE_STRING_NOT_EMPTY(identifier);

	if (getDynamicText(identifier) == nullptr)
		return;

	removeDynamicElement(identifier, m_texts, m_dynamicTexts, m_indexesForEachDynamicTexts); // Removes the text with the given identifier.
}

void MutableInterface::removeDynamicSprite(const std::string& identifier) noexcept
{
	ENSURE_STRING_NOT_EMPTY(identifier);

	if (getDynamicSprite(identifier) == nullptr)
		return;

	removeDynamicElement(identifier, m_sprites, m_dynamicSprites, m_indexesForEachDynamicSprites); // Removes the text with the given identifier.
}

TextWrapper* MutableInterface::getDynamicText(const std::string& identifier) const noexcept
{
	ENSURE_STRING_NOT_EMPTY(identifier);

	auto mapIterator{ m_dynamicTexts.find(identifier) };

	if (mapIterator == m_dynamicTexts.end())
		return nullptr;

	return &m_texts[mapIterator->second];
}

SpriteWrapper* MutableInterface::getDynamicSprite(const std::string& identifier) const noexcept
{
	ENSURE_STRING_NOT_EMPTY(identifier);

	auto mapIterator{ m_dynamicSprites.find(identifier) };

	if (mapIterator == m_dynamicSprites.end())
		return nullptr;

	return &m_sprites[mapIterator->second];
}

} // gui namespace