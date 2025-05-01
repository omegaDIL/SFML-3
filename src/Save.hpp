/*******************************************************************
 * @file Save.hpp
 * @brief Initializes the structures and functions for saving and loading anything safely from/into the files.
 *
 * @author OmegaDIL.
 * @date   July 2024.
 *********************************************************************/

#ifndef SAVE_HPP
#define SAVE_HPP

#include <fstream>
#include <filesystem>
#include <vector> 
#include <string>
#include <optional>
#include <memory>
#include <sstream>
#include <ios>


/**
 * @brief Classes, functions and exceptions to safely handle files.
 */
namespace SafeSaves
{
/**
 * @brief Checks if a file exists.
 * @complexity O(1)
 *
 * @param[in] path: Path to the file.
 *
 * @return True if the file exists, false otherwise.
 */
inline bool checkFileExistence(std::string const& path) noexcept
{
	return std::filesystem::exists(path);
}

/**
 * @brief Checks if a file is writable.
 * @complexity O(1)
 *
 * @param[in] path: Path to the file.
 *
 * @return True if the file is writable, false otherwise.
 */
inline bool checkFileWritable(std::string const& path) noexcept
{
	auto perms = std::filesystem::status(path).permissions();
	return (perms & std::filesystem::perms::owner_write) != std::filesystem::perms::none;
}


/**
 * @brief Use RAII to ensure proper handling errors while reading data.
 *
 * @note Call create() to open a file for reading.
 * 
 * @see std::ifstream, WritingStreamRAIIWrapper
 */
class ReadingStreamRAIIWrapper
{
public:

	ReadingStreamRAIIWrapper() noexcept = default;
	ReadingStreamRAIIWrapper(ReadingStreamRAIIWrapper const&) = delete;
	ReadingStreamRAIIWrapper(ReadingStreamRAIIWrapper&&) noexcept = default;
	ReadingStreamRAIIWrapper& operator=(ReadingStreamRAIIWrapper const&) = delete;
	ReadingStreamRAIIWrapper& operator=(ReadingStreamRAIIWrapper&&) noexcept = default;

	/**
	 * @brief Ensures the file is closed.
	 * @complexity O(1).
	 */
	inline ~ReadingStreamRAIIWrapper() noexcept
	{
		if (m_fileStream && m_fileStream->is_open())
			m_fileStream->close();
	}

	/**
	 * @brief Opens a file for reading and throws an error if the operation fails.
	 * @complexity O(1)
	 *
	 * @param[in] path: Path to the file.
	 * @param[in] mode: File open mode.
	 *
	 * @pre The file must exist.
	 * @post The file is opened for reading.
	 * @throw FileFailure if the file cannot be accessed.
	 *        Strong exceptions guarrantee: you can call this function again to try opening a file.
	 */
	ReadingStreamRAIIWrapper(std::string const& path, std::ios::openmode mode, bool create);

	/**
	 * @brief Opens a file for reading and throws an error if the operation fails.
	 * @complexity O(1)
	 *
	 * @param[in] path: Path to the file.
	 * @param[in] mode: File open mode.
	 *
	 * @pre The file must exist.
	 * @post The file is opened for reading.
	 * @throw FileFailure if the file cannot be accessed.
	 *        Strong exceptions guarrantee: you can call this function again to try opening a file.
	 */
	void create(std::string const& path, std::ios::openmode mode = std::ios::in);


	/**
	 * @complexity O(1).
	 *
	 * @return The file stream in a std::optional in case it has been badly instanciated.
	 */
	inline std::optional<std::ifstream*> stream() const noexcept
	{
		return (m_fileStream) ? std::make_optional<std::ifstream*>(m_fileStream.get()) : std::nullopt;
	}

private:

	/// File stream managed by the wrapper.
	std::unique_ptr<std::ifstream> m_fileStream;
};

/**
 * @brief Use RAII to ensure proper handling errors while saving data.
 * 
 * @note Call create() to open a file for reading.
 *
 * @see std::ofstream, ReadingStreamRAIIWrapper
 */
class WritingStreamRAIIWrapper
{
public:

