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
UserInteractableGraphicalInterface::IdentifierInteractableItem UserInteractableGraphicalInterface::m_hoveredElement{ std::make_pair(InteractableItem::None, nullptr) };
std::string const FixedGraphicalInterface::ressourcePath{ "../res/" };
std::unordered_multimap<sf::RenderWindow*, FixedGraphicalInterface*> FixedGraphicalInterface::allInterfaces{};


FixedGraphicalInterface::FixedGraphicalInterface(sf::RenderWindow* window, std::string const& backgroundFileName)
	: m_window{ window }, m_sprites{}, m_texts{}, m_defaultBackground{ true }
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

		for (auto& text : curInterface->m_texts)
			text.windowResized(scalingFactor, minScalingFactor);

		if (curInterface->m_defaultBackground)
			curInterface->loadBackground("");
		else
			curInterface->m_sprites[0].first.setPosition(sf::Vector2f{ window->getSize().x / 2.f, window->getSize().y / 2.f });

		for (int j{ 1 }; j < curInterface->m_sprites.size(); j++)
		{	// Avoiding the background by skipping index 0.
			sf::Sprite& curSprite{ curInterface->m_sprites[j].first };

			curSprite.scale(minScalingFactor);
			curSprite.setPosition(sf::Vector2f{ curSprite.getPosition().x * scalingFactor.x, curSprite.getPosition().y * scalingFactor.y });
		}
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
		m_defaultBackground = false; // The default background is not used anymore.
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
	if (m_dynamicSpritesIds.find(identifier) != m_dynamicSpritesIds.end() || identifier.empty())
		return false;

	addSprite(std::move(sprite), std::move(texture));
	m_dynamicSpritesIds[identifier] = m_sprites.size() - 1;

	return true;
}

bool DynamicGraphicalInterface::addDynamicShape(std::string const& identifier, sf::Shape* shape, bool smooth) noexcept
{
	if (m_dynamicSpritesIds.find(identifier) != m_dynamicSpritesIds.end() || identifier.empty())
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
	auto toRemoveText{ m_dynamicTextsIds.find(identifier) };

	if (toRemoveText == m_dynamicTextsIds.end())
		return false; // No effect if it does not exist.

	// Decreases the indexes of the items that points to a graphical element, "higher" in the
	// vector, than the one that'll be removed. If we remove an item at index 2, the map that had the
	// indexes 1, 4, 6 will have to be updated to 1, 3, 5. The index 1 don't need to change because
	// it is lower than 2. The index 4 will be updated to 3 because it is higher, and so on.
	for (auto& it : m_dynamicTextsIds)
		if (it.second > toRemoveText->second)
			it.second--;

	m_texts.erase(std::next(m_texts.begin(), toRemoveText->second)); // Erase from the vector.
	m_dynamicTextsIds.erase(toRemoveText); // Erase from the "dynamic" map.

	return true;
}

bool DynamicGraphicalInterface::removeDSprite(std::string const& identifier) noexcept
{
	auto toRemoveSprite{ m_dynamicSpritesIds.find(identifier) };

	if (toRemoveSprite == m_dynamicSpritesIds.end() || identifier == "_background")
		return false; // No effect if it does not exist.

	// Decreases the indexes of the items that points to a graphical element, "higher" in the
	// vector, than the one that'll be removed. If we remove an item at index 2, the map that had the
	// indexes 1, 4, 6 will have to be updated to 1, 3, 5. The index 1 don't need to change because
	// it is lower than 2. The index 4 will be updated to 3 because it is higher, and so on.
	for (auto& it : m_dynamicSpritesIds)
		if (it.second > toRemoveSprite->second)
			it.second--; 

	m_sprites.erase(std::next(m_sprites.begin(), toRemoveSprite->second)); // Erase from the vector.
	m_dynamicSpritesIds.erase(toRemoveSprite); // Erase from the "dynamic" map.

	// The pointer to the texture becomes invalid because we removed something to the vector.
	resetTextureForSprites();

	return true;
}


bool UserInteractableGraphicalInterface::addButton(std::string const& id, std::function<void()> function) noexcept
{
	if (m_buttons.find(id) != m_buttons.end() || m_dynamicTextsIds.find(id) == m_dynamicTextsIds.end())
		return false; 

	m_buttons[id] = std::make_pair(m_dynamicTextsIds.at(id), function);

	return true;
}

