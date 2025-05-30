#include <SFML/Graphics.hpp>
#include <string>
#include <optional>
#include "GUI.hpp"
#include "Save.hpp"
#include "Utils.hpp"

using SafeSaves::Save;


int main()
{
	sf::RenderWindow window{ sf::VideoMode{ sf::Vector2u{ 1000, 1000 } }, "Template sfml 3" };

	UserInteractableGraphicalInterface mainInterface{ &window };
	FixedGraphicalInterface* curInterface{ &mainInterface };

	mainInterface.addSlider("slider1", sf::Vector2u{ 20, 400 }, sf::Vector2f{ 500, 500 }, [](float x) { return x; }, true); 

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