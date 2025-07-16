#include <SFML/Graphics.hpp>
#include <variant>
#include <sstream>
#include <iostream>
#include "GUI/GraphicalRessourcers.hpp"
#include "GUITexturesLoader.hpp"








sf::Texture loadDefaultTexture(sf::Vector2f size) noexcept
{
	sf::RectangleShape topLeft{ size / 2.f };
	topLeft.setFillColor(sf::Color::Red);

	sf::RectangleShape topRight{ size / 2.f };
	topRight.setFillColor(sf::Color::Blue);
	topRight.setPosition(sf::Vector2f{ size.x / 2.f, 0.f });

	sf::RectangleShape bottomLeft{ size / 2.f };
	bottomLeft.setFillColor(sf::Color::Green);
	bottomLeft.setPosition(sf::Vector2f{ 0.f , size.y / 2.f });

	sf::RectangleShape bottomRight{ size / 2.f };
	bottomRight.setFillColor(sf::Color::Yellow);
	bottomRight.setPosition(sf::Vector2f{ size.x / 2.f, size.y / 2.f });

	return createTextureFromDrawables(topLeft, topRight, bottomLeft, bottomRight);
}

sf::Texture loadSolidRectangeShapeWithOutline(sf::Vector2u size, sf::Color fill, sf::Color outline, unsigned int thickness) noexcept
{
	sf::RectangleShape shape{ static_cast<sf::Vector2f>(size) };
	shape.setFillColor(fill);
	shape.setOutlineColor(outline);
	shape.setOutlineThickness(static_cast<float>(std::min(size.x, size.y) / thickness));

	return createTextureFromDrawables(shape);
}

sf::Texture loadCheckBoxTexture(sf::Vector2u size)
{
	sf::Color fillColor{ 20, 20, 20 };
	sf::Color outlineColor{ 80, 80, 80 };

	auto texture = loadSolidRectangeShapeWithOutline(size);
	sf::Image image{ texture.copyToImage() };

	unsigned int checkThickness{ static_cast<unsigned int>(std::min(size.x, size.y) / 5) };
	for (unsigned int i{ 0 }; i < image.getSize().x; ++i)
	{
		for (unsigned int j{ 0 }; j < image.getSize().y; ++j)
		{
			if (image.getPixel(sf::Vector2u{ i, j }) == outlineColor)
				continue; // It is visualky better if we don't draw the diagonal on the outline.

			if (std::abs(static_cast<int>(i) - static_cast<int>(j)) < checkThickness)
				image.setPixel(sf::Vector2u{ i, j }, outlineColor);
			else if (std::abs(static_cast<int>(image.getSize().x) - static_cast<int>(i) - static_cast<int>(j)) < checkThickness)
				image.setPixel(sf::Vector2u{ i, j }, outlineColor);
		}
	}
	
	if (!texture.loadFromImage(image))
		throw LoadingGraphicalRessourceFailure{ "Failed to load checkbox texture from image." };
	return texture;	
}