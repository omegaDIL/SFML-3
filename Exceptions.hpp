/*******************************************************************
 * @file Exceptions.hpp
 * @brief Contains execptions for specific errors.
 *
 * @author OmegaDIL.
 * @date   March 2025.
 * 
 * @note This file does not have a .cpp file.
 *********************************************************************/

#ifndef EXCEPTIONS_HPP
#define EXCEPTIONS_HPP

#include <stdexcept>
#include <ios>
#include <string>


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
		{
		}


		/**
		 * @complexity O(1).
		 *
		 * @return the error message.
		 */
		inline virtual const char* what() const noexcept final
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
		{
		}


		/**
		 * @complexity O(1).
		 *
		 * @return the error message.
		 */
		inline virtual const char* what() const noexcept final
		{
			return FileException::what();
		}
	};
} // namespace SafeSaves


/** @struct LoadingUIRessourceException
 * @brief Exception thrown for file-related errors.
 *
 * @see std::runtime_error.
 */
struct LoadingUIRessourceException : public std::runtime_error
{
public:

	/**
	 * @brief Initializes the exception with a message.
	 * @complexity O(1)
	 *
	 * @param[in] message: The error message.
	 */
	inline explicit LoadingUIRessourceException(const std::string& message) : std::runtime_error{ message }
	{}


	/**
	 * @complexity O(1).
	 *
	 * @return the error message.
	 */
	inline virtual const char* what() const noexcept override
	{
		return std::runtime_error::what();
	}
};

#endif //EXCEPTIONS_HPP