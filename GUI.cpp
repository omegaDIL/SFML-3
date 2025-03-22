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
#include <iomanip>
#include "GUI.hpp"

static std::string const mediaPath{ "media/" }; // The path to load the media
static float alignerCursorUI{ 0.99f };
static float alignerButtonBackgroundUI{ 1.015f };
extern sf::VideoMode windowSize;
extern std::string nameOfSoftware;


std::vector<Interface*> Interface::m_everyMenu{};
size_t MenuInterface::m_pointingButton{ 0 };
sf::Sprite MenuInterface::m_backgroundPointingButton{};


//////////////////////////////////Interface classes//////////////////////////////////

Interface::InterfaceText::InterfaceText(std::string const& content, sf::Vector2f pos, unsigned int characterSize) noexcept
	: sf::Text{}
{
	static std::unique_ptr<sf::Font> font{ nullptr };
	if (!font)
	{
		font = std::make_unique<sf::Font>();
		font->loadFromFile(mediaPath + "font.ttf");
		font->setSmooth(true);
	}

	setCharacterSize(characterSize);
	setFont(*font);
	setString(content);
	updatePos(pos);
	float factor = std::min(windowSize.height, windowSize.width) / 720.f;
	setScale(sf::Vector2f(factor, factor)); // The scaling factor must be the same. 
}

Interface::Interface(sf::RenderWindow* window, std::string backgroundPath)
	: m_window{ window }, m_spriteBackground{}, m_actualTextureBackground{}, m_textureBackground{}
{
	if (!m_window)
		throw std::invalid_argument{ "Window is nullptr.\nOccured inside the constructor of Interface.\n" };

	m_everyMenu.push_back(this);

	bool failedToLoad{ false }; // If it fails an exception is thrown.
	if (backgroundPath != "" && !m_actualTextureBackground.loadFromFile(mediaPath + backgroundPath))
		failedToLoad = true;
	else if (backgroundPath != "")
		m_actualTextureBackground.setSmooth(true);

	creatingBackgroundTexture();

	if (failedToLoad)
		throw std::invalid_argument{ "Media path to load interface's backround invalid.\nPath " + mediaPath + backgroundPath + ".\n" };
}

void Interface::windowResized(sf::Vector2f scalingFactor) noexcept
{
	for (auto i : m_everyMenu)
		i->resizingElements(scalingFactor);
}

void Interface::resizingElements(sf::Vector2f scalingFactor) noexcept
{
	creatingBackgroundTexture();
}

void Interface::creatingBackgroundTexture() noexcept
{
	sf::RenderTexture bufferForTexture{}; // This buffer contains the texture of the background.
	bufferForTexture.create(windowSize.width, windowSize.height);

	sf::Image imgDefaultBackground{}; // Contains the default color of the background in an image.
	imgDefaultBackground.create(windowSize.width, windowSize.height, sf::Color(20, 20, 20));
	sf::Texture textureDefaultBackground{}; // Contains the default color of the background in a texture.
	textureDefaultBackground.loadFromImage(imgDefaultBackground);
	textureDefaultBackground.setSmooth(true);
	sf::Sprite defaultBackground{ textureDefaultBackground }; // Contains the default color of the background in a sprite.
	bufferForTexture.draw(defaultBackground);

	sf::Sprite background{ m_actualTextureBackground }; // Contains the texture that user wants to have
	background.setPosition(windowSize.width / 2.f, windowSize.height / 2.f);
	background.setOrigin(m_actualTextureBackground.getSize().x / 2.f, m_actualTextureBackground.getSize().y / 2.f);
	bufferForTexture.draw(background); // Will override the default texture. If smaller than the default one, the parts that weren't cover will be displayed to fill in the rest.

	m_textureBackground.loadFromImage(bufferForTexture.getTexture().copyToImage());
	m_spriteBackground.setTexture(m_textureBackground, true);
	m_spriteBackground.setPosition(windowSize.width / 2.f, windowSize.height / 2.f);
	m_spriteBackground.setOrigin(windowSize.width / 2.f, windowSize.height / 2.f);
}


