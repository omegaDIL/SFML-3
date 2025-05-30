#include <SFML/Graphics.hpp>
#include <optional>
#include <string>
#include <memory>
#include "Exceptions.hpp"
#include "GUITexturesLoader.hpp"


void convertShapeToSprite(sf::Shape* shape, std::unique_ptr<sf::Sprite>& sprite, std::unique_ptr<sf::Texture>& texture, bool smooth) noexcept
{
	if (shape == nullptr)
		return;

	auto setTransformable = [](sf::Transformable* transformable, sf::Vector2f const& origin, sf::Vector2f position, sf::Vector2f scale, sf::Angle rotation) noexcept
	{
		transformable->setOrigin(origin);
		transformable->setPosition(position);
		transformable->setScale(scale);
		transformable->setRotation(rotation);
	};

	float outlineThinkness{ shape->getOutlineThickness() };

	// We need to reset the transformables of the shape to display the shape at the top left corner
	// of the render texture. And we avoid putting the transformables as the default values of the 
	// new sprite.
	sf::Vector2f origin{ shape->getOrigin() };
	sf::Vector2f position{ shape->getPosition() };
	sf::Vector2f scale{ shape->getScale() };
	sf::Angle rotation{ shape->getRotation() };
	setTransformable(shape, sf::Vector2f{ 0.f, 0.f }, sf::Vector2f{ outlineThinkness, outlineThinkness }, sf::Vector2f{ 1.f, 1.f }, sf::degrees(0));

	// Creates a render texture to draw the shape.
	sf::RenderTexture renderTexture{ static_cast<sf::Vector2u>(sf::Vector2f{ shape->getLocalBounds().size.x + outlineThinkness, shape->getLocalBounds().size.y + outlineThinkness }) };
	renderTexture.clear(sf::Color::Transparent);
	renderTexture.draw(*shape);
	renderTexture.display();

	texture = std::make_unique<sf::Texture>(renderTexture.getTexture().copyToImage());
	texture->setSmooth(smooth);
	sprite = std::make_unique<sf::Sprite>(*texture);
	setTransformable(sprite.get(), origin, position, scale, rotation);

	// Reset the transformables of the shape to their original values.
	setTransformable(shape, origin, position, scale, rotation);
}


sf::Image loadDefaultSliderTexture(sf::Vector2u size) noexcept
{
	sf::RectangleShape slider{ static_cast<sf::Vector2f>(size) };
	slider.setFillColor(sf::Color{ 20, 20, 20 });
	slider.setOutlineThickness(std::min(size.x, size.y) / 5.f);
	slider.setOutlineColor(sf::Color{ 80, 80, 80 });

	std::unique_ptr<sf::Sprite> buffer{ nullptr }; // Is not used. It was created to satisfy the paramters of the function 'convertShapeToSprite()'.
	std::unique_ptr<sf::Texture> newTextureSlider{ nullptr };
	convertShapeToSprite(&slider, buffer, newTextureSlider, true);

	return newTextureSlider->copyToImage();
}