bool UserInteractableGraphicalInterface::addSlider(std::string const& id, float length, bool displayValue, float size, float maxValue, float minValue) noexcept
{
	return false;
}

std::function<void()>& UserInteractableGraphicalInterface::getFunctionOfButton(std::string const& identifier)
{
	return m_buttons.at(identifier).second; // Throw an exception if not there.
}

bool UserInteractableGraphicalInterface::removeDText(std::string const& identifier) noexcept
{
	auto toUpdateButton{ m_dynamicTextsIds.find(identifier) };

	if (toUpdateButton == m_dynamicTextsIds.end())
		return false; // No effect if it does not exist.

	// Decreases the indexes of the items that points to a graphical element, "higher" in the
	// vector, than the one that'll be removed. If we remove an item at index 2, the map that had the
	// indexes 1, 4, 6 will have to be updated to 1, 3, 5. The index 1 don't need to change because
	// it is lower than 2. The index 4 will be updated to 3 because it is higher, and so on.
	for (auto& it : m_buttons) // An element of the inner graphical elements vector is removed so adjusting the indexes of the buttons.
		if (it.second.first > toUpdateButton->second)
			it.second.first--;

	DynamicGraphicalInterface::removeDText(identifier); // Actually Removing the text + updating the map of the dynamic texts

	auto toRemoveButton{ m_buttons.find(identifier) };

	if (toRemoveButton != m_buttons.end())
		m_buttons.erase(toRemoveButton); // Erase from the map.

	return DynamicGraphicalInterface::removeDText(identifier);
}

UserInteractableGraphicalInterface::IdentifierInteractableItem UserInteractableGraphicalInterface::mouseMoved(FixedGraphicalInterface* activeGUI) noexcept
{
	// Resets the hovered element.
	m_hoveredElement = std::make_pair(InteractableItem::None, nullptr);

	UserInteractableGraphicalInterface* isDerived{ dynamic_cast<UserInteractableGraphicalInterface*>(activeGUI) };

	if (!isDerived)
		return getHoveredInteractableItem(); // Check if the gui is an interactable one.

	sf::Vector2f mousePos{ static_cast<sf::Vector2f>(sf::Mouse::getPosition(*(isDerived->m_window))) };

	// Check if the mouse is over a button or a slider.
	for (auto& button : isDerived->m_buttons)
	{
		if (isDerived->m_texts[button.second.first].getText().getGlobalBounds().contains(mousePos))
		{
			m_hoveredElement = std::make_pair(InteractableItem::Button, &button.first);
			return getHoveredInteractableItem();
		}
	}

	//for (auto& slider : isDerived->m_sliders)
	//{
	//	if (isDerived->m_sprites[slider.second.m_index].first.getGlobalBounds().contains(mousePos))
	//	{
	//		m_hoveredElement = std::make_pair(InteractableItem::Slider, &slider.first);
	//		return getHoveredInteractableItem();
	//	}
	//}

	return getHoveredInteractableItem();
}

UserInteractableGraphicalInterface::IdentifierInteractableItem UserInteractableGraphicalInterface::mousePressed(FixedGraphicalInterface* activeGUI) noexcept
{
	UserInteractableGraphicalInterface* isDerived{ dynamic_cast<UserInteractableGraphicalInterface*>(activeGUI) };

	if (m_hoveredElement.first == InteractableItem::None || !isDerived)
		return getHoveredInteractableItem(); // Check if the gui is an interactable one.
	//if (m_hoveredElement.first == InteractableItem::Slider)
		//
	// Button is executed when the mouse is released to avoid multiple calls for each frame it is kept pressed.

	return getHoveredInteractableItem();
}

UserInteractableGraphicalInterface::IdentifierInteractableItem UserInteractableGraphicalInterface::mouseUnpressed(FixedGraphicalInterface* activeGUI) noexcept
{
	UserInteractableGraphicalInterface* isDerived{ dynamic_cast<UserInteractableGraphicalInterface*>(activeGUI) };

	if (m_hoveredElement.first == InteractableItem::None || !isDerived)
		return getHoveredInteractableItem(); // Check if the gui is an interactable one.
	if (m_hoveredElement.first == InteractableItem::Button)
		isDerived->getFunctionOfButton(*m_hoveredElement.second)();

	return getHoveredInteractableItem();
}

