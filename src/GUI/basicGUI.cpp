#include <SFML/Graphics.hpp>
#include "../Exceptions.hpp"
#include "../GUITexturesLoader.hpp"
#include "basicGUI.hpp"

sf::Font FixedGraphicalInterface::TextWrapper::s_font{};
std::string const FixedGraphicalInterface::ressourcePath{ "../res/" };
std::list<sf::Texture> FixedGraphicalInterface::SpriteWrapper::s_sharedTextures{};
std::unordered_multimap<sf::RenderWindow*, FixedGraphicalInterface*> FixedGraphicalInterface::allInterfaces{};
std::unordered_map<std::string, std::list<sf::Texture>::iterator> FixedGraphicalInterface::SpriteWrapper::s_accessingSharedTexture{};




std::optional<std::string> FixedGraphicalInterface::TextWrapper::loadFont() noexcept
{
	if (s_font.getInfo().family != "")
		return std::nullopt; // Font already loaded.

	try
	{
		s_font = loadDefaultFont(); // Load the default font.
	}
	catch (LoadingGUIRessourceFailure const& error)
	{
		return error.what();
	}

	return std::nullopt;
}

void FixedGraphicalInterface::TextWrapper::computeNewOrigin() noexcept
{
	sf::Vector2f originTopLeft{ 0, 0 };
	sf::Vector2f originBottomRight{ getLocalBounds().size };
	sf::Vector2f origin{ getLocalBounds().getCenter() }; // Center origin by default.

	uint8_t value = static_cast<uint8_t>(m_alignment);
	if ((value >> 3) & 1)
		origin.x = originTopLeft.x; // Left side.
	else if ((value >> 2) & 1)
		origin.x = originBottomRight.x; // Right side.

	if ((value >> 1) & 1)
		origin.y = originTopLeft.y; // Top side.
	else if ((value >> 0) & 1)
		origin.y = originBottomRight.y; // Bottom side.

	setOrigin(origin);
}

FixedGraphicalInterface::SpriteWrapper::SpriteWrapper(std::string const& sharedTexture, sf::Vector2f pos, sf::Vector2f scale, sf::Angle rot, sf::IntRect rectangle, sf::Color color)
	: sf::Sprite{ *s_accessingSharedTexture.at(sharedTexture) }, m_curTextureIndex{ 0 }, hide{ false }
{
	m_textures.push_back(&(*s_accessingSharedTexture.at(sharedTexture))); // Add the texture to the vector of textures.

	if (rectangle.size == sf::Vector2i{ 0, 0 }) [[likely]]
		setTextureRect(sf::IntRect{ sf::Vector2i{}, static_cast<sf::Vector2i>(getTexture().getSize()) });

	setPosition(pos);
	setScale(scale);
	setRotation(rot);
	setColor(color);
	setOrigin(getLocalBounds().getCenter());
}

FixedGraphicalInterface::SpriteWrapper::SpriteWrapper(sf::Texture texture, sf::Vector2f pos, sf::Vector2f scale, sf::Angle rot, sf::IntRect rectangle, sf::Color color) noexcept
	: sf::Sprite{ texture }, m_curTextureIndex{ 0 }, hide{ false }
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
	setOrigin(getLocalBounds().getCenter());
}

void FixedGraphicalInterface::SpriteWrapper::addSharedTexture(std::string const& identifier, sf::Texture textures) noexcept
{
	s_sharedTextures.push_front(std::move(textures)); // Add the texture to the static collection of textures.
	s_accessingSharedTexture[identifier] = s_sharedTextures.begin(); // Map the identifier to the texture in the static collection.
}

void FixedGraphicalInterface::SpriteWrapper::removeSharedTexture(std::string const& identifier) noexcept
{
	s_sharedTextures.erase(s_accessingSharedTexture.at(identifier)); // Remove the texture from the static collection of textures.
	s_accessingSharedTexture.erase(identifier); // Remove the identifier from the static collection of textures.
}


FixedGraphicalInterface::FixedGraphicalInterface(FixedGraphicalInterface&& other) noexcept
	: m_window{ other.m_window }, m_sprites{ std::move(other.m_sprites) }, m_texts{ std::move(other.m_texts) }
{
	// Update the static collection of interfaces to point to the new object.
	auto interfaceRange = allInterfaces.equal_range(m_window);
	for (auto it = interfaceRange.first; it != interfaceRange.second; ++it)
	{
		if (it->second == &other)
		{
			it->second = this;
			break;
		}
	}

	// Leave the moved-from object in a valid state
	other.m_window = nullptr;
}

FixedGraphicalInterface& FixedGraphicalInterface::operator=(FixedGraphicalInterface&& other) noexcept
{
	std::swap(m_sprites, other.m_sprites);
	std::swap(m_texts, other.m_texts);

	if (m_window == other.m_window)
		return *this; // No need to update the GUI map.

	// Update the static collection of interfaces to update to the new memory address.
	auto interfaceRange = allInterfaces.equal_range(m_window);
	for (auto it = interfaceRange.first; it != interfaceRange.second;)
	{
		if (it->second == this)
		{	// Remove the interface from the collection as it is being moved.
			it = allInterfaces.erase(it);
		}
		else if (it->second == &other)
		{	// Update the moved object's to this object.
			it->second = this;
			it++;
		}
		else
		{
			it++;
		}
	}

	m_window = other.m_window;
	other.m_window = nullptr; // Leave the moved-from object in a valid state

	return *this;
}

