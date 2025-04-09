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


 /**  
  * @brief Classes, functions and exceptions to safely handle files.
  */
namespace SafeSaves
{
	/**
	 * @brief Exception thrown for file-related errors.
	 *
	 * @see std::ios_base::failure.
	 */
	struct FileFailure : public std::ios_base::failure
	{
	public:

		/**
		 * @brief Initializes the exception with a message.
		 * @complexity O(1)
		 *
		 * @param[in] message: The error message.
		 */
		inline explicit FileFailure(const std::string& message) : std::ios_base::failure{ message }
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

	/**
	 * @brief Exception thrown for file-related errors when opening files.
	 *
	 * @see FileFailure, FileFailureWhileInUse.
	 */
	struct FileFailureWhileOpening final : public FileFailure
	{
	public:

		/**
		 * @brief Initializes the exception with a message.
		 * @complexity O(1)
		 *
		 * @param[in] message: The error message.
		 */
		inline explicit FileFailureWhileOpening(const std::string& message) : FileFailure{ message }
		{}


		/**
		 * @complexity O(1).
		 *
		 * @return the error message.
		 */
		inline virtual const char* what() const noexcept final
		{
			return FileFailure::what();
		}
	};

	/**
	  * @brief Exception thrown for file-related errors when using files (reading/writing).
	  *
	  * @see FileFailure, FileFailureWhileOpening.
	  */
	struct FileFailureWhileInUse final : public FileFailure
	{
	public:

		/**
		 * @brief Initializes the exception with a message.
		 * @complexity O(1)
		 *
		 * @param[in] message: The error message.
		 */
		inline explicit FileFailureWhileInUse(const std::string& message) : FileFailure{ message }
		{}


		/**
		 * @complexity O(1).
		 *
		 * @return the error message.
		 */
		inline virtual const char* what() const noexcept final
		{
			return FileFailure::what();
		}
	};
} // namespace SafeSaves


/** 
 * @brief Exception thrown for graphical errors when loading ressources.
 *
 * @see std::runtime_error.
 */
struct LoadingGUIRessourceFailure : public std::runtime_error
{
public:

	/**
	 * @brief Initializes the exception with a message.
	 * @complexity O(1)
	 *
	 * @param[in] message: The error message.
	 */
	inline explicit LoadingGUIRessourceFailure(const std::string& message) : std::runtime_error{ message }
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