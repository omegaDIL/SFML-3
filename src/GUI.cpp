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


void GraphicalUserInteractableInterface::addButton(std::string const& id, std::function<void()> function = []() {}) noexcept
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
