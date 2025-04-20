#include <SFML/Graphics.hpp>
#include <unordered_map>
#include <string>
#include <vector>
#include <utility>
#include <span>
#include <memory>
#include <ranges>
#include <valarray>
#include <algorithm>
#include <initializer_list>
#include <stdexcept>
#include <functional>
#include <sstream>
#include <iostream>
#include <iomanip>
#include "GUI.hpp"
#include "GUITexturesLoader.hpp"
#include "Exceptions.hpp"

sf::Font GraphicalFixedInterface::TextWrapper::m_font{};
std::string GraphicalFixedInterface::ressourcePath{ "../res/" };
std::unordered_multimap<sf::RenderWindow*, GraphicalFixedInterface*> GraphicalFixedInterface::allInterfaces{};


GraphicalFixedInterface::GraphicalFixedInterface(sf::RenderWindow* window) noexcept
	: m_window{ window }, m_sprites{}, m_texts{}
{	
	// Add this interface to the collection.
	allInterfaces.emplace(std::make_pair(window, this));

	// Creating the background.
	sf::RectangleShape background{ static_cast<sf::Vector2f>(window->getSize()) };
	background.setFillColor(sf::Color{ 20, 20, 20 });
	background.setOrigin(window->getView().getCenter()); // Putting the origin at the center 
	background.setPosition(window->getView().getCenter());
	addShape(&background, false); // Adds the background to the interface.
}

std::optional<std::string> GraphicalFixedInterface::create(std::string const& fileName)
{
	if (!m_window)
		throw std::logic_error{ "Window of an interface is nullptr." };

	std::ostringstream oss{};

	// Loads the font.
	auto error{ TextWrapper::loadFont() };
	if (error.has_value())
		oss << error.value() ;

	// If default background is used, no need to load its texture.
	if (fileName.empty())
		return (oss.str().empty()) ? std::nullopt : std::make_optional<std::string>(oss.str());

	// Loads the background.
	try
	{
		std::string path{ ressourcePath + fileName };

		sf::Texture textureBackground{};
		if (!textureBackground.loadFromFile(path))
			throw LoadingGUIRessourceFailure{ "Failed to load background at: " + path };

		textureBackground.setSmooth(true);
		m_sprites[0].second = std::move(textureBackground); // The first sprite is the background.
		m_sprites[0].first = sf::Sprite{ m_sprites[0].second };
	}
	catch (LoadingGUIRessourceFailure const& error)
	{
		oss << error.what() << "\n";
		oss << "error: the default background is displayed instead\n\n";
		
		return std::make_optional<std::string>(oss.str());
	}	
	
	return (oss.str().empty()) ? std::nullopt : std::make_optional<std::string>(oss.str());
}

GraphicalFixedInterface::GraphicalFixedInterface(GraphicalFixedInterface&& other) noexcept
	: m_window{ other.m_window }, m_sprites{ std::move(other.m_sprites) }, m_texts{ std::move(other.m_texts) }
{
	// Update the static collection of interfaces to point to the new object
	auto interfaceRange = allInterfaces.equal_range(m_window);
	for (auto it = interfaceRange.first; it != interfaceRange.second; ++it)
	{
		if (it->second == &other)
		{
			it->second = this;
			break;
		}
	}
	
	for (auto& sprite : m_sprites)
		sprite.first.setTexture(sprite.second);
	
	// Leave the moved-from object in a valid state
	other.m_window = nullptr;
}

GraphicalFixedInterface& GraphicalFixedInterface::operator=(GraphicalFixedInterface&& other) noexcept
{
	if (this != &other) // Avoid self-assignment
	{
		// Remove this object from the static collection if it already exists
		auto interfaceRange = allInterfaces.equal_range(m_window);
		for (auto it = interfaceRange.first; it != interfaceRange.second;)
		{
			if (it->second == this)
				it = allInterfaces.erase(it);
			else
				++it;
		}

		// Transfer ownership of resources
		m_window = other.m_window;
		m_sprites = std::move(other.m_sprites);
		m_texts = std::move(other.m_texts);

		// Update the static collection of interfaces to point to the new object
		interfaceRange = allInterfaces.equal_range(m_window);
		for (auto it = interfaceRange.first; it != interfaceRange.second; ++it)
		{
			if (it->second == &other)
			{
				it->second = this;
				break;
			}
		}

		// Reset textures for sprites to ensure they are valid
		for (auto& sprite : m_sprites)
			sprite.first.setTexture(sprite.second);
		
		// Leave the moved-from object in a valid state
		other.m_window = nullptr;
	}
	return *this;
}

