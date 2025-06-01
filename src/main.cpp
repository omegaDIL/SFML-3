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

	mainInterface.addDynamicText("a", "ca marche", sf::Vector2f{ 200, 200 }, 20, 1.F);
	mainInterface.addSlider("slider1", sf::Vector2u{ 20, 400 }, sf::Vector2f{ 500, 500 }, [](float x) { return 200 * x+40; }, [&mainInterface](float x) mutable {mainInterface.getDText("a").updateColor(sf::Color(x/2, x, 255)); }, -1, false);

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