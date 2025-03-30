#include <SFML/Graphics.hpp>
#include <string>
#include <optional>
#include "GUI.hpp"
#include "Save.hpp"
#include "Utils.hpp"
#include <iostream>
#include <filesystem>

using SafeSaves::Save;

sf::VideoMode windowSize{ sf::Vector2u{ 1000, 1000 } };
std::string nameOfSoftware{ "Template sfml 3" };


int main()
{
	sf::RenderWindow window{ windowSize, nameOfSoftware };
	GInterface mainInterface{ &window };

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