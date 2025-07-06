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
#include <cstdint>
#include <format>
#include <initializer_list>
#include <stdexcept>
#include <functional>
#include <sstream>

#include <iomanip>
#include "GUI.hpp"
#include "GUITexturesLoader.hpp"
#include "Exceptions.hpp"

sf::Font FGInterface::TextWrapper::s_font{};
std::string const FGInterface::ressourcePath{ "../res/" };
std::list<sf::Texture> FGInterface::SpriteWrapper::s_sharedTextures{};
IGInterface::IdentifierInteractableItem IGInterface::m_hoveredElement{}; // The type of the button that is currently hovered.

std::unordered_map<std::string, std::list<sf::Texture>::iterator> FGInterface::SpriteWrapper::s_accessingSharedTexture{};
std::unordered_multimap<sf::RenderWindow*, FGInterface*> FGInterface::allInterfaces{};
IGInterface* IGInterface::m_interfaceWithHoveredElement{ nullptr };


std::optional<std::string> FGInterface::TextWrapper::loadFont() noexcept
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

void FGInterface::TextWrapper::computeNewOrigin() noexcept
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

FGInterface::SpriteWrapper::SpriteWrapper(std::string const& sharedTexture, sf::Vector2f pos, sf::Vector2f scale, sf::Angle rot, sf::IntRect rectangle, sf::Color color)
	: sf::Sprite{ *s_accessingSharedTexture.at(sharedTexture) }, m_curTextureIndex{ 0 }, hide{ false }
{
	m_textures.push_back(&(*s_accessingSharedTexture.at(sharedTexture))); // Add the texture to the vector of textures.

	if (rectangle.size == sf::Vector2i{ 0, 0 }) [[likely]]
		setTextureRect(sf::IntRect{ sf::Vector2i{}, static_cast<sf::Vector2i>(getTexture().getSize())});
	
	setPosition(pos);
	setScale(scale);
	setRotation(rot);
	setColor(color);
	setOrigin(getLocalBounds().getCenter());
}

FGInterface::SpriteWrapper::SpriteWrapper(sf::Texture texture, sf::Vector2f pos, sf::Vector2f scale, sf::Angle rot, sf::IntRect rectangle, sf::Color color) noexcept
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


bool DynamicGraphicalInterface::addDynamicSprite(std::string const& identifier, std::string const& texture, sf::Vector2f pos, sf::Vector2f scale, sf::Angle rot, sf::IntRect rectangle, sf::Color color) noexcept
{
	if (identifier.empty() || m_dynamicSprites.find(identifier) != m_dynamicSprites.end() )
		return false; // Not already added or identifier is empty.

	addSprite(texture, pos, scale, rot, rectangle, color);
	m_dynamicSprites[identifier] = m_sprites.size() - 1; // The last sprite added has the highest index.
	m_indexesForEachDynamicSprites[m_sprites.size() - 1] = identifier; // Add the index to the vector of indexes for dynamic sprites.

	return true;
}

bool DynamicGraphicalInterface::addDynamicSprite(std::string const& identifier, sf::Texture texture, sf::Vector2f pos, sf::Vector2f scale, sf::Angle rot, sf::IntRect rectangle, sf::Color color) noexcept
{
	if (identifier.empty() || m_dynamicSprites.find(identifier) != m_dynamicSprites.end())
		return false; // Not already added or identifier is empty.

	addSprite(texture, pos, scale, rot, rectangle, color);
	m_dynamicSprites[identifier] = m_sprites.size() - 1; // The last sprite added has the highest index.
	m_indexesForEachDynamicSprites[m_sprites.size() - 1] = identifier; // Add the index to the vector of indexes for dynamic sprites.

	return true;
}


FixedGraphicalInterface::TextWrapper& DynamicGraphicalInterface::getDText(std::string const& identifier)
{
	return m_texts[m_dynamicTexts.at(identifier)]; // Throws an exception if not there.
}

FixedGraphicalInterface::SpriteWrapper& DynamicGraphicalInterface::getDSprite(std::string const& identifier)
{
	return m_sprites[m_dynamicSprites.at(identifier)]; // Throws an exception if not there.
}

bool DynamicGraphicalInterface::removeDText(std::string const& identifier) noexcept
{
	return removeDynamicElement(identifier, m_texts, m_dynamicTexts, m_indexesForEachDynamicTexts); // Removes the text with the given identifier.
}

