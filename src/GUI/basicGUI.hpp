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




/// The type must be streamable to `std::basic_ostream`.
template <typename T>
concept Ostreamable = requires(std::ostream & os, T t)
{
	{ os << t } -> std::same_as<std::ostream&>;
};


/**
 * @brief Manages items to create a basic GUI. You can display texts, sprites, a background...
 * @details All elements are fixed and can't be edited nor removed.
 *
 * @note This class stores UI componenents; it will use a considerable amount of memory.
 * @warning Avoid deleting the `sf::RenderWindow` passed as an argument while this class is using it.
 *
 * @code
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
 * @endcode
 *
 * @see sf::RenderWindow, TextWrapper, SpriteWrapper, DynamicGraphicalInterface.
 */
class FixedGraphicalInterface
{
public:

	/**
	 * @brief A wrapper for `sf::Text` that initializes the text, keeps the font, manages resizing.
	 *
	 * @see FixedGraphicalInterface, sf::Text, loadFont().
	 */
	class TextWrapper : private sf::Text
	{
	public:

		/**
		 * @brief Initializes the text with the specified parameters.
		 * @complexity O(1).
		 *
		 * @tparam T: Type that can be streamed to `std::basic_ostream`.
		 * @param[in] content: The text label.
		 * @param[in] position: The position of the text.
		 * @param[in] characterSize: The size of the text characters.
		 * @param[in] scale: The scale of the text.
		 * @param[in] color: The color of the text.
		 * @param[in] rot: The rotation of the text.
		 */
		template<Ostreamable T>
		inline TextWrapper(T const& content, unsigned int characterSize, sf::Vector2f pos, sf::Vector2f scale, sf::Color color = sf::Color::White, Alignment alignment = Alignment::Center, sf::Text::Style style = sf::Text::Style::Regular, sf::Angle rot = sf::degrees(0)) noexcept
			: sf::Text{ s_font, "", characterSize }, hide{ false }, m_alignment{ alignment }
		{
			sf::Text::setFillColor(color);
			sf::Text::setRotation(rot);
			sf::Text::setPosition(pos);
			sf::Text::setScale(scale);
			sf::Text::setStyle(style);
			setContent(content); // Also compute the origin of the text with the correct alignment.
		}

		TextWrapper() noexcept = delete;
		TextWrapper(TextWrapper const&) noexcept = default;
		TextWrapper(TextWrapper&&) noexcept = default;
		TextWrapper& operator=(TextWrapper const&) noexcept = default;
		TextWrapper& operator=(TextWrapper&&) noexcept = default;
		virtual ~TextWrapper() noexcept = default;


		/**
		 * @brief Updates the text content.
		 * @complexity O(1).
		 *
		 * @tparam T: Type that can be streamed to `std::basic_ostream`.
		 * @param[in] content: The new content for the text.
		 *
		 * @see sf::Text::setString().
		 */
		template<Ostreamable T>
		inline void setContent(T const& content) noexcept
		{
			std::ostringstream oss{}; // Convert the content to a string
			oss << content; // Assigning the content to the variable.

			setString(oss.str());
			computeNewOrigin();
		}

		/**
		 * @brief Updates the position of the text.
		 * @complexity O(1).
		 *
		 * @param[in] pos: The new position for the text.
		 *
		 * @see sf::Text::setPosition().
		 */
		inline void setPosition(sf::Vector2f pos) noexcept
		{
			sf::Text::setPosition(pos);
		}

		/**
		 * @brief Updates the rotation of the text.
		 * @complexity O(1).
		 *
		 * @param[in] rot: The new rotation for the text.
		 *
		 * @see sf::Text::setRotation.
		 */
		inline void setRotation(sf::Angle rot) noexcept
		{
			sf::Text::setRotation(rot);
		}

		/**
		 * @brief Updates the scale of the text.
		 * @complexity O(1).
		 *
		 * @param[in] scale: Multiplies the scale to the current one.
		 *
		 * @note Encourages to kee
		 *
		 * @see sf::Text::scale.
		 */
		inline void scale(sf::Vector2f scale) noexcept
		{
			sf::Text::scale(scale);
		}

		/**
		 * @brief Updates the size of the text.
		 * @complexity O(1).
		 *
		 * @param[in] size: The size of the characters.
		 *
		 * @see sf::Text::setCharacterSize().
		 */
		inline void setCharacterSize(unsigned int size) noexcept
		{
			sf::Text::setCharacterSize(size);
			computeNewOrigin(); // Recomputes the origin of the text based on the new character size.
		}

		/**
		 * @brief Updates the color of the text.
		 * @complexity O(1).
		 *
		 * @param[in] color: The new color for the text.
		 *
		 * @see sf::Text::setFillColor().
		 */
		inline void setColor(sf::Color color) noexcept
		{
			sf::Text::setFillColor(color);
		}

		inline void setStyle(sf::Text::Style style) noexcept
		{
			sf::Text::setStyle(style);
		}

		inline void setAlignment(Alignment alignment) noexcept
		{
			if (m_alignment == alignment)
				return; // No need to update the alignment if it is already the same.

			m_alignment = alignment; // Update the alignment of the text.
			computeNewOrigin();
		}

		/**
		 * @brief Resizes the text when the window is resized.
		 * @complexity O(1).
		 *
		 * @param[in] scalingFactor: The scaling factor between the previous window's size and the new one.
		 * @param[in] scalingFactorMin: The scaling factor between the previous smallest window's size in x or y axis, and the new smallest one.
		 */
		inline void windowResized(sf::Vector2f scalingFactor, sf::Vector2f scalingFactorMin) noexcept
		{
			sf::Text::scale(scalingFactorMin);
			sf::Text::setPosition(sf::Vector2f{ getPosition().x * scalingFactor.x, getPosition().y * scalingFactor.y });
		}

