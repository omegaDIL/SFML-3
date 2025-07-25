#include "InteractiveInterface.hpp"

namespace gui
{
void InteractiveInterface::removeDynamicText(const std::string& identifier) noexcept
{
	ENSURE_STRING_NOT_EMPTY(identifier);

	if (getDynamicText(identifier) == m_writingText)
		m_writingText = nullptr;

	if (getDynamicSprite(identifier) == nullptr && getButton(identifier) != nullptr)
		m_buttons.erase(identifier);

	MutableInterface::removeDynamicText(identifier);
}

void InteractiveInterface::removeDynamicSprite(const std::string& identifier) noexcept
{
	ENSURE_STRING_NOT_EMPTY(identifier);

	if (getDynamicText(identifier) == nullptr && getButton(identifier) != nullptr)
		m_buttons.erase(identifier);

	MutableInterface::removeDynamicSprite(identifier);
}

void InteractiveInterface::addButton(const std::string& identifier, InteractiveFunction function) noexcept
{
	ENSURE_STRING_NOT_EMPTY(identifier);

	m_buttons[identifier] = function; // Adds the button with the text id and the function.
}
	
InteractiveInterface::InteractiveFunction* InteractiveInterface::getButton(const std::string& identifier) const noexcept
{
	ENSURE_STRING_NOT_EMPTY(identifier);

	auto mapIterator{ m_buttons.find(identifier) };

	if (mapIterator == m_buttons.end())
		return nullptr;

	return &mapIterator->second;
}

void InteractiveInterface::setWritingText(const std::string& identifier, WritableFunction function) noexcept
{
	ENSURE_STRING_NOT_EMPTY(identifier);

	TextWrapper* newWritingText{ getDynamicText(identifier) };

	ENSURE_VALID_PTR(newWritingText);

	m_writingText = newWritingText;
	m_writingFunction = function;
}

} // gui namespace