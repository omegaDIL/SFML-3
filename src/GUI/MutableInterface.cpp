#include "MutableInterface.hpp"

namespace gui
{

void MutableInterface::addDynamicSprite(std::string identifier, std::string_view textureName, sf::Vector2f pos, sf::Vector2f scale, sf::IntRect rect, sf::Angle rot, Alignment alignment, sf::Color color)
{
	if (m_dynamicSprites.find(identifier) != m_dynamicSprites.end())
		return;

	addSprite(textureName, pos, scale, rect, rot, alignment, color);
	auto mapIterator{ m_dynamicSprites.insert(std::make_pair(std::move(identifier), m_sprites.size() - 1)).first };
	m_indexesForEachDynamicSprites[m_sprites.size() - 1] = mapIterator;
}

void MutableInterface::addDynamicSprite(std::string identifier, sf::Texture texture, sf::Vector2f pos, sf::Vector2f scale, sf::IntRect rect, sf::Angle rot, Alignment alignment, sf::Color color) noexcept
{
	if (m_dynamicSprites.find(identifier) != m_dynamicSprites.end())
		return;

  	addSprite(texture, pos, scale, rect, rot, alignment, color);
	auto mapIterator{ m_dynamicSprites.insert(std::make_pair(std::move(identifier), m_sprites.size() - 1)).first };
	m_indexesForEachDynamicSprites[m_sprites.size() - 1] = mapIterator;
}

void MutableInterface::removeDynamicText(std::string_view identifier) noexcept
{
	const auto mapIterator{ m_dynamicTexts.find(identifier) };

	if (mapIterator == m_dynamicTexts.end())
		return;

	swapElement(mapIterator->second, m_texts.size()-1, m_texts, m_dynamicTexts, m_indexesForEachDynamicTexts); 
	m_indexesForEachDynamicTexts.erase(m_texts.size() - 1);
	m_dynamicTexts.erase(mapIterator); 
	m_texts.pop_back();
}

void MutableInterface::removeDynamicSprite(std::string_view identifier) noexcept
{
	const auto mapIterator{ m_dynamicSprites.find(identifier) };

	if (mapIterator == m_dynamicSprites.end())
		return;

	swapElement(m_sprites.size() - 1, mapIterator->second, m_sprites, m_dynamicSprites, m_indexesForEachDynamicSprites);
	m_indexesForEachDynamicSprites.erase(m_sprites.size() - 1);
	m_dynamicSprites.erase(mapIterator); 
	m_sprites.pop_back();
}

TextWrapper* MutableInterface::getDynamicText(std::string_view identifier) noexcept
{
	const auto mapIterator{ m_dynamicTexts.find(identifier) };

	if (mapIterator == m_dynamicTexts.end())
		return nullptr;

	return &m_texts[mapIterator->second];
}

SpriteWrapper* MutableInterface::getDynamicSprite(std::string_view identifier) noexcept
{
	const auto mapIterator{ m_dynamicSprites.find(identifier) };

	if (mapIterator == m_dynamicSprites.end())
		return nullptr;

	return &m_sprites[mapIterator->second];
}

} // gui namespace