/*******************************************************************
 * @file   GUI.hpp
 * @brief  Declare the entities for creating and managing graphical user interfaces.
 *
 * @author OmegaDIL.
 * @date   July 2024.
 *
 * @note This file depends on the SFML library.
 *********************************************************************/

#ifndef GUI_HPP
#define GUI_HPP

#include <SFML/Graphics.hpp>
#include <string>
#include <unordered_map>
#include <vector>
#include <span>
#include <valarray>
#include <utility>
#include <optional>
#include <stdexcept>
#include <array>
#include <algorithm>
#include <initializer_list>
#include <memory>
#include <functional>
#include <sstream>
#include "Exceptions.hpp"
#include "GUITexturesLoader.hpp"


/**
 * @brief Must be a valid data types for the function << of std::basic_ostream.
 *
 * @param os: To check if it is valid with basic_ostream.
 * @tparam T: The type to check.
 */
template <typename T>
concept Ostreamable = requires(std::ostream & os, T t)
{
	{ os << t } -> std::same_as<std::ostream&>;
};

/**
 * @brief Manages items to create a basic GUI. You can display texts, sprites, shapes and a background.
 * @details All elements are fixed and can't be changed.
 *
 * @note This class stores UI componenents; it will consume a considerable amount of memory. 
 * @note Sprites refer to sf::Sprites/sf::Textures and sf::Textures.
 * @note The background will always be the first element in the vector.
 * @warning Avoid deleting the `sf::RenderWindow` passed as an argument while this class is using it.
 *
 * @see sf::RenderWindow, GraphicalFixedInterface::TextWrapper.
 */
class GraphicalFixedInterface
{
public:

	GraphicalFixedInterface() noexcept = delete;
	GraphicalFixedInterface(GraphicalFixedInterface const&) noexcept = default;
	GraphicalFixedInterface(GraphicalFixedInterface&&) noexcept = default;
	GraphicalFixedInterface& operator=(GraphicalFixedInterface const&) noexcept = default;
	GraphicalFixedInterface& operator=(GraphicalFixedInterface&&) noexcept = default;

	/**
	 * @brief Constructs The interface with a default background.
	 * @complexity O(1).
	 * 
	 * @param[out] window: The window where the interface elements will be rendered. 
	 * 
	 * @see create().
	 */
	explicit GraphicalFixedInterface(sf::RenderWindow* window) noexcept;

	/**
	 * @brief Creates the interface: adds a background and load the default font.
	 * @complexity O(1).
	 * 
	 * @param[in] fileName: The name of the background.
	 * 
	 * @return A string if the font or the background loading failed.
	 * 
	 * @pre the name should be a valid path to a file or empty.
	 * @throw LoadingGUIRessourceFailure if either the font or the background fails to load.
	 * 
	 * @pre The window must be valid.
	 * @post This class is safe to use.
	 * @throw std::logic_error if the window is nullptr.
	 * 
	 * @note If the fileName is empty, the default background will be used.
	 * @note If you already have a background, it will be replaced.
	 * @note If you already have a font and don't want to load a background, it is useless to call this
	 *		 function.
	 */
	[[nodiscard]] virtual std::optional<std::string> create(std::string const& fileName = "");

	virtual ~GraphicalFixedInterface() noexcept
	{
		auto interfaceRange{ allInterfaces.equal_range(m_window) }; // All interfaces associated with the resized window.
		
		for (auto elem{ interfaceRange.first }; elem != interfaceRange.second;)
		{
			if (elem->second == this)
				elem = allInterfaces.erase(elem); // Remove the interface from the collection.
			else	
				elem++;	
		}
	}


	/**
	 * @brief Adds a text element to the interface.
	 * @complexity O(1) : average case.
	 * @complexity O(N) : worst case, where N is the number of texts already added. If vector resizes.
	 * 
	 * @tparam T: Type that can be streamed to `std::basic_ostream`.
	 * @param[in] content: The text label.
	 * @param[in] position: The position of the text.
	 * @param[in] characterSize: The size of the text characters.
	 * @param[in] color: The color of the text.
	 * 	 * 
	 * @see GraphicalFixedInterface::TextWrapper, addSprite(), addShape().
	 */
	template<Ostreamable T>
	void addText(T const& content, sf::Vector2f position, unsigned int characterSize, sf::Color color = sf::Color{ 255, 255, 255 }, sf::Angle rot = sf::degrees(0)) noexcept
	{
		TextWrapper newText{ content, position, characterSize, color };
		newText.updateRotation(rot);
		m_texts.push_back(std::move(newText));
	}

