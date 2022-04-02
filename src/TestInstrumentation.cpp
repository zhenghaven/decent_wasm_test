#include <cstdio>

#include <iostream>
#include <string>

#include "Instrumentation.hpp"

template<typename _RetType>
static _RetType ReadFile2Buffer(const std::string& filename)
{
	FILE *file;
	size_t file_size, read_size;

	if ((file = fopen(filename.c_str(), "rb")) == nullptr)
	{
		throw std::runtime_error(
			"Read file to buffer failed: open file " + filename + " failed");
	}

	fseek(file, 0, SEEK_END);
	file_size = ftell(file);
	fseek(file, 0, SEEK_SET);

	_RetType buffer;
	buffer.resize(file_size);

	read_size = fread(&(buffer[0]), 1, file_size, file);
	fclose(file);

	if (read_size < file_size)
	{
		throw std::runtime_error(
			"Read file " + filename + " to buffer failed: read file content failed");
	}

	return buffer;
}

template<typename _InputType>
static void WriteBuffer2File(
	const std::string& filename,
	const _InputType& buffer)
{
	FILE *file;
	size_t writeSize = 0;

	if ((file = fopen(filename.c_str(), "wb")) == nullptr)
	{
		throw std::runtime_error(
			"write buffer to file failed: open file " + filename + " failed");
	}

	writeSize = fwrite(buffer.data(), 1, buffer.size(), file);
	fclose(file);

	if(writeSize < buffer.size())
	{
		throw std::runtime_error(
			"write buffer to file " + filename + " failed: write file content failed");
	}
}

int main(int argc, char**argv)
{
	if (argc < 2)
	{
		std::cerr << "ERROR: "
			<< "Please provide a path to the WAT file." << std::endl;
		return -1;
	}
	if (argc < 3)
	{
		std::cerr << "ERROR: "
			<< "Please provide a path to the output file." << std::endl;
		return -1;
	}

	std::string watFilename = argv[1];
	std::string outFilename = argv[2];

	std::cout << "INFO: "
		<< "Specified WAT file @ " << watFilename << std::endl;

	auto watBuf = ReadFile2Buffer<std::string>(watFilename);

	AccTEE::Instrumentation ist(watBuf, "\n");

	auto istWatBuf = ist.CombineIntoString("\n");

	WriteBuffer2File(outFilename, istWatBuf);
}
