#include "GUI.hpp"

void showErrorsUsingWindow(std::string const& errorTitle, std::string const& errorMessage) noexcept
{
	//sf::RenderWindow errorWindow{ sf::VideoMode{ sf::Vector2u{ 720, 720 } }, errorTitle };
	//IGInterface gui{ &errorWindow }; // Create the interface to use the GUI.

	//gui.addDynamicText("message", errorMessage, sf::Vector2f{ 360, 260 }, 16, 1.f);
	//gui.addDynamicText("close", "ok I understand - close this window", sf::Vector2f{ 360, 600 }, 12, 1.f);
	//gui.addButton("close", []() {}); // Add a button to close the window.

	//auto& text{ gui.getDText("message") };
	//auto rectSize{ text.getText().getGlobalBounds() };
	//do
	//{
	//	text.scale(sf::Vector2f{ 0.9f, 0.9f });
	//	rectSize = text.getText().getGlobalBounds();
	//} while (rectSize.position.x < 0 || rectSize.size.x > errorWindow.getSize().x);

	//while (errorWindow.isOpen())
	//{	// The function is blocking.
	//	while (const std::optional event = errorWindow.pollEvent())
	//	{
	//		if (event->is<sf::Event::Closed>() || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape))
	//			errorWindow.close();

	//		if (event->is<sf::Event::MouseMoved>() && !sf::Mouse::isButtonPressed(sf::Mouse::Button::Left))
	//			IGInterface::mouseMoved(&gui);

	//		if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left))
	//			if (IGInterface::mousePressed(&gui).first == IGInterface::InteractableItem::Button)
	//				errorWindow.close();

	//		if (event->is<sf::Event::Resized>())
	//			handleEventResize(&errorWindow);
	//	}

	//	errorWindow.clear();
	//	gui.draw();
	//	errorWindow.display();
	//}
}

void populateGUI(IGUI* gui) noexcept
{
	ENSURE_VALID_PTR(gui);

	//TODO: Populate and initialize your own gui.

	return;
}