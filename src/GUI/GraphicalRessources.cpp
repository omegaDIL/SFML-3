#include "GraphicalRessources.hpp"

std::list<sf::Font> TextWrapper::s_allFonts{};
std::unordered_map<std::string, std::list<sf::Font>::iterator> TextWrapper::s_accessToFonts{};
std::list<TextureHolder> SpriteWrapper::s_allTextures{};
std::unordered_map<std::string, std::list<TextureHolder>::iterator> SpriteWrapper::s_accessToTextures;
std::unordered_map<std::list<TextureHolder>::iterator, bool> SpriteWrapper::s_uniqueTextures;


///////////////////////////////////////////////////////////////////////////////////////////////////
/// A `sf::Transformable` wrapper.
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
	create(transformable, pos, scale, rot, alignment);
}

void TransformableWrapper::move(sf::Vector2f pos) noexcept
{
	ENSURE_VALID_PTR(m_transformable);
	m_transformable->move(pos);
}

void TransformableWrapper::scale(sf::Vector2f pos) noexcept
{
	ENSURE_VALID_PTR(m_transformable);
	m_transformable->scale(pos);
}

void TransformableWrapper::rotate(sf::Angle pos) noexcept
{
	ENSURE_VALID_PTR(m_transformable);
	m_transformable->rotate(pos);
}

void TransformableWrapper::setPosition(sf::Vector2f pos) noexcept
{
	ENSURE_VALID_PTR(m_transformable);
	m_transformable->setPosition(pos);
}

void TransformableWrapper::setScale(sf::Vector2f pos) noexcept
{
	ENSURE_VALID_PTR(m_transformable);
	m_transformable->setScale(pos);
}

void TransformableWrapper::setRotation(sf::Angle pos) noexcept
{
	ENSURE_VALID_PTR(m_transformable);
	m_transformable->setRotation(pos);
}

void TransformableWrapper::resized(sf::Vector2f windowScaleFactor, float relativeMinAxisScale) noexcept
{
	ENSURE_VALID_PTR(m_transformable);
	m_transformable->scale(sf::Vector2f{ relativeMinAxisScale, relativeMinAxisScale });

	sf::Vector2f previousPos{ m_transformable->getPosition() };
	m_transformable->setPosition(sf::Vector2f{ windowScaleFactor.x * previousPos.x, windowScaleFactor.x * previousPos.x });
}

