/*******************************************************************
 * @file Utils.hpp
 * @brief Contains execptions and utility functions for various conversions and operations.
 *
 * @author OmegaDIL.
 * @date   March 2025.
 *********************************************************************/

#ifndef UTILS_HPP
#define UTILS_HPP

#include <SFML/Graphics.hpp>
#include <concepts>
#include <vector>
#include <string>


///////////////////////////////////////////////////////////////////////////////////////////////////
/// Conversion functions
///////////////////////////////////////////////////////////////////////////////////////////////////

 /**
  * @brief Must be a numerical type.
  * 
  * @tparam T: The numerical type.
  */
template <typename T>
concept NumericalTypes = std::is_same_v<T, int> || std::is_same_v<T, long> || std::is_same_v<T, long long> ||
						 std::is_same_v<T, unsigned int> || std::is_same_v<T, unsigned long> || std::is_same_v<T, size_t> ||
						 std::is_same_v<T, float> || std::is_same_v<T, double> || std::is_same_v<T, long double>;

/**
 * @brief Converts a vector of numeric values to a vector of strings.
 * @complexity O(N) where N is the number of elements in the vector.
 *
 * @tparam T: The type of numeric values in the vector.
 * @param[in] toConvert: The vector of numeric values to convert.
 *
 * @return A vector of strings representing the converted values.
 */
template <NumericalTypes T>
inline std::vector<std::string> convertToString(std::vector<T> const& toConvert)
{
	std::vector<std::string> converted{};
	for (auto const& val : toConvert)
		converted.push_back(std::to_string(val));

	return converted;
}

/**
 * @brief Converts a vector of strings to a vector of numeric values.
 * @complexity O(N) where N is the number of elements in the vector.
 *
 * @tparam T: The type of numeric values to convert to.
 * @param[in] toConvert: The vector of numeric values to convert.
 *
 * @return A vector of strings representing the converted values.
 */
template <NumericalTypes T>
inline std::vector<T> convertBackFromString(std::vector<std::string> const& toConvert)
{
	std::vector<T> converted{};

	for (auto const& sval : toConvert)
	{
		if constexpr (std::is_same_v<T, int>)
			converted.push_back(std::stoi(sval));
		else if constexpr (std::is_same_v<T, long>)
			converted.push_back(std::stol(sval));
		else if constexpr (std::is_same_v<T, long long>)
			converted.push_back(std::stoll(sval));
		else if constexpr (std::is_same_v<T, float>)
			converted.push_back(std::stof(sval));
		else if constexpr (std::is_same_v<T, double>)
			converted.push_back(std::stod(sval));
		else if constexpr (std::is_same_v<T, long double>)
			converted.push_back(std::stold(sval));
	}

	return converted;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
/// Conversion functions
///////////////////////////////////////////////////////////////////////////////////////////////////


/**
 * @brief Creates a new instance of a window to display an error message.
 * @complexity constant O(1).
 *
 * @param[in] errorMessage: The message to be displayed.
 * @param[in] errorTitle: The title of the window.
 *
 * @note This function is blocking and will terminate once the user closes the new window.
 * @note Don't forget to put the character \n to avoid the text to not be seen entirely when you
 *       have a long line.
 *
 * @see sf::RenderWindow.
 */
void showErrorsUsingWindow(std::string const& errorMessage, std::string const& errorTitle) noexcept;


/**
 * @brief Handles the changes due to a resize of the window with its GUIs.
 * 
 * @param[in,out] window: the window.
 */
void handleEventResize(sf::RenderWindow* window) noexcept;

#endif //UTILS_HPP