MenuInterface::MenuInterface(sf::RenderWindow* window, std::string backgroundPath)
	: Interface{ window, backgroundPath }, m_texts{}, m_buttons{}
{
	static std::unique_ptr<sf::Texture> textureButton{ nullptr }; // The texture for the buttons.
	if (textureButton == nullptr)
	{
		// Buttons background texture setup.
		textureButton = createGradientTexture(sf::Vector2u(255, 51), 48, 48, 48);
		textureButton->setSmooth(true);
		initializeSprite(m_backgroundPointingButton, *textureButton);
	}
}

void MenuInterface::mouseMoved()
{
	if (!m_window)
		throw std::logic_error{ "Window is nullptr.\nOccured inside the mouseMoved function of MenuInterface.\n" };

	// Get the mouse position in the window.
	sf::Vector2f const& mousePos{ static_cast<sf::Vector2f>(sf::Mouse::getPosition(*m_window)) };

	for (auto const& buttonPair : m_buttons)
	{
		if (buttonPair.second.first.getText().getGlobalBounds().contains(mousePos))
		{
			// Update the size of the button sprite.
			sf::Vector2f posBackgroundButton{ buttonPair.second.first.getText().getPosition() };
			m_backgroundPointingButton.setPosition(posBackgroundButton.x, posBackgroundButton.y * alignerButtonBackgroundUI);

			// Update the value of m_pointedButton.
			m_pointingButton = buttonPair.first;

			return;
		}
	}

	// If the mouse is not over any button, reset the pointed button to None.
	m_pointingButton = 0;
}

void MenuInterface::resizingElements(sf::Vector2f scalingFactor) noexcept
{
	Interface::resizingElements(scalingFactor);

	float factor = std::min(windowSize.height, windowSize.width) / 720.f;
	m_backgroundPointingButton.setScale(sf::Vector2f(factor, factor)); // The scaling factor must be the same.

	for (auto& item : m_texts)
		item.resizingElements(scalingFactor);
	for (auto& item : m_buttons)
		item.second.first.resizingElements(scalingFactor);

	mouseMoved();
}

void MenuInterface::draw() const
{
	Interface::draw();

	// Draw all the texts.
	for (auto const& item : m_texts)
		m_window->draw(item.getText());

	// Draw all the buttons.
	for (auto const& item : m_buttons)
		m_window->draw(item.second.first.getText());
	if (m_pointingButton != 0)
		m_window->draw(m_backgroundPointingButton);
}


DynamicMenuInterface::DynamicMenuInterface(sf::RenderWindow* window, std::string backgroundPath) noexcept
	: MenuInterface{ window, backgroundPath }, m_dynamicTexts{}, m_affiliatedIdButtonOfDynamicElements{ 0 }, m_backgroundAffiliatedButton{}
{
	// Button's background texture setup.
	static std::unique_ptr<sf::Texture> textureButton{ createGradientTexture(sf::Vector2u(255, 51), 250, 200, 0) }; // The texture for the buttons.
	initializeSprite(m_backgroundAffiliatedButton, *textureButton);
}

void DynamicMenuInterface::setAffiliatedButton(size_t identifier) noexcept
{
	auto const buttonElem{ m_buttons.find(identifier) };

	if (buttonElem != m_buttons.end())
	{
		sf::Vector2f posBackgroundButton{ buttonElem->second.first.getText().getPosition() };
		m_backgroundAffiliatedButton.setPosition(posBackgroundButton.x, posBackgroundButton.y * alignerButtonBackgroundUI);
		m_affiliatedIdButtonOfDynamicElements = identifier;
	}
}

void DynamicMenuInterface::resizingElements(sf::Vector2f scalingFactor) noexcept
{
	MenuInterface::resizingElements(scalingFactor);

	float factor = std::min(windowSize.height, windowSize.width) / 720.f;
	m_backgroundAffiliatedButton.setScale(sf::Vector2f(factor, factor)); // The scaling factor must be the same.
	if (m_affiliatedIdButtonOfDynamicElements != 0)
		m_backgroundAffiliatedButton.setPosition(m_buttons.find(m_affiliatedIdButtonOfDynamicElements)->second.first.getText().getPosition());

	for (auto& item : m_dynamicTexts)
		item.second.resizingElements(scalingFactor);
}

