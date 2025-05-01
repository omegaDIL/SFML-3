#include <SFML/Graphics.hpp>
#include <string>
#include <optional>
#include "GUI.hpp"
#include "Save.hpp"
#include "Utils.hpp"

using SafeSaves::Save;


int main()
{
	sf::RenderWindow window{ sf::VideoMode{ sf::Vector2u{ 1920, 1080 } }, "Template sfml 3" };

	UserInteractableGraphicalInterface otherInterface{ &window };
	UserInteractableGraphicalInterface mainInterface{ &window };
	FixedGraphicalInterface* interface{ &mainInterface };
	
	mainInterface.addDynamicText("rgdf", "rgdf", sf::Vector2f{ 100, 100 }, 12, 1.f);
	mainInterface.addButton("rgdf", [&interface, &otherInterface]() mutable -> void { interface = &otherInterface; });
	mainInterface.addSlider("azerty", sf::Vector2f{ 500, 500 }, 500, 1.f, 2, 8, 1);

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

			if (event->is<sf::Event::MouseButtonReleased>())
				mainInterface.mouseUnpressed();
		}

		if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left))
			mainInterface.mousePressed();


		window.clear();
		interface->draw();
		window.display();
	}

	return 0;
}
//Faire les testes