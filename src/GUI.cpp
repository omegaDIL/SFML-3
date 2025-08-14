#include "GUI.hpp"

void showErrorsUsingWindow(const std::string& errorTitle, const std::ostringstream& errorMessage) noexcept
{
	sf::Vector2u windowSize{ sf::Vector2u{ 720, 720 } };
	sf::RenderWindow window{ sf::VideoMode{ windowSize }, errorTitle };
	MGUI gui{ &window }; // Create the interface to use the GUI.

	gui.addDynamicText("message", errorMessage.str(), sf::Vector2f{360, 260});
	gui.addText("ok I understand - press any key", sf::Vector2f{ 360, 600 });

	auto* text{ gui.getDynamicText("message") };
	auto rectSize{ text->getText().getGlobalBounds() };

	while (rectSize.position.x < 0 || rectSize.size.x > window.getSize().x)	
	{
		text->scale(sf::Vector2f{ 0.9f, 0.9f });
		rectSize = text->getText().getGlobalBounds();
	} 

	while (window.isOpen()) // The function is blocking.
	{	
		while (const std::optional event = window.pollEvent())
		{
			if (event->is<sf::Event::Closed>() 
			||  event->is<sf::Event::KeyPressed>()
			||  event->is<sf::Event::TouchBegan>())
				window.close();

			if (event->is<sf::Event::Resized>())
				BGUI::windowResized(&window, windowSize);
		}

		window.clear();
		gui.draw();
		window.display();
	}
}

void populateGUI(IGUI* gui) noexcept
{
	ENSURE_VALID_PTR(gui, "gui was nullptr when populateGUI was called");

	//TODO: Populate and initialize your own gui.

	return;
}