		/**
		 * @brief Accesses the base `sf::Text` object.
		 * @complexity O(1).
		 *
		 * @return A reference to the base `sf::Text` object.
		 */
		[[nodiscard]] inline sf::Text const& getText() const noexcept
		{
			return *this;
		}

		/**
		 * @brief Creates the font if it is not already created.
		 * @complexity O(1).
		 *
		 * @return a string if failed.
		 */
		[[nodiscard]] static std::optional<std::string> loadFont() noexcept;


		bool hide;

	private:

		void computeNewOrigin() noexcept;

		Alignment m_alignment; // Alignment of the text.

		//static std::unordered_map<std::string, std::list<sf::Font>::iterator> s_accessToFonts;
		//static std::list<sf::Font> s_fonts; // The font used for the texts.
		static sf::Font s_font; // The font used for the texts.
	};

	


	/**
	 * @brief Constructs the interface with a default background.
	 * @complexity O(1).
	 *
	 * @param[in,out] window: The window where the interface elements will be rendered.
	 * @param[in] backgroundFileName: The file name of the background.
	 *
	 * @pre The window must be valid, and have a valid size.
	 * @post The instance is safe to use and the background is set with the appropriate size.
	 * @throw std::logic_error Basic exception guarrantee.
	 *
	 * @pre The font should be named font.ttf and located at where ressourcePath points to.
	 * @post The font is loaded.
	 * @throw LoadingGraphicalRessourceFailure Strong exception guarrantee, but no text can be displayed.
	 *
	 * @pre The path of the background should be valid within the res folder.
	 * @post Your background is loaded.
	 * @throw LoadingGraphicalRessourceFailure Strong exception guarrantee, but the default background is used.
	 *
	 * @note If you don't want a background, just pass an empty string and no exceptions will be thrown.
	 * @note The font is loaded is common to all texts, and therefore, it is loaded once (when the
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
	 * @brief Adds a text element to the interface.
	 * @complexity amortized O(1).
	 *
	 * @tparam T: Type that can be streamed to `std::basic_ostream`.
	 * @param[in] content: The text label.
	 * @param[in] position: The position of the text.
	 * @param[in] characterSize: The size of the text characters.
	 * @param[in] scale: The scale of the text.
	 * @param[in] color: The color of the text.
	 * @param[in] rot: The rotation of the text.
	 *
	 * @note The scale is the same for both x and y axis.
	 *
	 * @see FixedGraphicalInterface::TextWrapper, addSprite(), addShape().
	 */
	template<Ostreamable T>
	void addText(T const& content, sf::Vector2f position, unsigned int characterSize, float scale, sf::Color color = sf::Color{ 255, 255, 255 }, sf::Angle rot = sf::degrees(0)) noexcept
	{
		TextWrapper newText{ content, characterSize, position, sf::Vector2f{ scale, scale }, color };
		m_texts.push_back(std::move(newText));
	}

	/**
	 * @brief Adds a sprite element to the interface.
	 * @complexity amortized O(1).
	 *
	 * @param[in] sprite: The sprite to add.
	 * @param[in] texture: The texture of the sprite.
	 *
	 * @see addText(), addShape().
	 */
	void addSprite(std::string const& texture, sf::Vector2f pos, sf::Vector2f scale, sf::Angle rot = sf::degrees(0), sf::IntRect rectangle = sf::IntRect{ sf::Vector2i{}, sf::Vector2i{} }, sf::Color color = sf::Color::White);

	/**
	 * @brief Adds a sprite element to the interface.
	 * @complexity amortized O(1).
	 *
	 * @param[in] sprite: The sprite to add.
	 * @param[in] texture: The texture of the sprite.
	 *
	 * @see addText(), addShape().
	 */
	void addSprite(sf::Texture texture, sf::Vector2f pos, sf::Vector2f scale, sf::Angle rot = sf::degrees(0), sf::IntRect rectangle = sf::IntRect{ sf::Vector2i{}, sf::Vector2i{} }, sf::Color color = sf::Color::White) noexcept;

	/**
	 * @brief Renders the interface.
	 * @complexity O(N), where N is the number of graphical elements.
	 *
	 * @pre The window must be valid.
	 * @post Safe to draw.
	 * @throw std::logic_error Basic exception guarrantee.
	 */
	virtual void draw() const;


	/**
	 * @brief Refreshes the transformables after the window is resized.
	 * @complexity O(N), where N is the number of graphical elements within all interfaces associated with the resized window.
	 *
	 * @param[in] window: The window which was resized, and for which the interfaces will be resized.
	 * @param[in] scalingFactor: The scaling factor between the previous window's size and the new one.
	 *
	 * @note Resizes only interfaces associated with the resized window.
	 */
	static void windowResized(sf::RenderWindow* window, sf::Vector2f scalingFactor) noexcept;

protected:

	/**
	 * @brief Loads/Resizes the background image and resizes it to fit the window.
	 * @details Loads the default background first, and then loads the custom background (if needed, that
	 * 			is to say if the file name is not empty). If it fails, it uses the default background.
	 * @complexity O(1).
	 *
	 * @param[in] fileName: The file name of the background.
	 *
	 * @return A string containing an error message if the loading failed.
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

#endif // BASICGUI_HPP