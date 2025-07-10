#ifndef BASICGUI_HPP
#define BASICGUI_HPP

#include <SFML/Graphics.hpp>
#include <string>
#include <vector>
#include <list>
#include <unordered_map>
#include <optional>
#include <stdexcept>
#include <concepts>
#include <sstream>
#include <cstdint>
#include "GUIWrappers.hpp"

//namespace gui







/**
 * \brief Manages items to create a basic GUI. You can display texts, sprites, a background...
 * \details All elements are fixed and can't be edited nor removed.
 *
 * \note This class stores UI componenents; it will use a considerable amount of memory.
 * \warning Avoid deleting the `sf::RenderWindow` passed as an argument while this class is using it.
 *
 * \code
 * sf::RenderWindow window{ ... };
 * FixedGraphicalInterface gui{ &window };
 *
 * ...
 *
 * gui.addText("Hello, World", sf::Vector2f{ 100, 100 }, 12, windowSizeMin / 1080.f);
 * gui.addText("Bonjour, le Monde", sf::Vector2f{ 300, 100 }, 12, windowSizeMin / 1080.f, sf::Color{ 255, 0, 0 });
 * gui.addSprite(myTexture, sf);
 *
 * ...
 *
 * window.clear();
 * gui.draw();
 * window.display();
 * \endcode
 *
 * \see sf::RenderWindow, TextWrapper, SpriteWrapper, DynamicGraphicalInterface.
 */
class FixedGraphicalInterface
{
public:

	/**
	 * \brief Constructs the interface with a default background.
	 * \complexity O(1).
	 *
	 * \param[in,out] window: The window where the interface elements will be rendered.
	 * \param[in] backgroundFileName: The file name of the background.
	 *
	 * \pre The window must be valid, and have a valid size.
	 * \post The instance is safe to use and the background is set with the appropriate size.
	 * \throw std::logic_error Basic exception guarrantee.
	 *
	 * \pre The font should be named font.ttf and located at where ressourcePath points to.
	 * \post The font is loaded.
	 * \throw LoadingGraphicalRessourceFailure Strong exception guarrantee, but no text can be displayed.
	 *
	 * \pre The path of the background should be valid within the res folder.
	 * \post Your background is loaded.
	 * \throw LoadingGraphicalRessourceFailure Strong exception guarrantee, but the default background is used.
	 *
	 * \note If you don't want a background, just pass an empty string and no exceptions will be thrown.
	 * \note The font is loaded is common to all texts, and therefore, it is loaded once (when the
	 *		 first gui instance is created).
	 */
	explicit FixedGraphicalInterface(sf::RenderWindow* window, std::string const& backgroundFileName = "");

	FixedGraphicalInterface() noexcept = delete;
	FixedGraphicalInterface(FixedGraphicalInterface const&) noexcept = delete;
	FixedGraphicalInterface(FixedGraphicalInterface&& other) noexcept;
	FixedGraphicalInterface& operator=(FixedGraphicalInterface const&) noexcept = delete;
	FixedGraphicalInterface& operator=(FixedGraphicalInterface&& other) noexcept;
	virtual ~FixedGraphicalInterface() noexcept;


	/**
	 * \brief Adds a text element to the interface.
	 * \complexity amortized O(1).
	 *
	 * \tparam T: Type that can be streamed to `std::basic_ostream`.
	 * \param[in] content: The text label.
	 * \param[in] position: The position of the text.
	 * \param[in] characterSize: The size of the text characters.
	 * \param[in] scale: The scale of the text.
	 * \param[in] color: The color of the text.
	 * \param[in] rot: The rotation of the text.
	 *
	 * \note The scale is the same for both x and y axis.
	 *
	 * \see FixedGraphicalInterface::TextWrapper, addSprite(), addShape().
	 */
	template<Ostreamable T>
	void addText(T const& content, sf::Vector2f position, unsigned int characterSize, float scale, sf::Color color = sf::Color{ 255, 255, 255 }, sf::Angle rot = sf::degrees(0)) noexcept
	{
		TextWrapper newText{ content, characterSize, position, sf::Vector2f{ scale, scale }, color };
		m_texts.push_back(std::move(newText));
	}

