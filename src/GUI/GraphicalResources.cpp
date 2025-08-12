#include "GraphicalResources.hpp"
#include <utility>

namespace gui
{

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


void TransformableWrapper::move(sf::Vector2f pos) noexcept
{
	ENSURE_VALID_PTR(m_transformable, "Pointer to sf::Transformable in TransformableWrapper is nullptr when the function move is called");
	m_transformable->move(pos);
}

void TransformableWrapper::scale(sf::Vector2f pos) noexcept
{
	ENSURE_VALID_PTR(m_transformable, "Pointer to sf::Transformable in TransformableWrapper is nullptr when the function scale is called");
	m_transformable->scale(pos);
}

void TransformableWrapper::rotate(sf::Angle pos) noexcept
{
	ENSURE_VALID_PTR(m_transformable, "Pointer to sf::Transformable in TransformableWrapper is nullptr when the function rotate is called");
	m_transformable->rotate(pos);
}

void TransformableWrapper::setPosition(sf::Vector2f pos) noexcept
{
	ENSURE_VALID_PTR(m_transformable, "Pointer to sf::Transformable in TransformableWrapper is nullptr when the function setPosition is called");
	m_transformable->setPosition(pos);
}

void TransformableWrapper::setScale(sf::Vector2f pos) noexcept
{
	ENSURE_VALID_PTR(m_transformable, "Pointer to sf::Transformable in TransformableWrapper is nullptr when the function setScale is called");
	m_transformable->setScale(pos);
}

void TransformableWrapper::setRotation(sf::Angle pos) noexcept
{
	ENSURE_VALID_PTR(m_transformable, "Pointer to sf::Transformable in TransformableWrapper is nullptr when the function setRotation is called");
	m_transformable->setRotation(pos);
}

void TransformableWrapper::create(sf::Transformable* transformable, sf::Vector2f pos, sf::Vector2f scale, sf::Angle rot, Alignment alignment) noexcept
{
	ENSURE_VALID_PTR(transformable, "Precondition violated; sf::Transformable pointer in TransformableWrapper is nullptr when the function create is called");

	m_transformable = transformable;
	m_alignment = alignment;
	hide = false;

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

TextWrapper::TextWrapper(const TextWrapper& other) noexcept
	: TransformableWrapper{}, m_wrappedText{ other.m_wrappedText }
{
	this->m_alignment = other.m_alignment;
	this->hide = other.hide;

	this->m_transformable = &m_wrappedText;
}

TextWrapper::TextWrapper(TextWrapper&& other) noexcept
	: TransformableWrapper{}, m_wrappedText{ std::move(other.m_wrappedText) }
{
	std::swap(this->m_alignment, other.m_alignment);
	std::swap(this->hide,		 other.hide);

	other.m_transformable = nullptr;
	this->m_transformable = &m_wrappedText;
}

TextWrapper& TextWrapper::operator=(const TextWrapper& other) noexcept
{
	this->m_wrappedText = other.m_wrappedText;
	this->m_alignment =	  other.m_alignment;
	this->hide =		  other.hide;

	this->m_transformable = &m_wrappedText;

	return *this;
}

TextWrapper& TextWrapper::operator=(TextWrapper&& other) noexcept
{
	std::swap(this->m_wrappedText, other.m_wrappedText);
	std::swap(this->m_alignment,   other.m_alignment);
	std::swap(this->hide,		   other.hide);

	other.m_transformable = nullptr;
	this->m_transformable = &m_wrappedText;

	return *this;
}

bool TextWrapper::setFont(std::string_view name) noexcept
{
	sf::Font* font{ getFont(name) };

	if (!font)
		return false;

	m_wrappedText.setFont(*font);
	return true;
}

void TextWrapper::setCharacterSize(unsigned int size) noexcept
{
	m_wrappedText.setCharacterSize(size);
	m_wrappedText.setOrigin(computeNewOrigin(m_wrappedText.getLocalBounds(), m_alignment));
}

void TextWrapper::setColor(sf::Color color) noexcept
{
	m_wrappedText.setFillColor(color);
}

void TextWrapper::setStyle(std::uint32_t style) noexcept
{
	m_wrappedText.setStyle(style);
}

void TextWrapper::setAlignment(Alignment alignment) noexcept
{
	m_alignment = alignment;
	m_wrappedText.setOrigin(computeNewOrigin(m_wrappedText.getLocalBounds(), m_alignment));
}

void TextWrapper::createFont(std::string name, std::string_view fileName)
{
	if (getFont(name) != nullptr)
		return;

	std::ostringstream errorMessage{};
	auto optFont{ loadFontFromFile(errorMessage, fileName) };
	if (!optFont.has_value()) [[unlikely]]
		throw LoadingGraphicalResourceFailure{ errorMessage.str() };
	
	createFont(std::move(name), std::move(optFont.value()));
}

void TextWrapper::createFont(std::string name, sf::Font font) noexcept
{
	if (getFont(name) != nullptr)
		return;

	// We add the font using push_front so we know that it is at the beginning.
	// Therefore, using the function begin() we have a direct iterator pointing to it.
	s_allFonts.push_front(std::move(font));
	s_accessToFonts.insert(std::make_pair(std::move(name), s_allFonts.begin()));
}

void TextWrapper::removeFont(std::string_view name) noexcept
{
	const auto mapIterator{ s_accessToFonts.find(name) };

	if (mapIterator == s_accessToFonts.end())
		return;

	s_allFonts.erase(mapIterator->second); // First, removing the actual font.
	s_accessToFonts.erase(mapIterator); // Then, the accessing item within the map.
}

sf::Font* TextWrapper::getFont(std::string_view name) noexcept
{
	const auto mapIterator{ s_accessToFonts.find(name) };

	if (mapIterator == s_accessToFonts.end()) [[unlikely]]
		return nullptr;

	return &*mapIterator->second;
}


std::optional<sf::Font> loadFontFromFile(std::ostringstream& errorMessage, std::string_view fileName, std::string_view path) noexcept
{
	try
	{
		sf::Font font{};
		std::filesystem::path completePath{ std::filesystem::path(path) / fileName };

		if (!std::filesystem::exists(completePath)) [[unlikely]]
			throw LoadingGraphicalResourceFailure{ "Font file does not exist: " + completePath.string() + '\n' };

		if (!font.openFromFile(completePath)) [[unlikely]]
			throw LoadingGraphicalResourceFailure{ "Failed to load font from file " + completePath.string() + '\n' };

		font.setSmooth(true);
		return font;
	}
	catch (const LoadingGraphicalResourceFailure& error)
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

SpriteWrapper::SpriteWrapper(std::string_view textureName, sf::Vector2f pos, sf::Vector2f scale, sf::IntRect rect, sf::Angle rot, Alignment alignment, sf::Color color)
	: TransformableWrapper{}, m_wrappedSprite{ s_defaultTexture }, m_curTextureIndex{ 0 }, m_textures{}, m_uniqueTextures{}
{
	create(&m_wrappedSprite, pos, scale, rot, alignment);

	if (!addTexture(textureName, rect))
		throw std::invalid_argument{ "Preconditition violated; the texture " + std::string{ textureName } + " was not found when the constructor of SpriteWrapper was called" };
	
	switchToNextTexture(0);
	setColor(color);
	setAlignment(alignment);
}

SpriteWrapper::SpriteWrapper(SpriteWrapper&& other) noexcept
	: TransformableWrapper{}, m_wrappedSprite{ std::move(other.m_wrappedSprite) }, m_curTextureIndex{ other.m_curTextureIndex }, m_textures{ std::move(other.m_textures) }, m_uniqueTextures{ std::move(other.m_uniqueTextures) }
{
	std::swap(this->m_alignment, other.m_alignment);
	std::swap(this->hide,		 other.hide);

	other.m_transformable = nullptr;
	this->m_transformable = &m_wrappedSprite;
}

SpriteWrapper& SpriteWrapper::operator=(SpriteWrapper&& other) noexcept
{
	std::swap(this->m_wrappedSprite,   other.m_wrappedSprite);
	std::swap(this->m_curTextureIndex, other.m_curTextureIndex);
	std::swap(this->m_textures,		   other.m_textures);
	std::swap(this->m_uniqueTextures,  other.m_uniqueTextures);
	std::swap(this->m_alignment,	   other.m_alignment);
	std::swap(this->hide,			   other.hide);

	other.m_transformable = nullptr;
	this->m_transformable = &m_wrappedSprite;

	return *this;
}

SpriteWrapper::~SpriteWrapper() noexcept
{
	for (auto& reservedTexture : m_uniqueTextures)
	{
		auto mapAccessIterator{ s_accessToTextures.find(reservedTexture) };
		s_allUniqueTextures.erase(&*mapAccessIterator->second); // Remove the texture from the reserved map.
		s_allTextures.erase(mapAccessIterator->second); // Remove the actual texture from the list.
		s_accessToTextures.erase(mapAccessIterator); // Remove the access toward the texture from the access map.
	}

	m_textures.clear();
	m_uniqueTextures.clear();
}

void SpriteWrapper::setColor(sf::Color color) noexcept
{
	m_wrappedSprite.setColor(color);
}

void SpriteWrapper::setAlignment(Alignment alignment) noexcept
{
	m_alignment = alignment;
	m_wrappedSprite.setOrigin(computeNewOrigin(m_wrappedSprite.getLocalBounds(), m_alignment));
}

void SpriteWrapper::switchToNextTexture(long long indexOffset)
{
	const long long totalIndex{ static_cast<long long>(m_curTextureIndex) + indexOffset };
	const size_t textureSize{ m_textures.size() };
	m_curTextureIndex = ((totalIndex % textureSize) + textureSize) % textureSize; // Correctly handle negative indices and wrap around.
	
	TextureInfo& textureInfo{ m_textures[m_curTextureIndex] };
	ENSURE_VALID_PTR(textureInfo.texture, "A textureHolder within a TextureInfo was nullptr somehow when the switchToNextTexture funcion was called in SpriteWrapper");
	std::unique_ptr<sf::Texture>& newTexture{ textureInfo.texture->actualTexture }; // From texture holder

	if (newTexture == nullptr) [[unlikely]]
	{	// Not loaded yet, so we need to load it first.
		std::ostringstream errorMessage{};
		auto optTexture{ loadTextureFromFile(errorMessage, textureInfo.texture->fileName) };
		
		if (!optTexture.has_value()) [[unlikely]]
			throw LoadingGraphicalResourceFailure{ errorMessage.str() };

		newTexture = std::make_unique<sf::Texture>(std::move(optTexture.value()));
	}	
	
	if (textureInfo.displayedTexturePart == sf::IntRect{}) [[unlikely]] // If rect is 0,0 then the rect should cover the whole texture.
		textureInfo.displayedTexturePart.size = static_cast<sf::Vector2i>(newTexture->getSize());

	m_wrappedSprite.setTextureRect(textureInfo.displayedTexturePart);
	m_wrappedSprite.setTexture(*newTexture);
}

void SpriteWrapper::switchToTexture(size_t index)
{
	ENSURE_NOT_OUT_OF_RANGE(index, m_textures.size(), "Precondition violated; index is out of range for the texture vector in the function switchToTexture of SpriteWrapper");

	if (index == m_curTextureIndex) [[unlikely]]
		return;

	m_curTextureIndex = index; 
	switchToNextTexture(0);
}

void SpriteWrapper::createTexture(std::string name, std::string fileName, Reserved shared, bool loadImmediately)
{
	if (getTexture(name) != nullptr)
		return;

	TextureHolder newTexture{ .fileName = std::move(fileName) };
	newTexture.actualTexture = nullptr;

	if (loadImmediately)
	{
		std::ostringstream errorMessage{};
		auto optTexture{ loadTextureFromFile(errorMessage, newTexture.fileName) };

		if (!optTexture.has_value()) [[unlikely]]
			throw LoadingGraphicalResourceFailure{ errorMessage.str() };
		else
			newTexture.actualTexture = std::make_unique<sf::Texture>(std::move(optTexture.value()));
	}

	// We add the font using push_front so we know that it is at the beginning.
	// Therefore, using the function begin() we have a direct iterator pointing to it.
	s_allTextures.push_front(std::move(newTexture));
	s_accessToTextures.insert(std::make_pair(std::move(name), s_allTextures.begin()));

	if (shared == Reserved::Yes)
		s_allUniqueTextures[&*s_allTextures.begin()] = false;
}

void SpriteWrapper::createTexture(std::string name, sf::Texture texture, Reserved shared) noexcept
{	
	if (getTexture(name) != nullptr)
		return;

	createTexture(std::move(name), "", shared, false); // No file name provided so do not load it. 
	s_allTextures.front().actualTexture = std::make_unique<sf::Texture>(std::move(texture)); // The texture is added.
}

void SpriteWrapper::removeTexture(std::string_view name) noexcept
{
	auto mapIterator{ s_accessToTextures.find(name) };

	if (mapIterator == s_accessToTextures.end())
		return;
	
	assert(s_allUniqueTextures.find(&*mapIterator->second) == s_allUniqueTextures.end() && "Precondition violated: a reserved texture cannot be removed using the removeTexture function of SpriteWrapper");

	s_allTextures.erase(mapIterator->second); // First, removing the actual texture.
	s_accessToTextures.erase(mapIterator); // Then, the accessing item within the map.
}

sf::Texture* SpriteWrapper::getTexture(std::string_view name) noexcept
{
	auto mapIterator{ s_accessToTextures.find(name) };

	if (mapIterator == s_accessToTextures.end())
		return nullptr;

	return mapIterator->second->actualTexture.get();
}

bool SpriteWrapper::loadTexture(std::string_view name, bool failingImpliesRemoval)
{
	auto mapIterator{ s_accessToTextures.find(name) };

	if (mapIterator == s_accessToTextures.end())
		return false;

	TextureHolder* textureHolder{ &*mapIterator->second };
	
	if (textureHolder->actualTexture != nullptr)
		return true; // Already loaded.
	// Texture with no path are always loaded.

	std::ostringstream errorMessage{};
	auto optTexture{ loadTextureFromFile(errorMessage, textureHolder->fileName) };
	if (!optTexture.has_value()) [[unlikely]]
	{
		if (failingImpliesRemoval && s_allUniqueTextures.find(&*mapIterator->second) == s_allUniqueTextures.end()) 
			removeTexture(name);

		throw LoadingGraphicalResourceFailure{ errorMessage.str() };
	}

	textureHolder->actualTexture = std::make_unique<sf::Texture>(std::move(optTexture.value()));
	return true;
}

bool SpriteWrapper::unloadTexture(std::string_view name) noexcept
{
	auto mapIterator{ s_accessToTextures.find(name) };

	if (mapIterator == s_accessToTextures.end())
		return false;

	TextureHolder* textureHolder{ &*mapIterator->second };
	
	if (textureHolder->fileName == "")
		return false; // TextureHolder was not found or no file name provided: loading would be impossible afterwards.
	if (textureHolder->actualTexture == nullptr)
		return true; // Already unloaded.

	textureHolder->actualTexture = nullptr;
	return true;
}


std::optional<sf::Texture> loadTextureFromFile(std::ostringstream& errorMessage, std::string_view fileName, std::string_view path) noexcept
{
	sf::Texture texture{};

	try
	{
		std::filesystem::path completePath{ std::filesystem::path(path) / fileName };

		if (!std::filesystem::exists(completePath)) [[unlikely]]
			throw LoadingGraphicalResourceFailure{ "Texture file does not exist: " + completePath.string() + '\n' };

		if (!texture.loadFromFile(completePath)) [[unlikely]]
			throw LoadingGraphicalResourceFailure{ "Failed to load texture from file " + completePath.string() + '\n' };

		texture.setSmooth(true); // Enable smooth rendering for the font.
	}
	catch (const LoadingGraphicalResourceFailure & error)
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

} // namespace gui