	/**
	 * @brief Adds a sprite element to the interface.
	 * @complexity O(1) : average case.
	 * @complexity O(N) : worst case, where N is the number of sprites already added. If vector resizes.
	 *
	 * @param[in] sprite: The sprite to add.
	 * @param[in] texture: The texture of the sprite.
	 * 
	 * @see addText(), addShape().
	 */
	void addSprite(sf::Sprite sprite, sf::Texture texture) noexcept;

	/**
	 * @brief Adds a sprite element to the interface.
	 * @complexity O(1) : average case.
	 * @complexity O(N) : worst case, where N is the number of sprites already added. If vector resizes.
	 *
	 * @param[in] shape: The shape to add.
	 * @param[in] smooth: True if the shape needs to be smoothed.
	 * 
	 * @see addText(), addSprite(), convertShapeToSprite().
	 */
	void addShape(sf::Shape* shape, bool smooth = true) noexcept;

	/** 
	 * @brief Renders the interface.
	 * @complexity O(N + K), where N is the number of texts, and K the number of sprites.
	 *
	 * @pre The window must have been valid when passed to the constructor and still exists.
	 * @post The elements are drawn on the window.
	 * @throw std::logic_error if window is nullptr.
	 */
	virtual void draw() const;


	/**
	 * @brief Refreshes the transformables after the window is resized.
	 *
	 * @param[in] window: The window which was resized, and for which the interfaces will be resized.
	 * @param[in] scalingFactor: The scaling factor between the previous window's size and the new one.
	 * 
	 * @note Be aware of windowSize: it should have the new size.
	 */
	static void windowResized(sf::RenderWindow* window, sf::Vector2f scalingFactor) noexcept;


	// The relative path to the res folder used to load ressources.
	static std::string ressourcePath;

protected:

	/**
	 * @details The pointers to the textures become invalid when adding or removing an element
	 *          to the sprite vector.
	 * @complexity O(N), where N is the size of 'm_sprites'
	 */
	void resetTextureForSprites() noexcept;


	/**
	 * @brief A wrapper for `sf::Text` that initializes the text, keeps the font, manages resizing.
	 *
	 * @note The font is common to all instances of this class.
	 * @warning Call TextWrapper::loadFont() to initialize properly the font, before creating any other
	 *          functions.
	 *
	 * @see GraphicalFixedInterface, sf::Text, create().
	 */
	struct TextWrapper : private sf::Text
	{
	public:

		TextWrapper(TextWrapper const&) noexcept = default;
		TextWrapper(TextWrapper&&) noexcept = default;
		virtual ~TextWrapper() noexcept = default;
		virtual TextWrapper& operator=(TextWrapper const&) noexcept = default;
		TextWrapper& operator=(TextWrapper&&) noexcept = default;

		/**
		 * @brief Initializes the text with the specified parameters.
		 * @complexity O(1).
		 *
		 * @tparam T: Type that can be streamed to `std::basic_ostream`.
		 * @param[in] string: Its content.
		 * @param[in] pos: The position to place the text.
		 * @param[in] characterSize: The size of the text characters.
		 * @param[in] color: The color of the text.
		 */
		template<Ostreamable T>
		inline TextWrapper(T const& content, sf::Vector2f pos, unsigned int characterSize, sf::Color color = sf::Color{ 255, 255, 255 }, sf::Angle rot = sf::degrees(0)) noexcept
			: sf::Text{ m_font, "", characterSize}
		{
			setFillColor(color);
			setRotation(rot);
			updateContent(content);
			setPosition(pos);
		}


