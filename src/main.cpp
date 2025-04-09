#include <SFML/Graphics.hpp>
#include <string>
#include <iostream>
#include <optional>
#include "GUI.hpp"
#include "Save.hpp"
#include "Utils.hpp"

using SafeSaves::Save;

sf::VideoMode windowSize{ sf::Vector2u{ 700, 700 } };
std::string nameOfSoftware{ "Template sfml 3" };


int main()
{
	sf::ContextSettings settings{};
	settings.antiAliasingLevel = 16;

	sf::RenderWindow window{ windowSize, nameOfSoftware, sf::Style::Default, sf::State::Windowed, settings };
	GraphicalDynamicInterface mainInterface{ &window };
	auto err = mainInterface.create();
	if (err.has_value())
	{
		showErrorsUsingGUI(err.value(), "Error while creating the GUI");
		return -1;
	}

	mainInterface.addDynamicText("texte 1", "aa", sf::Vector2f{ windowSize.size.x / 2.f, windowSize.size.y / 2.f }, 12);

	sf::CircleShape circle{ 50.f };
	circle.setFillColor(sf::Color::Red);
	circle.setPosition(sf::Vector2f{ windowSize.size.x / 4.f, windowSize.size.y / 4.f });
	mainInterface.addDynamicShape("shape 1", &circle);

	while (window.isOpen())
	{
		while (const std::optional event = window.pollEvent())
		{
			if (event->is<sf::Event::Closed>())
				window.close();

			if (event->is<sf::Event::Resized>())
				handleEventResize(&window, nameOfSoftware, sf::Style::Default, sf::State::Windowed);
		}

		window.clear();
		mainInterface.draw();
		window.display();
	}

	return 0;
}
//Faire les testes