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

extern sf::VideoMode windowSize;

sf::Font GraphicalFixedInterface::WrapperText::m_font{};
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
	auto error{ WrapperText::loadFont() };
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
	float const factor = std::min(scalingFactor.x, scalingFactor.y);

	auto interfaceRange{ allInterfaces.equal_range(window) }; // All interfaces associated with the resized window.
	for (auto elem{ interfaceRange.first }; elem != interfaceRange.second; elem++)
	{
		auto* curInterface{ elem->second };

		for (auto& text : curInterface->m_texts)
			text.windowResized(scalingFactor);

		// The background is the first sprite.
		curInterface->m_sprites[0].first.setPosition(sf::Vector2f{ window->getSize().x / 2.f, window->getSize().y / 2.f });
		//curInterface->m_sprites[0].first.setScale(static_cast<sf::Vector2f>(window->getSize()));

		for (int j{ 1 }; j < curInterface->m_sprites.size(); j++)
		{	// Avoiding the background by skipping index 0.
			sf::Sprite& curSprite{ curInterface->m_sprites[j].first };

			curSprite.scale(sf::Vector2f{ factor, factor });
			curSprite.setPosition(sf::Vector2f{ curSprite.getPosition().x * scalingFactor.x, curSprite.getPosition().y * scalingFactor.y });
		}
	}
}	// TODO: revoir resize pour changement de ratio avec background pas default (resolution). 

void GraphicalFixedInterface::resetTextureForSprites() noexcept
{
	for (auto& sprite : m_sprites)
		sprite.first.setTexture(sprite.second);
}

std::optional<std::string> GraphicalFixedInterface::WrapperText::loadFont() noexcept
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

