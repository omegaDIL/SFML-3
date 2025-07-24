/*******************************************************************
 * \file   MutableInterface.hpp, MutableInterface.cpp
 * \brief  Declare a gui that can add, edit or remove texts and sprites
 *
 * \author OmegaDIL.
 * \date   July 2025.
 *
 * \note These files depend on the SFML library.
 *********************************************************************/

#ifndef MUTABLEINTERFACE_HPP
#define MUTABLEINTERFACE_HPP

#include "BasicInterface.hpp"
#include <SFML/Graphics.hpp>
#include <string>
#include <unordered_map>

#ifndef NDEBUG 
#define ENSURE_STRING_NOT_EMPTY(string) \
	assert(!(string).empty() && "ENSURE_STRING_NOT_EMPTY failed: string is empty.")
#else
#define ENSURE_STRING_NOT_EMPTY()
#endif

namespace gui
{
	
/**
 * \brief  Manages an interface with changeable contents: texts and shapes.
 *
 * \note This class stores UI componenents; it will use a considerable amount of memory.
 * \warning Avoid deleting the `sf::RenderWindow` passed as an argument while this class is using it.
 *			The progam will assert otherwise. 
 * 
 * \see `BasicInterface`.
 *
 * \code
 * sf::Vector2u size{ 2560, 1440 };
 * sf::RenderWindow window{ sf::VideoMode{ size }, "My project" };
 * MutableInterface mainInterface{ &window, 1080 }; // All elements will be scaled with	a factor 1440/1080 = 1.33f.
 * 
 * mainInterface.addDynamicText("greetings", "Welcome!!", { 1280, 500 }, sf::Color{ 255, 255, 255 }, 48, "__default", SpriteWrapper::Alignment::Center, sf::Style::Bold | sf::Style::Underlined);
 * mainInterface.addSprite("projectIcon.png", window.getView().getCenter(), sf::Vector{ 0.5f, 0.5f }); // You can still	add regular sprites/texts.
 * 
 * // Lets assume we are within the loop (see BasicInterface) and we met a particular condition.
 * mainInterface.getDynamicText("greetings")->setPosition(window.getView().getCenter()); // Access the TextWrapper class.
 * // Some other conditions met.
 * mainInterface.removeDynamicText("greetings");
 * // Same goes for sprites.
 * \endcode
 */
class MutableInterface : public BasicInterface
{
public:

	/**
	 * \brief Constructs the interface.
	 * \complexity O(1).
	 *
	 * \param[in,out] window: The window where the interface elements will be rendered.
	 * \param[in] relativeScaling: The scales of the `sf::Transformable`s are absolute and will make an
	 *			  item appear at a certain size. However, in smaller windows, the same scale will make a
	 *			  `sf::Transformable` appear larger compared to the window. Therefore a factor will be multiplied
	 *			  to the scales to adjust them. This factor is equal to 1.f with windows that have
	 *			  `relativeScalingDefinition` (this parameter) as their smallest in x or y axis. For example,
	 *			  with 1080: if your window is currently set to 1080 in x or y axis, then all scales are multiply
	 *			  by 1.f. But if the window is suddenly set to 2160, then all scales are multiply by 2.
	 *
	 * \note A default font is loaded when the first instance is called.
	 * \warning The program asserts if the window is not valid (nullptr or size 0).
	 * \warning The program asserts if the relativeScaling is set to 0.
	 *
	 * \pre A font should be named defaultFont.ttf in the assets folder.
	 * \post The font is loaded.
	 * \throw LoadingGraphicalRessourceFailure Strong exception guarrantee, but no text can use the
	 *		  default font and you'll have to load and use your own font.
	 */
	inline explicit MutableInterface(sf::RenderWindow* window, unsigned int relativeScalingDefinition = 1080)
		: BasicInterface{ window, m_relativeScalingDefinition }, m_dynamicTexts{}, m_dynamicSprites{}, m_indexesForEachDynamicTexts{}, m_indexesForEachDynamicSprites{}
	{}

	MutableInterface() noexcept = delete;
	MutableInterface(MutableInterface const&) noexcept = delete;
	MutableInterface(MutableInterface&&) noexcept = default;
	MutableInterface& operator=(MutableInterface const&) noexcept = delete;
	MutableInterface& operator=(MutableInterface&&) noexcept = default;
	virtual ~MutableInterface() noexcept = default;