GraphicalFixedInterface::~GraphicalFixedInterface() noexcept
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

void GraphicalFixedInterface::addSprite(sf::Sprite sprite, sf::Texture texture) noexcept
{
	m_sprites.push_back(std::make_pair(std::move(sprite), std::move(texture)));

	// The pointer to the texture becomes invalid because we added something to the vector.
	resetTextureForSprites();
}

void GraphicalFixedInterface::addShape(sf::Shape* shape, bool smooth) noexcept
{
	std::unique_ptr<sf::Sprite> sprite{ nullptr };
	std::unique_ptr<sf::Texture> texture{ nullptr };

	convertShapeToSprite(shape, sprite, texture, smooth);
	addSprite(std::move(*sprite), std::move(*texture));
}

void GraphicalFixedInterface::draw() const
{
	if (!m_window)
		throw std::logic_error{ "Window of an interface is nullptr." };

	for (auto const& sprite : m_sprites)
		m_window->draw(sprite.first);
	for (auto const& text : m_texts)
		m_window->draw(text.getText());
}

void GraphicalFixedInterface::windowResized(sf::RenderWindow* window, sf::Vector2f scalingFactor) noexcept
{
	float minOfCurrentWindowSize{ static_cast<float>(std::min(window->getSize().x, window->getSize().y)) };
	float minOfPreviousWindowSize{ static_cast<float>(std::min(window->getSize().x / scalingFactor.x, window->getSize().y / scalingFactor.y)) };
	sf::Vector2f minScalingFactor{ minOfCurrentWindowSize / minOfPreviousWindowSize, minOfCurrentWindowSize / minOfPreviousWindowSize };

	auto interfaceRange{ allInterfaces.equal_range(window) }; // All interfaces associated with the resized window.
	for (auto elem{ interfaceRange.first }; elem != interfaceRange.second; elem++)
	{
		auto* curInterface{ elem->second };

		// The background is the first sprite.
		curInterface->m_sprites[0].first.setPosition(sf::Vector2f{ window->getSize().x / 2.f, window->getSize().y / 2.f });
		//curInterface->m_sprites[0].first.setScale(static_cast<sf::Vector2f>(window->getSize()));

		for (int j{ 1 }; j < curInterface->m_sprites.size(); j++)
		{	// Avoiding the background by skipping index 0.
			sf::Sprite& curSprite{ curInterface->m_sprites[j].first };

			curSprite.scale(minScalingFactor);
			curSprite.setPosition(sf::Vector2f{ curSprite.getPosition().x * scalingFactor.x, curSprite.getPosition().y * scalingFactor.y });
		}

		for (auto& text : curInterface->m_texts)
			text.windowResized(scalingFactor, minScalingFactor);
	}
}

void GraphicalFixedInterface::resetTextureForSprites() noexcept
{
	for (auto& sprite : m_sprites)
		sprite.first.setTexture(sprite.second);
}

std::optional<std::string> GraphicalFixedInterface::TextWrapper::loadFont() noexcept
{
	if (m_font.getInfo().family != "")
		return std::nullopt; // Font already loaded.

	try
	{
		std::string path{ GraphicalFixedInterface::ressourcePath + "font.ttf" };

		if (!m_font.openFromFile(path))
			throw LoadingGUIRessourceFailure{ "Failed to load font at: " + path };

		m_font.setSmooth(true);
	}
	catch (LoadingGUIRessourceFailure const& error)
	{
		std::ostringstream message{ error.what() };
		message << "Fatal error: No text can be displayed\n\n";
		return message.str();
	}

	return std::nullopt;
}