	WritingStreamRAIIWrapper() noexcept = default;
	WritingStreamRAIIWrapper(WritingStreamRAIIWrapper const&) = delete;
	WritingStreamRAIIWrapper(WritingStreamRAIIWrapper&&) noexcept = default;
	WritingStreamRAIIWrapper& operator=(WritingStreamRAIIWrapper const&) = delete;
	WritingStreamRAIIWrapper& operator=(WritingStreamRAIIWrapper&&) noexcept = default;

	/**
	 * @brief Ensures that the file is closed.
	 * @complexity O(1).
	 */
	inline ~WritingStreamRAIIWrapper() noexcept
	{
		if (m_fileStream && m_fileStream->is_open())
			m_fileStream->close();
	}

	/**
	 * @brief Opens a file for writing and throws an error if the operation fails.
	 * @complexity O(1)
	 *
	 * @param[in] path: Path to the file.
	 * @param[in] mode: File open mode.
	 * @param[in] create: True if the file has to be created.
	 *
	 * @pre The file must exist and be writable.
	 * @post The file is opened for writing.
	 * @throw FileFailureWhileOpening if the file cannot be accessed.
	 *        Strong exceptions guarrantee: you can call this function again to try opening a file.
	 */
	WritingStreamRAIIWrapper(std::string const& path, std::ios::openmode mode = std::ios::out | std::ios::trunc, bool create = false);

	/**
	 * @brief Opens a file for writing and throws an error if the operation fails.
	 * @complexity O(1)
	 *
	 * @param[in] path: Path to the file.
	 * @param[in] mode: File open mode.
	 * @param[in] create: True if the file has to be created.
	 *
	 * @pre The file must exist and be writable.
	 * @post The file is opened for writing.
	 * @throw FileFailureWhileOpening if the file cannot be accessed. 
	 *        Strong exceptions guarrantee: you can call this function again to try opening a file.
	 */
	void create(std::string const& path, std::ios::openmode mode = std::ios::out | std::ios::trunc, bool create = false);


	/**
	 * @complexity O(1).
	 *
	 * @return The file stream in a std::optional in case it has been badly instanciated.
	 */
	inline std::optional<std::ofstream*> stream() noexcept
	{
		return (m_fileStream) ? std::make_optional<std::ofstream*>(m_fileStream.get()) : std::nullopt;
	}

private:

	std::unique_ptr<std::ofstream> m_fileStream; // File stream managed by the wrapper.
};


/**
 * @brief Provides static functions for saving and loading data.
 *
 * @note Don't expect this structure to be fast: it interacts with files.
 * @note This struct is non-instantiable and only contains static methods.
 * @note If any optional string is instantiated, the function called didn't satisfy its postconditions.
 *
 * @see SafeSaves, FileFailure
 */
struct Save
{
public:

	Save() noexcept = delete;
	Save(Save const&) noexcept = delete;
	Save(Save&&) noexcept = delete;
	Save& operator=(Save const&) noexcept = delete;
	Save& operator=(Save&&) noexcept = delete;
	virtual ~Save() noexcept = delete;


	/**
	 * @brief Reads data from a file
	 * @details Reads as many lines in the file as the number of string instantiated in the vector.
	 *          The first line is put in the first string, then the second line on the second string,
	 *			and so on.
	 * @complexity O(N) where N is the number of string instantatied within the vector passed in argument.
	 *
	 * @param[in] fileName: The name of the file.
	 * @param[out] valuesToLoad: The vector in which the data will be stored in.
	 * @param[in] decrypt: True if the values need to be decrypted.
	 *
	 * @return an optional string that contains an error message.
	 *
	 * @see writing().
	 */
	[[nodiscard]] static std::optional<std::string> reading(std::string const& fileName, std::vector<std::string>& valuesToLoad, bool decrypt = true) noexcept;