void DynamicMenuInterface::draw() const
{
	MenuInterface::draw();

	for (auto const& item : m_dynamicTexts)
		m_window->draw(item.second.getText());
	if (m_affiliatedIdButtonOfDynamicElements != 0)
		m_window->draw(m_backgroundAffiliatedButton);
}


UserWritableMenuInterface::UserWritableMenuInterface(sf::RenderWindow* window, std::string backgroundPath) noexcept
	: DynamicMenuInterface{ window, backgroundPath }, m_editableTexts{}, m_currentText{ nullptr }, m_previousValue{}, m_currentTextOnlyNumbers{ false }, m_cursor{}
{
	unsigned int minSize = std::min(windowSize.height, windowSize.width);
	m_cursor.setSize(sf::Vector2f{ minSize / 120.f, minSize / 24.f });
	m_cursor.setFillColor(sf::Color{ 127, 127, 127 });
}

std::string UserWritableMenuInterface::userTyping(int character) noexcept
{
	if (m_currentText == nullptr)
		return "";

	std::string content{ m_currentText->getText().getString() };
	sf::FloatRect textBounds{ m_currentText->getText().getGlobalBounds() };

	if (m_currentTextOnlyNumbers && (character < 48 || character > 57)) // Only numbers.
	{
		return "";
	}
	else if (character == 13)
	{	// 13 is carriage return.
		if (content.empty())
		{
			m_currentText->updateContent(m_previousValue);
			content = m_previousValue;
		}

		m_currentText = nullptr;
		m_previousValue = "";
		m_previousValue.shrink_to_fit();
		return content;
	}
	else if (character == 8) // 8 is backspace.
	{
		if (!content.empty())
			content.pop_back();
	}
	else if (textBounds.left > m_currentText->getText().getCharacterSize() && (textBounds.left + textBounds.width) < (windowSize.width - m_currentText->getText().getCharacterSize()))
	{
		content += static_cast<char>(character);
	}
	else
	{	// Default case.
		return "";
	}

	m_currentText->updateContent(content);
	textBounds = m_currentText->getText().getGlobalBounds();
	m_cursor.setPosition(textBounds.left + textBounds.width, m_currentText->getText().getPosition().y * alignerCursorUI);
	return "";
}

std::string UserWritableMenuInterface::mousePressed() noexcept
{
	// Get the mouse position in the window.
	sf::Vector2f const& mousePos{ static_cast<sf::Vector2f>(sf::Mouse::getPosition(*m_window)) };

	for (auto& buttonPair : m_editableTexts)
	{
		if (buttonPair.first.getText().getGlobalBounds().contains(mousePos))
		{
			// Reseting previous entry
			if (m_currentText != nullptr)
			{
				m_currentText->updateContent(m_previousValue);
				m_previousValue = "";
			}

			// Setting up for next entry.
			m_currentText = &buttonPair.first;
			m_previousValue = m_currentText->getText().getString();
			m_previousValue.shrink_to_fit();
			m_currentTextOnlyNumbers = buttonPair.second;
			sf::FloatRect textBounds{ m_currentText->getText().getGlobalBounds() };
			m_cursor.setPosition(textBounds.left + textBounds.width, m_currentText->getText().getPosition().y * alignerCursorUI);

			// No need to check further if the mouse is inside a button.
			return "";
		}
	}

	return MenuInterface::mousePressed();
}

void UserWritableMenuInterface::resizingElements(sf::Vector2f scalingFactor) noexcept
{
	DynamicMenuInterface::resizingElements(scalingFactor);

	for (auto& item : m_editableTexts)
		item.first.resizingElements(scalingFactor);

	float factor = std::min(windowSize.height, windowSize.width) / 720.f;
	m_cursor.setScale(sf::Vector2f(factor, factor)); // The scaling factor must be the same.
	if (m_currentText != nullptr)
	{
		sf::FloatRect textBounds{ m_currentText->getText().getGlobalBounds() };
		m_cursor.setPosition(textBounds.left + textBounds.width, m_currentText->getText().getPosition().y * alignerCursorUI);
	}
}

