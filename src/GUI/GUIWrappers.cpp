#include "GUIWrappers.hpp"

std::list<sf::Texture> SpriteWrapper::s_sharedTextures{};
std::unordered_map<std::string, std::list<sf::Texture>::iterator> SpriteWrapper::s_accessingSharedTexture{};


Alignment operator|(Alignment lhs, Alignment rhs) noexcept
{
	auto newAlignment{ static_cast<uint8_t>(lhs) | static_cast<uint8_t>(rhs) };
	
	// Checking if alignment are not compatible.
	if (((newAlignment & 0b00000011) == 0b00000011)
	|| ((newAlignment & 0b00001100) == 0b00001100))
		return lhs;

	return static_cast<Alignment>(newAlignment);
}

sf::Vector2f computeNewOrigin(sf::FloatRect bound, Alignment alignment) noexcept
{
	sf::Vector2f originTopLeft{ 0, 0 };
	sf::Vector2f originBottomRight{ bound.size };
	sf::Vector2f origin{ bound.getCenter() }; // Center origin by default.

	uint8_t value = static_cast<uint8_t>(alignment);
	if ((value >> 3) & 1)
		origin.x = originTopLeft.x; // Left side.
	else if ((value >> 2) & 1)
		origin.x = originBottomRight.x; // Right side.

	if ((value >> 1) & 1)
		origin.y = originTopLeft.y; // Top side.
	else if ((value >> 0) & 1)
		origin.y = originBottomRight.y; // Bottom side.

	return origin;
}


SpriteWrapper::SpriteWrapper(std::string const& sharedTexture, sf::Vector2f pos, sf::Vector2f scale, Alignment alignment, sf::Angle rot, sf::Color color)
	: sf::Sprite{ *s_accessingSharedTexture.at(sharedTexture) }, TransformableWrapper{ alignment }, m_curTextureIndex{ 0 }
{
	m_textures.push_back(&(*s_accessingSharedTexture.at(sharedTexture))); // Add the texture to the vector of textures.

	if (rectangle.size == sf::Vector2i{ 0, 0 }) [[likely]]
		setTextureRect(sf::IntRect{ sf::Vector2i{}, static_cast<sf::Vector2i>(getTexture().getSize()) });

	setPosition(pos);
	setScale(scale);
	setRotation(rot);
	setColor(color);
	setOrigin(computeNewOrigin(getLocalBounds(), alignment));
}

SpriteWrapper::SpriteWrapper(sf::Texture texture, sf::Vector2f pos, sf::Vector2f scale, Alignment alignment, sf::Angle rot, sf::Color color) noexcept
	: sf::Sprite{ texture }, TransformableWrapper{ alignment }, m_curTextureIndex{ 0 }
{
	m_uniqueTextures.push_back(std::move(texture)); // Add the texture to the vector of unique textures.
	m_textures.push_back(&m_uniqueTextures.back()); // Add the texture to the vector of textures.
	setTexture(*m_textures.back()); // Has been moved, so reset the texture of the sprite.

	if (rectangle.size == sf::Vector2i{ 0, 0 }) [[likely]]
		setTextureRect(sf::IntRect{ sf::Vector2i{}, static_cast<sf::Vector2i>(getTexture().getSize()) });
	setPosition(pos);
	setScale(scale);
	setRotation(rot);
	setColor(color);
	setOrigin(computeNewOrigin(getLocalBounds(), alignment));
}

void SpriteWrapper::addSharedTexture(std::string const& identifier, sf::Texture textures) noexcept
{
	s_sharedTextures.push_front(std::move(textures)); // Add the texture to the static collection of textures.
	s_accessingSharedTexture[identifier] = s_sharedTextures.begin(); // Map the identifier to the texture in the static collection.
}

void SpriteWrapper::removeSharedTexture(std::string const& identifier) noexcept
{
	s_sharedTextures.erase(s_accessingSharedTexture.at(identifier)); // Remove the texture from the static collection of textures.
	s_accessingSharedTexture.erase(identifier); // Remove the identifier from the static collection of textures.
}
