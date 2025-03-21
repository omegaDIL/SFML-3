/*******************************************************************
 * @file errorHandling.hpp
 * @brief Contains classes/functions for exceptions/errors handling.
 *
 * @author OmegaDIL.
 * @date   February 2025.
 *********************************************************************/

#ifndef ERROR_HANDLING_HPP
#define ERROR_HANDLING_HPP

#include <stdexcept>
#include <ios>
#include <string>


///////////////////////////////////////////////////////////////////////////////
/// Exceptions
///////////////////////////////////////////////////////////////////////////////

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

///////////////////////////////////////////////////////////////////////////////
/// Exceptions
///////////////////////////////////////////////////////////////////////////////

#endif //ERROR_HANDLING_HPP