FixedGraphicalInterface::~FixedGraphicalInterface() noexcept
{
	auto interfaceRange{ allInterfaces.equal_range(m_window) }; // All interfaces associated with the resized window.

	for (auto elem{ interfaceRange.first }; elem != interfaceRange.second;)
	{
		if (elem->second == this)
			elem = allInterfaces.erase(elem); // Remove the interface from the collection.
		else
			elem++;
	}
}

FixedGraphicalInterface::FixedGraphicalInterface(sf::RenderWindow* window, std::string const& backgroundFileName)
	: m_window{ window }, m_sprites{}, m_texts{}
{
	if (!m_window || m_window->getSize() == sf::Vector2u{ 0, 0 })
		throw std::logic_error{ "Window of an interface is nullptr or invalid." };

	// Add this interface to the collection.
	allInterfaces.emplace(std::make_pair(window, this));

	// Loads the font.
	auto errorFont{ TextWrapper::loadFont() };
	if (errorFont.has_value())
		throw LoadingGUIRessourceFailure{ errorFont.value() };

	// Loads the background. n  
	auto errorBackground{ loadBackground(backgroundFileName) };
	if (errorBackground.has_value())
		throw LoadingGUIRessourceFailure{ errorBackground.value() };
}

void FixedGraphicalInterface::addSprite(std::string const& texture, sf::Vector2f pos, sf::Vector2f scale, sf::Angle rot, sf::IntRect rectangle, sf::Color color)
{
	m_sprites.push_back(SpriteWrapper{ texture, pos, scale, rot, rectangle, color });
}

void FixedGraphicalInterface::addSprite(sf::Texture texture, sf::Vector2f pos, sf::Vector2f scale, sf::Angle rot, sf::IntRect rectangle, sf::Color color) noexcept
{
	m_sprites.push_back(SpriteWrapper{ std::move(texture), pos, scale, rot, rectangle, color });
}

void FixedGraphicalInterface::draw() const
{
	if (!m_window) [[unlikely]]
		throw std::logic_error{ "Window of an interface is nullptr." };

	for (auto const& sprite : m_sprites)
		if (!sprite.hide)
			m_window->draw(sprite.getSprite());
	for (auto const& text : m_texts)
		if (!text.hide)
			m_window->draw(text.getText());
}

void FixedGraphicalInterface::windowResized(sf::RenderWindow* window, sf::Vector2f scalingFactor) noexcept
{
	float minOfCurrentWindowSize{ static_cast<float>(std::min(window->getSize().x, window->getSize().y)) };
	float minOfPreviousWindowSize{ static_cast<float>(std::min(window->getSize().x / scalingFactor.x, window->getSize().y / scalingFactor.y)) };
	sf::Vector2f minScalingFactor{ minOfCurrentWindowSize / minOfPreviousWindowSize, minOfCurrentWindowSize / minOfPreviousWindowSize };

	auto interfaceRange{ allInterfaces.equal_range(window) }; // All interfaces associated with the resized window.
	for (auto elem{ interfaceRange.first }; elem != interfaceRange.second; elem++)
	{
		auto* curInterface{ elem->second };

		// Updating texts.
		for (auto& text : curInterface->m_texts)
			text.windowResized(scalingFactor, minScalingFactor);

		// Updating sprites.
		for (int i{ 1 }; i < curInterface->m_sprites.size(); ++i) // Avoiding the background by skipping index 0.
			curInterface->m_sprites[i].windowResized(scalingFactor, minScalingFactor);

		// Background is always the first sprite, so it is at index 0.
		curInterface->m_sprites[0].setPosition(sf::Vector2f{ window->getSize().x / 2.f, window->getSize().y / 2.f });
		if (curInterface->m_sprites[0].getCurrentTextureIndex() == 0) // Only if the default background is used.	
			curInterface->m_sprites[0].scale(scalingFactor); // The default background is a solid color, so we can scale it without issues.
	}
}

std::optional<std::string> FixedGraphicalInterface::loadBackground(std::string const& backgroundFileName) noexcept
{
	if (!SpriteWrapper::checkIfTextureExists("__defaultTextureForBackground__")) [[unlikely]]
	{
		sf::Image backgroundImage{ sf::Vector2u{ 1u, 1u }, sf::Color{ 20, 20, 20 } };
		sf::Texture defaultTexture{ std::move(backgroundImage) };
		defaultTexture.setRepeated(true);

		SpriteWrapper::addSharedTexture("__defaultTextureForBackground__", std::move(defaultTexture)); // Adds the texture to the static collection of textures.
	}

	addSprite("__defaultTextureForBackground__", m_window->getView().getCenter(), sf::Vector2f{ 1.f, 1.f }); // Adds the background sprite with a solid color.

	// Creating the background.
	if (backgroundFileName.empty())
	{
		m_sprites[0].scale(static_cast<sf::Vector2f>(m_window->getSize()));
		return std::nullopt; // No need to load the background if it is empty.
	}

	try
	{
		std::string path{ ressourcePath + backgroundFileName };

		sf::Texture customTextureBackground{};
		if (!customTextureBackground.loadFromFile(path))
			throw LoadingGUIRessourceFailure{ "Failed to load background at: " + path };
		customTextureBackground.setSmooth(true);

		m_sprites[0].addTexture(std::move(customTextureBackground)); // Adds the texture to the sprite.
		m_sprites[0].switchToNextTexture();
	}
	catch (LoadingGUIRessourceFailure const& error)
	{
		std::ostringstream oss{};

		oss << error.what() << "\n";
		oss << "error: the default background is displayed instead\n\n";

		m_sprites[0].scale(static_cast<sf::Vector2f>(m_window->getSize()));
		return std::make_optional<std::string>(oss.str());
	}

	return std::nullopt;
}
