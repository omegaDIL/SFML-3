#include "BasicInterface.hpp"
#include <utility>

namespace gui
{

BasicInterface::BasicInterface(sf::RenderWindow* window, unsigned int relativeScalingDefinition)
	: m_window{ window }, m_texts{}, m_sprites{}, m_relativeScalingDefinition{ relativeScalingDefinition }
{
	ENSURE_SFML_WINDOW_VALIDITY(m_window, "Precondition violated; the window is invalid in the constructor of BasicInterface");

	// Add this interface to the collection.
	s_allInterfaces.emplace(std::make_pair(window, this));

	// Loads the default font.
	if (TextWrapper::getFont("__default") == nullptr) // Does not exist yet.
		TextWrapper::createFont("__default", "defaultFont.ttf"); // Throws an exception if loading fails.
} 

BasicInterface::BasicInterface(BasicInterface&& other) noexcept
	: m_window{ other.m_window }, m_texts{ std::move(other.m_texts) }, m_sprites{ std::move(other.m_sprites) }, m_relativeScalingDefinition{ other.m_relativeScalingDefinition }
{
	auto interfaceRange{ s_allInterfaces.equal_range(other.m_window) };
	for (auto it{ interfaceRange.first }; it != interfaceRange.second; it++)
	{
		if (it->second == &other)
		{
			it->second = this; // Update the pointer to point to this instance instead of the moved-from one.
			break;
		}
	}

	// The moved-from object should not be in the collection anymore.

	// Leave the moved-from object in a valid state
	other.m_window = nullptr;
}

BasicInterface& BasicInterface::operator=(BasicInterface&& other) noexcept
{
	// If the two interfaces are associated with different windows,
	// we need to update the global static collection s_allInterfaces accordingly.
	if (other.m_window != m_window)
	{
		// 1. Reassign the mapping of `other` in s_allInterfaces:
		//    - We're about to move `other` into `*this`, so the pointer to `other`
		//      in s_allInterfaces should now point to `this`.
		auto interfaceRange{ s_allInterfaces.equal_range(other.m_window) };
		for (auto it{ interfaceRange.first }; it != interfaceRange.second; ++it)
		{
			if (it->second == &other)
			{
				it->second = this;
				break;
			}
		}

		// As they have different windows, we do not need to worry about collision between those two loops

		// 2. Reassign the mapping of `this` in s_allInterfaces:
		//    - After the move, `other` will hold the old state of `*this`.
		//      So the entry in s_allInterfaces that previously pointed to `this`
		//      under m_window should now point to `other`.
		interfaceRange = s_allInterfaces.equal_range(this->m_window);
		for (auto it{ interfaceRange.first }; it != interfaceRange.second; ++it)
		{
			if (it->second == this)
			{
				it->second = &other;
				break;
			}
		}
	}

	// Else: If both `this` and `other` share the same window, we don’t update s_allInterfaces.
	// Why?
	// - Each pointer (`this` and `other`) stays associated with the same window after the swap.
	// - We’re just exchanging internal data; the identity (address) of the interface tied to the window doesn’t change.
	// - No need to touch the mapping — it's still valid and consistent.

	// Swap the internal state between *this and other.
	// This includes all relevant members to fully transfer ownership.
	std::swap(this->m_window, other.m_window);
	std::swap(this->m_texts, other.m_texts);
	std::swap(this->m_sprites, other.m_sprites);
	std::swap(this->m_relativeScalingDefinition, other.m_relativeScalingDefinition);

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
	ENSURE_SFML_WINDOW_VALIDITY(m_window, "The window is invalid in the function addSprite of BasicInterface");

	float relativeScalingValue{ 1.f };
	if (m_relativeScalingDefinition != 0)
		relativeScalingValue *= std::min(m_window->getSize().x, m_window->getSize().y) / static_cast<float>(m_relativeScalingDefinition);

	SpriteWrapper newSprite{ textureName, pos, scale * relativeScalingValue, rect, rot, alignment, color };
	m_sprites.push_back(std::move(newSprite));
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
	ENSURE_SFML_WINDOW_VALIDITY(m_window, "The window is invalid in the function draw of BasicInterface");

	for (auto const& sprite : m_sprites)
		if (!sprite.hide)
			m_window->draw(sprite.getSprite());

	for (auto const& text : m_texts)
		if (!text.hide)
			m_window->draw(text.getText());
}

void BasicInterface::proportionKeeper(sf::RenderWindow* resizedWindow, sf::Vector2f scaleFactor, float relativeMinAxisScale) noexcept
{	
	ENSURE_SFML_WINDOW_VALIDITY(resizedWindow, "Precondition violated; The window is invalid in the function proportionKeeper of BasicInterface");
	ENSURE_NOT_ZERO(relativeMinAxisScale, "Precondition violated; relativeMinAxisScale is equal to 0 in proportionKeeper of BasicInterface");

	sf::Vector2f minScaling2f{ relativeMinAxisScale, relativeMinAxisScale };

	auto interfaceRange{ s_allInterfaces.equal_range(resizedWindow) }; // All interfaces associated with the resized window.
	for (auto elem{ interfaceRange.first }; elem != interfaceRange.second; elem++)
	{
		auto* curInterface{ elem->second };

		if (curInterface->m_relativeScalingDefinition == 0)
			continue; // No scaling definition, so no need to scale.

		// Updating texts.
		for (auto& text : curInterface->m_texts)
		{
			text.scale(minScaling2f);

			const auto pos{ text.getText().getPosition() };
			text.setPosition(sf::Vector2f{ pos.x * scaleFactor.x, pos.y * scaleFactor.y }); // Update position to match the new scale.
		}

		// Updating sprites.
		for (auto& sprite : curInterface->m_sprites)
		{
			sprite.scale(minScaling2f);

			const auto pos{ sprite.getSprite().getPosition() };
			sprite.setPosition(sf::Vector2f{ pos.x * scaleFactor.x, pos.y * scaleFactor.y }); // Update position to match the new scale.
		}
	}
}

} // gui namespace