bool DynamicGraphicalInterface::removeDSprite(std::string const& identifier) noexcept
{
	return removeDynamicElement(identifier, m_sprites, m_dynamicSprites, m_indexesForEachDynamicSprites); // Removes the text with the given identifier.
}


bool UserInteractableGraphicalInterface::addButton(std::string const& identifier, std::function<void()> function) noexcept
{
	if (m_buttons.find(identifier) != m_buttons.end() || m_dynamicTexts.find(identifier) == m_dynamicTexts.end())
		return false; 

	m_buttons[identifier] = function; // Adds the button with the text id and the function.

	return true;
}

bool UserInteractableGraphicalInterface::addSlider(std::string const& identifier, sf::Vector2u size, sf::Vector2f pos, sf::Vector2f scale, std::function<float(float)> mathFunction, std::function<void(float)> userFunction, int interval, bool showValueWithText) noexcept
{
	if (m_sliders.find(identifier) != m_sliders.end()) [[unlikely]]
		return false; // Already added.

	if (!SpriteWrapper::checkIfTextureExists("__textureForSliderBackground")) [[unlikely]]
		SpriteWrapper::addSharedTexture("__textureForSliderBackground", loadSolidRectangeShapeWithOutline(size)); // Loads the default slider texture.
	if (!SpriteWrapper::checkIfTextureExists("__textureForSliderCursor")) [[unlikely]]
		SpriteWrapper::addSharedTexture("__textureForSliderCursor", loadSolidRectangeShapeWithOutline(sf::Vector2u{ static_cast<unsigned int>(size.x * 1.618), size.x })); // Loads the default slider texture.
		 
	addDynamicSprite("__sb" + identifier, "__textureForSliderBackground", pos, scale); // Adds a sprite for the slider background.
	addDynamicSprite("__sc" + identifier, "__textureForSliderCursor", getDSprite("__sb" + identifier).getSprite().getGlobalBounds().getCenter(), scale); // Adds a sprite for the slider cursor.
	//implementer le texte

	m_sliders[identifier] = Slider{ interval, userFunction, mathFunction };
	changeValueSlider(identifier, getDSprite("__sc" + identifier).getSprite().getPosition().y); // Set the initial value of the slider.

	return true;
}

bool UserInteractableGraphicalInterface::addMQB(std::string const& identifier, sf::Vector2f posInit, sf::Vector2f posDelta, sf::Vector2f scale, unsigned int numberOfBoxes, bool multipleChoices, unsigned int defaultChecked) noexcept
{
	if (m_mqbs.find(identifier) != m_mqbs.end() || identifier.empty() || numberOfBoxes == 0)
		return false; // Not already added or identifier is empty or invalid.

	if (!SpriteWrapper::checkIfTextureExists("__textureForUncheckedBoxMQB")) [[unlikely]]
		SpriteWrapper::addSharedTexture("__textureForUncheckedBoxMQB", loadSolidRectangeShapeWithOutline(sf::Vector2u{ 20, 20 })); // Loads the default unchecked texture.
	if (!SpriteWrapper::checkIfTextureExists("__textureForCheckedBoxMQB")) [[unlikely]]
		SpriteWrapper::addSharedTexture("__textureForCheckedBoxMQB", loadCheckBoxTexture(sf::Vector2u{ 20, 20 })); // Loads the default checked texture.

	for (unsigned int i{ 0 }; i < numberOfBoxes; ++i)
	{
		sf::Vector2f curPos{ posInit.x + posDelta.x * i, posInit.y + posDelta.y * i }; // Calculate the position of the checkbox.

		std::string id{ "__" + std::to_string(i) + "_" + identifier }; // Create the id for the checkbox.
		addDynamicSprite(id, "__textureForUncheckedBoxMQB", curPos, scale);
		getDSprite(id).addTexture("__textureForCheckedBoxMQB"); // Adds the texture to the sprite.

		if (i == defaultChecked) // If the box is checked by default, switch to the checked texture.
			getDSprite(id).switchToNextTexture();
	}

	m_mqbs[identifier] = MQB{ multipleChoices, numberOfBoxes, defaultChecked };

	return true;
}
//TODO: continuer à partir d'ici
std::function<void()>& UserInteractableGraphicalInterface::getFunctionOfButton(std::string const& identifier)
{
	return m_buttons.at(identifier); // Throw an exception if not there.
}