		/**
		 * @brief Updates the text content.
		 * @complexity O(1).
		 *
		 * @tparam T: Type that can be streamed to `std::basic_ostream`.
		 * @param[in] content: The new content for the text.
		 *
		 * @see updatePosition(), updateColor().
		 */
		template<Ostreamable T>
		inline void updateContent(T const& content) noexcept
		{
			std::ostringstream oss{}; // Convert the content to a string
			oss << content; // Assigning the content to the variable.
			
			setString(oss.str());
			setOrigin(sf::Vector2f{ getLocalBounds().size.x / 2.f, getLocalBounds().size.y / 2.f }); // Recenters the text.
		}

		/**
		 * @brief Updates the position of the text.
		 * @complexity O(1).
		 *
		 * @param[in] pos: The new position for the text.
		 *
		 * @see sf::Text::setPosition().
		 */
		inline void updatePosition(sf::Vector2f pos) noexcept
		{
			setPosition(pos);
		}

		/**
		 * @brief Updates the rotation of the text.
		 * @complexity O(1).
		 *
		 * @param[in] rot: The new rotation for the text.
		 *
		 * @see sf::Text::setRotation.
		 */
		inline void updateRotation(sf::Angle rot) noexcept
		{
			setRotation(rot);
		}

		/**
		 * @brief Updates the scale of the text.
		 * @complexity O(1).
		 *
		 * @param[in] scale: Adds the scale to the current one.
		 * 
		 * @note Doesn't override the previous scale : use scale() instead of setScale().
		 *
		 * @see sf::Text::scale.
		 */
		inline void updateScale(sf::Vector2f scale) noexcept
		{
			this->scale(scale);
		}

		/**
		 * @brief Updates the size of the text.
		 * @complexity O(1).
		 *
		 * @param[in] size: The size of the characters.
		 *
		 * @see sf::Text::scale().
		 */
		inline void updateCharacterSize(unsigned int size) noexcept
		{
			setCharacterSize(size);
			setOrigin(sf::Vector2f{ getLocalBounds().size.x / 2.f, getGlobalBounds().size.y / 2.f}); // Recenters the text.
		}

		/**
		 * @brief Updates the color of the text.
		 * @complexity O(1).
		 *
		 * @param[in] color: The new color for the text.
		 *
		 * @see sf::Text::setFillColor().
		 */
		inline void updateColor(sf::Color color) noexcept
		{
			setFillColor(color);
		}

		/**
		 * @brief Resizes the text when the window is resized.
		 *
		 * @param[in] scalingFactor: The scaling factor between the previous window's size and the new one.
		 */
		inline void windowResized(sf::Vector2f scalingFactor, sf::Vector2f scalingFactorMin) noexcept
		{
			scale(scalingFactorMin);
			setPosition(sf::Vector2f{ getPosition().x * scalingFactor.x, getPosition().y * scalingFactor.y });
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
		 *
		 * @return a string if failed.
		 */
		[[nodiscard]] static std::optional<std::string> loadFont() noexcept;

	private:

		static sf::Font m_font; // The font used for the texts.
	};


	// Pointer to the window.
	mutable sf::RenderWindow* m_window; 
	
	using GraphicalElement = std::pair<sf::Sprite, sf::Texture>;
	// Collection of sprites in the interface.
	std::vector<GraphicalElement> m_sprites;

	// Collection of texts in the interface.
	std::vector<GraphicalFixedInterface::TextWrapper> m_texts; 

private:

	// Collection of all interfaces to perform the same operation (resizing for example). Different 
	// for each window.
	static std::unordered_multimap<sf::RenderWindow*, GraphicalFixedInterface*> allInterfaces;
};


/**
 * @brief  Manages an interface with changeable contents of elements (texts and shapes).
 *
 * @note D stands for dynamic.
 * @note This class stores UI componenents; it will consume a considerable amount of memory.
 * @note The background is dynamic and changeable with the getSprite() function using the attribut
 *       "background".
 * @warning Do not delete the `sf::RenderWindow` passed as an argument while this class is in use.
 *
 * @see GraphicalFixedInterface, sf::RenderWindow.
 */
class GraphicalDynamicInterface : public GraphicalFixedInterface
{
public:

	GraphicalDynamicInterface() noexcept = delete;
	GraphicalDynamicInterface(GraphicalDynamicInterface const&) noexcept = default;
	GraphicalDynamicInterface(GraphicalDynamicInterface&&) noexcept = default;
	virtual ~GraphicalDynamicInterface() noexcept = default;
	GraphicalDynamicInterface& operator=(GraphicalDynamicInterface const&) noexcept = default;
	GraphicalDynamicInterface& operator=(GraphicalDynamicInterface&&) noexcept = default;

	/**
	 * @brief Constructs the interface with a default background.
	 * @complexity O(1).
	 *
	 * @param[out] window: The window where the interface elements will be rendered.
	 *
	 * @see createBackground().
	 */
	inline explicit GraphicalDynamicInterface(sf::RenderWindow* window) noexcept
		: GraphicalFixedInterface{ window }, m_dynamicTextsIds{}, m_dynamicSpritesIds{}
	{}


	/**
	 * @brief Adds a dynamic text to the interface.
	 * @complexity O(1) : average case.
	 * @complexity O(N) : worst case, where N is the number of texts already added (static and dynamic).
	 * 
	 * @tparam T: Type that can be streamed to `std::basic_ostream`.
	 * @param[in] identifier:     The text identifier.
	 * @param[in] content:		  The text label.
	 * @param[in] position:       The position of the text.
	 * @param[in] characterSize:  The size of the text characters.
	 * @param[in] color:		  The color of the text.
	 *
	 * @note No effect if a text has the same id.
	 * @note The identifiers must be unique between texts.
	 * @note The ids between one text and one sprite can be the same.
	 * 
	 * @see addText().
	 */
	template<Ostreamable T>
	void addDynamicText(std::string const& identifier, T const& content, sf::Vector2f position, unsigned int characterSize, sf::Color color = sf::Color{ 255, 255, 255 }, sf::Angle rot = sf::degrees(0))
	{
		if (m_dynamicTextsIds.find(identifier) != m_dynamicTextsIds.end())
			return;

		addText(content, position, characterSize, color, rot);
		m_dynamicTextsIds[identifier] = m_texts.size() - 1;
	}

	/**
	 * @brief Adds a dynamic sprite element to the interface.
	 * @complexity O(N) where N is the number of shapes + sprites already added.
	 *
	 * @param[in] identifier: The text identifier.
	 * @param[in] sprite: The sprite to add.
	 * @param[in] texture: The texture of the sprite.
	 *
	 * @note No effect if a sprite has the same id.
	 * @note The identifiers must be unique between shapes/sprites.
	 * @note The ids between one text and one sprite can be the same.
	 *
	 * @see addSprite().
	 */
	void addDynamicSprite(std::string const& identifier, sf::Sprite sprite, sf::Texture texture) noexcept;

	/**
	 * @brief Adds a dynamic shape element to the interface.
	 * @complexity O(N) where N is the number of shapes + sprites already added.
	 *
	 * @param[in] identifier: The text identifier.
	 * @param[in] shape: The shape to add.
	 * @param[in] smooth: True if the shape needs to be smoothed.
	 *
	 * @note No effect if a sprite has the same id.
	 * @note The identifiers must be unique between shapes/sprites.
	 * @note The ids between one text and one shape can be the same.
	 *
	 * @see addShape().
	 */
	void addDynamicShape(std::string const& identifier, sf::Shape* shape, bool smooth = true) noexcept;

	/**
	 * @brief Allows you to change some attributes of a dynamic text.
	 * 
	 * @param[in] identifier: The id of the text you want to access.
	 * 
	 * @return A reference to the text you want to access.
	 * 
	 * @pre Be sure that you added a text with this id.
	 * @post The appropriate text is returned.
	 * @throw std::out_of_range if id not there.
	 */
	GraphicalFixedInterface::TextWrapper& getDText(std::string const& identifier);

