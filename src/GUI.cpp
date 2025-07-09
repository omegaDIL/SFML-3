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


IGInterface::IdentifierInteractableItem IGInterface::m_hoveredElement{}; // The type of the button that is currently hovered.

IGInterface* IGInterface::m_interfaceWithHoveredElement{ nullptr };




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


TextWrapper& DynamicGraphicalInterface::getDText(std::string const& identifier)
{
	return m_texts[m_dynamicTexts.at(identifier)]; // Throws an exception if not there.
}

SpriteWrapper& DynamicGraphicalInterface::getDSprite(std::string const& identifier)
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
// TODO: empecher le texte de slider de se superposer avec le slider
// TODO: userinteractablegraphicalinterface -> interactablegraphicalinterface
// TODO: find().second -> at() + faire des reserves pour les maps et les vectors afin d'eviter les reallocations de memoire.
// TODO: MouseMove d'abord verifier que le hovered actuel ne l 'est tjr pas avant de boucler sur le reste
// TODO: _ + mqb + to_string -> _ + to_string + _ + mqb
// TODO: Make the background srite fixed for the user.
// TODO: erasure texture ? -> parler a victor
// TODO: mettre __ au lieu de _ lorsque l'utilisateur ne doit pas interagir avec l'élément (ex: __cursorEditing pour le curseur d'édition de texte)
// TODO: ajouter aux lambdas un argument pour l'interface courante.
// TODO: mettre des int a la place des unsigned int pour les MQB, car c le bordel de faire -1 et on aura jamais plus de 2 millards de boxes dans 1 mqb
// TODO: see attribute [[assume]] for save and its wrappers
// TODO: rendre coherent le nommage des elements dynamiques -> __ + id quand ajoute par Interface && __nom__ quand ajoute par interface + sans id
// TODO: permettre de decocher tous les mqb.
// TODO: remove pointer from interactable element
// TODO: renommer fixed en basic

// TODO: Futur udpate :
// Multiple fonts
// module C++20
// Ajouter un parametre d'interface courante dans les std::functions
// Ajouter une std::function pour les mqb
// Ajouter un intRect/scale pour chaque texture dans SpriteWrapper
//