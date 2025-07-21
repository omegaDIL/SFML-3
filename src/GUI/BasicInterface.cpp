#include "BasicInterface.hpp"
#include <stdexcept>
#include <utility>

namespace gui
{

std::unordered_multimap<sf::RenderWindow*, BasicInterface*> BasicInterface::s_allInterfaces{};

BasicInterface::BasicInterface(sf::RenderWindow* window, unsigned int relativeScaling)
	: m_window{ window }, m_texts{}, m_sprites{}, m_relativeScaling{ relativeScaling }
{
	ENSURE_SFML_WINDOW_VALIDITY(m_window);

	// Add this interface to the collection.
	s_allInterfaces.emplace(std::make_pair(window, this));

	// Loads the default font.
	if (TextWrapper::getFont("__default") == nullptr) // Does not exist yet.
		TextWrapper::createFont("__default", "defaultFont.ttf"); // Throws an exception if loading fails.
}

BasicInterface::BasicInterface(BasicInterface&& other) noexcept
	: m_window{ other.m_window }, m_texts{ std::move(other.m_texts) }, m_sprites{ std::move(other.m_sprites) }, m_relativeScaling{ other.m_relativeScaling }
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
	std::swap(m_relativeScaling, other.m_relativeScaling);

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
	float relativeScalingValue{ std::min(m_window->getSize().x / m_relativeScaling, m_window->getSize().y / m_relativeScaling) };

	m_sprites.push_back(SpriteWrapper{ textureName, rect, pos, scale * relativeScalingValue, rot, alignment, color });
}

void BasicInterface::addSprite(sf::Texture texture, sf::Vector2f pos, sf::Vector2f scale, sf::IntRect rect, sf::Angle rot, Alignment alignment, sf::Color color) noexcept
{
	std::ostringstream craftUniqueName{}; // Avoid collision with other alias
	craftUniqueName << "_" << m_sprites.size();
	craftUniqueName << "_" << reinterpret_cast<size_t>(this);
	craftUniqueName << "_" << m_texts.size();

	while (SpriteWrapper::getTexture(craftUniqueName.str()) != nullptr)
		craftUniqueName << "_"; // If this exact name was given, we add an underscore until it is available.

	// Will not be findable.
	SpriteWrapper::createTexture(craftUniqueName.str(), std::move(texture), SpriteWrapper::Reserved::Yes);
	addSprite(craftUniqueName.str(), pos, scale, rect, rot, alignment, color); // Will scale with the right proportion
}

void BasicInterface::draw() const
{
	ENSURE_SFML_WINDOW_VALIDITY(m_window);

	for (auto const& sprite : m_sprites)
		if (!sprite.hide)
			m_window->draw(sprite.getSprite());
	for (auto const& text : m_texts)
		if (!text.hide)
			m_window->draw(text.getText());
}

void BasicInterface::windowResized(sf::RenderWindow* window) noexcept
{
	ENSURE_VALID_PTR(window);
	
	const sf::Vector2u maxSize{ sf::VideoMode::getDesktopMode().size };
	const sf::Vector2f previsousSize{ window->getView().getSize() }; // View size is the inner size of the window.
	sf::Vector2u newSize{ window->getSize() }; // window size returns the visual size of the window.

	// Windows (OS) does not like windows (app) that are larger than the screen definition.
	if (newSize.x > maxSize.x)
		newSize.x = maxSize.x;
	if (newSize.y > maxSize.y)
		newSize.y = maxSize.y;

	const sf::Vector2f scalingFactor{ newSize.x / previsousSize.x, newSize.y / previsousSize.y };
	float minSize;

	sf::View view{ window->getView() };
	view.setSize(static_cast<sf::Vector2f>(newSize));
	view.setCenter(sf::Vector2f{ newSize.x / 2.f, newSize.y / 2.f });

	window->setView(view);
	window->setSize(newSize); // Avoid having a window that is larger than the screen.

	auto interfaceRange{ s_allInterfaces.equal_range(window) }; // All interfaces associated with the resized window.
	for (auto elem{ interfaceRange.first }; elem != interfaceRange.second; elem++)
	{
		auto* curInterface{ elem->second };

		// Updating texts.
		for (auto& text : curInterface->m_texts)
			text.resized(scalingFactor, a);

		// Updating sprites.
		for (auto& sprite : curInterface->m_sprites)
			sprite.resized(scalingFactor, a);
	}
}


void handleEventResize(sf::RenderWindow* window) noexcept
{
	// With SFML, when a window is resized, it occupies the corresponding area of the screen which is 
	// a certain number of pixel. However, the inner size of the window, that is to say the number of 
	// pixel it was created with, is not updated. Therefore, it still displays the same view, with 
	// distorded pixels to match the new size of the window. Thus, we need to manually update the inner
	// size.

	sf::Vector2u const maxSize{ sf::VideoMode::getDesktopMode().size };
	sf::Vector2u newSize{ window->getSize() }; // window size returns the visual size of the window.

	// Windows (OS) does not like windows (app) that are larger than the screen definition.
	if (newSize.x > maxSize.x)
		newSize.x = maxSize.x;
	if (newSize.y > maxSize.y)
		newSize.y = maxSize.y;

	sf::Vector2f const previsousSize{ window->getView().getSize() }; // View size is the inner size of the window.
	sf::Vector2f const scalingFactor{ newSize.x / previsousSize.x, newSize.y / previsousSize.y };

	sf::View view{ window->getView() };
	view.setSize(static_cast<sf::Vector2f>(newSize));
	view.setCenter(sf::Vector2f{ newSize.x / 2.f, newSize.y / 2.f });

	window->setView(view);
	window->setSize(newSize); // Avoid having a window that is larger than the screen.
	BasicInterface::windowResized(window);
}

} // gui namespace