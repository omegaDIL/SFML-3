#include <fstream>
#include <filesystem>
#include <vector> 
#include <string>
#include <optional>
#include <memory>
#include <sstream>
#include <ios>
#include "Save.hpp"
#include "Exceptions.hpp"

using namespace SafeSaves;

static std::string const savesPath{ "saves/" };
static std::string const tokensOfConfirmation{ "/%)'{]\"This file has been succesfully saved}\"#'[]?(" };


void ReadingStreamRAIIWrapper::create(std::string const& path, std::ios::openmode mode)
{
	if (!checkFileExistence(path))
		throw FileExceptionWhileOpening{ "File does not exist: " + path };

	m_fileStream = std::make_unique<std::ifstream>(path, mode);

	if (!m_fileStream->is_open()) [[unlikely]]
	{
		m_fileStream.reset(); // Resetting the pointer to nullptr.
		throw FileExceptionWhileOpening("Unable to open the file, unknow reasons: " + path);
	}
}

void WritingStreamRAIIWrapper::create(std::string const& path, std::ios::openmode mode, bool create)
{
	if (!create && !checkFileExistence(path))
		throw FileExceptionWhileOpening{ "File does not exist: " + path };
	if (!create &&!checkFileWritable(path)) [[unlikely]]
		throw FileExceptionWhileOpening{ "File exists but cannot be opened for writing: " + path };

	m_fileStream = std::make_unique<std::ofstream>(path, mode);

	if (!m_fileStream->is_open()) [[unlikely]]
	{
		m_fileStream.reset(); // Resetting the pointer to nullptr.
		throw FileExceptionWhileOpening("Unable to open the file, unknow reasons: " + path);  
	}
}


std::optional<std::string> Save::reading(std::string const& fileName, std::vector<std::string>& valuesToLoad, bool decrypt) noexcept
{
	std::string path{ savesPath + fileName };
	std::ostringstream errorMessage{};

	// Checks files, clean the folder, and open a stream.
	ReadingStreamRAIIWrapper fileWrapped{ openReadingStream(path, errorMessage) };
	if (!fileWrapped.stream().has_value()) [[unlikely]]
		return std::optional<std::string>{ errorMessage.str() };

	std::ifstream* savingStream{ fileWrapped.stream().value() };

	try
	{
		// Line by line in the file to store string by string in the vector.
		for (auto& toLoad : valuesToLoad)
		{	// Stop when the vector is full.
			std::string temp{};
			std::getline(*savingStream, temp);

			toLoad = ((decrypt) ? encryptDecrypt(temp) : temp);
			
			if (savingStream->fail()) [[unlikely]]
				throw FileExceptionWhileInUse{ "Error reading from the file: " + path};
		}
	}
	catch (FileExceptionWhileInUse const& error)
	{
		errorMessage << error.what() << '\n';
		errorMessage << "Critical error: the file is corrupted abd further saves are unavailable\n\n";
	}

	return (errorMessage.str().empty()) ? std::nullopt : std::optional<std::string>{ errorMessage.str() };
}

std::optional<std::string> Save::writing(std::string const& fileName, std::vector<std::string> const& valuesToSave, bool encrypt) noexcept
{
	std::string path{ savesPath + fileName };
	std::ostringstream errorMessage{}; 

	// Checks files, clean the folder, and open a stream.
	WritingStreamRAIIWrapper fileWrapped{ openWritingStream(path, errorMessage) };
	if (!fileWrapped.stream().has_value()) [[unlikely]]
		return std::optional<std::string>{ errorMessage.str() };

	std::ofstream* savingStream{ fileWrapped.stream().value() };

	try
	{
		for (auto const& toSave : valuesToSave)
		{
			*savingStream << ((encrypt) ? encryptDecrypt(toSave) : toSave) << '\n';

			if (savingStream->fail()) [[unlikely]]
				throw FileExceptionWhileInUse{ "Error writing into the file: " + path };
		}

		// Last tokens: confirm that every lines before has been successfully saved.
		*savingStream << tokensOfConfirmation; 
	}
	catch (FileExceptionWhileInUse const& error)
	{
		errorMessage << error.what() << '\n';
		errorMessage << "Critical error: the file is corrupted and further saves are lost\n\n";
	}

	savingStream->close(); // To clean up the files, no writing stream can be openend to these same files.
	cleanUpFiles(path); // Won't throw any execptions as it sure that the tmp file is valid.
	return (errorMessage.str().empty()) ? std::nullopt : std::optional<std::string>{ errorMessage.str() };
}

std::optional<std::string> SafeSaves::Save::createFile(std::string const& fileName) noexcept
{
	std::string path{ savesPath + fileName };
	std::ostringstream errorMessage{};

	try
	{
		WritingStreamRAIIWrapper fileWrapped{};
		fileWrapped.create(path, std::ios::out | std::ios::trunc, true);

		// We don't need to check the validity of the pointer using has_value()
		// The call to create() would have thrown an exception if badly instantiated
		std::ofstream* savingStream{ fileWrapped.stream().value() };
		*savingStream << tokensOfConfirmation;
		 
		if (savingStream->fail()) [[unlikely]]
			throw FileExceptionWhileInUse{ "Error writing confirmation tokens into the file: " + path };
	}
	catch (FileException const& error)
	{
		errorMessage << error.what() << '\n';
		errorMessage << "error: impossible to create file" << "\n\n";

		std::filesystem::remove(path);
	}

	return (errorMessage.str().empty()) ? std::nullopt : std::optional<std::string>{ errorMessage.str() };
}

