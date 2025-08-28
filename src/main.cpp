#include <SFML/Graphics.hpp> 
#include <string>
#include <optional>
#include <iostream>
#include "GUI.hpp"
#include "Save.hpp"

using SafeSaves::Save;

int main()
{
	sf::Vector2u windowSize{ 1000, 1000 };
	sf::RenderWindow window{ sf::VideoMode{ windowSize }, "Template sfml 3" };
	AGUI mainInterface{ &window, 1080 };
	AGUI otherInterface{ &window, 1080 };
	BGUI* curInterface{ &mainInterface };


	mainInterface.addText("Hi!!\nWelcome to my GUI", sf::Vector2f{ 200, 150 }, 48, sf::Color{255, 255, 255}, "__default", gui::Alignment::Left);

	mainInterface.addDynamicText("text1", "entry", { 500, 400 }, 60);
	mainInterface.addInteractive("text1", [](IGUI* igui) {igui->setWritingText("text1"); });

	mainInterface.addDynamicText("text2", "entry", {500, 500});
	mainInterface.addInteractive("text2", [](IGUI* igui) {igui->setWritingText("text2"); });

	mainInterface.addDynamicText("other", "switch", { 500, 800 });
	mainInterface.addInteractive("other", [&otherInterface, &curInterface](IGUI*) mutable {curInterface = &otherInterface; });

	mainInterface.addSlider("azer", { 200, 500 });
	mainInterface.addSlider("azerr", { 500, 500 }, 600, 30, nullptr, [](float x) {return 3 + 5 * x; });

	sf::RectangleShape rect{ { 50, 50 } };
	otherInterface.addDynamicSprite("colorChanger", gui::createTextureFromDrawables(rect), sf::Vector2f{500, 850});
	otherInterface.addInteractive("colorChanger");

	otherInterface.addDynamicText("main", "switch", { 500, 500 });
	otherInterface.addInteractive("main", [&mainInterface, &curInterface](IGUI*) mutable {curInterface = &mainInterface; });

	otherInterface.addMQB("myMQB", { 201, 200 }, { 0, 30 }, 5, false);

	mainInterface.getDynamicText("text1")->setRotation(sf::degrees(30));

	IGUI::Item curItem{};
	while (window.isOpen())
	{
		while (const std::optional event = window.pollEvent())
		{ 
			if (event->is<sf::Event::MouseMoved>() && !sf::Mouse::isButtonPressed(sf::Mouse::Button::Left))
				curItem = IGUI::eventUpdateHovered(curInterface, window.mapPixelToCoords(event->getIf<sf::Event::MouseMoved>()->position));

			if (event->is<sf::Event::MouseButtonPressed>())
			{
				IGUI::eventPressed(curInterface); // Is not necessary since no interactables have a pressed button.
				auto* a = otherInterface.getMQB("myMQB");
				for (auto elem : a->getChecked())
				{
					std::cout << elem << '\n';
				}
				std::cout << "\n\n\n";
			}

			if (event->is<sf::Event::TextEntered>())
				IGUI::textEntered(curInterface, event->getIf<sf::Event::TextEntered>()->unicode);

			if (event->is<sf::Event::Resized>())
				BGUI::windowResized(&window, windowSize);

			if (event->is<sf::Event::Closed>() || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape))
				window.close();
		}

		if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left))
			AGUI::pressed(curInterface, window.mapPixelToCoords(sf::Mouse::getPosition(window)));

		// If you ignore this feature, you could delete the variable curItem entirely.
		// However, this is less limited since you can use more arguments, watch for more events...
		// Moreover, this is better for perfomance-critical functions because storing a function in a
		// `std::function` impacts the fps.
		if (curItem.igui == &otherInterface && curItem.identifier == "colorChanger")
			otherInterface.getDynamicSprite("colorChanger")->rotate(sf::degrees(1));

		window.clear();
		curInterface->draw();
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