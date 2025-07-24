#include "BasicInterface.hpp"
#include <utility>

namespace gui
{

std::unordered_multimap<sf::RenderWindow*, BasicInterface*> BasicInterface::s_allInterfaces{};

BasicInterface::BasicInterface(sf::RenderWindow* window, unsigned int relativeScalingDefinition)
	: m_window{ window }, m_texts{}, m_sprites{}, m_relativeScalingDefinition{ relativeScalingDefinition }
{
	ENSURE_SFML_WINDOW_VALIDITY(m_window);
	ENSURE_NOT_ZERO(relativeScalingDefinition);

	// Add this interface to the collection.
	s_allInterfaces.emplace(std::make_pair(window, this));

	// Loads the default font.
	if (TextWrapper::getFont("__default") == nullptr) // Does not exist yet.
		TextWrapper::createFont("__default", "defaultFont.ttf"); // Throws an exception if loading fails.
} 

BasicInterface::BasicInterface(BasicInterface&& other) noexcept
	: m_window{ other.m_window }, m_texts{ std::move(other.m_texts) }, m_sprites{ std::move(other.m_sprites) }, m_relativeScalingDefinition{ other.m_relativeScalingDefinition }
{
	// Update the static collection of interfaces to point to the new object.
	auto interfaceRange = s_allInterfaces.equal_range(m_window);
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

	ENSURE_SFML_WINDOW_VALIDITY(m_window);
}

BasicInterface& BasicInterface::operator=(BasicInterface&& other) noexcept
{
	if (other.m_window != m_window)
	{
		// Updating the collection. 
		auto interfaceRange{ s_allInterfaces.equal_range(other.m_window) }; 
		for (auto elem{ interfaceRange.first }; elem != interfaceRange.second; elem++)
		{
			if (elem->second == &other)
			{
				elem->second = this;
				break;
			}
		}

		// Not the same window, therefore the iterators are completely different and no collision risk.
		interfaceRange = s_allInterfaces.equal_range(m_window);
		for (auto elem{ interfaceRange.first }; elem != interfaceRange.second; elem++)
		{
			if (elem->second == this)
			{
				elem->second = &other; // Remove the interface from the collection.
				break;
			}
		}
	}

	std::swap(m_window, other.m_window);
	std::swap(m_texts, other.m_texts);
	std::swap(m_sprites, other.m_sprites);
	std::swap(m_relativeScalingDefinition, other.m_relativeScalingDefinition);

	ENSURE_SFML_WINDOW_VALIDITY(m_window);

	return *this;
}

BasicInterface::~BasicInterface() noexcept
{
	auto interfaceRange{ s_allInterfaces.equal_range(m_window) }; // All interfaces associated with the resized window.
	for (auto elem{ interfaceRange.first }; elem != interfaceRange.second; elem++)
	{
		if (elem->second == this)
		{
			s_allInterfaces.erase(elem); // Remove the interface from the collection.
			break;
		}
	}

	m_window = nullptr;
	m_sprites.clear();
	m_texts.clear();
}

void BasicInterface::addSprite(const std::string& textureName, sf::Vector2f pos, sf::Vector2f scale, sf::IntRect rect, sf::Angle rot, Alignment alignment, sf::Color color)
{
	ENSURE_SFML_WINDOW_VALIDITY(m_window);
	float relativeScalingValue{ std::min(m_window->getSize().x, m_window->getSize().y) / static_cast<float>(m_relativeScalingDefinition) };

	m_sprites.push_back(SpriteWrapper{ textureName, rect, pos, scale * relativeScalingValue, rot, alignment, color });
}

void BasicInterface::addSprite(sf::Texture texture, sf::Vector2f pos, sf::Vector2f scale, sf::IntRect rect, sf::Angle rot, Alignment alignment, sf::Color color) noexcept
{
	// We need to craft a unique texture name to avoid collision with other existing or futur textures.
	// Since the user didn't want a specific name, he lost the capacity to retrieve the texture. 

	std::ostringstream craftUniqueName{};
	craftUniqueName << "_" << m_sprites.size();
	craftUniqueName << "_" << reinterpret_cast<size_t>(this);
	craftUniqueName << "_" << m_texts.size();

	while (SpriteWrapper::getTexture(craftUniqueName.str()) != nullptr)
		craftUniqueName << "_"; // If this exact name was given, we add an underscore until it is available.

	SpriteWrapper::createTexture(craftUniqueName.str(), std::move(texture), SpriteWrapper::Reserved::Yes);
	addSprite(craftUniqueName.str(), pos, scale, rect, rot, alignment, color); 
}

void BasicInterface::draw() const noexcept
{
	ENSURE_SFML_WINDOW_VALIDITY(m_window);

	for (auto const& sprite : m_sprites)
		if (!sprite.hide)
			m_window->draw(sprite.getSprite());
	for (auto const& text : m_texts)
		if (!text.hide)
			m_window->draw(text.getText());
}

void BasicInterface::proportionKeeper(sf::RenderWindow* resizedWindow, sf::Vector2f windowScaleFactor, float relativeMinAxisScale) noexcept
{	
	ENSURE_VALID_PTR(resizedWindow);
	ENSURE_NOT_ZERO(windowScaleFactor.x);
	ENSURE_NOT_ZERO(windowScaleFactor.y);
	ENSURE_NOT_ZERO(relativeMinAxisScale);

	auto interfaceRange{ s_allInterfaces.equal_range(resizedWindow) }; // All interfaces associated with the resized window.
	for (auto elem{ interfaceRange.first }; elem != interfaceRange.second; elem++)
	{
		auto* curInterface{ elem->second };

		curInterface->m_relativeScalingDefinition *= relativeMinAxisScale;

		// Updating texts.
		for (auto& text : curInterface->m_texts)
			text.resized(windowScaleFactor, relativeMinAxisScale);

		// Updating sprites.
		for (auto& sprite : curInterface->m_sprites)
			sprite.resized(windowScaleFactor, relativeMinAxisScale);
	}
}

} // gui namespace