void TransformableWrapper::create(sf::Transformable* transformable, sf::Vector2f pos, sf::Vector2f scale, sf::Angle rot, Alignment alignment)
{
	if (m_transformable == nullptr) [[unlikely]]
		throw std::invalid_argument{ "Wrapped sf::Transformable was nullptr when passed as argument\n" };

	m_transformable = transformable;
	m_alignment = alignment;
	hide = true;

	setPosition(pos);
	setScale(scale);
	setRotation(rot);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
/// A `sf::Transformable` wrapper.
///////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////
/// A `sf::Text` wrapper.
///////////////////////////////////////////////////////////////////////////////////////////////////

bool TextWrapper::setFont(const std::string& name) noexcept
{
	sf::Font* font{ getFont(name) };

	if (!font)
		return false;

	m_wrappedText->setFont(*font);
	return true;
}

void TextWrapper::setCharacterSize(unsigned int size) noexcept
{
	m_wrappedText->setCharacterSize(size);
	m_wrappedText->setOrigin(computeNewOrigin(m_wrappedText->getLocalBounds(), m_alignment));
}

void TextWrapper::setColor(sf::Color color) noexcept
{
	m_wrappedText->setFillColor(color);
}

void TextWrapper::setStyle(sf::Text::Style style) noexcept
{
	m_wrappedText->setStyle(style);
}

void TextWrapper::setAlignment(Alignment alignment) noexcept
{
	m_alignment = alignment;
	m_wrappedText->setOrigin(computeNewOrigin(m_wrappedText->getLocalBounds(), m_alignment));
}

sf::Font* TextWrapper::getFont(const std::string& name) noexcept
{
	auto mapIterator{ s_accessToFonts.find(name) };

	if (mapIterator != s_accessToFonts.end()) [[unlikely]]
		return nullptr;

	return &*mapIterator->second;
}

void TextWrapper::createFont(const std::string& name, const std::string& fileName)
{
	std::ostringstream errorMessage{};
	auto optFont{ loadFontFromFile(errorMessage, fileName) };
	if (!optFont.has_value())
		throw LoadingGraphicalRessourceFailure{ errorMessage.str() };

	createFont(name, std::move(optFont.value()));
}

void TextWrapper::createFont(const std::string& name, sf::Font font) noexcept
{
	auto mapIterator{ s_accessToFonts.find(name) };

	if (mapIterator != s_accessToFonts.end())
	{	
		// This name is already affiliated with a font so we replace it.
		// We dereference the iterator to reach the right position in the list
		// Then we replace it with the new font to this pos.
		// The move assignment operator will deallocate the previous font.

		*mapIterator->second = std::move(font);
		return;
	}

	// We add the font using push_front so we know that it is at the beginning.
	// Therefore, using the function begin() we have a direct iterator pointing to it.
	s_allFonts.push_front(std::move(font));
	s_accessToFonts[name] = s_allFonts.begin();
}

void TextWrapper::removeFont(const std::string& name) noexcept
{
	auto mapIterator{ s_accessToFonts.find(name) };

	if (mapIterator != s_accessToFonts.end()) [[unlikely]]
		return;

	s_allFonts.erase(mapIterator->second); // First, removing the actual font.
	s_accessToFonts.erase(mapIterator); // Then, the accessing item within the map.
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

///////////////////////////////////////////////////////////////////////////////////////////////////
/// A `sf::Text` wrapper.
///////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////
/// A `sf::Sprite` wrapper.
///////////////////////////////////////////////////////////////////////////////////////////////////

SpriteWrapper::~SpriteWrapper() noexcept
{
	for (auto& reservedTexture : m_allUniqueTextures)
	{
		auto mapAccessIterator{ s_accessToTextures.find(reservedTexture) };
		s_allTextures.erase(mapAccessIterator->second); // Remove the actual texture from the list.
		s_uniqueTextures.erase(mapAccessIterator->second); // Remove the texture from the reserved map.
		s_accessToTextures.erase(mapAccessIterator); // Remove the access toward the texture from the access map.
	}

	m_wrappedSprite = nullptr;
	m_textures.clear();
	m_allUniqueTextures.clear();
}

void SpriteWrapper::setColor(sf::Color color) noexcept
{
	m_wrappedSprite->setColor(color);
}

void SpriteWrapper::setAlignment(Alignment alignment) noexcept
{
	m_alignment = alignment;
	m_wrappedSprite->setOrigin(computeNewOrigin(m_wrappedSprite->getLocalBounds(), m_alignment));
}

inline void SpriteWrapper::switchToNextTexture()
{
	m_curTextureIndex = (++m_curTextureIndex) % m_textures.size();
	TextureInfo& newTextureInfo{ m_textures[m_curTextureIndex] };

	m_wrappedSprite->setTextureRect(newTextureInfo.displayedTexturePart);

	// Comparing pointers to see if it
	std::unique_ptr<sf::Texture>& newTexture{ newTextureInfo.texture->actualTexture };
	if (newTexture.get() == &m_wrappedSprite->getTexture())
		return; // If both addresses are the same, then no need to reapply it. 

	if (newTexture == nullptr)
	{	// Not loaded yet, so we need to load it first.
		std::ostringstream errorMessage{};
		auto optTexture{ loadTextureFromFile(errorMessage, newTextureInfo.texture->fileName) };
		
		if (!optTexture.has_value())
			throw LoadingGraphicalRessourceFailure{ errorMessage.str() };

		newTexture = std::make_unique<sf::Texture>(std::move(optTexture.value()));
	}

	m_wrappedSprite->setTexture(*newTexture); 
}

inline void SpriteWrapper::switchToNextTexture(size_t index)
{
	ENSURE_NOT_OUT_OF_RANGE(index, m_textures.size());

	if (index == m_curTextureIndex)
		return;

	m_curTextureIndex = --index; // The call to switchToNextTexture will add one.
	switchToNextTexture();
}

TextureHolder* SpriteWrapper::getTexture(const std::string& name) noexcept
{
	auto mapIterator{ s_accessToTextures.find(name) };

	if (mapIterator == s_accessToTextures.end())
		return nullptr;

	return &*mapIterator->second;
}

void SpriteWrapper::createTexture(const std::string& name, const std::string& fileName, Reserved shared, bool loadImmediately)
{
	TextureHolder newTexture{ .fileName = fileName };
	newTexture.actualTexture = nullptr;

	auto mapIterator{ s_accessToTextures.find(name) };

	if (mapIterator != s_accessToTextures.end())
	{
		// This name is already affiliated with a texture so we replace it.
		// We dereference the iterator to reach the right position in the list
		// Then we replace it with the new texture to this pos.
		// The move assignment operator will deallocate the previous texture.

		*mapIterator->second = std::move(newTexture);
		return;
	}

	// We add the font using push_front so we know that it is at the beginning.
	// Therefore, using the function begin() we have a direct iterator pointing to it.
	s_allTextures.push_front(std::move(newTexture));
	s_accessToTextures[name] = s_allTextures.begin();

	if (shared == Reserved::Yes)
		s_uniqueTextures[s_allTextures.begin()] = false;

	if (loadImmediately)
		loadTexture(name, true); // True means we delete the texture if the loading fails.
}

void SpriteWrapper::createTexture(const std::string& name, sf::Texture texture, Reserved shared) noexcept
{
	createTexture(name, "", shared, false);
	// false means it will not load immediately the texture, which is needed since there's no given
	// path. We want to set the texture ourselves.

	s_accessToTextures.at(name)->actualTexture = std::make_unique<sf::Texture>(texture);
}

void SpriteWrapper::removeTexture(const std::string& name) noexcept
{
	auto mapIterator{ s_accessToTextures.find(name) };

	if (mapIterator == s_accessToTextures.end() 
	|| s_uniqueTextures.find(mapIterator->second) != s_uniqueTextures.end()) // If it is a unique texture.
		return;

	s_allTextures.erase(mapIterator->second); // First, removing the actual texture.
	s_accessToTextures.erase(mapIterator); // Then, the accessing item within the map.
}

bool SpriteWrapper::loadTexture(const std::string& name, bool failingImpliesRemoval)
{
	TextureHolder* textureHolder{ getTexture(name) };
	
	if (textureHolder == nullptr || textureHolder->fileName == "")
		return false; // TextureHolder was not found or no file name provided: loading impossible.
	if (textureHolder->actualTexture != nullptr)
		return true; // Already loaded.

	std::ostringstream errorMessage{};
	auto optTexture{ loadTextureFromFile(errorMessage, textureHolder->fileName) };
	if (!optTexture.has_value())
	{
		if (failingImpliesRemoval)
			removeTexture(name);

		throw LoadingGraphicalRessourceFailure{ errorMessage.str() };
	}

	textureHolder->actualTexture = std::make_unique<sf::Texture>(std::move(optTexture.value()));
	return true;
}

bool SpriteWrapper::unloadTexture(const std::string& name) noexcept
{
	TextureHolder* textureHolder{ getTexture(name) };
	
	if (textureHolder == nullptr || textureHolder->fileName == "")
		return false; // TextureHolder was not found or no file name provided: loading would be impossible afterwards.
	if (textureHolder->actualTexture == nullptr)
		return true; // Already unloaded.

	textureHolder->actualTexture = nullptr;
	return true;
}


std::optional<sf::Texture> loadTextureFromFile(std::ostringstream& errorMessage, std::string const& fileName, std::string const& path) noexcept
{
	sf::Texture texture{};

	try
	{
		std::filesystem::path completePath{ std::filesystem::path(path) / fileName };

		if (!std::filesystem::exists(completePath)) [[unlikely]]
			throw LoadingGraphicalRessourceFailure{ "Texture file does not exist: " + completePath.string() + '\n' };

		if (!texture.loadFromFile(completePath)) [[unlikely]]
			throw LoadingGraphicalRessourceFailure{ "Failed to load texture from file " + completePath.string() + '\n' };

		texture.setSmooth(true); // Enable smooth rendering for the font.
	}
	catch (LoadingGraphicalRessourceFailure const& error)
	{
		errorMessage << error.what();
		errorMessage << "This texture cannot be displayed\n";
		return std::nullopt;
	}

	return std::make_optional(texture);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
/// A `sf::Sprite` wrapper.
///////////////////////////////////////////////////////////////////////////////////////////////////