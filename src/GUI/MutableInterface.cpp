#include "MutableInterface.hpp"

namespace gui
{

void MutableInterface::addDynamicSprite(const std::string& identifier, const std::string& textureName, sf::Vector2f pos, sf::Vector2f scale, sf::IntRect rect, sf::Angle rot, Alignment alignment, sf::Color color)
{
	if (m_dynamicSprites.find(identifier) != m_dynamicSprites.end())
		removeDynamicText(identifier);

	addSprite(textureName, pos, scale, rect, rot, alignment, color);
	m_dynamicSprites[identifier] = m_sprites.size() - 1;
	m_indexesForEachDynamicSprites[m_sprites.size() - 1] = m_dynamicSprites.find(identifier); 
}

void MutableInterface::addDynamicSprite(const std::string& identifier, sf::Texture texture, sf::Vector2f pos, sf::Vector2f scale, sf::IntRect rect, sf::Angle rot, Alignment alignment, sf::Color color) noexcept
{
	if (m_dynamicSprites.find(identifier) != m_dynamicSprites.end())
		removeDynamicText(identifier);

	addSprite(texture, pos, scale, rect, rot, alignment, color);
	m_dynamicSprites[identifier] = m_sprites.size() - 1;
	m_indexesForEachDynamicSprites[m_sprites.size() - 1] = m_dynamicSprites.find(identifier); 
}

void MutableInterface::removeDynamicText(const std::string& identifier) noexcept
{
	auto mapIterator{ m_dynamicTexts.find(identifier) };

	if (mapIterator == m_dynamicTexts.end())
		return;

	swapElement(mapIterator->second, m_texts.size()-1, m_texts, m_dynamicTexts, m_indexesForEachDynamicTexts); 
	m_indexesForEachDynamicTexts.erase(m_texts.size() - 1);
	m_dynamicTexts.erase(mapIterator); 
	m_texts.pop_back();
}

void MutableInterface::removeDynamicSprite(const std::string& identifier) noexcept
{
	auto mapIterator{ m_dynamicSprites.find(identifier) };

	if (mapIterator == m_dynamicSprites.end())
		return;

	swapElement(m_sprites.size() - 1, mapIterator->second, m_sprites, m_dynamicSprites, m_indexesForEachDynamicSprites);
	m_indexesForEachDynamicSprites.erase(m_sprites.size() - 1);
	m_dynamicSprites.erase(mapIterator); 
	m_sprites.pop_back();
}

TextWrapper* MutableInterface::getDynamicText(const std::string& identifier) noexcept
{
	auto mapIterator{ m_dynamicTexts.find(identifier) };

	if (mapIterator == m_dynamicTexts.end())
		return nullptr;

	return &m_texts[mapIterator->second];
}

SpriteWrapper* MutableInterface::getDynamicSprite(const std::string& identifier) noexcept
{
	auto mapIterator{ m_dynamicSprites.find(identifier) };

	if (mapIterator == m_dynamicSprites.end())
		return nullptr;

	return &m_sprites[mapIterator->second];
}

} // gui namespace