void GraphicalFixedInterface::WrapperText::updateTransformables(sf::Vector2f pos) noexcept
{
	float const factor = std::min(windowSize.size.x, windowSize.size.y) / 1080.f;
	sf::FloatRect const textBounds{ getLocalBounds() }; // Cache bounds to avoid multiple calls.
	
	setScale(sf::Vector2f{ factor, factor }); // The scaling factor must be the same.
	setOrigin(sf::Vector2f{ textBounds.size.x / 2.f, textBounds.size.y / 2.f }); // Updating the origin is needed only when the content changes or when size character changes.
	setPosition(pos); // Update the position according to the new origin.
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

GraphicalFixedInterface::WrapperText& GraphicalDynamicInterface::getDText(std::string const& identifier)
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


/*
void GraphicalUserInteractableInterface::addButton(std::string const& identifier, Text text, sf::Texture texture) noexcept
{
	sf::Sprite button{ texture };

	auto rect{ button.getGlobalBounds() };
	button.setOrigin(sf::Vector2f{ rect.size.x / 2, rect.size.y / 2 });
	
	sf::Text const& getTransformables{ text.getText() };
	button.setRotation(getTransformables.getRotation());
	button.setScale(getTransformables.getScale());
	button.setPosition(getTransformables.getPosition());

	addText(std::move(text));
	addSprite(button, texture);

	m_indexButtons[identifier] = std::make_pair(m_sprites.size()-1, m_texts.size()-1);
}

sf::Sprite& GraphicalUserInteractableInterface::getButtonSprite(std::string const& identifier)
{
	return m_sprites[m_indexButtons.at(identifier).first].first;
}

sf::Texture& GraphicalUserInteractableInterface::getButtonTexture(std::string const& identifier)
{
	return m_sprites[m_indexButtons.at(identifier).second].second;
}

void GraphicalUserInteractableInterface::removeDText(std::string const& identifier) noexcept
{
	auto toRemove{ m_dynamicTextsIds.find(identifier) };

	if (toRemove == m_dynamicTextsIds.end())
		return;

	for (auto& it : m_indexButtons)
		if (it.second.second > toRemove->second)
			it.second.second--;

	GraphicalDynamicInterface::removeDText(identifier);
}

void GraphicalUserInteractableInterface::removeDSprite(std::string const& identifier) noexcept
{
	auto toRemove{ m_dynamicSpritesIds.find(identifier) };

	if (toRemove == m_dynamicSpritesIds.end())
		return;

	for (auto& it : m_indexButtons)
		if (it.second.first > toRemove->second)
			it.second.first--;

	GraphicalDynamicInterface::removeDSprite(identifier);
}

void GraphicalUserInteractableInterface::removeDButton(std::string const& identifier) noexcept
{
	auto toRemove{ m_indexButtons.find(identifier) };

	if (toRemove == m_indexButtons.end())
		return;

	size_t index{ toRemove->second.first };

	m_sprites.erase(std::next(m_sprites.begin(), index));
	m_texts.erase(std::next(m_texts.begin(), index));
	m_indexButtons.erase(toRemove);

	for (auto& it : m_indexButtons)
		if (it.second.first > index)
			it.second.first--;

	for (auto& it : m_dynamicSpritesIds)
		if (it.second > index)
			it.second--;

	for (auto& it : m_dynamicTextsIds)
		if (it.second > index)
			it.second--;
}

std::string GraphicalUserInteractableInterface::mouseMoved()
{
	if (!m_window)
		throw std::logic_error{ "Window of an interface is nullptr." };

	// Get the mouse position in the window.
	sf::Vector2f const& mousePos{ static_cast<sf::Vector2f>(sf::Mouse::getPosition(*m_window)) };

	for (auto const& button : m_indexButtons)
	{
		if (!m_texts[button.second.second].getText().getGlobalBounds().contains(mousePos))
			continue;

		// Set the value of the hovering button to the hovered button.
		m_hoveringButtonIdentifier = button.first;

		return m_hoveringButtonIdentifier;
	}

	sf::Sprite& cursor{ *m_slider.m_cursorSlider };
	cursor.setPosition(sf::Vector2f{ cursor.getPosition().x, mousePos.y});
	
	float maxPosCursor{ m_slider.m_backgroundSlider->getGlobalBounds().size.y + m_slider.m_backgroundSlider->getGlobalBounds().position.y };
	if (cursor.getPosition().y > maxPosCursor)
		cursor.setPosition(sf::Vector2f{ cursor.getPosition().x, maxPosCursor });
	float minPosCursor{ m_slider.m_backgroundSlider->getGlobalBounds().position.y };
	if (cursor.getPosition().y < minPosCursor)
		cursor.setPosition(sf::Vector2f{ cursor.getPosition().x, minPosCursor });

	return "";
}

void GraphicalUserInteractableInterface::mousePressed() noexcept
{
	if (!m_window)
		throw std::logic_error{ "Window of an interface is nullptr." };

	// Get the mouse position in the window.
	sf::Vector2f const& mousePos{ static_cast<sf::Vector2f>(sf::Mouse::getPosition(*m_window)) };

	
}

void GraphicalUserInteractableInterface::draw() const
{
	GraphicalFixedInterface::draw();

	m_window->draw(*m_slider.m_backgroundSlider);
	m_window->draw(*m_slider.m_cursorSlider);
}

GraphicalUserInteractableInterface::Slider::Slider(sf::Vector2f pos, unsigned int length, bool integers) noexcept
	: m_backgroundSlider{ nullptr }, m_cursorSlider{ nullptr } // TODO: Create the slider and the background to avoid using nullptr pointers.
{
	sf::Vector2u size{ 12u, length };

	loadDefaultTexture(size);
	m_backgroundSlider.reset(new sf::Sprite{ m_textureBackgroundSlider });
	m_cursorSlider.reset(new sf::Sprite{ m_textureCursorSlider });
	
	m_backgroundSlider->setPosition(pos);
	m_cursorSlider->setPosition(pos);

	sf::Vector2f const& backgroundSize{ m_backgroundSlider->getGlobalBounds().size };
	m_backgroundSlider->setOrigin(sf::Vector2f{ backgroundSize.x / 2.f, backgroundSize.y / 2.f } );
	sf::Vector2f const& cursorSize{ m_cursorSlider->getGlobalBounds().size };
	m_cursorSlider->setOrigin(sf::Vector2f{ cursorSize.x / 2.f, cursorSize.y / 2.f });


	//TODO: integers argument
}

void GraphicalUserInteractableInterface::Slider::changeTexture(std::string backgroundSlider, std::string cursorSlider)
{
	//TODO: completer cette fonction
}

float GraphicalUserInteractableInterface::Slider::getValue() const noexcept
{
	float cursorPos{ m_cursorSlider->getPosition().y };

	float minPos{ m_backgroundSlider->getGlobalBounds().position.y };
	float maxPos{ minPos + m_backgroundSlider->getGlobalBounds().size.y };

	return 1 - (cursorPos - minPos)/(maxPos - minPos);
}

void GraphicalUserInteractableInterface::Slider::loadDefaultTexture(sf::Vector2u size) noexcept
{
	static bool flag{ false }; // To check if the texture has already been created.
	if (flag) // Already created
		return;	

	constexpr sf::Color defaultFillColor{ 40, 40, 40 };
	constexpr sf::Color defaultOutlineColor{ 80, 80, 80 };

	sf::Vector2u sizeSlider{ static_cast<sf::Vector2u>(size) };
	float sizeOutline { std::min(sizeSlider.x, sizeSlider.y) / 5.f };
	sf::Image defaultBackgroundImg{ sizeSlider, defaultFillColor};
	for (unsigned int i{ 0 }; i < sizeSlider.x; i++)
		for (unsigned int j{ 0 }; j < sizeSlider.y; j++)
			if (i < sizeOutline || j < sizeOutline || sizeSlider.x - i - 1 < sizeOutline || sizeSlider.y - j - 1< sizeOutline)
				defaultBackgroundImg.setPixel(sf::Vector2u{ i, j }, defaultOutlineColor);

	sizeSlider = sf::Vector2u{ static_cast<unsigned int>(size.x * 4), static_cast<unsigned int>(size.x) };
	sizeOutline = std::min(sizeSlider.x, sizeSlider.y) / 5.f;
	sf::Image defaultSliderImg{ sizeSlider, defaultFillColor };
	for (unsigned int i{ 0 }; i < sizeSlider.x; i++)
		for (unsigned int j{ 0 }; j < sizeSlider.y; j++)
			if (i < sizeOutline || j < sizeOutline || sizeSlider.x - i - 1 < sizeOutline || sizeSlider.y - j - 1 < sizeOutline)
				defaultSliderImg.setPixel(sf::Vector2u{ i, j }, defaultOutlineColor);

	if (m_textureBackgroundSlider.loadFromImage(defaultBackgroundImg))
		m_textureBackgroundSlider.setSmooth(true);

	if(m_textureCursorSlider.loadFromImage(defaultSliderImg))
		m_textureCursorSlider.setSmooth(true);

	flag = true;
}

//void GraphicalDynamicInterface::setNewPos(std::string const& identifier, sf::Vector2f pos) noexcept
//{
//	auto it = m_dynamicTextsIds.find(identifier);
//	if (it != m_dynamicTextsIds.end())
//		m_texts[it->second].updatePosition(pos);
//
//	auto it = m_dynamicSpritesIds.find(identifier);
//	if (it != m_dynamicSpritesIds.end())
//		m_sprites[it->second].first.setPosition(pos);
//}
//
//void GraphicalDynamicInterface::setNewRot(std::string const& identifier, sf::Angle ang) noexcept
//{
//	auto it = m_dynamicTextsIds.find(identifier);
//	if (it != m_dynamicTextsIds.end())
//		m_texts[it->second].updatePosition(pos);
//
//	auto it = m_dynamicSpritesIds.find(identifier);
//	if (it != m_dynamicSpritesIds.end())
//		m_sprites[it->second].first.setPosition(pos);
//}
//
//void GraphicalDynamicInterface::setNewScale(std::string const& identifier, sf::Vector2f scale) noexcept
//{
//
//}
//
//void GraphicalDynamicInterface::setNewFillColor(std::string const& identifier, sf::Color color) noexcept
//{
//	auto it = m_dynamicTextsIds.find(identifier);
//	if (it != m_dynamicTextsIds.end())
//		m_texts[it->second].updateColor(color);
//
//	auto it = m_dynamicSpritesIds.find(identifier);
//	if (it != m_dynamicSpritesIds.end())
//		m_sprites[it->second].first.setColor(color);
//}

//static float const alignerCursorUI{ 0.99f };
//static float const alignerButtonBackgroundUI{ 1.015f };

//MenuInterface::MenuInterface(sf::RenderWindow* window, std::string backgroundPath)
//	: Interface{ window, backgroundPath }, m_texts{}//, m_buttons{}
//{
//	static std::unique_ptr<sf::Texture> textureButton{ nullptr }; // The texture for the buttons.
//
//	if (textureButton != nullptr)
//		return;
//
//	// Buttons background texture setup.
//	textureButton = createGradientTexture(sf::Vector2u(255, 51), 48, 48, 48);
//	textureButton->setSmooth(true);
//	m_backgroundPointingButton = std::make_unique<sf::Sprite>(*textureButton);
//	initializeSprite(*m_backgroundPointingButton);
//}
//
//void MenuInterface::mouseMoved()
//{
//	if (!m_window)
//		throw std::logic_error{ "Window is nullptr.\nOccured inside the mouseMoved function of MenuInterface.\n" };
//
//	// Get the mouse position in the window.
//	sf::Vector2f const& mousePos{ static_cast<sf::Vector2f>(sf::Mouse::getPosition(*m_window)) };
//
//	//for (auto const& buttonPair : m_buttons)
//	//{
//	//	if (buttonPair.second.first.getText().getGlobalBounds().contains(mousePos))
//	//	{
//	//		// Update the size of the button sprite.
//	//		sf::Vector2f posBackgroundButton{ buttonPair.second.first.getText().getPosition() };
//	//		m_backgroundPointingButton->setPosition(sf::Vector2f{ posBackgroundButton.x, posBackgroundButton.y * alignerButtonBackgroundUI });
//
//	//		// Update the value of m_pointedButton.
//	//		m_pointingButton = buttonPair.first;
//
//	//		return;
//	//	}
//	//}
//
//	// If the mouse is not over any button, reset the pointed button to None.
//	m_pointingButton = 0;
//}
//
//void MenuInterface::resizingElements(sf::Vector2f scalingFactor) noexcept
//{
//	Interface::resizingElements(scalingFactor);
//
//	float factor = std::min(windowSize.size.y, windowSize.size.x) / 720.f;
//	m_backgroundPointingButton->setScale(sf::Vector2f(factor, factor)); // The scaling factor must be the same.
//
//	for (auto& item : m_texts)
//		item.resizingElements(scalingFactor);
//	/*for (auto& item : m_buttons)
//		item.second.first.resizingElements(scalingFactor);*/
//
//	mouseMoved();
//}
//
//void MenuInterface::draw() const
//{
//	Interface::draw();
//
//	// Draw all the texts.
//	for (auto const& item : m_texts)
//		m_window->draw(item.getText());
//
//	// Draw all the buttons.
//	//for (auto const& item : m_buttons)
//		//m_window->draw(item.second.first.getText());
//	if (m_pointingButton != 0)
//		m_window->draw(*m_backgroundPointingButton);
//}
//
//
//DynamicMenuInterface::DynamicMenuInterface(sf::RenderWindow* window, std::string backgroundPath) noexcept
//	: MenuInterface{ window, backgroundPath }, m_dynamicTexts{}, m_affiliatedIdButtonOfDynamicElements{ 0 }, m_backgroundAffiliatedButton{nullptr}
//{
//	// Button's background texture setup.
//	static std::unique_ptr<sf::Texture> textureButton{ createGradientTexture(sf::Vector2u(255, 51), 250, 200, 0) }; // The texture for the buttons.
//	textureButton->setSmooth(true);
//	m_backgroundAffiliatedButton = std::make_unique<sf::Sprite>(*textureButton);
//	initializeSprite(*m_backgroundAffiliatedButton);
//}
//
//void DynamicMenuInterface::setAffiliatedButton(size_t identifier) noexcept
//{
//	/*auto const buttonElem{ m_buttons.find(identifier) };
//
//	if (buttonElem == m_buttons.end())
//		return;
//
//	sf::Vector2f posBackgroundButton{ buttonElem->second.first.getText().getPosition() };
//	m_backgroundAffiliatedButton->setPosition(sf::Vector2f{ posBackgroundButton.x, posBackgroundButton.y * alignerButtonBackgroundUI });
//	m_affiliatedIdButtonOfDynamicElements = identifier;*/
//}
//
//void DynamicMenuInterface::resizingElements(sf::Vector2f scalingFactor) noexcept
//{
//	MenuInterface::resizingElements(scalingFactor);
//
//	float factor = std::min(windowSize.size.y, windowSize.size.x) / 720.f;
//	m_backgroundAffiliatedButton->setScale(sf::Vector2f(factor, factor)); // The scaling factor must be the same.
//	/*if (m_affiliatedIdButtonOfDynamicElements != 0)
//		m_backgroundAffiliatedButton->setPosition(m_buttons.find(m_affiliatedIdButtonOfDynamicElements)->second.first.getText().getPosition());*/
//
//	for (auto& item : m_dynamicTexts)
//		item.second.resizingElements(scalingFactor);
//}
//
//void DynamicMenuInterface::draw() const
//{
//	MenuInterface::draw();
//
//	for (auto const& item : m_dynamicTexts)
//		m_window->draw(item.second.getText());
//	if (m_affiliatedIdButtonOfDynamicElements != 0)
//		m_window->draw(*m_backgroundAffiliatedButton);
//}
//
//
//UserWritableMenuInterface::UserWritableMenuInterface(sf::RenderWindow* window, std::string backgroundPath) noexcept
//	: DynamicMenuInterface{ window, backgroundPath }, m_editableTexts{}, m_currentText{ nullptr }, m_previousValue{}, m_currentTextOnlyNumbers{ false }, m_cursor{}
//{
//	unsigned int minSize = std::min(windowSize.size.y, windowSize.size.x);
//	m_cursor.setSize(sf::Vector2f{ minSize / 120.f, minSize / 24.f });
//	m_cursor.setFillColor(sf::Color{ 127, 127, 127 });
//}
//
//std::string UserWritableMenuInterface::userTyping(int character) noexcept
//{
//	if (m_currentText == nullptr)
//		return "";
//
//	std::string content{ m_currentText->getText().getString() };
//	sf::FloatRect textBounds{ m_currentText->getText().getGlobalBounds() };
//
//	if (m_currentTextOnlyNumbers && (character < 48 || character > 57)) // Only numbers.
//	{
//		return "";
//	}
//	else if (character == 13)
//	{	// 13 is carriage return.
//		if (content.empty())
//		{
//			m_currentText->updateContent(m_previousValue);
//			content = m_previousValue;
//		}
//
//		m_currentText = nullptr;
//		m_previousValue = "";
//		m_previousValue.shrink_to_fit();
//		return content;
//	}
//	else if (character == 8) // 8 is backspace.
//	{
//		if (!content.empty())
//			content.pop_back();
//	}
//	else if (textBounds.position.x > m_currentText->getText().getCharacterSize() && (textBounds.position.x + textBounds.size.x) < (windowSize.size.x - m_currentText->getText().getCharacterSize()))
//	{
//		content += static_cast<char>(character);
//	}
//	else
//	{	// Default case.
//		return "";
//	}
//
//	m_currentText->updateContent(content);
//	textBounds = m_currentText->getText().getGlobalBounds();
//	m_cursor.setPosition(sf::Vector2f{ textBounds.position.x + textBounds.size.x, m_currentText->getText().getPosition().y * alignerCursorUI });
//	return "";
//}
//
//std::string UserWritableMenuInterface::mousePressed() noexcept
//{
//	// Get the mouse position in the window.
//	sf::Vector2f const& mousePos{ static_cast<sf::Vector2f>(sf::Mouse::getPosition(*m_window)) };
//
//	for (auto& buttonPair : m_editableTexts)
//	{
//		if (buttonPair.first.getText().getGlobalBounds().contains(mousePos))
//		{
//			// Reseting previous entry
//			if (m_currentText != nullptr)
//			{
//				m_currentText->updateContent(m_previousValue);
//				m_previousValue = "";
//			}
//
//			// Setting up for next entry.
//			m_currentText = &buttonPair.first;
//			m_previousValue = m_currentText->getText().getString();
//			m_previousValue.shrink_to_fit();
//			m_currentTextOnlyNumbers = buttonPair.second;
//			sf::FloatRect textBounds{ m_currentText->getText().getGlobalBounds() };
//			m_cursor.setPosition(sf::Vector2f{ textBounds.position.x + textBounds.size.x, m_currentText->getText().getPosition().y * alignerCursorUI });
//
//			// No need to check further if the mouse is inside a button.
//			return "";
//		}
//	}
//
//	return MenuInterface::mousePressed();
//}
//
//void UserWritableMenuInterface::resizingElements(sf::Vector2f scalingFactor) noexcept
//{
//	DynamicMenuInterface::resizingElements(scalingFactor);
//
//	for (auto& item : m_editableTexts)
//		item.first.resizingElements(scalingFactor);
//
//	float factor = std::min(windowSize.size.y, windowSize.size.x) / 720.f;
//	m_cursor.setScale(sf::Vector2f(factor, factor)); // The scaling factor must be the same.
//	if (m_currentText != nullptr)
//	{
//		sf::FloatRect textBounds{ m_currentText->getText().getGlobalBounds() };
//		m_cursor.setPosition(sf::Vector2f{ textBounds.position.x + textBounds.size.x, m_currentText->getText().getPosition().y * alignerCursorUI } );
//	}
//}
//
//void UserWritableMenuInterface::draw() const noexcept
//{
//	DynamicMenuInterface::draw();
//
//	for (auto const& item : m_editableTexts)
//		m_window->draw(item.first.getText());
//	if (m_currentText != nullptr)
//		m_window->draw(m_cursor);
//}


//GameInterface::GameInterface(sf::RenderWindow* window, RulesGame const* const rules, std::string backgroundPath)
//	: Interface{ window, backgroundPath }, m_scoreText{ "", sf::Vector2f{ windowSize.size.x * 0.1f, windowSize.size.y * 0.1f }, 30u }, m_score{ nullptr }, m_currentRules{ rules }, m_chronoText{ "", sf::Vector2f{ windowSize.size.x * 0.9f, windowSize.size.y * 0.1f }, 30u }, m_chronoTimer{}, m_chronoPoints{}
//{
//	createTexture();
//
//	float constexpr numberButtons{ 3.f }; // The number of buttons in this interface.
//	float const sizeWindowBorder{ 5.f };  // To avoid putting anything at the border, we set a margin.
//	float const xGapBetweenTexts{ (windowSize.size.x / sizeWindowBorder) / numberButtons }; // The gap between each text.
//	for (char i{ 0 }; i < 3; i++) // There are three slots of hearts
//	{
//		m_hearts.push_back(sf::Sprite());
//		initializeSprite(m_hearts[i], *m_textures["Heart"], sf::Vector2f(xGapBetweenTexts * (i + 1), windowSize.size.y * 0.9f));
//	}
//	for (char i{ 0 }; i < 3; i++) // There are three slots of power ups
//	{
//		m_power_ups.push_back(sf::Sprite());
//		initializeSprite(m_power_ups[i], *m_textures["EmptySlot"], sf::Vector2f(windowSize.size.x - (xGapBetweenTexts * (i + 1)), windowSize.size.y * 0.9f));
//	}
//}
//
//void GameInterface::resizingElements(sf::Vector2f scalingFactor) noexcept
//{
//	m_scoreText.resizingElements(scalingFactor);
//	m_chronoText.resizingElements(scalingFactor);
//}
//
//void GameInterface::draw() const noexcept
//{
//
//
//	// Calculation of points
//	//if (m_chronoPoints.getElapsedTime().asMilliseconds() > m_currentRules.)
//	//{
//	//	m_chronoPoints.restart();
//	//	m_score += m_currentRules.
//	//}
//
//	// Drawing.
//	Interface::draw();
//
//	m_chronoText.updateContent(m_chronoTimer.getElapsedTime().asSeconds());
//	m_window->draw(m_scoreText.getText());
//	m_window->draw(m_chronoText.getText());
//}
//
//void GameInterface::createTexture() noexcept
//{
//
//	sf::Vector2u sizeSmallIconTexture{ windowSize.size.x / 12, windowSize.size.y / 12 }; // The size of all textures
//
//	// Texture empty power up slots
//	sf::Image powerUpEmptyImage{}; // Creating a special texture for the empty slots of the power-up
//	powerUpEmptyImage.create(sizeSmallIconTexture.x, sizeSmallIconTexture.y, sf::Color(0, 0, 0));
//	for (int unsigned i{ 0 }; i < sizeSmallIconTexture.x; i++)
//	{
//		for (int unsigned j = 0; j < sizeSmallIconTexture.y; j++)
//		{	// Based on the coordinates, the pixel has a different color
//			if ((i >= sizeSmallIconTexture.x * 0.20 && i <= sizeSmallIconTexture.x * 0.80) && (j <= sizeSmallIconTexture.y * 0.20 || j >= sizeSmallIconTexture.y * 0.80))
//				powerUpEmptyImage.setPixel(i, j, sf::Color(127, 127, 127));
//			else if ((j >= sizeSmallIconTexture.y * 0.20 && j <= sizeSmallIconTexture.y * 0.80) && (i <= sizeSmallIconTexture.x * 0.20 || i >= sizeSmallIconTexture.x * 0.80))
//				powerUpEmptyImage.setPixel(i, j, sf::Color(127, 127, 127));
//			else if ((i >= sizeSmallIconTexture.x * 0.30 && i <= sizeSmallIconTexture.x * 0.70) && (j >= sizeSmallIconTexture.y * 0.45 && j <= sizeSmallIconTexture.y * 0.55))
//				powerUpEmptyImage.setPixel(i, j, sf::Color(127, 127, 127));
//			else if ((i >= sizeSmallIconTexture.x * 0.45 && i <= sizeSmallIconTexture.x * 0.55) && (j >= sizeSmallIconTexture.y * 0.30 && j <= sizeSmallIconTexture.y * 0.70))
//				powerUpEmptyImage.setPixel(i, j, sf::Color(127, 127, 127));
//		}
//	}
//	std::shared_ptr<sf::Texture> powerUpEmptyTexture{ std::make_shared<sf::Texture>() }; // The texture of the empty slots of the power-up
//	powerUpEmptyTexture->loadFromImage(powerUpEmptyImage);
//	m_textures["EmptySlot"] = std::move(powerUpEmptyTexture);
//
//	// Creating the texture of the lost hearts
//	sf::Image noHeartTextureImage{}; // Creating a special texture for the lost hearts
//	noHeartTextureImage.create(sizeSmallIconTexture.x, sizeSmallIconTexture.y, sf::Color(0, 0, 0));
//	for (int unsigned i{ 0 }; i < sizeSmallIconTexture.x; i++)
//	{
//		for (int unsigned j = 0; j < sizeSmallIconTexture.y; j++)
//		{
//			if ((j >= sizeSmallIconTexture.y * 0.2 && j <= sizeSmallIconTexture.y * 0.6) && (i <= sizeSmallIconTexture.x * 0.2 || i >= sizeSmallIconTexture.x * 0.8))
//				noHeartTextureImage.setPixel(i, j, sf::Color(255, 0, 0));
//			else if ((j <= sizeSmallIconTexture.y * 0.2 || (j >= sizeSmallIconTexture.y * 0.6 && j <= sizeSmallIconTexture.y * 0.8)) && ((i >= sizeSmallIconTexture.x * 0.2 && i <= sizeSmallIconTexture.x * 0.4) || (i >= sizeSmallIconTexture.x * 0.6 && i <= sizeSmallIconTexture.x * 0.8)))
//				noHeartTextureImage.setPixel(i, j, sf::Color(255, 0, 0));
//			else if (((j >= sizeSmallIconTexture.y * 0.2 && j <= sizeSmallIconTexture.y * 0.4) || j >= sizeSmallIconTexture.y * 0.8) && (i >= sizeSmallIconTexture.x * 0.4 && i <= sizeSmallIconTexture.x * 0.6))
//				noHeartTextureImage.setPixel(i, j, sf::Color(255, 0, 0));
//		}
//	}
//	std::shared_ptr<sf::Texture> noHeartTexture{ std::make_shared<sf::Texture>() }; // The texture of the lost hearts
//	noHeartTexture->loadFromImage(noHeartTextureImage);
//	m_textures["NoHeart"] = noHeartTexture;
//
//	// Creating the texture of the hearts
//	sf::Image heartTextureImage{}; // Creating a special texture for the hearts
//	heartTextureImage.create(sizeSmallIconTexture.x, sizeSmallIconTexture.y, sf::Color(0, 0, 0));
//	for (int unsigned i{ 0 }; i < sizeSmallIconTexture.x; i++)
//	{
//		for (int unsigned j = 0; j < sizeSmallIconTexture.y; j++)
//		{
//			if (j >= sizeSmallIconTexture.y * 0.2 && j <= sizeSmallIconTexture.y * 0.6)
//				heartTextureImage.setPixel(i, j, sf::Color(255, 0, 0));
//			else if ((i >= sizeSmallIconTexture.x * 0.2 && i <= sizeSmallIconTexture.x * 0.8) && (j >= sizeSmallIconTexture.y * 0.6 && j <= sizeSmallIconTexture.y * 0.8))
//				heartTextureImage.setPixel(i, j, sf::Color(255, 0, 0));
//			else if (((i >= sizeSmallIconTexture.x * 0.2 && i <= sizeSmallIconTexture.x * 0.4) || (i >= sizeSmallIconTexture.x * 0.6 && i <= sizeSmallIconTexture.x * 0.8)) && j <= sizeSmallIconTexture.y * 0.2)
//				heartTextureImage.setPixel(i, j, sf::Color(255, 0, 0));
//			else if ((i >= sizeSmallIconTexture.x * 0.4 && i <= sizeSmallIconTexture.x * 0.6) && j >= sizeSmallIconTexture.y * 0.8)
//				heartTextureImage.setPixel(i, j, sf::Color(255, 0, 0));
//		}
//	}
//	std::shared_ptr<sf::Texture> heartTexture{ std::make_shared<sf::Texture>() }; // The texture of the hearts
//	heartTexture->loadFromImage(heartTextureImage);
//	m_textures["Heart"] = heartTexture;
//}

//////////////////////////////////Interface classes//////////////////////////////////


//////////////////////////////////Helper functions//////////////////////////////////

//std::unique_ptr<sf::Texture> createGradientTexture(sf::Vector2u size, uint8_t red, uint8_t green, uint8_t blue) noexcept
//{
//	float factor = std::min(windowSize.size.x, windowSize.size.y) / 720.f;
//	sf::Vector2u adjustedSize{ static_cast<unsigned int>(size.x * factor), static_cast<unsigned int>(size.y * factor) };
//
//	sf::Image imgTexture{ adjustedSize, sf::Color{ red, blue, green, 255 } }; // The image for the button texture.
//	
//	uint8_t alpha{ 0 }; // The alpha compenent that'll override the prvious one to make a gradient effect.
//	unsigned short intervale = static_cast<unsigned short>(ceilf(imgTexture.getSize().x / 255.f)); // The intervale between which we increase the alpha component.
//	intervale = intervale == 0 ? 1 : intervale; // Avoid division by 0.
//	for (unsigned int x{ 0 }; x < adjustedSize.x; x++)
//	{
//		if (x % intervale == 0)
//			alpha++;
//
//		for (unsigned int y{ 0 }; y < adjustedSize.y; y++)
//			imgTexture.setPixel(sf::Vector2u{ x, y }, sf::Color{ red, green, blue, alpha }); // The new alpha value overrides the previous one.	
//	}
//
//	auto texture{ std::make_unique<sf::Texture>() }; // The actual texture.
//	texture->loadFromImage(imgTexture);
//	return texture;
//}
//
////////////////////////////////////Helper functions//////////////////////////////////
//
//
////////////////////////////////////Factory & managing functions//////////////////////////////////
//
//void createElemForMainInterface(MenuInterface& menu, sf::RenderWindow& window) noexcept
//{
//	// Text.
//	menu.addText(nameOfSoftware, sf::Vector2f(windowSize.size.x / 2.f, windowSize.size.y / 8.00f), 60u);
//	menu.addText("by OmegaDIL", sf::Vector2f(windowSize.size.x / 2.f, windowSize.size.y / 1.05f), 10u);
//
//
//	// Buttons.
//	float constexpr numberButtons{ 3.f }; // The number of buttons in this interface.
//	float const sizeWindowBorder{ 2.2f }; // To avoid putting anything at the border, we set a margin.
//	float const xGapBetweenTexts{ (windowSize.size.y / sizeWindowBorder) / numberButtons }; // The gap between each text.
//
//	menu.addButton(1, "Play", sf::Vector2f(windowSize.size.x * 0.5f, windowSize.size.y * 0.95f - xGapBetweenTexts * 3), 30u);
//	menu.addButton(2, "Score", sf::Vector2f(windowSize.size.x * 0.5f, windowSize.size.y * 0.95f - xGapBetweenTexts * 2), 30u);
//	menu.addButton(3, "Close", sf::Vector2f(windowSize.size.x * 0.5f, windowSize.size.y * 0.95f - xGapBetweenTexts * 1), 30u, [&window]() { window.close(); });
//
//	menu.addButton(4, "Settings", sf::Vector2f(100.f + (windowSize.size.x - 720) / 64.f, windowSize.size.y * 0.05f), 20u);
//	menu.addButton(5, "Keys", sf::Vector2f(100.f + (windowSize.size.x - 720) / 64.f, windowSize.size.y * 0.15f), 20u);
//}
//
//void createElemForScoreInterface(DynamicMenuInterface& menu, std::span<size_t> const& easyScore, std::span<size_t> const& mediumScore, std::span<size_t> const& hardScore, std::span<size_t> const& competitionScore, std::span<size_t> const& custom1Score, std::span<size_t> const& custom2Score, std::span<size_t> const& custom3Score) noexcept
//{
//	{   // Buttons.
//		float constexpr numberButtons{ 7.f };	  // The number of buttons in this interface.
//		float constexpr sizeWindowBorder{ 1.6f }; // To avoid putting anything at the border, we set a margin.
//		float const xGapBetweenTexts{ (windowSize.size.y / sizeWindowBorder) / numberButtons }; // The gap between each text.
//
//		menu.addButton(1, "Easy", sf::Vector2f(windowSize.size.x / 7.f, windowSize.size.y * 0.05f + xGapBetweenTexts * 1), 20u, [&menu, &easyScore]() { managingScoreInterfaceButtonPressed(menu, 1, easyScore); });
//		menu.addButton(2, "Medium", sf::Vector2f(windowSize.size.x / 7.f, windowSize.size.y * 0.05f + xGapBetweenTexts * 2), 20u, [&menu, &mediumScore]() { managingScoreInterfaceButtonPressed(menu, 2, mediumScore); });
//		menu.addButton(3, "Hard", sf::Vector2f(windowSize.size.x / 7.f, windowSize.size.y * 0.05f + xGapBetweenTexts * 3), 20u, [&menu, &hardScore]() { managingScoreInterfaceButtonPressed(menu, 3, hardScore); });
//		menu.addButton(4, "Competiton", sf::Vector2f(windowSize.size.x / 7.f, windowSize.size.y * 0.05f + xGapBetweenTexts * 4), 20u, [&menu, competitionScore]() { managingScoreInterfaceButtonPressed(menu, 4, competitionScore); });
//		menu.addButton(5, "Custom 1", sf::Vector2f(windowSize.size.x / 7.f, windowSize.size.y * 0.05f + xGapBetweenTexts * 5), 20u, [&menu, custom1Score]() { managingScoreInterfaceButtonPressed(menu, 5, custom1Score); });
//		menu.addButton(6, "Custom 2", sf::Vector2f(windowSize.size.x / 7.f, windowSize.size.y * 0.05f + xGapBetweenTexts * 6), 20u, [&menu, custom2Score]() { managingScoreInterfaceButtonPressed(menu, 6, custom2Score); });
//		menu.addButton(7, "Custom 3", sf::Vector2f(windowSize.size.x / 7.f, windowSize.size.y * 0.05f + xGapBetweenTexts * 7), 20u, [&menu, custom3Score]() { managingScoreInterfaceButtonPressed(menu, 7, custom3Score); });
//		menu.addButton(8, "Set", sf::Vector2f(windowSize.size.x / 2.f, windowSize.size.y * 0.800f), 20u, [&menu]() { menu.setNewValueText("currentDif", menu.getAffiliatedButton()); });
//		menu.addButton(9, "See the rules", sf::Vector2f(windowSize.size.x / 2.f, windowSize.size.y * 0.875f), 20u);
//		menu.addButton(10, "Back", sf::Vector2f(windowSize.size.x / 2.f, windowSize.size.y * 0.950f), 20u);
//	}
//
//	{	// Texts.
//		float constexpr numberButtons{ 10.f };	  // The number of buttons in this interface.
//		float constexpr sizeWindowBorder{ 1.8f }; // To avoid putting anything at the border, we set a margin.
//		float const xGapBetweenTexts{ (windowSize.size.y / sizeWindowBorder) / numberButtons }; // The gap between each text.
//
//		menu.addText("current:\n", sf::Vector2f(windowSize.size.x / 7.f, windowSize.size.y / 1.125f), 20u);
//		menu.addText("average:\n", sf::Vector2f(windowSize.size.x / 2.f, windowSize.size.y * 0.650f), 20u);
//		menu.addDynamicText("sc1", "", sf::Vector2f(windowSize.size.x / 2.f, xGapBetweenTexts * 1), 16u);
//		menu.addDynamicText("sc2", "", sf::Vector2f(windowSize.size.x / 2.f, xGapBetweenTexts * 2), 16u);
//		menu.addDynamicText("sc3", "", sf::Vector2f(windowSize.size.x / 2.f, xGapBetweenTexts * 3), 16u);
//		menu.addDynamicText("sc4", "", sf::Vector2f(windowSize.size.x / 2.f, xGapBetweenTexts * 4), 16u);
//		menu.addDynamicText("sc5", "", sf::Vector2f(windowSize.size.x / 2.f, xGapBetweenTexts * 5), 16u);
//		menu.addDynamicText("sc6", "", sf::Vector2f(windowSize.size.x / 2.f, xGapBetweenTexts * 6), 16u);
//		menu.addDynamicText("sc7", "", sf::Vector2f(windowSize.size.x / 2.f, xGapBetweenTexts * 7), 16u);
//		menu.addDynamicText("sc8", "", sf::Vector2f(windowSize.size.x / 2.f, xGapBetweenTexts * 8), 16u);
//		menu.addDynamicText("sc9", "", sf::Vector2f(windowSize.size.x / 2.f, xGapBetweenTexts * 9), 16u);
//		menu.addDynamicText("sc10", "", sf::Vector2f(windowSize.size.x / 2.f, xGapBetweenTexts * 10), 16u);
//		menu.addDynamicText("scAverage", "", sf::Vector2f(windowSize.size.x / 2.f, windowSize.size.y * 0.675f), 20u);
//		menu.addDynamicText("currentDif", "", sf::Vector2f(windowSize.size.x / 7.f, windowSize.size.y / 1.1f), 20u);
//	}
//
//	// Default values of dynamic elements.
//	managingScoreInterfaceButtonPressed(menu, 1, easyScore);
//}
//
//void managingScoreInterfaceButtonPressed(DynamicMenuInterface& menu, size_t difficulty, std::span<size_t const> const score) noexcept
//{
//	// Updates the affiliated button.
//	menu.setAffiliatedButton(difficulty);
//
//	// Updates the scores.
//	std::array<size_t, 10> largestScores{}; // Contains the 10 biggest score.
//	std::partial_sort_copy(score.begin(), score.end(), largestScores.begin(), largestScores.begin() + 10, std::greater<int>());
//	menu.setNewValueText("sc1", largestScores[0]);
//	menu.setNewValueText("sc2", largestScores[1]);
//	menu.setNewValueText("sc3", largestScores[2]);
//	menu.setNewValueText("sc4", largestScores[3]);
//	menu.setNewValueText("sc5", largestScores[4]);
//	menu.setNewValueText("sc6", largestScores[5]);
//	menu.setNewValueText("sc7", largestScores[6]);
//	menu.setNewValueText("sc8", largestScores[7]);
//	menu.setNewValueText("sc9", largestScores[8]);
//	menu.setNewValueText("sc10", largestScores[9]);
//
//	// Updates the average.
//	size_t average{ 0 }; // Contains the average score.
//	if (score.size())
//	{
//		std::ranges::for_each(score, [&average](auto i) { average += i; });
//		average /= score.size();
//	}
//	menu.setNewValueText("scAverage", average);
//
//	// Shows if it is a customizable difficulty.
//	std::string isCustomDifficulty{}; // Change the content of the button: "customize it" if it is a custom difficulty, otherwise "see the rules".
//	if (difficulty == 5
//		|| difficulty == 6
//		|| difficulty == 7)
//		isCustomDifficulty = "Customize it";
//	else if (difficulty == 1
//		|| difficulty == 2
//		|| difficulty == 3
//		|| difficulty == 4)
//		isCustomDifficulty = "See the rules";
//	menu.setNewValueButton(9, isCustomDifficulty);
//}
//
//void createElemForCustomDiffInterface(UserWritableMenuInterface& menu, std::span<size_t> const& easyScore, std::span<size_t> const& mediumScore, std::span<size_t> const& hardScore, std::span<size_t> const& competitionScore, std::span<size_t> const& custom1Score, std::span<size_t> const& custom2Score, std::span<size_t> const& custom3Score) noexcept
//{
//	menu.addButton(1, "Back", sf::Vector2f(windowSize.size.x * 0.5f, windowSize.size.y * 0.8f), 20u);
//
//}

//////////////////////////////////Factory & managing functions//////////////////////////////////