	/**
	 * \brief Adds a dynamic text element to the interface, that is editable and removable.
	 * If the identifier was already there, the text is replaced. 
	 * \complexity amortized O(1).
	 *
	 * \param[in] identifier: The identfier, with which, you'll be able to access the text.
	 * \param[in] content: What the `sf::Text` will display.
	 * \param[in] fontName: The name of the font that'll be used.
	 * \param[in] characterSize: The character size of the `sf::Text`.
	 * \param[in] pos: The position of the `sf::Text`.
	 * \param[in] color: The color of the `sf::Text`.
	 * \param[in] alignment: The alignment of the `sf::Text`.
	 * \param[in] style: The style of the `sf::Text` (regular, italic, underlined...).
	 * \param[in] scale: The scale of the `sf::Text`.
	 * \param[in] rot: The rotation of the `sf::Text`.
	 *
	 * \note Some styles may not be available with your font.
	 * \note You should not put an underscore before dynamic elements' identifiers, as they are reserved
	 *       for the class.
	 * \note Ids must be unique between texts, and between sprites. But one text and one sprite can have
	 *		 a similar identifier.
	 * \warning Asserts if the identifier is empty.
	 *
	 * \pre   A font should have been loaded with the name you gave.
	 * \post  The correct font will be used.
	 * \throw std::invalid_argument strong exception guarantee: nothing happens.
	 *
	 * \see `TextWrapper`, `addText`.
	 */
	template<Ostreamable T>
	void addDynamicText(const std::string& identifier, const T& content, sf::Vector2f pos, sf::Color color = sf::Color::White, unsigned int characterSize = 30u, const std::string& fontName = "__default", Alignment alignment = Alignment::Center, sf::Text::Style style = sf::Text::Style::Regular, sf::Vector2f scale = sf::Vector2f{ 1, 1 }, sf::Angle rot = sf::degrees(0))
	{
		ENSURE_STRING_NOT_EMPTY(identifier);

		if (m_dynamicTexts.find(identifier) != m_dynamicTexts.end())
			removeDynamicText(identifier);

		addText(content, pos, color, characterSize, fontName, alignment, style, scale, rot);
		m_dynamicTexts[identifier] = m_texts.size() - 1;
		m_indexesForEachDynamicTexts[m_texts.size() - 1] = m_dynamicTexts.find(identifier); // Add the index to the vector of indexes for dynamic sprites.
	}

	/**
	 * \brief Adds a dynamic sprite element to the interface, that is editable and removable.
	 * If the identifier was already there, the sprite is replaced.
	 * \complexity amortized O(1).
	 *
	 * \param[in] identifier: The identfier, with which, you'll be able to access the sprite.
	 * \param[in] textureName: The alias of the texture.
	 * \param[in] pos: The position of the `sf::Sprite`.
	 * \param[in] scale: The scale of the `sf::Sprite`.
	 * \param[in] rect: The display part of the texture. If size is 0, it is set to the whole texture size.
	 * \param[in] rot: The rotation of the `sf::Sprite`.
	 * \param[in] alignment: The alignment of the `sf::Sprite`.
	 * \param[in] color: The color of the `sf::Sprite` that will be multiply by the texture color.
	 *
	 * \note You should not put an underscore before dynamic elements' identifiers, as they are reserved
	 *       for the class.
	 * \note Ids must be unique between texts, and between sprites. But one text and one sprite can have
	 *		 a similar identifier.
	 * \warning Asserts if the identifier is empty.
	 *
	 * \pre   A texture should have loaded with the name you gave.
	 * \pre   It should not be already reserved.
	 * \post  The correct texture will be used.
	 * \throw std::invalid_argument strong exception guarantee: nothing happens.
	 *
	 * \see `SpriteWrapper`, `addSprite`.
	 */
	void addDynamicSprite(const std::string& identifier, const std::string& textureName, sf::Vector2f pos, sf::Vector2f scale = sf::Vector2f{ 1.f, 1.f }, sf::IntRect rect = sf::IntRect{}, sf::Angle rot = sf::degrees(0), Alignment alignment = Alignment::Center, sf::Color color = sf::Color::White);

