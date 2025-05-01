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


void showErrorsUsingWindow(std::string const& errorMessage, std::string const& errorTitle) noexcept
{
	sf::RenderWindow errorWindow{ sf::VideoMode{ sf::Vector2u{ 100, 100 } }, errorTitle };
	DynamicGraphicalInterface gui{ &errorWindow }; // Create the interface to use the GUI.
	gui.addDynamicText("text", errorMessage, errorWindow.getView().getCenter(), 18, 1.f);

	sf::Vector2f sizeOfText{ gui.getDText("text").getText().getGlobalBounds().size };
	sizeOfText.x += 20.f;
	sizeOfText.y += 20.f;

	errorWindow.setSize(static_cast<sf::Vector2u>(sizeOfText));
	handleEventResize(&errorWindow); // Resize the window to fit the text.

	while (errorWindow.isOpen())
	{	// The function is blocking.
		while (const std::optional event = errorWindow.pollEvent())
		{
			if (event->is<sf::Event::Closed>() || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape))
				errorWindow.close();

			if (event->is<sf::Event::Resized>())
				handleEventResize(&errorWindow);
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
	FixedGraphicalInterface::windowResized(window, scalingFactor);
}

//TODO: fix showErrorsUsingWindow