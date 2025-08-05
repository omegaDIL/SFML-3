/*******************************************************************
 * \file   MutableInterface.hpp, MutableInterface.cpp
 * \brief  Declare a gui that can add, edit or remove texts and sprites
 *
 * \author OmegaDIL.
 * \date   July 2025.
 *
 * \note These files depend on the SFML library.
 * \note All assertions are disabled in release mode. If broken, undefined behavior will occur.
 *********************************************************************/

#ifndef MUTABLEINTERFACE_HPP
#define MUTABLEINTERFACE_HPP

#include "BasicInterface.hpp"
#include <SFML/Graphics.hpp>
#include <string>
#include <unordered_map>
#include <utility>

namespace gui
{
	
/**
 * \brief  Manages an interface with changeable contents: texts and shapes.
 *
 * \note This class stores UI componenents; it will use a considerable amount of memory.
 * \note Each Mutable elements might consume a little more memory than their fixed counterparts.
 * \warning Avoid deleting the `sf::RenderWindow` passed as an argument while this class is using it.
 *			The progam will assert otherwise. 
 * 
 * \see `BasicInterface`.
 *
 * \code
 * sf::Vector2u windowSize{ 1000, 1000 };
 * sf::RenderWindow window{ sf::VideoMode{ windowSize }, "Template sfml 3" };
 * MGUI myInterface{ &window, 1080 }; // Create the interface with the window and the relative scaling definition.
 *
 * myInterface.addDynamicText("welc", "Welcome to the GUI!", { 500, 200 }, 48, sf::Color{ 255, 255, 255 }, "__default", gui::Alignment::Center, sf::Text::Bold | sf::Text::Underlined);
 * myInterface.addDynamicText("tes", "test1", sf::Vector2f{ 500, 500 }, 32, sf::Color{ 255, 0, 255 }, "__default", gui::Alignment::Center, sf::Text::Italic | sf::Text::Underlined);
 *
 * sf::RectangleShape rect{ sf::Vector2f{ 200, 200 } };
 * myInterface.addDynamicSprite("icon", gui::createTextureFromDrawables(rect), {500, 500}, {1.f, 1.f}, sf::IntRect{}, sf::degrees(0), gui::Alignment::Center, sf::Color::White);
 *
 * // At some point, we access that text.
 * myInterface.getDynamicText("welc")->move({ 0, -100 });
 *
 * // Here, we would want to remove a sprite.
 * myInterface.removeDynamicSprite("icon");
 * \endcode
 */
class MutableInterface : public BasicInterface
{
public:

	/**
	 * \brief Constructs the mutable graphical interface.
	 * \complexity O(1)
	 *
	 * \param[in,out] window A valid pointer to the SFML window where interface elements will be rendered.
	 * \param[in] relativeScalingDefinition A scaling baseline to ensure consistent visual proportions across
	 *			  various window sizes. Scales of `sf::Transformable` elements are adjusted based on the window size
	 *			  relative to this reference value:
	 *
	 *			  - If the smallest dimension (width or height) of the window equals `relativeScalingDefinition`,
	 *				no scaling is applied (scaling factor is 1.0).
	 *			  - If the window is smaller, the scaling factor becomes less than 1.0, making elements appear smaller.
	 *			  - If the window is larger, the scaling factor becomes greater than 1.0, making elements appear larger.
	 *
	 *			  For example, if `relativeScalingDefinition` is set to 1080:
	 *			  - A window of size 1080x1920 → factor = 1.0
	 *			  - A window of size 540x960   → factor = 0.5
	 *			  - A window of size 2160x3840 → factor = 2.0
	 *
	 *			  If set to 0, no scaling is applied regardless of window size.
	 *
	 * \note When the first instance of the interface is created, a default font named `defaultFont.ttf` is
	 *       loaded from the `assets/` directory and registered under the name `__default`.
	 *
	 * \pre `window` must be a valid.
	 * \warning The program will assert otherwise.
	 *
	 * \pre A font file named `defaultFont.ttf` must exist in the `assets/` folder.
	 * \post The default font is loaded and available for use.
	 * \throw LoadingGraphicalRessourceFailure If the default font cannot be loaded. This follows the strong
	 *        exception guarantee: the interface remains in a valid state, but no text will be rendered using
	 *        the default font. Users must manually load and assign a font to display text.
	 */
	inline explicit MutableInterface(sf::RenderWindow* window, unsigned int relativeScalingDefinition = 1080)
		: BasicInterface{ window, relativeScalingDefinition }, m_dynamicTexts{}, m_dynamicSprites{}, m_indexesForEachDynamicTexts{}, m_indexesForEachDynamicSprites{}
	{}