void GraphicalDynamicInterface::addDynamicSprite(std::string const& identifier, sf::Sprite sprite, sf::Texture texture) noexcept
{
	if (m_dynamicSpritesIds.find(identifier) != m_dynamicSpritesIds.end())
		return;

	addSprite(std::move(sprite), std::move(texture));
	m_dynamicSpritesIds[identifier] = m_sprites.size() - 1;
}

void GraphicalDynamicInterface::addDynamicShape(std::string const& identifier, sf::Shape* shape, bool smooth) noexcept
{
	if (m_dynamicSpritesIds.find(identifier) != m_dynamicSpritesIds.end())
		return;

	addShape(shape, smooth);
	m_dynamicSpritesIds[identifier] = m_sprites.size() - 1;
}

GraphicalFixedInterface::TextWrapper& GraphicalDynamicInterface::getDText(std::string const& identifier)
{
	return m_texts[m_dynamicTextsIds.at(identifier)]; // Throw an exception if not there.
}

std::pair<sf::Sprite*, sf::Texture*> GraphicalDynamicInterface::getDSprite(std::string const& identifier)
{
	std::pair<sf::Sprite*, sf::Texture*> sprite{ std::make_pair(nullptr, nullptr) };
	auto& pair{ m_sprites[m_dynamicSpritesIds.at(identifier)] }; // Throw an exception if not there.

	sprite.first = &pair.first;
	sprite.second = &pair.second;

	return sprite;
}

void GraphicalDynamicInterface::removeDText(std::string const& identifier) noexcept
{
	auto toRemove{ m_dynamicTextsIds.find(identifier) };

	if (toRemove == m_dynamicTextsIds.end())
		return; // No effect if it does not exist.

	// For all Indexes that are greater than the one we want to remove, we decrease their index by 1.
	// Otherwise, we will have a gap in the vector.
	for (auto& it : m_dynamicTextsIds)
		if (it.second > toRemove->second)
			it.second--;

	m_texts.erase(std::next(m_texts.begin(), toRemove->second)); // Erase from the vector.
	m_dynamicTextsIds.erase(toRemove); // Erase from the "dynamic" map.
}

void GraphicalDynamicInterface::removeDSprite(std::string const& identifier) noexcept
{
	auto toRemove{ m_dynamicSpritesIds.find(identifier) };

	if (toRemove == m_dynamicSpritesIds.end())
		return; // No effect if it does not exist.

	// For all Indexes that are greater than the one we want to remove, we decrease their index by 1.
	// Otherwise, we will have a gap in the vector.
	for (auto& it : m_dynamicSpritesIds)
		if (it.second > toRemove->second)
			it.second--; 

	m_sprites.erase(std::next(m_sprites.begin(), toRemove->second)); // Erase from the vector.
	m_dynamicSpritesIds.erase(toRemove); // Erase from the "dynamic" map.

	// The pointer to the texture becomes invalid because we removed something to the vector.
	resetTextureForSprites();
}


void GraphicalUserInteractableInterface::addButton(std::string const& id, std::function<void()> function) noexcept
{
	if (m_buttons.find(id) != m_buttons.end() || m_dynamicTextsIds.find(id) == m_dynamicTextsIds.end())
		return;

	m_buttons[id] = std::make_pair(m_dynamicTextsIds.find(id)->second, function);
}

std::function<void()>& GraphicalUserInteractableInterface::getFunctionOfButton(std::string const& identifier)
{
	return m_buttons.at(identifier).second; // Throw an exception if not there.
}

void GraphicalUserInteractableInterface::removeButton(std::string const& identifier) noexcept
{
	if (m_buttons.find(identifier) == m_buttons.end())
		return;

	m_buttons.erase(identifier);
	removeDText(identifier);
}

void GraphicalUserInteractableInterface::addSlider(std::string const& id, sf::Vector2f pos, sf::Vector2u size, float scale, int maxValue, int intervalle, bool displayCurrentValue) noexcept
{
	if (m_sliders.find(id) != m_sliders.end())
		return;
	
	m_sliders[id] = Slider{ pos, size, scale, maxValue, intervalle, displayCurrentValue };
}

GraphicalUserInteractableInterface::IdentifierInteractableItem GraphicalUserInteractableInterface::mouseMoved()
{
	return IdentifierInteractableItem();
}

