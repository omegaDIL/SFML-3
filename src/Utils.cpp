#include <SFML/Graphics.hpp>
#include <string>
#include <vector>
#include <stdexcept>
#include <ios>
#include <optional>
#include <algorithm>
#include <sstream>
#include "Utils.hpp"
#include "GUI.hpp"

extern sf::VideoMode windowSize;
extern std::string nameOfSoftware;


sf::Vector2u getStringSizeForDisplay(std::string const& str, unsigned int characterSize) noexcept
{
	unsigned int const sizeHeight{ 20 + static_cast<unsigned int>(std::count(str.begin(), str.end(), '\n'))*20 };
	unsigned int sizeWidth{ 10 };

	std::istringstream stream{ str };
	// Count the max number of characters in one single line.
	std::vector<std::string> lines{ std::istream_iterator<std::string>{ stream }, std::istream_iterator<std::string>{} };

	auto longest = std::max_element(lines.begin(), lines.end(), [](const std::string& a, const std::string& b) -> size_t
		{
			return a.length() < b.length();
		});

	if (longest != lines.end())
		sizeWidth = static_cast<unsigned int>(longest->length()) * 10;

	return { sizeWidth, sizeHeight };
}


void showErrorsUsingGUI(std::string const& errorMessage, std::string const& errorTitle) noexcept
{
	// Calculate the size needed to display the whole string.
	sf::VideoMode const windowErrorSize{ getStringSizeForDisplay(errorMessage) };
	
	GIText textError{};
	auto error{ textError.create(errorMessage, sf::Vector2f{ windowErrorSize.size.x / 2.f, windowErrorSize.size.y / 2.f }, 12) };

	if (error.has_value())
		return; // Ipossible to display the error in a GUI if there's no font.

	sf::RenderWindow errorWindow{ windowErrorSize, errorTitle };
	while (errorWindow.isOpen())
	{	// The function is blocking.
		while (const std::optional event = errorWindow.pollEvent())
		{
			if (event->is<sf::Event::Closed>() || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape))
				errorWindow.close();

			if (event->is<sf::Event::Resized>())
				errorWindow.setSize(sf::Vector2u{ windowErrorSize.size.x, windowErrorSize.size.y });
		}

		errorWindow.clear();
		errorWindow.draw(textError.getText());
		errorWindow.display();
	}
}


void handleEventResize(sf::RenderWindow* window) noexcept
{
	sf::Vector2u const maxSize{ sf::VideoMode::getDesktopMode().size };
	sf::Vector2u newSize{ static_cast<sf::Vector2f>(window->getSize()) };

	// Windows (OS) does not like window that are larger than its definition.$
	if (newSize.x > maxSize.x)
		newSize.x = maxSize.x;
	if (newSize.y > maxSize.y)
		newSize.y = maxSize.y;
	
	sf::Vector2f factor{ static_cast<float>(newSize.x) / windowSize.size.x, static_cast<float>(newSize.y) / windowSize.size.y };
	windowSize.size = newSize;
	
	window->create(windowSize, nameOfSoftware);
	GInterface::windowResized(factor);
}