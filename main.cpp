#include <SFML/Graphics.hpp>
#include <string>
#include <optional>
#include "Save.hpp"

using SafeSaves::Save;


int main()
{
	sf::RenderWindow window(sf::VideoMode({ 1000, 1000 }), "SFML works!");
	sf::CircleShape shape{ 100.f };
	shape.setFillColor(sf::Color::Green);
	shape.setOrigin({ shape.getRadius() / 2, shape.getRadius() / 2 });

	while (window.isOpen())
	{
		while (const std::optional event = window.pollEvent())
		{
			if (event->is<sf::Event::Closed>())
				window.close();
			if (event->is<sf::Event::MouseMoved>())
				shape.setPosition(static_cast<sf::Vector2f>(sf::Mouse::getPosition(window)));
		}

		window.clear();
		window.draw(shape);
		window.display();
	}
}