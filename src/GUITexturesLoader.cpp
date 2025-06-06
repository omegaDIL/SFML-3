#include <SFML/Graphics.hpp>
#include <optional>
#include <string>
#include <memory>
#include "Exceptions.hpp"
#include "GUITexturesLoader.hpp"


std::shared_ptr<sf::Texture> createTextureFromShape(sf::Shape& shape, bool smooth) noexcept
{
	auto setTransformable = [](sf::Transformable& transformable, sf::Vector2f const& origin, sf::Vector2f position, sf::Vector2f scale, sf::Angle rotation) noexcept
	{
		transformable.setOrigin(origin);
		transformable.setPosition(position);
		transformable.setScale(scale);
		transformable.setRotation(rotation);
	};

	// We need to reset the transformables of the shape to display the shape at the top left corner
	// of the render texture. And we avoid putting the transformables as the default values of the 
	// new sprite.
	sf::Vector2f origin{ shape.getOrigin() };
	sf::Vector2f position{ shape.getPosition() };
	sf::Vector2f scale{ shape.getScale() };
	sf::Angle rotation{ shape.getRotation() };
	float outlineThinkness{ shape.getOutlineThickness() };
	setTransformable(shape, sf::Vector2f{ 0.f, 0.f }, sf::Vector2f{ outlineThinkness, outlineThinkness }, sf::Vector2f{ 1.f, 1.f }, sf::degrees(0));

	// Creates a render texture to draw the shape.
	sf::RenderTexture renderTexture{ static_cast<sf::Vector2u>(sf::Vector2f{ shape.getLocalBounds().size.x, shape.getLocalBounds().size.y }) };
	renderTexture.clear(sf::Color::Transparent);
	renderTexture.draw(shape);
	renderTexture.display();
	
	setTransformable(shape, origin, position, scale, rotation); // Reset the transformables of the shape to their original values.
	std::shared_ptr<sf::Texture> texture{ std::make_shared<sf::Texture>(renderTexture.getTexture()) }; // Gets the texture from the render texture.
	texture->setSmooth(smooth); // Sets the smoothness of the texture.

	return texture;
}


std::shared_ptr<sf::Texture> loadSolidRectangeShapeWithOutline(sf::Vector2u size, sf::Color fill, sf::Color outline, unsigned int thickness) noexcept
{
	sf::RectangleShape shape{ static_cast<sf::Vector2f>(size) };
	shape.setFillColor(fill);
	shape.setOutlineColor(outline);
	shape.setOutlineThickness(std::min(size.x, size.y) / thickness);

	return createTextureFromShape(shape, true);
}

std::shared_ptr<sf::Texture> loadCheckBoxTexture(sf::Vector2u size) noexcept
{
	sf::Color fillColor{ 20, 20, 20 };
	sf::Color outlineColor{ 80, 80, 80 };

	auto texture = loadSolidRectangeShapeWithOutline(size);
	sf::Image image{ texture->copyToImage() };

	unsigned int checkThickness{ static_cast<unsigned int>(std::min(size.x, size.y) / 5) };
	for (unsigned int i{ 0 }; i < image.getSize().x; i++)
	{
		for (unsigned int j{ 0 }; j < image.getSize().y; j++)
		{
			if (image.getPixel(sf::Vector2u{ i, j }) == outlineColor)
				continue; // It is visualky better if we don't draw the diagonal on the outline.

			if (std::abs(static_cast<int>(i) - static_cast<int>(j)) < checkThickness)
				image.setPixel(sf::Vector2u{ i, j }, outlineColor);
			else if (std::abs(static_cast<int>(image.getSize().x) - static_cast<int>(i) - static_cast<int>(j)) < checkThickness)
				image.setPixel(sf::Vector2u{ i, j }, outlineColor);
		}
	}
	
	texture->loadFromImage(image);
	return texture;	
}