	MutableInterface() noexcept = delete;
	MutableInterface(MutableInterface const&) noexcept = delete;
	MutableInterface(MutableInterface&&) noexcept = default;
	MutableInterface& operator=(MutableInterface const&) noexcept = delete;
	MutableInterface& operator=(MutableInterface&&) noexcept = default;
	virtual ~MutableInterface() noexcept = default;


	/**
	 * \brief Adds a mutable text, that you can edit and remove later.
	 * \complexity Amortized O(1).
	 *
	 * Replaces the text with the same identifier if it already exists.
	 *
	 * \param[in] identifier The identifier, with which, you'll be able to access the text.
	 * \param[in] content What the `sf::Text` will display.
	 * \param[in] fontName The name of the font that'll be used.
	 * \param[in] characterSize The character size of the `sf::Text`.
	 * \param[in] pos The position of the `sf::Text`.
	 * \param[in] scale The scale of the `sf::Text`.
	 * \param[in] color The color of the `sf::Text`.
	 * \param[in] alignment The alignment of the `sf::Text`.
	 * \param[in] style The style of the `sf::Text` (regular, italic, underlined...).
	 * \param[in] rot The rotation of the `sf::Text`.
	 *
	 * \note You should not put an underscore before identifiers, as they are reserved for the class.
	 * \note Identifiers must be unique between texts and sprites. But one text and one sprite can have
	 *		 a similar identifier.
	 * 
	 * \pre   A font must have been loaded with the name you gave, either using the function `createFont`
	 *		  or being the default font under the name __default. 
	 * \post  A font will be used.
	 * \throw std::invalid_argument Strong exception guarantee: nothing happens
	 *
	 * \see `addText`.
	 */
	template<Ostreamable T>
	void addDynamicText(const std::string& identifier, const T& content, sf::Vector2f pos, unsigned int characterSize = 30u, sf::Color color = sf::Color::White, const std::string& fontName = "__default", Alignment alignment = Alignment::Center, std::uint32_t style = 0, sf::Vector2f scale = sf::Vector2f{ 1, 1 }, sf::Angle rot = sf::degrees(0))
	{
		if (m_dynamicTexts.find(identifier) != m_dynamicTexts.end())
			removeDynamicText(identifier);

		addText(content, pos, characterSize, color, fontName, alignment, style, scale, rot);
		m_dynamicTexts[identifier] = m_texts.size() - 1;
		m_indexesForEachDynamicTexts[m_texts.size() - 1] = m_dynamicTexts.find(identifier); // Add the index to the vector of indexes for dynamic sprites.
	}

	/**
	 * \brief Adds a sprite, that you can edit and remove later.
	 * \complexity Amortized O(1).
	 *
	 * Replaces the sprite with the same identifier if it already exists.
	 * 
	 * \param[in] identifier The identifier, with which, you'll be able to access the sprite.
	 * \param[in] textureName: The alias of the texture.
	 * \param[in] pos: The position of the `sf::Sprite`.
	 * \param[in] scale: The scale of the `sf::Sprite`.
	 * \param[in] rect: The display part of the texture. If the rect is equal to `sf::IntRect{}` (the
	 *					default constructor), it is set to the whole texture size.
	 * \param[in] rot: The rotation of the `sf::Sprite`.
	 * \param[in] alignment: The alignment of the `sf::Sprite`.
	 * \param[in] color: The color of the `sf::Sprite` that will be multiply by the texture color.
	 * 
	 * \note You should not put an underscore before identifiers, as they are reserved for the class.
	 * \note Identifiers must be unique between texts and sprites. But one text and one sprite can have
	 *		 a similar identifier.
	 *
	 * \pre `fileName` must refer to a valid texture file in the assets directory.
	 * \post The texture is available for use via the alias `name`.
	 * \throw invalid_argument Strong exception guarantee: nothing happens.
	 *
	 * \see `addSprite`.
	 */
	void addDynamicSprite(const std::string& identifier, const std::string& textureName, sf::Vector2f pos, sf::Vector2f scale = sf::Vector2f{ 1.f, 1.f }, sf::IntRect rect = sf::IntRect{}, sf::Angle rot = sf::degrees(0), Alignment alignment = Alignment::Center, sf::Color color = sf::Color::White);

	/**
	 * \see Similar to `addDynamicSprite`, but adds a reserved texture for it as well.
	 */
	void addDynamicSprite(const std::string& identifier, sf::Texture texture, sf::Vector2f pos, sf::Vector2f scale = sf::Vector2f{ 1.f, 1.f }, sf::IntRect rect = sf::IntRect{}, sf::Angle rot = sf::degrees(0), Alignment alignment = Alignment::Center, sf::Color color = sf::Color::White) noexcept;

	/**
	 * \brief Removes a text from the GUI. No effet if not there.
	 * \complexity O(1).
	 *
	 * \param[in] name: The identifier of the text.
	 *
	 * \see `removeDynamicSprite`.
	 */
	virtual void removeDynamicText(const std::string& identifier) noexcept;