	/**
	 * @brief Writes data into a file
	 * @details Each string instantatied within the vector is stored in the file in its own line.
	 *          The first string is saved on the first line, then the second string on the second line,
	 *			and so on.
	 * @complexity O(N) where N is the number of string instantatied within the vector passed in argument.
	 *
	 * @param[in] fileName: The name of the file.
	 * @param[in] valuesToSave: The vector that contains the data to save.
	 * @param[in] encrypt: True if the values need to be encrypted.
	 *
	 * @return an optional string that contains an error message.
	 *
	 * @note Avoid putting the character \n in any string.
	 *
	 * @see writing(), createFile().
	 */
	[[nodiscard]] static std::optional<std::string> writing(std::string const& fileName, std::vector<std::string> const& valuesToSave, bool encrypt = true) noexcept;

	/**
	 * @brief Creates a valid file .txt to store information, or resets a file.
	 * @complexity O(1).
	 * 
	 * @param[in] fileName: The name of the file.
	 * 
	 * @return an optional string that contains an error message.
	 * 
	 * @note The file is cleard if it already exists 
	 * 
	 * @see std::filesystem for removal or getting the names of existent files.
	 */
	[[nodiscard]] static std::optional<std::string> createFile(std::string const& fileName) noexcept;

private:

	/**
	 * @brief Opens a safe stream to a file.
	 * 
	 * @param[in] path: The path to the file.
	 * @param[out] errorMessage: The error message if a critical error occured.
	 * 
	 * @return ReadingStreamRAIIWrapper that contains the stream.
	 *
	 * @note You need to check the value of the wrapper to see its validity.
	 * @note If a message was added in errorMessage then the stream is not valid.
	 * 
	 * @see cleanUpFiles(), reading().
	 */
	[[nodiscard]] static ReadingStreamRAIIWrapper openReadingStream(std::string const& path, std::ostringstream& errorMessage) noexcept;

	/**
	 * @brief Opens a safe stream to a file.
	 *
	 * @param[in] path: The path to the file.
	 * @param[out] errorMessage: The error message if a critical error occured.
	 *
	 * @return WritingStreamRAIIWrapper that contains the stream.
	 *
	 * @note You need to check the value of the wrapper to see its validity.
	 * @note If a message was added in errorMessage then the stream is not valid.
	 * 
	 * @see cleanUpFiles(), writing().
	 */
	[[nodiscard]] static WritingStreamRAIIWrapper openWritingStream(std::string const& path, std::ostringstream& errorMessage) noexcept;

	/**
	* @brief Cleans up files by removing temporary or corrupted files.
	* @complexity O(1).
	* 
	* @param[in] path: The path to the file to clean up.
	* 
	* @pre The file must be readable and valid (or its tmp).
	* @throw FileFailureWhileOpening if no valid (existing or non corrputed) files exist.
	* @throw std::exceptions if an important error occured and is unknown.
	* 		 Strong exceptions guarrantee.
	* 
	* @see openReadingStream(), openWritingStream(), checkingContentValdity()
	*/
	static void cleanUpFiles(std::string const& path);

	/**
	 * @brief Checks if a file has a valid content - isn't corrupted.
	 * @complexity O(1).
	 * 
	 * @param[in] loading: the stream to the file to check.
	 * 
	 * @note the cursor of loading will be changed
	 * 
	 * @return True if the file is valid.
	 * 
	 * @see CleanUpFiles().
	 */
	[[nodiscard]] static bool checkingContentValdity(std::ifstream* loading) noexcept;

	/**
	 * @brief Encrypt or decrypt the data using several involutive algorithms.
	 * @complexity O(N) where N is the number of value to encrypt.
	 * 
	 * @param[in] data: The data to encrypt.
	 * @param[in] key: The key to use.
	 * 
	 * @note The longer the key is, the safer the data will be. However a key longer than the data
	 * 		 is useless.
	 * 
	 * @return the data encrypted.
	 */
	[[nodiscard]] static std::string encryptDecrypt(std::string const& data, std::string const& key = "7gK9!wZp2FhJ8@qL") noexcept;

	
	static std::string const savesPath; // The relative path to the saves folder.
	static std::string const tokensOfConfirmation; // The tokens that confirm that the file has been correctly saved.
};
} // namespace SafeSaves

#endif //SAVE_HPP