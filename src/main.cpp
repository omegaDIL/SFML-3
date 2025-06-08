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

	WGInterface mainInterface{ &window };
	FixedGraphicalInterface* curInterface{ &mainInterface };

	mainInterface.addDynamicText("a", "ca marche", sf::Vector2f{ 100, 200 }, 20, 1.f);
	mainInterface.addSlider("slider1", sf::Vector2u{ 20, 400 }, sf::Vector2f{ 500, 500 }, [](float x) { return 200 * x+40; }, [&mainInterface](float x) mutable { mainInterface.getDText("a").updateColor(sf::Color(x/2, x, 255-x/2)); }, 10, true);
	mainInterface.addMQB("mqb1", true, 10, sf::Vector2f{ 300, 900 }, sf::Vector2f{ 330, 900 });
	
	mainInterface.addDynamicText("ab", "ca marche", sf::Vector2f{ 700, 200 }, 20, 1.f);
	mainInterface.addButton("ab", [&mainInterface]() { mainInterface.setCurrentlyEditedText("ab"); });

	mainInterface.addDynamicText("abb", "test", sf::Vector2f{ 700, 700 }, 20, 1.f);
	mainInterface.addButton("abb", [&mainInterface]() { mainInterface.setCurrentlyEditedText("abb"); });


	while (window.isOpen())
	{
		while (const std::optional event = window.pollEvent())
		{
			if (event->is<sf::Event::TextEntered>())
				WGInterface::textEntered(curInterface, event->getIf<sf::Event::TextEntered>()->unicode);

			if (event->is<sf::Event::Resized>())
				handleEventResize(&window);

			if (event->is<sf::Event::MouseMoved>() && !sf::Mouse::isButtonPressed(sf::Mouse::Button::Left))
				IGInterface::mouseMoved(curInterface);

			if (event->is<sf::Event::MouseButtonReleased>())
				IGInterface::mouseUnpressed(curInterface);

			if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left))
				IGInterface::mousePressed(curInterface);

			if (event->is<sf::Event::Closed>() || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape))
				window.close();
		}

		window.clear();
		curInterface->draw();
		window.display();
	}

	return 0;
}

//TODO: tester avec view
//TODO: Faire les testes
//TODO: Refactor Utils.cpp, GUITexturesLoader.cpp (+ GUIInitializer).