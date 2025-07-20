#include "basicGUI.hpp"
#include <stdexcept>
#include <utility>

std::unordered_multimap<sf::RenderWindow*, BasicInterface*> BasicInterface::allInterfaces{};


BasicInterface::BasicInterface(sf::RenderWindow* window, unsigned int relativeScalingInit)
	: relativeScaling{ relativeScalingInit }, m_window{ window }, m_sprites{}, m_texts{}
{
	ENSURE_VALID_PTR(m_window);
	ENSURE_NOT_ZERO(m_window->getSize().x);
	ENSURE_NOT_ZERO(m_window->getSize().y);

	// Add this interface to the collection.
	allInterfaces.emplace(std::make_pair(window, this));

	// Loads the default font.
	if (TextWrapper::getFont("__default") == nullptr) // Does not exist yet.
		TextWrapper::createFont("__default", "defaultFont.ttf"); // Throws an exception if loading fails.
}


BasicInterface::BasicInterface(BasicInterface&& other) noexcept
	: relativeScaling{ other.relativeScaling }, m_window{ other.m_window }, m_sprites{ std::move(other.m_sprites) }, m_texts{ std::move(other.m_texts) }
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

	ENSURE_VALID_PTR(m_window);
	ENSURE_NOT_ZERO(m_window->getSize().x);
	ENSURE_NOT_ZERO(m_window->getSize().y);
}

BasicInterface& BasicInterface::operator=(BasicInterface&& other) noexcept
{
	if (other.m_window != m_window)
	{	// Useless if same window
		// Updating the collection. 
		auto interfaceRange{ allInterfaces.equal_range(other.m_window) }; 
		for (auto elem{ interfaceRange.first }; elem != interfaceRange.second; elem++)
		{
			if (elem->second == &other)
			{
				elem->second = this;
				break;
			}
		}

		// Not the same window, therefor iterators are completely different and no collision risk.
		interfaceRange = allInterfaces.equal_range(m_window);
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
	std::swap(m_sprites, other.m_sprites);
	std::swap(m_texts, other.m_texts);
	std::swap(relativeScaling, other.relativeScaling);

	ENSURE_VALID_PTR(m_window);
	ENSURE_NOT_ZERO(m_window->getSize().x);
	ENSURE_NOT_ZERO(m_window->getSize().y);

	return *this;
}

BasicInterface::~BasicInterface() noexcept
{
	auto interfaceRange{ allInterfaces.equal_range(m_window) }; // All interfaces associated with the resized window.
	for (auto elem{ interfaceRange.first }; elem != interfaceRange.second; elem++)
	{
		if (elem->second == this)
		{
			allInterfaces.erase(elem); // Remove the interface from the collection.
			break;
		}
	}

	m_window = nullptr;
	m_sprites.clear();
	m_texts.clear();
}

void BasicInterface::addSprite(std::string const& texture, sf::Vector2f pos, sf::Vector2f scale, sf::Angle rot, sf::IntRect rectangle, sf::Color color)
{
	m_sprites.push_back(SpriteWrapper{ texture, pos, scale });
}

void BasicInterface::addSprite(sf::Texture texture, sf::Vector2f pos, sf::Vector2f scale, sf::Angle rot, sf::IntRect rectangle, sf::Color color) noexcept
{
	m_sprites.push_back(SpriteWrapper{ std::move(texture), pos, scale }); //TODO: add the rest of the arg
}

void BasicInterface::draw() const
{
	ENSURE_VALID_PTR(m_window);
	ENSURE_NOT_ZERO(m_window->getSize().x);
	ENSURE_NOT_ZERO(m_window->getSize().y);

	for (auto const& sprite : m_sprites)
		if (!sprite.hide)
			m_window->draw(sprite.getSprite());
	for (auto const& text : m_texts)
		if (!text.hide)
			m_window->draw(text.getText());
}

void BasicInterface::windowResized(sf::RenderWindow* window, sf::Vector2f scalingFactor) noexcept
{
	float minOfCurrentWindowSize{ static_cast<float>(std::min(window->getSize().x, window->getSize().y)) };
	float minOfPreviousWindowSize{ static_cast<float>(std::min(window->getSize().x / scalingFactor.x, window->getSize().y / scalingFactor.y)) };
	float a = minOfCurrentWindowSize / minOfPreviousWindowSize;
	sf::Vector2f minScalingFactor{ a, a };
	auto interfaceRange{ allInterfaces.equal_range(window) }; // All interfaces associated with the resized window.
	for (auto elem{ interfaceRange.first }; elem != interfaceRange.second; elem++)
	{
		auto* curInterface{ elem->second };

		// Updating texts.
		for (auto& text : curInterface->m_texts)
			text.resized(scalingFactor, a);

		// Updating sprites.
		for (int i{ 1 }; i < curInterface->m_sprites.size(); ++i) // Avoiding the background by skipping index 0.
			curInterface->m_sprites[i].resized(scalingFactor, a);

		// Background is always the first sprite, so it is at index 0.
		curInterface->m_sprites[0].setPosition(sf::Vector2f{ window->getSize().x / 2.f, window->getSize().y / 2.f });
		if (curInterface->m_sprites[0].getCurrentTextureIndex() == 0) // Only if the default background is used.	
			curInterface->m_sprites[0].scale(scalingFactor); // The default background is a solid color, so we can scale it without issues.
	}
}