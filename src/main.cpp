#include <SFML/Graphics.hpp> 
#include <string>
#include <optional>
#include "GUI.hpp"
#include "Save.hpp"

using SafeSaves::Save;

int main()
{
	sf::Vector2u windowSize{ 1000, 1000 };
	sf::RenderWindow window{ sf::VideoMode{ windowSize }, "Template sfml 3" };
	IGUI mainInterface{ &window, 1080 };

	mainInterface.addDynamicText("uhg", "Hello, World", sf::Vector2f{ 200, 100 });
	mainInterface.addDynamicText("cHrfgtz", "Bonjour", sf::Vector2f{ 200, 200 });
	mainInterface.addDynamicText("dH(rt-hye", "Hola", sf::Vector2f{ 200, 300 });
	mainInterface.addDynamicText("ghiurt(o", "Buongiorno", sf::Vector2f{ 200, 400 });
	mainInterface.addDynamicText("agrpuhçea", "Guten tag", sf::Vector2f{ 200, 500 });

	mainInterface.addInteractive("dH(rt-hye");
	mainInterface.addInteractive("agrpuhçea", nullptr, IGUI::Button::When::hovered);

	mainInterface.setWritingText("cHrfgtz");

	sf::RectangleShape shape{ sf::Vector2f{ 100, 100 } };
	mainInterface.addDynamicSprite("ubyhg", gui::createTextureFromDrawables(shape), sf::Vector2f{ 500, 500 });
	shape.setFillColor(sf::Color{ 0, 255, 0 });
	mainInterface.addDynamicSprite("uhg", gui::createTextureFromDrawables(std::move(shape)), sf::Vector2f{ 700, 500 });

	mainInterface.addInteractive("uhg", [&window](IGUI* i) {i->removeDynamicSprite("uhg"); i->removeDynamicText("uhg"); }, IGUI::Button::When::unpressed);
	mainInterface.addInteractive("ubyhg");
	mainInterface.addInteractive("ubyhg");

	while (window.isOpen())
	{
		while (const std::optional event = window.pollEvent())
		{ 
			if (event->is<sf::Event::Resized>())
				BGUI::windowResized(&window, windowSize);

			if (event->is<sf::Event::MouseMoved>() && !sf::Mouse::isButtonPressed(sf::Mouse::Button::Left))
				IGUI::updateHovered(&mainInterface, window.mapPixelToCoords(event->getIf<sf::Event::MouseMoved>()->position));

			if (event->is<sf::Event::MouseButtonPressed>() && sf::Mouse::isButtonPressed(sf::Mouse::Button::Left))
				IGUI::pressed(&mainInterface);

			if (event->is<sf::Event::MouseButtonReleased>() && !sf::Mouse::isButtonPressed(sf::Mouse::Button::Left))
				IGUI::unpressed(&mainInterface);

			if (event->is<sf::Event::TextEntered>())
				IGUI::textEntered(&mainInterface, event->getIf<sf::Event::TextEntered>()->unicode);

			if (event->is<sf::Event::Closed>() || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape))
				window.close();

			if (event->is<sf::Event::KeyPressed>())
			{
				mainInterface.removeDynamicSprite("ubyhg");
			}
		}

		window.clear();
		mainInterface.draw();
		window.display();
	}

	return 0;
}


/*
//mainInterface.addDynamicText("a", "ca marche", sf::Vector2f{ 100, 200 }, 20, 1.f);
//mainInterface.addSlider("slider1", sf::Vector2u{ 20, 400 }, sf::Vector2f{ 500, 500 }, sf::Vector2f{ 1.f, 1.f }, [](float x) { return 200 * x + 40; }, [&mainInterface](float x) mutable { mainInterface.getDText("a").setColor(sf::Color(x / 2, x, 255 - x / 2)); }, -1, true);
//mainInterface.addMQB("mqb1", sf::Vector2f{ 100, 500 }, sf::Vector2f{ 0, 50 }, sf::Vector2f{ 1.f, 1.f }, 5, true, 3);
//mainInterface.addDynamicText("ab", "deleted", sf::Vector2f{ 700, 200 }, 20, 1.f);
//mainInterface.addButton("ab", [&mainInterface]() { mainInterface.setCurrentlyEditedText("ab"); });
//mainInterface.removeDText("ab"); // Remove the text with the identifier "ab".
//mainInterface.addDynamicText("abb", "test", sf::Vector2f{ 700, 700 }, 20, 1.f);
//mainInterface.addButton("abb", [&mainInterface]() { mainInterface.setCurrentlyEditedText("abb"); });



			if (event->is<sf::Event::TextEntered>())
				WGInterface::textEntered(curInterface, event->getIf<sf::Event::TextEntered>()->unicode);



			if (event->is<sf::Event::MouseMoved>() && !sf::Mouse::isButtonPressed(sf::Mouse::Button::Left))
				IGInterface::mouseMoved(curInterface);

			if (event->is<sf::Event::MouseButtonReleased>())
				IGInterface::mouseUnpressed(curInterface);

			if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left))
				IGInterface::mousePressed(curInterface);


mainInterface.addMQB("mqb1", sf::Vector2f{ 100, 500 }, sf::Vector2f{ 0, 50 }, sf::Vector2f{ 1.f, 1.f }, 5, true, 3);

sf::RectangleShape shape{ sf::Vector2f{ 100, 100 } };
shape.setFillColor(sf::Color{ 255, 0, 0 });
mainInterface.addSprite(createTextureFromDrawables(std::move(shape)), sf::Vector2f{ 100, 100 }, sf::Vector2f{ 1, 1 });
mainInterface.addText("Hello, World", sf::Vector2f{ 100, 100 }, 18, window.getSize().x / 1080.f);

sf::Texture defaultTexture{ loadDefaultTexture(sf::Vector2f{ 20, 60 }) };
mainInterface.addSprite(defaultTexture, sf::Vector2f{ 500, 500 }, sf::Vector2f{ 1.f, 1.f });
*/

//TODO: tester avec view
//TODO: Faire les testes
//TODO: replace @ avec \
//TODO: Refactor Utils.cpp, GUITexturesLoader.cpp (+ GUIInitializer).