void UserWritableMenuInterface::draw() const noexcept
{
	DynamicMenuInterface::draw();

	for (auto const& item : m_editableTexts)
		m_window->draw(item.first.getText());
	if (m_currentText != nullptr)
		m_window->draw(m_cursor);
}


GameInterface::GameInterface(sf::RenderWindow* window, RulesGame const* const rules, std::string backgroundPath)
	: Interface{ window, backgroundPath }, m_scoreText{ "", sf::Vector2f{ windowSize.width * 0.1f, windowSize.height * 0.1f }, 30u }, m_score{ nullptr }, m_currentRules{ rules }, m_chronoText{ "", sf::Vector2f{ windowSize.width * 0.9f, windowSize.height * 0.1f }, 30u }, m_chronoTimer{}, m_chronoPoints{}
{
	createTexture();

	float constexpr numberButtons{ 3.f }; // The number of buttons in this interface.
	float const sizeWindowBorder{ 5.f };  // To avoid putting anything at the border, we set a margin.
	float const xGapBetweenTexts{ (windowSize.width / sizeWindowBorder) / numberButtons }; // The gap between each text.
	for (char i{ 0 }; i < 3; i++) // There are three slots of hearts
	{
		m_hearts.push_back(sf::Sprite());
		initializeSprite(m_hearts[i], *m_textures["Heart"], sf::Vector2f(xGapBetweenTexts * (i + 1), windowSize.height * 0.9f));
	}
	for (char i{ 0 }; i < 3; i++) // There are three slots of power ups
	{
		m_power_ups.push_back(sf::Sprite());
		initializeSprite(m_power_ups[i], *m_textures["EmptySlot"], sf::Vector2f(windowSize.width - (xGapBetweenTexts * (i + 1)), windowSize.height * 0.9f));
	}
}

void GameInterface::resizingElements(sf::Vector2f scalingFactor) noexcept
{
	m_scoreText.resizingElements(scalingFactor);
	m_chronoText.resizingElements(scalingFactor);
}

void GameInterface::draw() const noexcept
{
	//TODO: complete this part (GameInterface).

	// Calculation of points
	//if (m_chronoPoints.getElapsedTime().asMilliseconds() > m_currentRules.)
	//{
	//	m_chronoPoints.restart();
	//	m_score += m_currentRules.
	//}

	// Drawing.
	Interface::draw();

	m_chronoText.updateContent(m_chronoTimer.getElapsedTime().asSeconds());
	m_window->draw(m_scoreText.getText());
	m_window->draw(m_chronoText.getText());
}

