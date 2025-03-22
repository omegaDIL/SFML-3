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
#include <string>
#include <vector>
#include <stdexcept>
#include <ios>
#include <concepts>


/** @namespace SafeSaves
 * @brief Classes, functions and exceptions to safely handle files.
 */
namespace SafeSaves
{
/** @struct FileException
 * @brief Exception thrown for file-related errors.
 *
 * @see std::ios_base::failure.
 */
struct FileException : public std::ios_base::failure
{
public:

	/**
	 * @brief Initializes the exception with a message.
	 * @complexity O(1)
	 *
	 * @param[in] message: The error message.
	 */
	inline explicit FileException(const std::string& message) : std::ios_base::failure{ message }
	{}


	/**
	 * @complexity O(1).
	 *
	 * @return the error message.
	 */
	inline virtual const char* what() const noexcept override
	{
		return std::ios_base::failure::what();
	}
};

/** @struct FileExceptionWhileOpening
 * @brief Exception thrown for file-related errors when opening files.
 *
 * @see FileException, FileExceptionWhileInUse.
 */
struct FileExceptionWhileOpening final : public FileException
{
public:

	/**
	 * @brief Initializes the exception with a message.
	 * @complexity O(1)
	 *
	 * @param[in] message: The error message.
	 */
	inline explicit FileExceptionWhileOpening(const std::string& message) : FileException{ message }
	{}


	/**
	 * @complexity O(1).
	 *
	 * @return the error message.
	 */
	inline virtual const char* what() const noexcept override
	{
		return FileException::what();
	}
};

/** @struct FileExceptionWhileInUse
  * @brief Exception thrown for file-related errors when using files (reading/writing).
  *
  * @see FileException, FileExceptionWhileOpening.
  */
struct FileExceptionWhileInUse final : public FileException
{
public:

	/**
	 * @brief Initializes the exception with a message.
	 * @complexity O(1)
	 *
	 * @param[in] message: The error message.
	 */
	inline explicit FileExceptionWhileInUse(const std::string& message) : FileException{ message }
	{}


	/**
	 * @complexity O(1).
	 *
	 * @return the error message.
	 */
	inline virtual const char* what() const noexcept override
	{
		return FileException::what();
	}
};
} // namespace SafeSaves


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


/**
 * @brief Analyzes a string to determine the size needed of the text when displayed.
 * 
 * @param[in] str: The string to analyze.
 * @param[in] characterSize: The size of the characters in the string.
 * 
 * @return A Vector2f containing the width and height of the string when displayed.
 */
sf::Vector2u getStringSizeForDisplay(std::string const& str, unsigned int characterSize = 12) noexcept;


/**
 * @brief Creates a new instance of a window to display an error message.
 * @complexity constant O(1).
 *
 * @param[in] errorMessage: The message to be displayed.
 * @param[in] nature: The nature of the error (e.g. saving error, graphical error, etc.).
 *
 * @note This function is blocking and will terminate once the user closes the new window.
 * @note The class uses the `windowSize` variable to adjust the proportions of its elements.
 * @note The class uses the `nameOfSoftware` variable to name the window.
 *
 * @see sf::RenderWindow.
 */
void showErrorsUsingGUI(std::string const& errorMessage, std::string const& error) noexcept;

#endif //UTILS_HPP