GraphicalUserInteractableInterface::IdentifierInteractableItem GraphicalUserInteractableInterface::mousePressed() noexcept
{
	return IdentifierInteractableItem();
}

void GraphicalUserInteractableInterface::draw() const
{
	GraphicalDynamicInterface::draw();

	for (auto const& elem : m_sliders)
	{
		m_window->draw(elem.second.getBackground());
		m_window->draw(elem.second.getCursor());
	}
}

GraphicalUserInteractableInterface::Slider::Slider(sf::Vector2f pos, sf::Vector2u size, float scale, int maxValue, int intervalle, bool displayCurrentValue) noexcept
	: m_textureBackgroundSlider{ loadDefaultSliderTexture(size) }, m_textureCursorSlider{ loadDefaultSliderTexture(sf::Vector2u{ size.x * 3, size.x }) }, m_backgroundSlider{ std::make_unique<sf::Sprite>(m_textureBackgroundSlider) }, m_cursorSlider{ std::make_unique<sf::Sprite>(m_textureCursorSlider) }, m_currentValue{ nullptr }, m_maxValue{ maxValue }, m_intervalles{ intervalle }
{
	m_backgroundSlider->setOrigin(m_backgroundSlider->getLocalBounds().getCenter());
	m_backgroundSlider->setScale(sf::Vector2f{ scale, scale });
	m_backgroundSlider->setPosition(pos);

	m_cursorSlider->setOrigin(m_cursorSlider->getLocalBounds().getCenter());
	m_cursorSlider->setScale(sf::Vector2f{ scale, scale });
	m_cursorSlider->setPosition(pos);

	if (displayCurrentValue)
		m_currentValue = std::make_unique<TextWrapper>("", sf::Vector2f{ pos.x + size.x * 8, pos.y }, size.x, scale);
}

GraphicalUserInteractableInterface::Slider::Slider(Slider&& other) noexcept
	: m_textureBackgroundSlider{ std::move(other.m_textureBackgroundSlider) },
	m_textureCursorSlider{ std::move(other.m_textureCursorSlider) },
	m_backgroundSlider{ std::move(other.m_backgroundSlider) },
	m_cursorSlider{ std::move(other.m_cursorSlider) },
	m_currentValue{ std::move(other.m_currentValue) },
	m_maxValue{ other.m_maxValue },
	m_intervalles{ other.m_intervalles }
{
	// Réinitialiser les pointeurs de l'objet déplacé
	other.m_backgroundSlider = nullptr;
	other.m_cursorSlider = nullptr;
	other.m_currentValue = nullptr;

	m_backgroundSlider->setTexture(m_textureBackgroundSlider);
	m_cursorSlider->setTexture(m_textureCursorSlider);
}

GraphicalUserInteractableInterface::Slider& GraphicalUserInteractableInterface::Slider::operator=(Slider&& other) noexcept
{
	if (this != &other)
	{
		m_textureBackgroundSlider = std::move(other.m_textureBackgroundSlider);
		m_textureCursorSlider = std::move(other.m_textureCursorSlider);
		m_backgroundSlider = std::move(other.m_backgroundSlider);
		m_cursorSlider = std::move(other.m_cursorSlider);
		m_currentValue = std::move(other.m_currentValue);
		m_maxValue = other.m_maxValue;
		m_intervalles = other.m_intervalles;

		// Réinitialiser les pointeurs de l'objet déplacé
		other.m_backgroundSlider = nullptr;
		other.m_cursorSlider = nullptr;
		other.m_currentValue = nullptr;

		m_backgroundSlider->setTexture(m_textureBackgroundSlider);
		m_cursorSlider->setTexture(m_textureCursorSlider);
	}
	return *this;
}

void GraphicalUserInteractableInterface::Slider::changeValue(sf::Vector2u mousePos) noexcept
{

}

float GraphicalUserInteractableInterface::Slider::getValue() const noexcept
{
	return 0.0f;
}

float GraphicalUserInteractableInterface::Slider::setCursor(float relativePos) noexcept
{
	return 0.0f;
}

