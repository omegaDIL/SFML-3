#include <SFML/Graphics.hpp>
#include <string>
#include <vector>
#include <algorithm>
#include <iterator>
#include <sstream>
#include <optional>
#include "Utils.hpp"
#include "Exceptions.hpp"
#include "GUI.hpp"


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
}//TODO: characterSize


void showErrorsUsingGUI(std::string const& errorMessage, std::string const& errorTitle) noexcept
{
	// Calculate the size needed to display the whole string.
	sf::VideoMode const windowErrorSize{ getStringSizeForDisplay(errorMessage) };
	
	sf::RenderWindow errorWindow{ windowErrorSize, errorTitle };
	GraphicalFixedInterface gui{ &errorWindow }; // Create the interface to use the GUI.
	gui.addText(errorMessage, sf::Vector2f{ windowErrorSize.size.x / 2.f, windowErrorSize.size.y / 2.f }, 12, 1.f);

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
		gui.draw();
		errorWindow.display();
	}
}


void handleEventResize(sf::RenderWindow* window) noexcept
{
	// With SFML, when a window is resized, it occupies the corresponding area of the screen which is 
	// a certain number of pixel. However, the inner size of the window, that is to say the number of 
	// pixel it was created with, is not updated. Therefore, it still displays the same view, with 
	// distorded pixels to match the new size of the window. Thus, we need to manually update the inner
	// size.

	sf::Vector2u const maxSize{ sf::VideoMode::getDesktopMode().size };
	sf::Vector2u newSize{ window->getSize() }; // window size returns the visual size of the window.

	// Windows (OS) does not like windows (app) that are larger than the screen definition.
	if (newSize.x > maxSize.x)
		newSize.x = maxSize.x;
	if (newSize.y > maxSize.y)
		newSize.y = maxSize.y;
	
	sf::Vector2f const previsousSize{ window->getView().getSize() }; // View size is the inner size of the window.
	sf::Vector2f const scalingFactor{ newSize.x / previsousSize.x, newSize.y / previsousSize.y };

	sf::View view{ window->getView() };
	view.setSize(static_cast<sf::Vector2f>(newSize));
	view.setCenter(sf::Vector2f{ newSize.x / 2.f, newSize.y / 2.f });

	window->setView(view);
	window->setSize(newSize); // Avoid having a window that is larger than the screen.
	GraphicalFixedInterface::windowResized(window, scalingFactor);
}

//TODO: fix showErrorsUsingGUI