ReadingStreamRAIIWrapper Save::openReadingStream(std::string const& path, std::ostringstream& errorMessage) noexcept
{
	ReadingStreamRAIIWrapper openStream{};

	try
	{
		cleanUpFiles(path);
		openStream.create(path);
	}
	catch (FileExceptionWhileOpening const& error)
	{
		errorMessage << error.what() << '\n';
		errorMessage << "Fatal error: impossible to read the values" << "\n\n";
	}
	catch (std::exception const& error)
	{
		errorMessage << error.what() << '\n';
		errorMessage << "Error: gravity and effects unknown" << "\n\n";
	}

	return openStream;
}

WritingStreamRAIIWrapper Save::openWritingStream(std::string const& path, std::ostringstream& errorMessage) noexcept
{
	WritingStreamRAIIWrapper openStream{};

	try
	{
		cleanUpFiles(path); 
		std::filesystem::copy_file(path, path + ".tmp"); // We create a copy to have a valid file at any given moment.
		openStream.create(path);
	}
	catch (FileExceptionWhileOpening const& error)
	{
		errorMessage << error.what() << '\n';
		errorMessage << "Fatal error: impossible to read the values" << "\n\n";
	}
	catch (std::exception const& error)
	{
		errorMessage << error.what() << '\n';
		errorMessage << "Error: gravity and effects unknown" << "\n\n";
	}

	return openStream;
}

void Save::cleanUpFiles(std::string const& path)
{
	ReadingStreamRAIIWrapper fileWrapped{};

	try
	{	// If at any point the perm file is found not valid, it falls into the catch block and the tmp is next to be loaded.
		fileWrapped.create(path);
		
		if (!checkingContentValdity(fileWrapped.stream().value()))
 			throw FileExceptionWhileOpening{ "Perm file is not valid" };

		// No need to check for the tmp file as the perm file IS valid (would have fell otherwise).
		std::filesystem::remove(path + ".tmp"); // Delete the tmp file in case it is still there.
	}	
	catch (std::exception const&)
	{	// If the program falls in this catch, the permanent file is not valid.
		if (!checkFileExistence(path + ".tmp") || !checkFileWritable(path + "tmp"))
			throw FileExceptionWhileOpening{ "No file avalailable to load the saves: " + path };

		// To remove the perm file, no stream can be openend to that same file.
		if (fileWrapped.stream().has_value())
			fileWrapped.stream().value()->close(); 

		std::filesystem::remove(path); // Delete the perm file as it is not avalaible.
		std::filesystem::copy_file(path + ".tmp", path); // Turning the tmp file into the perm file.
		std::filesystem::remove(path + ".tmp"); // In two steps: so there's always at least one file storing the information.
		
		fileWrapped.create(path); // The tmp file has become the perm file.

		// We don't need to check the validity of the pointer using has_value()
		// The call to create() would have thrown an exception if badly instantiated
		if (!checkingContentValdity(fileWrapped.stream().value())) 
			throw FileExceptionWhileOpening{ "No valid file avalailable to load the saves: " + path };
	}
}

bool Save::checkingContentValdity(std::ifstream* reading) noexcept
{
	// At the beginning of the tokens's string.
	reading->seekg(-51, std::ios::end);

	std::string fileConfirmation{ (std::istreambuf_iterator<char>(*reading)), std::istreambuf_iterator<char>() }; 

	return ((reading->fail()) ? false : fileConfirmation == tokensOfConfirmation);
}

std::string Save::encryptDecrypt(std::string const& data, std::string const& key) noexcept
{
	std::string output{};
	output.reserve(data.size());

	auto lambdaXorCipher = [](char datum, char key) -> char { return datum ^ key; };
	auto lambdaComplement = [](uint8_t datum, uint8_t key) -> uint8_t { return 255 - (datum + key); };
	auto lambdaReverseBits = [](uint8_t datum) -> uint8_t
	{
		datum = ((datum & 0xF0) >> 4) | ((datum & 0x0F) << 4);
		datum = ((datum & 0xCC) >> 2) | ((datum & 0x33) << 2);
		datum = ((datum & 0xAA) >> 1) | ((datum & 0x55) << 1);
		return datum;
	};
	
	for (int i = 0; i < data.size(); i++)
	{	// Each letter of data will be encrypted with one letter of the key.
		char letter{ data[i] };
		uint8_t const current_key = key[i % key.size()]; // Ensure the key index is within bounds

		// Applying three different involutive algorithms in a specific order to ensure decryption works correctly.
		// so f(h(g(h(f(x))))) == y; f(h(g(h(f(y))))) == x.
		letter = lambdaXorCipher(letter, current_key);
		letter = lambdaComplement(letter, current_key);
		letter = lambdaReverseBits(letter);
		letter = lambdaComplement(letter, current_key);
		letter = lambdaXorCipher(letter, current_key);

		output.push_back(letter);
	}

	return output;
}