	/**
	 * \brief Adds a sprite element to the interface.
	 * \complexity amortized O(1).
	 *
	 * \param[in] sprite: The sprite to add.
	 * \param[in] texture: The texture of the sprite.
	 *
	 * \see addText(), addShape().
	 */
	void addSprite(std::string const& texture, sf::Vector2f pos, sf::Vector2f scale, sf::Angle rot = sf::degrees(0), sf::IntRect rectangle = sf::IntRect{ sf::Vector2i{}, sf::Vector2i{} }, sf::Color color = sf::Color::White);

	/**
	 * \brief Adds a sprite element to the interface.
	 * \complexity amortized O(1).
	 *
	 * \param[in] sprite: The sprite to add.
	 * \param[in] texture: The texture of the sprite.
	 *
	 * \see addText(), addShape().
	 */
	void addSprite(sf::Texture texture, sf::Vector2f pos, sf::Vector2f scale, sf::Angle rot = sf::degrees(0), sf::IntRect rectangle = sf::IntRect{ sf::Vector2i{}, sf::Vector2i{} }, sf::Color color = sf::Color::White) noexcept;

	/**
	 * \brief Renders the interface.
	 * \complexity O(N), where N is the number of graphical elements.
	 *
	 * \pre The window must be valid.
	 * \post Safe to draw.
	 * \throw std::logic_error Basic exception guarrantee.
	 */
	virtual void draw() const;


	/**
	 * \brief Refreshes the transformables after the window is resized.
	 * \complexity O(N), where N is the number of graphical elements within all interfaces associated with the resized window.
	 *
	 * \param[in] window: The window which was resized, and for which the interfaces will be resized.
	 * \param[in] scalingFactor: The scaling factor between the previous window's size and the new one.
	 *
	 * \note Resizes only interfaces associated with the resized window.
	 */
	static void windowResized(sf::RenderWindow* window, sf::Vector2f scalingFactor) noexcept;

protected:

	/**
	 * \brief Loads/Resizes the background image and resizes it to fit the window.
	 * \details Loads the default background first, and then loads the custom background (if needed, that
	 * 			is to say if the file name is not empty). If it fails, it uses the default background.
	 * \complexity O(1).
	 *
	 * \param[in] fileName: The file name of the background.
	 *
	 * \return A string containing an error message if the loading failed.
	 */
	std::optional<std::string> loadBackground(std::string const& backgroundFileName) noexcept;



	// Pointer to the window.
	mutable sf::RenderWindow* m_window;

	// Collection of sprites in the interface.
	std::vector<SpriteWrapper> m_sprites;

	// Collection of texts in the interface.
	std::vector<TextWrapper> m_texts;


	// The relative path to the res folder used to load ressources. Default is "../res/". DO NOT CHANGE if not needed.
	static std::string const ressourcePath;

private:

	// Collection of all interfaces to perform the same operation (resizing for example). Stored by window.
	static std::unordered_multimap<sf::RenderWindow*, FixedGraphicalInterface*> allInterfaces;
};


void loadDefaultFont()
{
	std::ostringstream errorMessage{};
	auto fontOpt = loadFontFromFile(errorMessage, "defaultFont.ttf");

	if (!fontOpt.has_value()) [[unlikely]]
	{
		errorMessage << "\nMajor error: No text can use the default font\n";
		errorMessage << "Reinstalling the software could solve the issue\n";
		errorMessage << "Sorry for the inconveniance";
		throw LoadingGraphicalRessourceFailure{ errorMessage.str() };
	}

	TextWrapper::loadFontIntoWrapper("default", fontOpt.value());
}

#endif // BASICGUI_HPP