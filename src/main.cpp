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
	FixedGraphicalInterface* curInterface{ &mainInterface };
	
	mainInterface.addDynamicText("d", "bouton", sf::Vector2f{ 300, 100 }, 12, 1.f);
	mainInterface.addDynamicText("de", "simple", sf::Vector2f{ 200, 200 }, 12, 1.f);
	mainInterface.addDynamicText("da", "simple", sf::Vector2f{ 300, 300 }, 12, 1.f);
	mainInterface.addDynamicText("dr", "bouton", sf::Vector2f{ 100, 300 }, 12, 1.f);
	mainInterface.addButton("d", [&curInterface, &otherInterface]() mutable -> void { curInterface = &otherInterface; });
	mainInterface.addButton("dr", [&curInterface, &otherInterface]() mutable -> void { curInterface = &otherInterface; });

	mainInterface.removeDText("dr"); // Removing a dynamic text, which is not a button.


	//mainInterface.addSlider("azerty", sf::Vector2f{ 500, 500 }, 500, 1.f, 2, 8.7f, 1);

	while (window.isOpen())
	{
		while (const std::optional event = window.pollEvent())
		{
			if (event->is<sf::Event::Closed>() || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape))
				window.close();

			if (event->is<sf::Event::Resized>())
				handleEventResize(&window);

			if (event->is<sf::Event::MouseMoved>() && !sf::Mouse::isButtonPressed(sf::Mouse::Button::Left))
				IGInterface::mouseMoved(curInterface);

			if (event->is<sf::Event::MouseButtonReleased>())
				IGInterface::mouseUnpressed(curInterface);

			if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left))
				IGInterface::mousePressed(curInterface);
		}

		window.clear();
		curInterface->draw();
		window.display();
	}

	return 0;
}

//TODO: Faire les testes
//TODO: Refactor Utils.cpp, GUITexturesLoader.cpp (+ GUIInitializer).