[[nodiscard]] inline std::vector<bool> const& UserInteractableGraphicalInterface::getStateOfMQB(std::string const& identifier)
{
	return m_mqbs.at(identifier).m_boxes; // Throws an exception if not there.
}
float UserInteractableGraphicalInterface::getValueOfSlider(std::string const& identifier)
{
	Slider* slider{ &m_sliders.at(identifier) }; // Throws an exception if not there.
	sf::Sprite const& cursor{ m_sprites[m_dynamicSprites["_c_" + identifier]].getSprite() };
	sf::Sprite const& background{ m_sprites[m_dynamicSprites['_' + identifier]].getSprite() };

	float minPos{ background.getGlobalBounds().position.y };
	float maxPos{ minPos + background.getGlobalBounds().size.y };
	float curPos{ cursor.getPosition().y};

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
	if (!doesSliderExist(identifier)) // Check if the slider exists.
		return false; // No effect if it does not exist.

	// No effect if not there
	removeDSprite("__sb" + identifier); // Remove the slider's background sprite.
	removeDSprite("__sc" + identifier); // Remove the slider's cursor sprite.
	//removeDText('_' + identifier); 

	m_sliders.erase(identifier); // Erase from the sliders map
	return true;
}

bool UserInteractableGraphicalInterface::removeMQB(std::string const& identifier) noexcept
{
	if (!doesMQBExist(identifier))
		return false; // No effect if it does not exist.

	unsigned int nbBox{ m_mqbs[identifier].m_numberOfBoxes }; 
	for (int i{ 0 }; i < nbBox; ++i)
	{
		std::string id{ "__" + std::to_string(i) + "_" + identifier }; // Create the id for the checkbox.
		removeDSprite(id); // Remove the sprite associated with the checkbox.
	}

	m_mqbs.erase(identifier); // Erase from the MQB map.
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
	m_interfaceWithHoveredElement = isDerived; // Update the interface with the hovered element.

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
		if (isDerived->getDSprite("__sb" + slider.first).getSprite().getGlobalBounds().contains(mousePos))
		{
			m_hoveredElement = std::make_pair(InteractableItem::Slider, &slider.first);
			return m_hoveredElement;
		}
	}

	for (auto& mqb : isDerived->m_mqbs)
	{
		size_t index{ isDerived->m_dynamicSprites.find("__0_" + mqb.first)->second };
		//std::string id{ "__" + std::to_string(i) + "_" + identifier }; // Create the id for the checkbox.


		for (int i{ 0 }; i < mqb.second.m_numberOfBoxes; ++i)
		{
			if (isDerived->m_sprites[index + i].getSprite().getGlobalBounds().contains(mousePos))
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

	if (m_hoveredElement.first == InteractableItem::None || !isDerived || isDerived != m_interfaceWithHoveredElement)
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
	
	if (isDerived == nullptr || isDerived != m_interfaceWithHoveredElement)
		return m_hoveredElement;

	InteractableItem const& itemType{ m_hoveredElement.first }; // Get the type of the hovered element.
	std::string const& identifier{ *m_hoveredElement.second }; // Get the identifier of the hovered element.

	if (itemType == InteractableItem::Button)
	{
		isDerived->getFunctionOfButton(identifier)(); // Call the function associated with the button.
	}
	else if (itemType == InteractableItem::MQB)
	{
		auto* mqb{ &isDerived->m_mqbs.at(identifier) };
		if (mqb->m_multipleChoices == true)
		{
			unsigned int currentBox{ mqb->m_currentlyHovered - 1 }; // Get the index of the currently hovered box.
			mqb->m_boxes[currentBox] = !mqb->m_boxes[currentBox];

			std::string id{ "__" + std::to_string(currentBox) + "_" + identifier }; // Create the id for the checkbox.
			isDerived->getDSprite(id).switchToNextTexture();
		}
		else
		{
			std::fill(mqb->m_boxes.begin(), mqb->m_boxes.end(), false); // Uncheck all boxes.
			mqb->m_boxes[mqb->m_currentlyHovered - 1] = true; // Check the currently hovered box.

			for (unsigned int i{ 0 }; i < mqb->m_numberOfBoxes; ++i)
			{
				std::string id{ "__" + std::to_string(i) + "_" + identifier }; // Create the id for the checkbox.
				int textureNumber{ (i == mqb->m_currentlyHovered - 1) ? 1 : 0 }; // 0 is unchecked texture, 1 is checked texture.

				isDerived->getDSprite(id).switchToNextTexture(textureNumber);
			}
		}
	}

	return m_hoveredElement;
}

void UserInteractableGraphicalInterface::changeValueSlider(std::string const& identifier, int mousePosY)
{
	Slider& slider{ m_sliders.at(identifier) }; // Throws an exception if not there.
	SpriteWrapper& cursor{ getDSprite("__sc" + identifier) };
	SpriteWrapper& background{ getDSprite("__sb" + identifier) };

	float minPos{ background.getSprite().getGlobalBounds().position.y};
	float maxPos{ minPos + background.getSprite().getGlobalBounds().size.y };
	float yPos{ static_cast<float>(mousePosY) };

	if (yPos < minPos)
		yPos = minPos;
	else if (yPos > maxPos)
		yPos = maxPos;
	
	if (slider.m_intervals >= 0)
	{
		float interval = 1.f / (slider.m_intervals + 1); // The intervals exclude the min and max positions, so we add 1 to the number of intervals.
		float relativePos = (yPos - minPos) / (maxPos - minPos); // Between 0 and 1.
		float intervalPos = round(relativePos / interval) * interval; // Rounding to the nearest interval position.
		yPos = minPos + ((maxPos - minPos) * intervalPos); // Recalculating the position based on the intervals.
	}

	cursor.setPosition(sf::Vector2f{ cursor.getSprite().getPosition().x, yPos });
	float value{ slider.m_mathFunction(1 - (yPos - minPos) / (maxPos - minPos)) }; // Get the value of the slider.

	if (m_dynamicTexts.find('_' + identifier) != m_dynamicTexts.end()) // If the slider has a text associated with it.
	{
		TextWrapper& text{ getDText('_' + identifier)};

		text.setPosition(sf::Vector2f{ text.getText().getPosition().x, yPos }); // Aligning the text with the cursor.
		text.setContent(value);
	}

	slider.m_userFunction(value); // Call the function associated with the slider.
}

WritableGraphicalInterface::WritableGraphicalInterface(sf::RenderWindow* window, std::string const& backgroundFileName, std::function<void(char32_t&, std::string&)> function) // TODO: retirer non parametre des autres lambda
	: UserInteractableGraphicalInterface{ window, backgroundFileName }, m_textEnteredFunction{ function }
{
	if (!SpriteWrapper::checkIfTextureExists("__writingCursorTexture__")) [[unlikely]]
	{
		sf::Image writingCursorImage{sf::Vector2u{1u, 1u}, sf::Color{20, 20, 20}};
		sf::Texture writingCursorTexture{ std::move(writingCursorImage) };
		writingCursorTexture.setRepeated(true);

		SpriteWrapper::addSharedTexture("__writingCursorTexture__", std::move(writingCursorTexture));
	}

	addDynamicSprite("__wc__", "__writingCursorTexture__", sf::Vector2f{ 0, 0 }, sf::Vector2f{ m_window->getSize().x / 500.f, window->getSize().x / 50.f });
	getDSprite("__wc__").hide = true; // Hide the cursor by default.
}

void WritableGraphicalInterface::setCurrentlyEditedText(std::string const& identifier) noexcept
{
	if (!m_currentlyEditedText.empty())
	{
		TextWrapper& text{ getDText(m_currentlyEditedText) }; // Get the text wrapper for the currently edited text.
		if (text.getText().getLocalBounds().size.x <= 0 || text.getText().getLocalBounds().size.y <= 0)
			text.setContent("N/A"); // If the text has no size, we set it to "N/A" to indicate that it is not valid.
	}

	sf::Text const& text{ getDText(identifier).getText() }; // Get the text wrapper for the currently edited text.
	SpriteWrapper& cursorSprite{ getDSprite("__wc__") };
	
	cursorSprite.setPosition(sf::Vector2f{ text.getGlobalBounds().position.x + text.getGlobalBounds().size.x, text.getGlobalBounds().getCenter().y });
	cursorSprite.hide = false; // Show the cursor.
	m_currentlyEditedText = identifier;
}


std::string WritableGraphicalInterface::textEntered(FixedGraphicalInterface* curInterface, char32_t unicodeValue) noexcept
{
	//TODO: verifier les printable characters.
	WGInterface* isDerived{ dynamic_cast<WGInterface*>(curInterface) };

	// check if the gui is writable and not currently editing a text and the validity of the unicode value.
	if (isDerived == nullptr || isDerived->m_currentlyEditedText == "" || unicodeValue > 0x007E)
		return ""; 

	TextWrapper* text{ &isDerived->getDText(isDerived->m_currentlyEditedText) }; // Get the text wrapper for the currently edited text.
	std::string newText{ text->getText().getString() };
	
	// Calling it before adding the new character to the text, so that the function can modify the text before it is updated.
	isDerived->m_textEnteredFunction(unicodeValue, newText); // Call the function associated with the text entered event.

	if (unicodeValue == 0x0008 && !newText.empty()) // Backspace
		newText.pop_back(); // Remove the last character.
	else if (unicodeValue == 0x000D || unicodeValue == 0x001B) // Enter or Escape 
		isDerived->m_currentlyEditedText = ""; // Stop editing the text.
	else if (unicodeValue != 0x0008) // If it is a printable character, add it to the text.
		newText += static_cast<char>(unicodeValue);
	
	isDerived->applyNewText(text, newText); // Apply the new text to the text wrapper.
	return newText; // Text was updated.
}

void WritableGraphicalInterface::applyNewText(TextWrapper* text, std::string& newText) noexcept
{
	SpriteWrapper& cursorSprite{ getDSprite("__wc__") };
	text->setContent(newText); // Update the text with the new content.

	if (m_currentlyEditedText == "" 
	&& (text->getText().getLocalBounds().size.x <= 0 || text->getText().getLocalBounds().size.y <= 0))
	{ // If the user finished updating the text, but it has no size, we set it to "N/A" to indicate that it is not valid.
		newText = "N/A"; // Not valid
		text->setContent(newText); // Update the text with the new content.
	}

	if (m_currentlyEditedText == "") // Hide the cursor
		cursorSprite.hide = true;
	else // Change the position of the cursor to the end of the text.
		cursorSprite.setPosition(sf::Vector2f{ text->getText().getGlobalBounds().position.x + text->getText().getGlobalBounds().size.x, text->getText().getGlobalBounds().getCenter().y });
	
}

// TODO: faire du statique plus souvent -> s_
// TODO: id -> IDENTIFIER
// TODO: multiple choice box -> multiple check boxes
// TODO: empecher le texte de slider de se superposer avec le slider
// TODO: userinteractablegraphicalinterface -> interactablegraphicalinterface
// TODO: find().second -> at() + faire des reserves pour les maps et les vectors afin d'eviter les reallocations de memoire.
// TODO: MouseMove d'abord verifier que le hovered actuel ne l 'est tjr pas avant de boucler sur le reste
// TODO: _ + mqb + to_string -> _ + to_string + _ + mqb
// TODO: Make the background srite fixed for the user.
// TODO: erasure texture ? -> parler a victor
// TODO: Fix mqb texture
// TODO: mettre __ au lieu de _ lorsque l'utilisateur ne doit pas interagir avec l'élément (ex: __cursorEditing pour le curseur d'édition de texte)
// TODO: ajouter aux lambdas un argument pour l'interface courante.
// TODO: deconseiller la suppression des dynamics, plutot les cacher
// TODO: mettre des int a la place des unsigned int pour les MQB, car c le bordel de faire -1 et on aura jamais plus de 2 millards de boxes dans 1 mqb
// TODO: see attribute [[assume]] for save and its wrappers
// TODO: rendre coherent le nommage des elements dynamiques -> __ + id quand ajoute par Interface && __nom__ quand ajoute par interface + sans id
// TODO: permettre de decocher tous les mqb.
// TODO: remove pointer from interactable element

// TODO: Futur udpate :
// Multiple fonts
// Ajouter un parametre d'interface courante dans les std::functions
// Ajouter une std::function pour les mqb
// Ajouter un intRect/scale pour chaque texture dans SpriteWrapper
// ajouter alignment
// 
// gauche et droite pour les textes. -> empecher texte de se superposer avec	le slider
