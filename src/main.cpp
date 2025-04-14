#include <SFML/Graphics.hpp>
#include <string>
#include <iostream>
#include <optional>
#include "GUI.hpp"
#include "Save.hpp"
#include "Utils.hpp"

using SafeSaves::Save;


int main()
{
	sf::RenderWindow window{ sf::VideoMode{ sf::Vector2u{ 720, 720 } }, "Template sfml 3" };
	GraphicalDynamicInterface mainInterface{ &window };
	auto err = mainInterface.create();
	if (err.has_value())
	{
		showErrorsUsingGUI(err.value(), "Error while creating the GUI");
		return -1;
	}

	while (window.isOpen())
	{
		while (const std::optional event = window.pollEvent())
		{
			if (event->is<sf::Event::Closed>())
				window.close();

			if (event->is<sf::Event::Resized>())
				handleEventResize(&window);
		}

		window.clear();
		mainInterface.draw();
		window.display();
	}

	return 0;
}
//Faire les testes