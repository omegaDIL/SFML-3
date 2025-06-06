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
UserInteractableGraphicalInterface::IdentifierInteractableItem UserInteractableGraphicalInterface::m_hoveredElement{ std::make_pair(InteractableItem::None, nullptr) };
std::shared_ptr<sf::Texture> UserInteractableGraphicalInterface::MQB::m_uncheckedTexture{ loadSolidRectangeShapeWithOutline(sf::Vector2u{ 15, 15 }) };
std::shared_ptr<sf::Texture> UserInteractableGraphicalInterface::MQB::m_checkedTexture{ loadCheckBoxTexture(sf::Vector2u{ 15, 15 }) };


FixedGraphicalInterface::FixedGraphicalInterface(FixedGraphicalInterface&& other) noexcept
	: m_window{ other.m_window }, m_sprites{ std::move(other.m_sprites) }, m_texts{ std::move(other.m_texts) }, m_defaultBackground{ other.m_defaultBackground }
{
	resetTextureForSprites(); // Reset the textures for the sprites after moving them.

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
	std::swap(m_defaultBackground, other.m_defaultBackground);
	resetTextureForSprites(); // Reset the textures for the sprites after moving them.

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

void FixedGraphicalInterface::addSprite(sf::Sprite sprite, std::shared_ptr<sf::Texture> texture) noexcept
{
	// Tracking capacity.
	size_t capacity{ m_sprites.capacity() };

	m_sprites.push_back(std::make_pair(std::move(sprite), texture));
	m_sprites.back().first.setTexture(*m_sprites.back().second); // Reset the texture of the sprite.

	// As we add something to the vector, the vector may resize and all pointers could become invalid.
	if (capacity != m_sprites.capacity())
		resetTextureForSprites();
}

void FixedGraphicalInterface::addShape(sf::Shape& shape, bool smooth) noexcept
{
	auto texture{ createTextureFromShape(shape, smooth) };
	sf::Sprite sprite{ *texture };

	// Setting the transformables of the sprite to match the shape.
	sprite.setOrigin(shape.getOrigin());
	sprite.setPosition(shape.getPosition());
	sprite.setScale(shape.getScale());
	sprite.setRotation(shape.getRotation());

	addSprite(std::move(sprite), texture);
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

		// Updating texts.
		for (auto& text : curInterface->m_texts)
			text.windowResized(scalingFactor, minScalingFactor);

		// Background is always the first sprite, so it is at index 0.
		curInterface->m_sprites[0].first.setPosition(sf::Vector2f{ window->getSize().x / 2.f, window->getSize().y / 2.f });
		if (curInterface->m_defaultBackground) // Only if the default background is used.	
			curInterface->m_sprites[0].first.scale(scalingFactor); // The default background is a solid color, so we can scale it without issues.
		
		// Updating sprites.
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
	for (auto& sprite : m_sprites) // Resetting the textures for all sprites.
		sprite.first.setTexture(*sprite.second);
}

std::optional<std::string> FixedGraphicalInterface::loadBackground(std::string const& backgroundFileName) noexcept
{
	sf::RectangleShape background{ static_cast<sf::Vector2f>(m_window->getSize()) };
	background.setFillColor(sf::Color{ 20, 20, 20 });
	background.setOrigin(m_window->getView().getCenter());
	background.setPosition(m_window->getView().getCenter());
	addShape(background, false); // Adds the background to the interface.

	// Creating the background.
	if (backgroundFileName.empty())
		return std::nullopt; // No need to load the background if it is empty.

	try
	{
		std::string path{ ressourcePath + backgroundFileName };

		std::shared_ptr<sf::Texture> textureBackground{ std::make_shared<sf::Texture>() };
		if (!textureBackground->loadFromFile(path))
			throw LoadingGUIRessourceFailure{ "Failed to load background at: " + path };

		textureBackground->setSmooth(true);
		m_sprites[0].second = std::move(textureBackground); // The first sprite is the background.
		m_sprites[0].first.setTexture(*m_sprites[0].second); // Reset the texture of the background sprite.
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


bool DynamicGraphicalInterface::addDynamicSprite(std::string const& identifier, sf::Sprite sprite, std::shared_ptr<sf::Texture> texture) noexcept
{
	if (m_dynamicSpriteIds.find(identifier) != m_dynamicSpriteIds.end() || identifier.empty())
		return false; // Not already added or identifier is empty.

	addSprite(std::move(sprite), std::move(texture));
	m_dynamicSpriteIds[identifier] = m_sprites.size() - 1; // The last sprite added has the highest index.

	return true;
}

bool DynamicGraphicalInterface::addDynamicShape(std::string const& identifier, sf::Shape& shape, bool smooth) noexcept
{
	if (m_dynamicSpriteIds.find(identifier) != m_dynamicSpriteIds.end() || identifier.empty())
		return false; // Not already added or identifier is empty.

	addShape(shape, smooth);
	m_dynamicSpriteIds[identifier] = m_sprites.size() - 1; // The last sprite added has the highest index.

	return true;
}

FixedGraphicalInterface::TextWrapper& DynamicGraphicalInterface::getDText(std::string const& identifier)
{
	return m_texts[m_dynamicTextIds.at(identifier)]; // Throws an exception if not there.
}

FixedGraphicalInterface::GraphicalElement* DynamicGraphicalInterface::getDSprite(std::string const& identifier)
{
	return &m_sprites[m_dynamicSpriteIds.at(identifier)]; // Throws an exception if not there.
}

bool DynamicGraphicalInterface::removeDText(std::string const& identifier) noexcept
{
	auto toRemoveText{ m_dynamicTextIds.find(identifier) };

	if (toRemoveText == m_dynamicTextIds.end())
		return false; // No effect if it does not exist.

	// Decreases the indexes of the items that points to a graphical element "higher" in the
	// vector than the one that'll be removed. If we remove an item at index 2, the map that had the
	// indexes 1, 4, 6 need to be updated to 1, 3, 5. The index 1 doesn't need to change because
	// it is lower than 2. The index 4 will be updated to 3 because it is higher, and so on.
	for (auto& it : m_dynamicTextIds)
		if (it.second > toRemoveText->second)
			it.second--;

	m_texts.erase(std::next(m_texts.begin(), toRemoveText->second)); // Erase from the vector.
	m_dynamicTextIds.erase(toRemoveText); // Erase from the "dynamic" map.

	return true;
}

bool DynamicGraphicalInterface::removeDSprite(std::string const& identifier) noexcept
{
	auto toRemoveSprite{ m_dynamicSpriteIds.find(identifier) };

	if (toRemoveSprite == m_dynamicSpriteIds.end() || identifier == "_background")
		return false; // No effect if it does not exist.

	// Decreases the indexes of the items that points to a graphical element "higher" in the
	// vector than the one that'll be removed. If we remove an item at index 2, the map that had the
	// indexes 1, 4, 6 need to be updated to 1, 3, 5. The index 1 doesn't need to change because
	// it is lower than 2. The index 4 will be updated to 3 because it is higher, and so on.
	for (auto& it : m_dynamicSpriteIds)
		if (it.second > toRemoveSprite->second)
			it.second--; 

	m_sprites.erase(std::next(m_sprites.begin(), toRemoveSprite->second)); // Erase from the vector.
	m_dynamicSpriteIds.erase(toRemoveSprite); // Erase from the "dynamic" map.

	// As we remove something from the vector, the vector has to resize and all pointers become invalid.
	resetTextureForSprites();

	return true;
}


bool UserInteractableGraphicalInterface::addButton(std::string const& id, std::function<void()> function) noexcept
{
	if (m_buttons.find(id) != m_buttons.end() || m_dynamicTextIds.find(id) == m_dynamicTextIds.end())
		return false; 

	m_buttons[id] = function; // Adds the button with the text id and the function.

	return true;
}

bool UserInteractableGraphicalInterface::addSlider(std::string const& id, sf::Vector2u size, sf::Vector2f pos, std::function<float(float)> mathFunction, std::function<void(float)> changeFunction, int interval, bool showValueWithText) noexcept
{
	if (m_sliders.find(id) != m_sliders.end())
		return false; // Already added.

	// Instantiating the slider's background.
	static std::shared_ptr<sf::Texture> textureBackgroundSlider{ loadSolidRectangeShapeWithOutline(size) }; // Loads the default slider texture.
	sf::Sprite sliderBackgroundSprite{ *textureBackgroundSlider };
	sliderBackgroundSprite.setPosition(pos);
	sliderBackgroundSprite.setOrigin(sliderBackgroundSprite.getLocalBounds().getCenter()); // Set the origin to the center of the sprite.
	
	// Instantiating the slider's cursor.
	static std::shared_ptr<sf::Texture> textureCursorSlider{ loadSolidRectangeShapeWithOutline(sf::Vector2u{static_cast<unsigned int>(size.x * 1.618), size.x}) }; // Loads the default slider texture.
	sf::Sprite sliderCursorSprite{ *textureCursorSlider };
	sliderCursorSprite.setPosition(sliderBackgroundSprite.getGlobalBounds().getCenter()); // Set the position of the cursor to the center of its background.
	sliderCursorSprite.setOrigin(sliderCursorSprite.getLocalBounds().getCenter()); // Set the origin to the center of the sprite.
	
	// Adding the slider's text.
	if (showValueWithText)
		addDynamicText('_' + id, "", sf::Vector2f{sliderCursorSprite.getPosition().x - m_window->getSize().x * 60 / 1080, sliderCursorSprite.getPosition().y}, 16, 1.f); //TODO: modify scale so it adapts the definition of the screen.
	-
	// Adding the slider's graphical items.
	addDynamicSprite('_' + id, std::move(sliderBackgroundSprite), std::move(textureBackgroundSlider)); // Adds a sprite for the slider cursor.
	addDynamicSprite("_c" + id, std::move(sliderCursorSprite), std::move(textureCursorSlider)); // Adds a sprite for the slider cursor.

	// Instantiate the slider.
	m_sliders[id] = Slider{ mathFunction, changeFunction, interval };
	changeValueSlider(id, getDSprite('_' + id)->first.getPosition().y); // Set the initial value of the slider.

	return true;
}

bool UserInteractableGraphicalInterface::addMQB(std::string const& id, bool multipleChoices, int numberOfBoxes, sf::Vector2f pos1, sf::Vector2f pos2, unsigned int defaultChecked) noexcept
{
	if ((m_mqbs.find(id) != m_mqbs.end() || id.empty()) && numberOfBoxes > 1)
		return false; // Not already added or identifier is empty or invalid.

	sf::Vector2f diffPos{ pos2 - pos1 }; // Difference between the two positions to place the boxes.
	for (unsigned int i{ 0 }; i < numberOfBoxes; i++)
	{
		sf::Vector2f pos{ pos1.x + diffPos.x * i, pos2.y + diffPos.y * i };

		sf::Sprite boxSprite{ (i + 1 == defaultChecked) ? *MQB::m_checkedTexture: *MQB::m_uncheckedTexture };
		boxSprite.setPosition(pos);
		boxSprite.setOrigin(boxSprite.getLocalBounds().getCenter()); // Set the origin to the center of the sprite.

		addDynamicSprite('_' + id + std::to_string(i), std::move(boxSprite), MQB::m_uncheckedTexture); // Adds a shape for the checkbox background.
	}

	m_mqbs[id] = MQB{ multipleChoices, numberOfBoxes, defaultChecked };
}	// Adapter la taille pourla mettre en relation avec la taille de la fenêtre.

std::function<void()>& UserInteractableGraphicalInterface::getFunctionOfButton(std::string const& identifier)
{
	return m_buttons.at(identifier); // Throw an exception if not there.
}

float UserInteractableGraphicalInterface::getValueOfSlider(std::string const& identifier)
{
	Slider* slider{ &m_sliders.at(identifier) }; // Throws an exception if not there.
	sf::Sprite* cursor{ &m_sprites[m_dynamicSpriteIds["_c" + identifier]].first };
	sf::Sprite* background{ &m_sprites[m_dynamicSpriteIds['_' + identifier]].first};

	float minPos{ background->getGlobalBounds().position.y };
	float maxPos{ minPos + background->getGlobalBounds().size.y };
	float curPos{ cursor->getPosition().y};

	return slider->m_mathFunction(1 - (curPos- minPos) / (maxPos - minPos));
}

UserInteractableGraphicalInterface::Slider& UserInteractableGraphicalInterface::getSlider(std::string const& identifier)
{
	return m_sliders.at(identifier); // Throws an exception if not there.
}

bool UserInteractableGraphicalInterface::removeDText(std::string const& identifier) noexcept
{
	auto removeInButtons{ m_buttons.find(identifier) }; // Check if the identifier is in the buttons map.

	if (removeInButtons != m_buttons.end())
		m_buttons.erase(removeInButtons); // Erase from the buttons map if it exists.

	return DynamicGraphicalInterface::removeDText(identifier);
}

bool UserInteractableGraphicalInterface::removeSlider(std::string const& identifier) noexcept
{
	// No effect if not there
	removeDSprite('_' + identifier); // Remove the slider's background sprite.
	removeDSprite("_c" + identifier); // Remove the slider's cursor sprite.
	removeDText('_' + identifier); 

	auto removeSlider{ m_sliders.find(identifier) }; // Check if the identifier is in the sliders map.
	if (removeSlider != m_sliders.end())
		return false; // No effect if it does not exist.

	m_sliders.erase(removeSlider); // Erase from the sliders map
	return true;
}

UserInteractableGraphicalInterface::IdentifierInteractableItem UserInteractableGraphicalInterface::mouseMoved(FixedGraphicalInterface* activeGUI) noexcept
{
	UserInteractableGraphicalInterface* isDerived{ dynamic_cast<UserInteractableGraphicalInterface*>(activeGUI) };

	// Resets the hovered element.
	m_hoveredElement = std::make_pair(InteractableItem::None, nullptr);

	if (!isDerived)
		return m_hoveredElement; // Check if the gui is an interactable one.

	sf::Vector2f mousePos{ static_cast<sf::Vector2f>(sf::Mouse::getPosition(*(isDerived->m_window))) };

	// Check if the mouse is over a button or a slider.
	for (auto& button : isDerived->m_buttons)
	{
		if (isDerived->getDText(button.first).getText().getGlobalBounds().contains(mousePos))
		{
			m_hoveredElement = std::make_pair(InteractableItem::Button, &button.first);
			return m_hoveredElement;
		}
	}

	for (auto& slider : isDerived->m_sliders)
	{
		if (isDerived->getDSprite('_' + slider.first)->first.getGlobalBounds().contains(mousePos))
		{
			m_hoveredElement = std::make_pair(InteractableItem::Slider, &slider.first);
			return m_hoveredElement;
		}
	}

	for (auto& mqb : isDerived->m_mqbs)
	{
		int index{ isDerived->m_dynamicSpriteIds.find('_' + mqb.first + '0')->second };

		for (int i{ 0 }; i < mqb.second.m_numberOfBoxes; i++)
		{
			if (isDerived->m_sprites[index + i].first.getGlobalBounds().contains(mousePos))
			{
				mqb.second.m_currentlyHovered = i+1; // Update the currently hovered box index.
				m_hoveredElement = std::make_pair(InteractableItem::MQB, &mqb.first);
				return m_hoveredElement;
			}
		}
	}

	return m_hoveredElement;
}

UserInteractableGraphicalInterface::IdentifierInteractableItem UserInteractableGraphicalInterface::mousePressed(FixedGraphicalInterface* activeGUI) noexcept
{
	UserInteractableGraphicalInterface* isDerived{ dynamic_cast<UserInteractableGraphicalInterface*>(activeGUI) };
	if (m_hoveredElement.first == InteractableItem::None || !isDerived)
		return getHoveredInteractableItem(); // Check if the gui is an interactable one.

	if (m_hoveredElement.first == InteractableItem::Slider)
		isDerived->changeValueSlider(*m_hoveredElement.second, sf::Mouse::getPosition(*(isDerived->m_window)).y);
	// Button is executed when the mouse is released to avoid multiple calls for each frame it is kept pressed.
	// Same for MQB.

	return m_hoveredElement;
}

UserInteractableGraphicalInterface::IdentifierInteractableItem UserInteractableGraphicalInterface::mouseUnpressed(FixedGraphicalInterface* activeGUI) noexcept
{
	UserInteractableGraphicalInterface* isDerived{ dynamic_cast<UserInteractableGraphicalInterface*>(activeGUI) };

	if (m_hoveredElement.first == InteractableItem::Button)
	{
		isDerived->getFunctionOfButton(*m_hoveredElement.second)();
	}
	else if (m_hoveredElement.first == InteractableItem::MQB)
	{
		
	}

	return m_hoveredElement;
}

void UserInteractableGraphicalInterface::changeValueSlider(std::string const& id, int mousePosY)
{
	Slider* slider{ &m_sliders.at(id) }; // Throws an exception if not there.
	sf::Sprite* cursor{ &m_sprites[m_dynamicSpriteIds["_c" + id]].first};
	sf::Sprite* background{ &m_sprites[m_dynamicSpriteIds['_' + id]].first};

	float minPos{ background->getGlobalBounds().position.y};
	float maxPos{ minPos + background->getGlobalBounds().size.y };
	float yPos{ static_cast<float>(mousePosY) };

	if (yPos < minPos)
		yPos = minPos;
	else if (yPos > maxPos)
		yPos = maxPos;
	
	if (slider->m_intervals >= 0)
	{
		float interval = 1.f / (slider->m_intervals + 1); // The intervals exclude the min and max positions, so we add 1 to the number of intervals.
		float relativePos = (yPos - minPos) / (maxPos - minPos); // Between 0 and 1.
		float intervalPos = round(relativePos / interval) * interval; // Rounding to the nearest interval position.
		yPos = minPos + ((maxPos - minPos) * intervalPos); // Recalculating the position based on the intervals.
	}

	cursor->setPosition(sf::Vector2f{ cursor->getPosition().x, yPos });
	float value{ slider->m_mathFunction(1 - (yPos - minPos) / (maxPos - minPos)) }; // Get the value of the slider.

	//if (slider->m_textIterator != m_dynamicTextsIds.end()) // If the slider has a text associated with it.
	//{
	//	TextWrapper* text{ &m_texts[slider->m_textIterator->second] };

	//	text->updatePosition(sf::Vector2f{ text->getText().getPosition().x, yPos }); // Aligning the text with the cursor.
	//	text->updateContent(value);
	//}

	slider->m_userFunction(value); // Call the function associated with the slider.
}

//TODO: texte de slider
//TODO: fonction de suppression MQB
//TODO: update window resize