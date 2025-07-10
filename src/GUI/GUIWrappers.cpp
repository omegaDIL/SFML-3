#include "GUIWrappers.hpp"

std::list<sf::Texture> SpriteWrapper::s_sharedTextures{};
std::unordered_map<std::string, std::list<sf::Texture>::iterator> SpriteWrapper::s_accessingSharedTexture{};
std::list<sf::Font> TextWrapper::s_fonts{};
std::unordered_map<std::string, std::list<sf::Font>::iterator> TextWrapper::s_accessToFonts{};


///////////////////////////////////////////////////////////////////////////////////////////////////
/// A `sf::Transformable` Wrapper.
///////////////////////////////////////////////////////////////////////////////////////////////////

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

TransformableWrapper::TransformableWrapper(sf::Transformable* transformable, sf::Vector2f pos, sf::Vector2f scale, sf::Angle rot, Alignment alignment)
	: m_transformable{ transformable }, m_alignment{ alignment }, hide{ false }
{
	if (m_transformable == nullptr) [[unlikely]]
		throw std::invalid_argument{ "Argument for TransformableWrapper was nullptr\n" };

	setPosition(pos);
	setScale(scale);
	setRotation(rot);
}


///////////////////////////////////////////////////////////////////////////////////////////////////
/// Sprite Wrapper.
///////////////////////////////////////////////////////////////////////////////////////////////////

SpriteWrapper::SpriteWrapper(sf::Texture texture, sf::Vector2f pos, sf::Vector2f scale, sf::Angle rot, Alignment alignment, sf::Color color) noexcept
	: sf::Sprite{ texture }, TransformableWrapper{ this, pos, scale, rot, alignment }, m_curTextureIndex{ 0 }, m_textures{}, m_uniqueTextures{}
{
	addTexture(texture);
	switchToNextTexture(0);

	setColor(color);
	setOrigin(computeNewOrigin(getLocalBounds(), m_alignment));
}

SpriteWrapper::SpriteWrapper(std::string const& texture, sf::Vector2f pos, sf::Vector2f scale, sf::Angle rot, Alignment alignment, sf::Color color)
	: sf::Sprite{ *s_accessingSharedTexture.at(texture)}, TransformableWrapper{ this, pos, scale, rot, alignment}, m_curTextureIndex{0}, m_textures{}, m_uniqueTextures{}
{
	addTexture(texture);

	setColor(color);
	setOrigin(computeNewOrigin(getLocalBounds(), m_alignment));
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


///////////////////////////////////////////////////////////////////////////////////////////////////
/// Text Wrapper.
///////////////////////////////////////////////////////////////////////////////////////////////////

void TextWrapper::setFont(std::string const& name)
{
	auto newFont{ s_accessToFonts.find(name) };

	if (newFont == s_accessToFonts.end()) [[unlikely]]
		throw std::invalid_argument{ "Font " + name + " does not exist or is not loaded yet\n"};

	sf::Text::setFont(*newFont->second);
}

bool TextWrapper::loadFontIntoWrapper(std::string const& name, sf::Font font) noexcept
{
	auto it{ s_accessToFonts.find(name) };

	if (it != s_accessToFonts.end())
	{	
		// This name is already affiliated with a font so we replace it.
		// We dereference the iterator to reach the right pos in the list
		// Then we move the new font to this pos.
		// The move assignment operator will deallocate the previous font.

		*it->second = std::move(font); 
		return true;
	}

	// We add the font using push_front so we know that it is at the beginning.
	// Therefore, using the function begin() we have a direct iterator pointing to it.
	s_fonts.push_front(std::move(font));
	s_accessToFonts[name] = s_fonts.begin(); 
	return false;
}

void TextWrapper::unloadFontFromWrapper(std::string const& name)
{
	auto it{ s_accessToFonts.find(name) };

	if (it != s_accessToFonts.end()) [[unlikely]]
		throw std::invalid_argument{ "Font " + name + " does not exist or has already been unloaded\n" };

	// Reaching this line means the font does exist.
	s_fonts.erase(it->second); // First, removing the actual font.
	s_accessToFonts.erase(it); // Then, the accessing item within the map.
}

sf::Font& TextWrapper::getFontForConstructor(std::string const& name)
{
	auto it{ s_accessToFonts.find(name) };
	if (it != s_accessToFonts.end()) [[likely]]
		return *it->second; 

	// Reaching this line means that no fonts have this name, therefore we throw an exception.
	throw std::invalid_argument{ "Font " + name + " does not exist or is not loaded yet\n" };
}

std::optional<sf::Font> loadFontFromFile(std::ostringstream& errorMessage, std::string const& fileName, std::string const& path) noexcept
{
	try
	{
		sf::Font font{};
		std::filesystem::path completePath{ std::filesystem::path(path) / fileName };

		if (!std::filesystem::exists(completePath)) [[unlikely]]
			throw LoadingGraphicalRessourceFailure{ "Font file does not exist: " + completePath.string() + '\n' };

		if (!font.openFromFile(completePath)) [[unlikely]]
			throw LoadingGraphicalRessourceFailure{ "Failed to load font from file " + completePath.string() + '\n' };

		font.setSmooth(true);
		return font;
	}
	catch (LoadingGraphicalRessourceFailure const& error)
	{
		errorMessage << error.what();
		errorMessage << "This font cannot be displayed\n";
		return std::nullopt;
	}
}