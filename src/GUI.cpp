#include "GUI.hpp"

void showErrorsUsingWindow(const std::string& errorTitle, const std::ostringstream& errorMessage) noexcept
{
	sf::Vector2u windowSize{ sf::Vector2u{ 720, 720 } };
	sf::RenderWindow window{ sf::VideoMode{ windowSize }, errorTitle };
	IGUI gui{ &window }; // Create the interface to use the GUI.

	gui::TextWrapper textWr{ errorMessage.str(), "__default", 16, sf::Vector2f{560, 260}, {1.f, 1.f} };

	gui.addDynamicText("message", errorMessage.str(), sf::Vector2f{360, 260});
	gui.addDynamicText("close", "ok I understand - close this window", sf::Vector2f{ 360, 600 });
	gui.addInteractive("close", [&window](IGUI*, std::string&) mutable {window.close(); }); // Add a button to close the window.

	auto* text{ gui.getDynamicText("message") };
	auto rectSize{ text->getText().getGlobalBounds() };

	while (rectSize.position.x < 0 || rectSize.size.x > window.getSize().x)	{
		text->scale(sf::Vector2f{ 0.9f, 0.9f });
		rectSize = text->getText().getGlobalBounds();
	} 

	while (window.isOpen())
	{	// The function is blocking.
		while (const std::optional event = window.pollEvent())
		{
			if (event->is<sf::Event::Closed>() || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape))
				window.close();

			if (event->is<sf::Event::MouseMoved>() && !sf::Mouse::isButtonPressed(sf::Mouse::Button::Left))
				IGUI::updateHovered(&gui, sf::Mouse::getPosition(window));

			if (event->is<sf::Event::Resized>())
				BGUI::windowResized(&window, windowSize);
		}

		window.clear();
		gui.draw();
		window.draw(textWr.getText());
		window.display();
	}
}

void populateGUI(IGUI* gui) noexcept
{
	ENSURE_VALID_PTR(gui, "gui was nullptr when populateGUI was called");

	//TODO: Populate and initialize your own gui.

	return;
}