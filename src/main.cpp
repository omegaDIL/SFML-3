﻿#include <SFML/Graphics.hpp>
#include <string>
#include <optional>
#include "GUI.hpp"
#include "Save.hpp"
#include "Utils.hpp"

using SafeSaves::Save;

sf::VideoMode windowSize{ sf::Vector2u{ 1000, 1000 } };
std::string nameOfSoftware{ "Template sfml 3" };


int main()
{
	sf::ContextSettings settings{};
	settings.antiAliasingLevel = 16;

	sf::RenderWindow window{ windowSize, nameOfSoftware, sf::Style::Default, sf::State::Windowed, settings };
	GUI mainInterface{ &window };

	while (window.isOpen())
	{
		while (const std::optional event = window.pollEvent())
		{
			if (event->is<sf::Event::Closed>())
				window.close();

			if (event->is<sf::Event::Resized>())
				handleEventResize(&window);

			if (event->is<sf::Event::MouseMoved>())
				mainInterface.mouseMoved();
		}


		window.clear();
		mainInterface.draw();
		window.display();
	}

	return 0;
}