	/**
	 * @brief Allows you to change some attributes of a dyanmic sprite.
	 *
	 * @param[in] identifier: The id of the sprite you want to access.
	 *
	 * @return A reference to the sprite you want to access.
	 *
	 * @pre Be sure that you added the sprite with this id.
	 * @post The appropriate sprite is returned.
	 * @throw std::out_of_range if id not there.
	 */
	std::pair<sf::Sprite*, sf::Texture*> getDSprite(std::string const& identifier);

	/**
	 * @brief Removes a dynamic text.
	 *
	 * @param[in] identifier: The id of the text you want to remove.
	 * 
	 * @note No effect if not there.
	 */
	virtual void removeDText(std::string const& identifier) noexcept;

	/**
	 * @brief Removes a sprite
	 *
	 * @param[in] identifier: The id of the sprite you want to remove.
	 * 
	 * @note No effect if not there.
	 */
	virtual void removeDSprite(std::string const& identifier) noexcept;

protected:

	std::unordered_map<std::string, size_t> m_dynamicTextsIds; // All index of dynamic texts in the interface.
	std::unordered_map<std::string, size_t> m_dynamicSpritesIds; // All index of dynamic sprites in the interface.
};


/** 
 * @brief  Manages an interface in which the user can interact such as writing, pressing buttons...
 *
 * @note This class stores UI componenents; it will consume a considerable amount of memory.
 * @warning Do not delete the `sf::RenderWindow` passed as an argument while this class is in use.
 *
 * @see GraphicalDynamicInterface, Slider, sf::RenderWindow.
 */
class GraphicalUserInteractableInterface : public GraphicalDynamicInterface
{
public:

	enum class InteractableItem
	{
		None,
		Button,
		Slider,
		MultipleQuestionBox
	};

	using IdentifierInteractableItem = std::pair<InteractableItem, std::string*>; 


	GraphicalUserInteractableInterface() noexcept = delete;
	GraphicalUserInteractableInterface(GraphicalUserInteractableInterface const&) noexcept = default;
	GraphicalUserInteractableInterface(GraphicalUserInteractableInterface&&) noexcept = default;
	virtual ~GraphicalUserInteractableInterface() noexcept = default;
	GraphicalUserInteractableInterface& operator=(GraphicalUserInteractableInterface const&) noexcept = default;
	GraphicalUserInteractableInterface& operator=(GraphicalUserInteractableInterface&&) noexcept = default;

	/**
	 * @brief Constructs the interface with a default background.
	 * @complexity O(1).
	 *
	 * @param[out] window: The window where the interface elements will be rendered.
	 *
	 * @see createBackground().
	 */
	inline explicit GraphicalUserInteractableInterface(sf::RenderWindow* window) noexcept
		: GraphicalDynamicInterface{ window }, m_buttons{}, m_sliders{}, m_mqbs{}, m_hoveredElement{ std::make_pair(InteractableItem::None, nullptr) }
	{}


	/**
	 * @brief Adds a button to the interface by turning a text into a button.
	 *
	 * @param[in] id: The id of the text you want to turn into a button
	 * @param[in] function: The function executed when the button is pressed.
	 *
	 * @note No effect if no text with the id, or there's already a button with the same id.
	 * @note The button id is the same as the text id.
	 * @note You can still access the text turned with the function getDText().
	 * @warning Don't remove the button using removeDText(), use removeButton() instead.
	 */
	void addButton(std::string const& id, std::function<void()> function = [](){}) noexcept;

	[[nodiscard]] std::function<void()>& getFunctionOfButton(std::string const& identifier);

	/**
	 * @brief Removes a button
	 *
	 * @param[in] identifier: The id of the button you want to remove.
	 *
	 * @note No effect if not there.
	 */
	void removeButton(std::string const& identifier) noexcept;

	/** 
	 * @brief Detects if the mouse is hovering over a button to update its visual state.
	 * @complexity O(N), where N is the number of buttons.
	 *
	 * @note You should call this function when you change the menu to position the button's background 
	 *       accordingly.
	 * 
	 * @pre The window must have been valid when passed to the constructor and still exists.
	 * @post The elements are drawn on the window.
	 * @throw std::logic_error if window is nullptr.
	 */
	IdentifierInteractableItem mouseMoved();

	IdentifierInteractableItem mousePressed() noexcept;