void GameInterface::createTexture() noexcept
{
	//TODO: complete this function (GameInterface).
	sf::Vector2u sizeSmallIconTexture{ windowSize.width / 12, windowSize.height / 12 }; // The size of all textures

	// Texture empty power up slots
	sf::Image powerUpEmptyImage{}; // Creating a special texture for the empty slots of the power-up
	powerUpEmptyImage.create(sizeSmallIconTexture.x, sizeSmallIconTexture.y, sf::Color(0, 0, 0));
	for (int unsigned i{ 0 }; i < sizeSmallIconTexture.x; i++)
	{
		for (int unsigned j = 0; j < sizeSmallIconTexture.y; j++)
		{	// Based on the coordinates, the pixel has a different color
			if ((i >= sizeSmallIconTexture.x * 0.20 && i <= sizeSmallIconTexture.x * 0.80) && (j <= sizeSmallIconTexture.y * 0.20 || j >= sizeSmallIconTexture.y * 0.80))
				powerUpEmptyImage.setPixel(i, j, sf::Color(127, 127, 127));
			else if ((j >= sizeSmallIconTexture.y * 0.20 && j <= sizeSmallIconTexture.y * 0.80) && (i <= sizeSmallIconTexture.x * 0.20 || i >= sizeSmallIconTexture.x * 0.80))
				powerUpEmptyImage.setPixel(i, j, sf::Color(127, 127, 127));
			else if ((i >= sizeSmallIconTexture.x * 0.30 && i <= sizeSmallIconTexture.x * 0.70) && (j >= sizeSmallIconTexture.y * 0.45 && j <= sizeSmallIconTexture.y * 0.55))
				powerUpEmptyImage.setPixel(i, j, sf::Color(127, 127, 127));
			else if ((i >= sizeSmallIconTexture.x * 0.45 && i <= sizeSmallIconTexture.x * 0.55) && (j >= sizeSmallIconTexture.y * 0.30 && j <= sizeSmallIconTexture.y * 0.70))
				powerUpEmptyImage.setPixel(i, j, sf::Color(127, 127, 127));
		}
	}
	std::shared_ptr<sf::Texture> powerUpEmptyTexture{ std::make_shared<sf::Texture>() }; // The texture of the empty slots of the power-up
	powerUpEmptyTexture->loadFromImage(powerUpEmptyImage);
	m_textures["EmptySlot"] = std::move(powerUpEmptyTexture);

	// Creating the texture of the lost hearts
	sf::Image noHeartTextureImage{}; // Creating a special texture for the lost hearts
	noHeartTextureImage.create(sizeSmallIconTexture.x, sizeSmallIconTexture.y, sf::Color(0, 0, 0));
	for (int unsigned i{ 0 }; i < sizeSmallIconTexture.x; i++)
	{
		for (int unsigned j = 0; j < sizeSmallIconTexture.y; j++)
		{
			if ((j >= sizeSmallIconTexture.y * 0.2 && j <= sizeSmallIconTexture.y * 0.6) && (i <= sizeSmallIconTexture.x * 0.2 || i >= sizeSmallIconTexture.x * 0.8))
				noHeartTextureImage.setPixel(i, j, sf::Color(255, 0, 0));
			else if ((j <= sizeSmallIconTexture.y * 0.2 || (j >= sizeSmallIconTexture.y * 0.6 && j <= sizeSmallIconTexture.y * 0.8)) && ((i >= sizeSmallIconTexture.x * 0.2 && i <= sizeSmallIconTexture.x * 0.4) || (i >= sizeSmallIconTexture.x * 0.6 && i <= sizeSmallIconTexture.x * 0.8)))
				noHeartTextureImage.setPixel(i, j, sf::Color(255, 0, 0));
			else if (((j >= sizeSmallIconTexture.y * 0.2 && j <= sizeSmallIconTexture.y * 0.4) || j >= sizeSmallIconTexture.y * 0.8) && (i >= sizeSmallIconTexture.x * 0.4 && i <= sizeSmallIconTexture.x * 0.6))
				noHeartTextureImage.setPixel(i, j, sf::Color(255, 0, 0));
		}
	}
	std::shared_ptr<sf::Texture> noHeartTexture{ std::make_shared<sf::Texture>() }; // The texture of the lost hearts
	noHeartTexture->loadFromImage(noHeartTextureImage);
	m_textures["NoHeart"] = noHeartTexture;

	// Creating the texture of the hearts
	sf::Image heartTextureImage{}; // Creating a special texture for the hearts
	heartTextureImage.create(sizeSmallIconTexture.x, sizeSmallIconTexture.y, sf::Color(0, 0, 0));
	for (int unsigned i{ 0 }; i < sizeSmallIconTexture.x; i++)
	{
		for (int unsigned j = 0; j < sizeSmallIconTexture.y; j++)
		{
			if (j >= sizeSmallIconTexture.y * 0.2 && j <= sizeSmallIconTexture.y * 0.6)
				heartTextureImage.setPixel(i, j, sf::Color(255, 0, 0));
			else if ((i >= sizeSmallIconTexture.x * 0.2 && i <= sizeSmallIconTexture.x * 0.8) && (j >= sizeSmallIconTexture.y * 0.6 && j <= sizeSmallIconTexture.y * 0.8))
				heartTextureImage.setPixel(i, j, sf::Color(255, 0, 0));
			else if (((i >= sizeSmallIconTexture.x * 0.2 && i <= sizeSmallIconTexture.x * 0.4) || (i >= sizeSmallIconTexture.x * 0.6 && i <= sizeSmallIconTexture.x * 0.8)) && j <= sizeSmallIconTexture.y * 0.2)
				heartTextureImage.setPixel(i, j, sf::Color(255, 0, 0));
			else if ((i >= sizeSmallIconTexture.x * 0.4 && i <= sizeSmallIconTexture.x * 0.6) && j >= sizeSmallIconTexture.y * 0.8)
				heartTextureImage.setPixel(i, j, sf::Color(255, 0, 0));
		}
	}
	std::shared_ptr<sf::Texture> heartTexture{ std::make_shared<sf::Texture>() }; // The texture of the hearts
	heartTexture->loadFromImage(heartTextureImage);
	m_textures["Heart"] = heartTexture;
}