void UserInteractableGraphicalInterface::changeValueSlider(std::string const& id, int mousePosY) noexcept
{
	//sf::Sprite* background{ &m_sprites[m_sliders.at(id).m_index].first }; // Throws an exception if not there.
	//sf::Sprite* cursor{ &m_sprites[m_sliders.at(id).m_index + 1].first }; // Throws an exception if not there.

	//double yPos{ static_cast<double>(mousePosY) };
	//double minPos{ background->getGlobalBounds().position.y };
	//double maxPos{ minPos + background->getGlobalBounds().size.y };

	//if (yPos < minPos)
	//	yPos = minPos;
	//else if (yPos > maxPos)
	//	yPos = maxPos;

	//cursor->setPosition(sf::Vector2f{ cursor->getPosition().x, static_cast<float>(yPos) });

	//TextWrapper* text{ &m_texts[*m_sliders.at(id).m_textIndex] }; // Throws an exception if not there.
	//if (text)
	//{
	//	text->updatePosition(sf::Vector2f{ text->getText().getPosition().x, static_cast<float>(yPos) });
	//	text->updateContent(getValue());
	//}
}



//void UserInteractableGraphicalInterface::Slider::changeValue(float mousePosY) noexcept
//{
//	double yPos{ static_cast<double>(mousePosY) };
//	double minPos{ m_backgroundSlider->getGlobalBounds().position.y };
//	double maxPos{ minPos + m_backgroundSlider->getGlobalBounds().size.y };
//
//	if (yPos < minPos)
//		yPos = minPos;
//	else if (yPos > maxPos)
//		yPos = maxPos;
//
//	m_cursorSlider->setPosition(sf::Vector2f{ m_cursorSlider->getPosition().x, static_cast<float>(yPos) });
//
//	if (m_currentValueText)
//	{
//		m_currentValueText->updatePosition(sf::Vector2f{ m_currentValueText->getText().getPosition().x, static_cast<float>(yPos)});
//		m_currentValueText->updateContent(getValue());
//	}
//}
//
//double UserInteractableGraphicalInterface::Slider::getValue() const noexcept
//{
//	double yPos{ m_cursorSlider->getPosition().y };
//	double minPos{ m_backgroundSlider->getGlobalBounds().position.y };
//	double maxPos{ minPos + m_backgroundSlider->getGlobalBounds().size.y };
//
//	double value{ 1 - (yPos - minPos) / (maxPos - minPos) };
//	value *= m_maxValue-m_minValue;
//	value += m_minValue;
//
//	return value;
//}
//
//double UserInteractableGraphicalInterface::Slider::setCursor(float relativePos) noexcept
//{
//	changeValue(m_backgroundSlider->getGlobalBounds().position.y + m_backgroundSlider->getGlobalBounds().size.y * relativePos);
//
//	return getValue();
//}


//bool UserInteractableGraphicalInterface::addSlider(std::string const& id, sf::Vector2f pos, unsigned int length, float scale, float minValue, float maxValue, int intervalle, bool displayCurrentValue) noexcept
//{
//	if (m_sliders.find(id) != m_sliders.end())
//		return false;
//
//	unsigned int minSize{ std::min(m_window->getSize().x, m_window->getSize().y) };
//	m_sliders[id] = Slider{ m_window, pos, sf::Vector2u{ minSize / 72, length }, scale, minValue, maxValue, intervalle, displayCurrentValue };
//
//	return true;
//}
//
//double UserInteractableGraphicalInterface::getValueSlider(std::string const& id) const
//{
//	return m_sliders.at(id).getValue();// Throws an exception if not there
//}
//
//bool UserInteractableGraphicalInterface::removeSlider(std::string const& identifier) noexcept
//{
//	if (m_sliders.find(identifier) == m_sliders.end())
//		return false;
//
//	m_sliders.erase(identifier);
//
//	return true;
//}


//TODO: FIX THE SLIDER
//TODO: faire intervalle et logarithme
//TODO: faire double checker et MQB
