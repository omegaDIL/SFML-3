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


void showErrorsUsingGUI(std::string const& errorMessage, std::string const& error) noexcept
{
	sf::VideoMode const windowErrorSize{ getStringSizeForDisplay(errorMessage) };
	std::string name{ nameOfSoftware + ": " + error + " error" };
	// The UI text to display the error message.
	Interface::InterfaceText textError{ errorMessage, sf::Vector2f{ windowErrorSize.size.x / 2.f, windowErrorSize.size.y / 2.f } , 12 }; 

	sf::RenderWindow errorWindow{ windowErrorSize, name };
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