//////////////////////////////////Interface classes//////////////////////////////////


//////////////////////////////////Helper functions//////////////////////////////////

std::unique_ptr<sf::Texture> createGradientTexture(sf::Vector2u size, sf::Uint8 red, sf::Uint8 green, sf::Uint8 blue) noexcept
{
	sf::Image imgTexture{}; // The image for the button texture.
	float factor = std::min(windowSize.height, windowSize.width) / 720.f;
	sf::Vector2u adjustedSize{ static_cast<unsigned int>(size.x * factor), static_cast<unsigned int>(size.y * factor) };
	imgTexture.create(adjustedSize.x, adjustedSize.y, sf::Color(red, blue, green, 255)); // The full texture has the 255 opacity by default.

	sf::Uint8 alpha{ 0 }; // The alpha compenent that'll override the prvious one to make a gradient effect.
	unsigned short intervale = static_cast<unsigned short>(ceilf(imgTexture.getSize().x / 255.f)); // The intervale between which we increase the alpha component.
	intervale = intervale == 0 ? 1 : intervale; // Avoid division by 0.
	for (unsigned int x{ 0 }; x < adjustedSize.x; x++)
	{
		if (x % intervale == 0)
			alpha++;

		for (unsigned int y{ 0 }; y < adjustedSize.y; y++)
			imgTexture.setPixel(x, y, sf::Color(red, green, blue, alpha)); // The new alpha value overrides the previous one.	
	}

	auto texture{ std::make_unique<sf::Texture>() }; // The actual texture.
	texture->loadFromImage(imgTexture);
	return texture;
}

//////////////////////////////////Helper functions//////////////////////////////////


//////////////////////////////////Factory & managing functions//////////////////////////////////

void createElemForMainInterface(MenuInterface& menu, sf::RenderWindow& window) noexcept
{
	// Text.
	menu.addText(nameOfSoftware, sf::Vector2f(windowSize.width / 2.f, windowSize.height / 8.00f), 60u);
	menu.addText("by OmegaDIL", sf::Vector2f(windowSize.width / 2.f, windowSize.height / 1.05f), 10u);


	// Buttons.
	float constexpr numberButtons{ 3.f }; // The number of buttons in this interface.
	float const sizeWindowBorder{ 2.2f }; // To avoid putting anything at the border, we set a margin.
	float const xGapBetweenTexts{ (windowSize.height / sizeWindowBorder) / numberButtons }; // The gap between each text.

	menu.addButton(1, "Play", sf::Vector2f(windowSize.width * 0.5f, windowSize.height * 0.95f - xGapBetweenTexts * 3), 30u);
	menu.addButton(2, "Score", sf::Vector2f(windowSize.width * 0.5f, windowSize.height * 0.95f - xGapBetweenTexts * 2), 30u);
	menu.addButton(3, "Close", sf::Vector2f(windowSize.width * 0.5f, windowSize.height * 0.95f - xGapBetweenTexts * 1), 30u, [&window]() { window.close(); });

	menu.addButton(4, "Settings", sf::Vector2f(100.f + (windowSize.width - 720) / 64.f, windowSize.height * 0.05f), 20u);
	menu.addButton(5, "Keys", sf::Vector2f(100.f + (windowSize.width - 720) / 64.f, windowSize.height * 0.15f), 20u);
}

