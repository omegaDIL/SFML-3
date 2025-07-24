#include "InteractiveInterface.hpp"

namespace gui
{
	
void InteractiveInterface::addButton(std::string const& identifier, InteractiveFunction function) noexcept
{
	ENSURE_STRING_NOT_EMPTY(identifier);

	m_buttons[identifier] = function; // Adds the button with the text id and the function.
}
	
InteractiveInterface::InteractiveFunction const* InteractiveInterface::getButton(std::string const& identifier) const noexcept
{
	ENSURE_STRING_NOT_EMPTY(identifier);

	auto mapIterator{ m_buttons.find(identifier) };

	if (mapIterator == m_buttons.end())
		return nullptr;

	return &mapIterator->second;
}

} // gui namespace