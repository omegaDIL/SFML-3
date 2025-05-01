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

sf::Font FixedGraphicalInterface::TextWrapper::m_font{};
std::string const FixedGraphicalInterface::ressourcePath{ "../res/" };
std::unordered_multimap<sf::RenderWindow*, FixedGraphicalInterface*> FixedGraphicalInterface::allInterfaces{};


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

	// Loads the background.
	auto errorBackground{ loadBackground(backgroundFileName) };
	if (errorBackground.has_value())
		throw LoadingGUIRessourceFailure{ errorBackground.value() };
}

FixedGraphicalInterface::FixedGraphicalInterface(FixedGraphicalInterface&& other) noexcept
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
	
	// Leave the moved-from object in a valid state
	other.m_window = nullptr;
}

FixedGraphicalInterface& FixedGraphicalInterface::operator=(FixedGraphicalInterface&& other) noexcept
{
	auto interfaceRange = allInterfaces.equal_range(m_window);

	// Remove this object from the static collection if it already exists
	for (auto it = interfaceRange.first; it != interfaceRange.second;)
	{
		if (it->second == this)
		{	// Remove the interface from the collection as it is being moved.
			it = allInterfaces.erase(it);
		}
		else if (this != &other && it->second == &other)
		{	// Update the moved object's to this object, except in self move where both are the same.
			it->second = this;
			it++;
		}
		else
		{
			it++;
		}
	}	

	// Transfer ownership of resources
	std::swap(m_sprites, other.m_sprites);
	std::swap(m_texts, other.m_texts);
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

void FixedGraphicalInterface::addSprite(sf::Sprite sprite, sf::Texture texture) noexcept
{
	m_sprites.push_back(std::make_pair(std::move(sprite), std::move(texture)));

	// The pointer to the texture becomes invalid because we added something to the vector.
	resetTextureForSprites();
}

void FixedGraphicalInterface::addShape(sf::Shape* shape, bool smooth) noexcept
{
	std::unique_ptr<sf::Sprite> sprite{ nullptr };
	std::unique_ptr<sf::Texture> texture{ nullptr };

	convertShapeToSprite(shape, sprite, texture, smooth);
	addSprite(std::move(*sprite), std::move(*texture));
}

void FixedGraphicalInterface::draw() const
{
	if (!m_window)
		throw std::logic_error{ "Window of an interface is nullptr." };

	for (auto const& sprite : m_sprites)
		m_window->draw(sprite.first);
	for (auto const& text : m_texts)
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

void FixedGraphicalInterface::resetTextureForSprites() noexcept
{
	for (auto& sprite : m_sprites)
		sprite.first.setTexture(sprite.second);
}

std::optional<std::string> FixedGraphicalInterface::loadBackground(std::string const& backgroundFileName) noexcept
{
	sf::RectangleShape background{ static_cast<sf::Vector2f>(m_window->getSize()) };
	background.setFillColor(sf::Color{ 20, 20, 20 });
	background.setOrigin(m_window->getView().getCenter()); // Putting the origin at the center 
	background.setPosition(m_window->getView().getCenter());
	addShape(&background, false); // Adds the background to the interface.

	// Creating the background.
	if (backgroundFileName.empty())
		return std::nullopt; // No need to load the background if it is empty.

	try
	{
		std::string path{ ressourcePath + backgroundFileName };

		sf::Texture textureBackground{};
		if (!textureBackground.loadFromFile(path))
			throw LoadingGUIRessourceFailure{ "Failed to load background at: " + path };

		textureBackground.setSmooth(true);
		m_sprites[0].second = std::move(textureBackground); // The first sprite is the background.
		m_sprites[0].first.setTexture(m_sprites[0].second);
	}
	catch (LoadingGUIRessourceFailure const& error)
	{
		std::ostringstream oss{};

		oss << error.what() << "\n";
		oss << "error: the default background is displayed instead\n\n";

		return std::make_optional<std::string>(oss.str());
	}

	return std::nullopt;
}

std::optional<std::string> FixedGraphicalInterface::TextWrapper::loadFont() noexcept
{
	if (m_font.getInfo().family != "")
		return std::nullopt; // Font already loaded.

	try
	{
		std::string path{ FixedGraphicalInterface::ressourcePath + "font.ttf" };

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


bool DynamicGraphicalInterface::addDynamicSprite(std::string const& identifier, sf::Sprite sprite, sf::Texture texture) noexcept
{
	if (m_dynamicSpritesIds.find(identifier) != m_dynamicSpritesIds.end())
		return false;

	addSprite(std::move(sprite), std::move(texture));
	m_dynamicSpritesIds[identifier] = m_sprites.size() - 1;

	return true;
}

bool DynamicGraphicalInterface::addDynamicShape(std::string const& identifier, sf::Shape* shape, bool smooth) noexcept
{
	if (m_dynamicSpritesIds.find(identifier) != m_dynamicSpritesIds.end())
		return false;

	addShape(shape, smooth);
	m_dynamicSpritesIds[identifier] = m_sprites.size() - 1;

	return true;
}

FixedGraphicalInterface::TextWrapper& DynamicGraphicalInterface::getDText(std::string const& identifier)
{
	return m_texts[m_dynamicTextsIds.at(identifier)]; // Throw an exception if not there.
}

std::pair<sf::Sprite*, sf::Texture*> DynamicGraphicalInterface::getDSprite(std::string const& identifier)
{
	std::pair<sf::Sprite*, sf::Texture*> sprite{ std::make_pair(nullptr, nullptr) };
	auto& pair{ m_sprites[m_dynamicSpritesIds.at(identifier)] }; // Throw an exception if not there.

	sprite.first = &pair.first;
	sprite.second = &pair.second;

	return sprite;
}

bool DynamicGraphicalInterface::removeDText(std::string const& identifier) noexcept
{
	auto toRemove{ m_dynamicTextsIds.find(identifier) };

	if (toRemove == m_dynamicTextsIds.end())
		return false; // No effect if it does not exist.

	// For all Indexes that are greater than the one we want to remove, we decrease their index by 1.
	// Otherwise, we will have a gap in the vector.
	for (auto& it : m_dynamicTextsIds)
		if (it.second > toRemove->second)
			it.second--;

	m_texts.erase(std::next(m_texts.begin(), toRemove->second)); // Erase from the vector.
	m_dynamicTextsIds.erase(toRemove); // Erase from the "dynamic" map.

	return true;
}

bool DynamicGraphicalInterface::removeDSprite(std::string const& identifier) noexcept
{
	auto toRemove{ m_dynamicSpritesIds.find(identifier) };

	if (toRemove == m_dynamicSpritesIds.end())
		return false; // No effect if it does not exist.

	// For all Indexes that are greater than the one we want to remove, we decrease their index by 1.
	// Otherwise, we will have a gap in the vector.
	for (auto& it : m_dynamicSpritesIds)
		if (it.second > toRemove->second)
			it.second--; 

	m_sprites.erase(std::next(m_sprites.begin(), toRemove->second)); // Erase from the vector.
	m_dynamicSpritesIds.erase(toRemove); // Erase from the "dynamic" map.

	// The pointer to the texture becomes invalid because we removed something to the vector.
	resetTextureForSprites();

	return true;
}


bool UserInteractableGraphicalInterface::addButton(std::string const& id, std::function<void()> function) noexcept
{
	if (m_buttons.find(id) != m_buttons.end() || m_dynamicTextsIds.find(id) == m_dynamicTextsIds.end())
		return false; 

	m_buttons[id] = std::make_pair(m_dynamicTextsIds.find(id)->second, function);

	return true;
}

std::function<void()>& UserInteractableGraphicalInterface::getFunctionOfButton(std::string const& identifier)
{
	return m_buttons.at(identifier).second; // Throw an exception if not there.
}

bool UserInteractableGraphicalInterface::removeButton(std::string const& identifier) noexcept
{
	if (m_buttons.find(identifier) == m_buttons.end())
		return false;

	m_buttons.erase(identifier);
	removeDText(identifier);

	return true;
}

bool UserInteractableGraphicalInterface::addSlider(std::string const& id, sf::Vector2f pos, unsigned int length, float scale, int minValue, int maxValue, int intervalle, bool displayCurrentValue) noexcept
{
	if (m_sliders.find(id) != m_sliders.end())
		return false;
	
	unsigned int minSize{ std::min(m_window->getSize().x, m_window->getSize().y) };
	m_sliders[id] = Slider{ m_window, pos, sf::Vector2u{ minSize / 72, length }, scale, minValue, maxValue, intervalle, displayCurrentValue };

	return true;
}

double UserInteractableGraphicalInterface::getValueSlider(std::string const& id) const
{
	return m_sliders.at(id).getValue();// Throws an exception if not there
}

bool UserInteractableGraphicalInterface::removeSlider(std::string const& identifier) noexcept
{
	if (m_sliders.find(identifier) == m_sliders.end())
		return false;

	m_sliders.erase(identifier);

	return true;
}

UserInteractableGraphicalInterface::IdentifierInteractableItem UserInteractableGraphicalInterface::mouseMoved()
{
	sf::Vector2f mousePos{ static_cast<sf::Vector2f>(sf::Mouse::getPosition(*m_window)) };

	for (auto& button : m_buttons)
	{
		if (m_texts[button.second.first].getText().getGlobalBounds().contains(mousePos))
		{
			m_hoveredElement = std::make_pair(InteractableItem::Button, &button.first);
			return m_hoveredElement;
		}
	}

	for (auto& slider : m_sliders)
	{
		if (slider.second.m_backgroundSlider->getGlobalBounds().contains(mousePos))
		{
			m_hoveredElement = std::make_pair(InteractableItem::Slider, &slider.first);
			return m_hoveredElement;
		}
	}

	m_hoveredElement = std::make_pair(InteractableItem::None, nullptr);
	return m_hoveredElement;
}

UserInteractableGraphicalInterface::IdentifierInteractableItem UserInteractableGraphicalInterface::mousePressed() noexcept
{
	if (m_hoveredElement.first == InteractableItem::None)
		return m_hoveredElement;
	else if (m_hoveredElement.first == InteractableItem::Button)
		getFunctionOfButton(*m_hoveredElement.second)();
	else if (m_hoveredElement.first == InteractableItem::Slider)
		m_sliders[*m_hoveredElement.second].changeValue(sf::Mouse::getPosition(*m_window).y);

	return m_hoveredElement;
}

UserInteractableGraphicalInterface::IdentifierInteractableItem UserInteractableGraphicalInterface::mouseUnpressed() noexcept
{
	if (m_hoveredElement.first == InteractableItem::Button)
		getFunctionOfButton(*m_hoveredElement.second)();

	return m_hoveredElement;
}

void UserInteractableGraphicalInterface::draw() const
{
	DynamicGraphicalInterface::draw();

	for (auto const& elem : m_sliders)
		elem.second.draw();
}

UserInteractableGraphicalInterface::Slider::Slider(sf::RenderWindow* window, sf::Vector2f pos, sf::Vector2u size, float scale, int minValue, int maxValue, int intervalle, bool displayCurrentValue) noexcept
	: m_window{ window }, m_textureBackgroundSlider{ loadDefaultSliderTexture(size) }, m_textureCursorSlider{ loadDefaultSliderTexture(sf::Vector2u{ size.x * 4, size.x }) }, m_backgroundSlider{ std::make_unique<sf::Sprite>(m_textureBackgroundSlider) }, m_cursorSlider{ std::make_unique<sf::Sprite>(m_textureCursorSlider) }, m_currentValueText{ nullptr }, m_intervalles{ intervalle }
{
	m_maxValue = std::max(minValue, maxValue);
	m_minValue = std::min(minValue, maxValue);

	m_backgroundSlider->setOrigin(m_backgroundSlider->getLocalBounds().getCenter());
	m_backgroundSlider->setScale(sf::Vector2f{ scale, scale });
	m_backgroundSlider->setPosition(pos);

	m_cursorSlider->setOrigin(m_cursorSlider->getLocalBounds().getCenter());
	m_cursorSlider->setScale(sf::Vector2f{ scale, scale });
	m_cursorSlider->setPosition(pos);

	if (displayCurrentValue)
		m_currentValueText = std::make_unique<TextWrapper>((m_maxValue - m_minValue) / 2. + m_minValue, sf::Vector2f{ pos.x + size.x * 4, pos.y }, size.x, scale);
}

UserInteractableGraphicalInterface::Slider::Slider(Slider&& other) noexcept
	: m_window{ other.m_window },
	m_textureBackgroundSlider{ std::move(other.m_textureBackgroundSlider) },
	m_textureCursorSlider{ std::move(other.m_textureCursorSlider) },
	m_backgroundSlider{ std::move(other.m_backgroundSlider) },
	m_cursorSlider{ std::move(other.m_cursorSlider) },
	m_currentValueText{ std::move(other.m_currentValueText) },
	m_maxValue{ other.m_maxValue },
	m_minValue{ other.m_minValue },
	m_intervalles{ other.m_intervalles }
{
	// Réinitialiser les pointeurs de l'objet déplacé
	other.m_window = nullptr;
	other.m_backgroundSlider = nullptr;
	other.m_cursorSlider = nullptr;
	other.m_currentValueText = nullptr;

	m_backgroundSlider->setTexture(m_textureBackgroundSlider);
	m_cursorSlider->setTexture(m_textureCursorSlider);
}

UserInteractableGraphicalInterface::Slider& UserInteractableGraphicalInterface::Slider::operator=(Slider&& other) noexcept
{
	if (this != &other)
	{
		m_window = other.m_window;
		m_textureBackgroundSlider = std::move(other.m_textureBackgroundSlider);
		m_textureCursorSlider = std::move(other.m_textureCursorSlider);
		m_backgroundSlider = std::move(other.m_backgroundSlider);
		m_cursorSlider = std::move(other.m_cursorSlider);
		m_currentValueText = std::move(other.m_currentValueText);
		m_maxValue = other.m_maxValue;
		m_minValue = other.m_minValue;
		m_intervalles = other.m_intervalles;

		// Réinitialiser les pointeurs de l'objet déplacé
		other.m_window = nullptr;
		other.m_backgroundSlider = nullptr;
		other.m_cursorSlider = nullptr;
		other.m_currentValueText = nullptr;

		m_backgroundSlider->setTexture(m_textureBackgroundSlider);
		m_cursorSlider->setTexture(m_textureCursorSlider);
	}
	return *this;
}

void UserInteractableGraphicalInterface::Slider::changeValue(float mousePosY) noexcept
{
	double yPos{ static_cast<double>(mousePosY) };
	double minPos{ m_backgroundSlider->getGlobalBounds().position.y };
	double maxPos{ minPos + m_backgroundSlider->getGlobalBounds().size.y };

	if (yPos < minPos)
		yPos = minPos;
	else if (yPos > maxPos)
		yPos = maxPos;

	m_cursorSlider->setPosition(sf::Vector2f{ m_cursorSlider->getPosition().x, static_cast<float>(yPos) });

	if (m_currentValueText)
	{
		m_currentValueText->updatePosition(sf::Vector2f{ m_currentValueText->getText().getPosition().x, static_cast<float>(yPos)});
		m_currentValueText->updateContent(getValue());
	}
}

double UserInteractableGraphicalInterface::Slider::getValue() const noexcept
{
	double yPos{ m_cursorSlider->getPosition().y };
	double minPos{ m_backgroundSlider->getGlobalBounds().position.y };
	double maxPos{ minPos + m_backgroundSlider->getGlobalBounds().size.y };

	double value{ 1 - (yPos - minPos) / (maxPos - minPos) };
	value *= m_maxValue-m_minValue;
	value += m_minValue;

	return value;
}

double UserInteractableGraphicalInterface::Slider::setCursor(float relativePos) noexcept
{
	changeValue(m_backgroundSlider->getGlobalBounds().position.y + m_backgroundSlider->getGlobalBounds().size.y * relativePos);

	return getValue();
}

void UserInteractableGraphicalInterface::Slider::draw() const noexcept
{
	if (!m_window)
		return;

	m_window->draw(*m_backgroundSlider);
	m_window->draw(*m_cursorSlider);
	
	if (m_currentValueText)
		m_window->draw(m_currentValueText->getText());
}
//TODO; mettre draw en no except.
//TODO: faire intervalle
//TODO: faire double checker et MQB