void createElemForScoreInterface(DynamicMenuInterface& menu, std::span<size_t> const& easyScore, std::span<size_t> const& mediumScore, std::span<size_t> const& hardScore, std::span<size_t> const& competitionScore, std::span<size_t> const& custom1Score, std::span<size_t> const& custom2Score, std::span<size_t> const& custom3Score) noexcept
{
	{   // Buttons.
		float constexpr numberButtons{ 7.f };	  // The number of buttons in this interface.
		float constexpr sizeWindowBorder{ 1.6f }; // To avoid putting anything at the border, we set a margin.
		float const xGapBetweenTexts{ (windowSize.height / sizeWindowBorder) / numberButtons }; // The gap between each text.

		menu.addButton(1, "Easy", sf::Vector2f(windowSize.width / 7.f, windowSize.height * 0.05f + xGapBetweenTexts * 1), 20u, [&menu, &easyScore]() { managingScoreInterfaceButtonPressed(menu, 1, easyScore); });
		menu.addButton(2, "Medium", sf::Vector2f(windowSize.width / 7.f, windowSize.height * 0.05f + xGapBetweenTexts * 2), 20u, [&menu, &mediumScore]() { managingScoreInterfaceButtonPressed(menu, 2, mediumScore); });
		menu.addButton(3, "Hard", sf::Vector2f(windowSize.width / 7.f, windowSize.height * 0.05f + xGapBetweenTexts * 3), 20u, [&menu, &hardScore]() { managingScoreInterfaceButtonPressed(menu, 3, hardScore); });
		menu.addButton(4, "Competiton", sf::Vector2f(windowSize.width / 7.f, windowSize.height * 0.05f + xGapBetweenTexts * 4), 20u, [&menu, competitionScore]() { managingScoreInterfaceButtonPressed(menu, 4, competitionScore); });
		menu.addButton(5, "Custom 1", sf::Vector2f(windowSize.width / 7.f, windowSize.height * 0.05f + xGapBetweenTexts * 5), 20u, [&menu, custom1Score]() { managingScoreInterfaceButtonPressed(menu, 5, custom1Score); });
		menu.addButton(6, "Custom 2", sf::Vector2f(windowSize.width / 7.f, windowSize.height * 0.05f + xGapBetweenTexts * 6), 20u, [&menu, custom2Score]() { managingScoreInterfaceButtonPressed(menu, 6, custom2Score); });
		menu.addButton(7, "Custom 3", sf::Vector2f(windowSize.width / 7.f, windowSize.height * 0.05f + xGapBetweenTexts * 7), 20u, [&menu, custom3Score]() { managingScoreInterfaceButtonPressed(menu, 7, custom3Score); });
		menu.addButton(8, "Set", sf::Vector2f(windowSize.width / 2.f, windowSize.height * 0.800f), 20u, [&menu]() { menu.setNewValueText("currentDif", menu.getAffiliatedButton()); });
		menu.addButton(9, "See the rules", sf::Vector2f(windowSize.width / 2.f, windowSize.height * 0.875f), 20u);
		menu.addButton(10, "Back", sf::Vector2f(windowSize.width / 2.f, windowSize.height * 0.950f), 20u);
	}

	{	// Texts.
		float constexpr numberButtons{ 10.f };	  // The number of buttons in this interface.
		float constexpr sizeWindowBorder{ 1.8f }; // To avoid putting anything at the border, we set a margin.
		float const xGapBetweenTexts{ (windowSize.height / sizeWindowBorder) / numberButtons }; // The gap between each text.

		menu.addText("current:\n", sf::Vector2f(windowSize.width / 7.f, windowSize.height / 1.125f), 20u);
		menu.addText("average:\n", sf::Vector2f(windowSize.width / 2.f, windowSize.height * 0.650f), 20u);
		menu.addDynamicText("sc1", "", sf::Vector2f(windowSize.width / 2.f, xGapBetweenTexts * 1), 16u);
		menu.addDynamicText("sc2", "", sf::Vector2f(windowSize.width / 2.f, xGapBetweenTexts * 2), 16u);
		menu.addDynamicText("sc3", "", sf::Vector2f(windowSize.width / 2.f, xGapBetweenTexts * 3), 16u);
		menu.addDynamicText("sc4", "", sf::Vector2f(windowSize.width / 2.f, xGapBetweenTexts * 4), 16u);
		menu.addDynamicText("sc5", "", sf::Vector2f(windowSize.width / 2.f, xGapBetweenTexts * 5), 16u);
		menu.addDynamicText("sc6", "", sf::Vector2f(windowSize.width / 2.f, xGapBetweenTexts * 6), 16u);
		menu.addDynamicText("sc7", "", sf::Vector2f(windowSize.width / 2.f, xGapBetweenTexts * 7), 16u);
		menu.addDynamicText("sc8", "", sf::Vector2f(windowSize.width / 2.f, xGapBetweenTexts * 8), 16u);
		menu.addDynamicText("sc9", "", sf::Vector2f(windowSize.width / 2.f, xGapBetweenTexts * 9), 16u);
		menu.addDynamicText("sc10", "", sf::Vector2f(windowSize.width / 2.f, xGapBetweenTexts * 10), 16u);
		menu.addDynamicText("scAverage", "", sf::Vector2f(windowSize.width / 2.f, windowSize.height * 0.675f), 20u);
		menu.addDynamicText("currentDif", "", sf::Vector2f(windowSize.width / 7.f, windowSize.height / 1.1f), 20u);
	}

	// Default values of dynamic elements.
	managingScoreInterfaceButtonPressed(menu, 1, easyScore);
}