	/**
	 * \brief Adds a dynamic sprite element to the interface, that is editable and removable.
	 * If the identifier was already there, the sprite is replaced.
	 * \complexity amortized O(1).
	 *
	 * \param[in] identifier: The identfier, with which, you'll be able to access the sprite.
	 * \param[in] texture: The texture of the sprite.
	 * \param[in] pos: The position of the `sf::Sprite`.
	 * \param[in] scale: The scale of the `sf::Sprite`.
	 * \param[in] rect: The display part of the texture. If size is 0, it is set to the whole texture size.
	 * \param[in] rot: The rotation of the `sf::Sprite`.
	 * \param[in] alignment: The alignment of the `sf::Sprite`.
	 * \param[in] color: The color of the `sf::Sprite` that will be multiply by the texture color.
	 *
	 * \note You should not put an underscore before dynamic elements' identifiers, as they are reserved
	 *       for the class.
	 * \note Ids must be unique between texts, and between sprites. But one text and one sprite can have
	 *		 a similar identifier.
	 * \warning Asserts if the identifier is empty.
	 *
	 * \see `SpriteWrapper`, `addSprite`.
	 */
	void addDynamicSprite(const std::string& identifier, sf::Texture texture, sf::Vector2f pos, sf::Vector2f scale = sf::Vector2f{ 1.f, 1.f }, sf::IntRect rect = sf::IntRect{}, sf::Angle rot = sf::degrees(0), Alignment alignment = Alignment::Center, sf::Color color = sf::Color::White) noexcept;

	/**
	 * \brief Removes a text from the GUI. No effet if not there.
	 * \complexity O(1).
	 *
	 * \param[in] name: The identifier of the text.
	 *
	 * \warning Asserts if the identifier is empty.
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
	 * \warning Asserts if the identifier is empty.
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
	 * \warning Asserts if the identifier is empty.
	 *
	 * \see `TextWrapper`.
	 */
	[[nodiscard]] TextWrapper* const getDynamicText(const std::string& identifier) const noexcept;

	/**
	 * \brief Returns a sprite Wrapper ptr, or nullptr if it does not exist.
	 * \complexity O(1).
	 *
	 * \param[in] identifier: The identifier of the sprite.
	 *
	 * \return The address of the sprite.
	 * 
	 * \warning Asserts if the identifier is empty.
	 *
	 * \see `SpriteWrapper`.
	 */
	[[nodiscard]] SpriteWrapper* const getDynamicSprite(const std::string& identifier) const noexcept;

protected:

	using UmapDynamicsIterator = std::unordered_map<std::string, size_t>::iterator;

	/**
	 * \brief Removes a dynamic element from the GUI. No effet if not there.
	 * \complexity O(1).
	 *
	 * \param[in] identifier: The identifier of the elment you want to remove.
	 * \param[out] vector: The vector that contains the dynamic element.
	 * \param[in,out] identifierMap: The map that enables accessing the container's elements using an
	 *				  identifier.
	 * \param[in,out] indexMap: The map that enables accessing the container's elements using indexes
	 *							in O(1) time complexity.
	 *	
	 * \see `removeDynamicSprite`, `removeDynamicText`.
	 */
	template<typename T> requires (std::same_as<T, TextWrapper> || std::same_as<T, SpriteWrapper>)
	void removeDynamicElement(const std::string& identifier, std::vector<T>& vector, std::unordered_map<std::string, size_t>& identifierMap, std::unordered_map<size_t, UmapDynamicsIterator>& indexMap) noexcept
	{
		auto iteratorToRemove{ identifierMap.find(identifier) };

		if (iteratorToRemove == identifierMap.end())
			return;

		const size_t targetIndex{ iteratorToRemove->second };
		const size_t lastIndex{ vector.size() - 1 }; 

		std::swap(vector[targetIndex], vector[lastIndex]);
		vector.pop_back();
		identifierMap.erase(iteratorToRemove);

		if (indexMap.find(lastIndex) == indexMap.end() || targetIndex == lastIndex)
		{	// If the last element was not dynamic, we don't need to update its maps.
			// Or if we swapped the element with itself (in the case it was already the last in the vector)
			indexMap.erase(targetIndex); // Just remove the index from the index map.
			return;
		}

		// It is more difficult if what we swapped is a dynamic element. In this case we need to update its
		// maps. In that matter, we avoid removing everything and readding everthing to reduce computional
		// time. We change the values of the map keys (that were immuable) to fit the swap.

		const std::string idLastElement{ indexMap.at(lastIndex)->first };
		identifierMap[idLastElement] = targetIndex;
		indexMap[targetIndex] = identifierMap.find(idLastElement);
		indexMap.erase(lastIndex); // lastIndex is out of range as the container lost 
	}

	
	std::unordered_map<std::string, size_t> m_dynamicTexts; // All indexes of dynamic texts in the interface.
	std::unordered_map<std::string, size_t> m_dynamicSprites; // All indexes of dynamic sprites in the interface.

	std::unordered_map<size_t, UmapDynamicsIterator> m_indexesForEachDynamicTexts; // Allows removal of dynamic texts in O(1).
	std::unordered_map<size_t, UmapDynamicsIterator> m_indexesForEachDynamicSprites; // Allows removal of dynamic sprites in O(1). 
};

} // gui namespace

#endif //MUTABLEINTERFACE_HPP