	/**
	 * @complexity O(1).
	 * 
	 * @return The id of the button that is currently hovered.
	 */
	[[nodiscard]] inline IdentifierInteractableItem getHoveredInteractableItem() const noexcept
	{
		return m_hoveredElement;
	}

	/**
	 * @brief Renders the interface.
	 * @complexity O(N + K + M), where N is the number of texts, K the number of shapes, and M the number
	 *			   of sliders.
	 *
	 * @pre The window must have been valid when passed to the constructor and still exists.
	 * @post The elements are drawn on the window.
	 * @throw std::logic_error if window is nullptr.
	 */
	virtual void draw() const override;

private:
	
	class Slider
	{
	public:

		Slider() noexcept = delete;
		Slider(Slider const&) noexcept = default;
		Slider(Slider&&) noexcept = default;
		~Slider() noexcept = default;
		Slider& operator=(Slider const&) noexcept = default;
		Slider& operator=(Slider&&) noexcept = default;

		Slider(sf::Vector2f pos, unsigned int length, bool integers = false) noexcept;

		/**
		 * 
		  * 
		 */
		void changeTexture(std::string backgroundSlider = "", std::string cursorSlider ="");


		/**
		 * .
		 * 
		 * \return 
		 */
		void mouseMoved() noexcept;

		/**
		 * .
		 * 
		 * \return 
		 */
		float getValue() const noexcept;

		float setCursor(float relativePos = 0.0f) noexcept;

		//TODO: add a function to resize when event is triggered.

	private:

		/**
		 * @brief Loads the default textures for the slider.
		 * 
		 * @note It is useless to call this function if the textures are already loaded.
		 */
		void loadDefaultTexture(sf::Vector2u size) noexcept;


		std::unique_ptr<sf::Sprite> m_backgroundSlider; // The background of the slider.
		std::unique_ptr<sf::Sprite> m_cursorSlider; // The slider itself.

		sf::Texture m_textureBackgroundSlider; // The texture of the background.
		sf::Texture m_textureCursorSlider; // The texture of the slider.;
	};

	class MultipleQuestionBox
	{
	public:

		MultipleQuestionBox() noexcept = delete;
		MultipleQuestionBox(MultipleQuestionBox const&) noexcept = default;
		MultipleQuestionBox(MultipleQuestionBox&&) noexcept = default;
		~MultipleQuestionBox() noexcept = default;
		MultipleQuestionBox& operator=(MultipleQuestionBox const&) noexcept = default;
		MultipleQuestionBox& operator=(MultipleQuestionBox&&) noexcept = default;

		/**
		 * 
		 * \param box
		 * \param onlyOne: True if only one box can be selected.
		 */
		MultipleQuestionBox(std::vector<sf::Vector2f> pos, bool onlyOne) noexcept;


		void addBox(sf::Vector2f pos) noexcept;

		void removeBox(size_t index) noexcept;
		
		void mousePressed() noexcept;

		std::vector<bool> getValues() const noexcept;

		void reset(bool value = false) noexcept;


		bool onlyeOne; // True if only one box can be selected.

	private:

		std::vector<std::pair<sf::Sprite, bool>> m_boxes; // The boxes of the MQB.
		sf::Texture m_textureBoxUnchecked; // The texture of the boxes.
		sf::Texture m_textureBoxChecked; // The texture of the boxes.
	};


	using Button = std::pair<size_t, std::function<void()>>; // The first is the element's index and the second is the text's index.
	using MQB = MultipleQuestionBox;

	std::unordered_map<std::string, Button> m_buttons; // Collection of buttons in the interface.
	std::unordered_map<std::string, Slider> m_sliders; // Collection of sliders in the interface.
	std::unordered_map<std::string, MQB> m_mqbs; // Collection of MQBs in the interface.

	IdentifierInteractableItem m_hoveredElement; // The type of the button that is currently hovered.
};


using GInterface = GraphicalFixedInterface;
using GDynamicInterface = GraphicalDynamicInterface;
using GInteractableInterface = GraphicalUserInteractableInterface;
using GUI = GraphicalUserInteractableInterface;

#endif // GUI_HPP