void managingScoreInterfaceButtonPressed(DynamicMenuInterface& menu, size_t difficulty, std::span<size_t const> const score) noexcept
{
	// Updates the affiliated button.
	menu.setAffiliatedButton(difficulty);

	// Updates the scores.
	std::array<size_t, 10> largestScores{}; // Contains the 10 biggest score.
	std::partial_sort_copy(score.begin(), score.end(), largestScores.begin(), largestScores.begin() + 10, std::greater<int>());
	menu.setNewValueText("sc1", largestScores[0]);
	menu.setNewValueText("sc2", largestScores[1]);
	menu.setNewValueText("sc3", largestScores[2]);
	menu.setNewValueText("sc4", largestScores[3]);
	menu.setNewValueText("sc5", largestScores[4]);
	menu.setNewValueText("sc6", largestScores[5]);
	menu.setNewValueText("sc7", largestScores[6]);
	menu.setNewValueText("sc8", largestScores[7]);
	menu.setNewValueText("sc9", largestScores[8]);
	menu.setNewValueText("sc10", largestScores[9]);

	// Updates the average.
	size_t average{ 0 }; // Contains the average score.
	if (score.size())
	{
		std::ranges::for_each(score, [&average](auto i) { average += i; });
		average /= score.size();
	}
	menu.setNewValueText("scAverage", average);

	// Shows if it is a customizable difficulty.
	std::string isCustomDifficulty{}; // Change the content of the button: "customize it" if it is a custom difficulty, otherwise "see the rules".
	if (difficulty == 5
		|| difficulty == 6
		|| difficulty == 7)
		isCustomDifficulty = "Customize it";
	else if (difficulty == 1
		|| difficulty == 2
		|| difficulty == 3
		|| difficulty == 4)
		isCustomDifficulty = "See the rules";
	menu.setNewValueButton(9, isCustomDifficulty);
}

void createElemForCustomDiffInterface(UserWritableMenuInterface& menu, std::span<size_t> const& easyScore, std::span<size_t> const& mediumScore, std::span<size_t> const& hardScore, std::span<size_t> const& competitionScore, std::span<size_t> const& custom1Score, std::span<size_t> const& custom2Score, std::span<size_t> const& custom3Score) noexcept
{
	menu.addButton(1, "Back", sf::Vector2f(windowSize.width * 0.5f, windowSize.height * 0.8f), 20u);
	//TODO: complete this function (CustomizeDiff factory).
}

//////////////////////////////////Factory & managing functions//////////////////////////////////