	/**
	 * \brief Removes a sprite from the GUI. No effet if not there.
	 * \complexity O(1).
	 *
	 * \param[in] name: The identifier of the sprite.
	 *
	 * \see `removeDynamicText`.
	 */
	virtual void removeDynamicSprite(const std::string& identifier) noexcept;

	/**
	 * \brief Returns a text Wrapper ptr, or nullptr if it does not exist.
	 * \complexity O(1).
	 *
	 * \param[in] identifier: The identifier of the text.
	 *
	 * \return The address of the text.
	 * 
	 * \warning The returned pointer is not guaranteed to be valid after ANY addition or removal of a
	 *			dynamic text.
	 *
	 * \see `TextWrapper`.
	 */
	[[nodiscard]] TextWrapper* getDynamicText(const std::string& identifier) noexcept;

	/**
	 * \brief Returns a sprite Wrapper ptr, or nullptr if it does not exist.
	 * \complexity O(1).
	 *
	 * \param[in] identifier: The identifier of the sprite.
	 *
	 * \return The address of the sprite.
	 *
	 * \warning The returned pointer is not guaranteed to be valid after ANY addition or removal of a
	 *			dynamic sprite.
	 * 
	 * \see `SpriteWrapper`.
	 */
	[[nodiscard]] SpriteWrapper* getDynamicSprite(const std::string& identifier) noexcept;

protected:


	std::unordered_map<std::string, size_t> m_dynamicTexts; // All indexes of dynamic texts in the interface.
	std::unordered_map<std::string, size_t> m_dynamicSprites; // All indexes of dynamic sprites in the interface.

	using UmapDynamicsIterator = std::unordered_map<std::string, size_t>::iterator;
	std::unordered_map<size_t, UmapDynamicsIterator> m_indexesForEachDynamicTexts; // Allows removal of dynamic texts in O(1).
	std::unordered_map<size_t, UmapDynamicsIterator> m_indexesForEachDynamicSprites; // Allows removal of dynamic sprites in O(1).


	/**
	 * \brief Swaps two elements in the vector, and updates the identifier map and index map accordingly.
	 * \complexity O(1).
	 * 
	 * \param[in] index1 The first  index of the vector to swap.
	 * \param[in] index2 The second index of the vector to swap.
	 * \param[out] vector The vector that contains the elements to swap.
	 * \param[out] identifierMap The map that enables accessing the container's elements using their identifier.
	 * \param[in,out] indexMap The map that enables accessing the container's elements using indexes.
	 * 
	 * \pre No index should be out of range.
	 * \post Those indexes will be swapped accordingly
	 * \warning Asserts if out of range.
	 */
	template<typename T> requires (std::same_as<T, TextWrapper> || std::same_as<T, SpriteWrapper>)
	void swapElement(size_t index1, size_t index2, std::vector<T>& vector, std::unordered_map<std::string, size_t>& identifierMap, std::unordered_map<size_t, UmapDynamicsIterator>& indexMap) noexcept
	{
		ENSURE_NOT_OUT_OF_RANGE(index1, vector.size(), "Precondition violated; the first  index to swap is out of range in the function swapElement of MutableInterface");
		ENSURE_NOT_OUT_OF_RANGE(index2, vector.size(), "Precondition violated; the second index to swap is out of range in the function swapElement of MutableInterface");

		if (index1 == index2) [[unlikely]]
			return;

		std::swap(vector[index1], vector[index2]);

		const auto mapIteratorIndex1{ indexMap.find(index1) };
		const auto mapIteratorIndex2{ indexMap.find(index2) };

		if (mapIteratorIndex1 == indexMap.end()
		&&  mapIteratorIndex2 == indexMap.end()) [[unlikely]]
			return; // Not dynamic

		if (mapIteratorIndex1 != indexMap.end()
		&&  mapIteratorIndex2 != indexMap.end())
		{	// When both are dynamics
			identifierMap[mapIteratorIndex2->second->first] = index1;
			identifierMap[mapIteratorIndex1->second->first] = index2;
			std::swap(indexMap[index1], indexMap[index2]);
			return;
		}

		// When only one is dynamic.
		const auto dynamicElementIterator{ (mapIteratorIndex1 != indexMap.end()) ? mapIteratorIndex1 : mapIteratorIndex2 };
		dynamicElementIterator->second->second = +index2 +index1 -dynamicElementIterator->second->second; // Update the index of the dynamic element.
		indexMap[dynamicElementIterator->second->second] = dynamicElementIterator->second;
		indexMap.erase(dynamicElementIterator);
	}
};

} // gui namespace

#endif //MUTABLEINTERFACE_HPP