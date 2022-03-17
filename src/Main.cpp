#include <cstdio>

#include <iostream>
#include <vector>
#include <stdexcept>
#include <string>

#include <sgx_urts.h>
#include <sgx_edger8r.h>

extern "C" {

void ocall_print(const char *str)
{
	printf("%s", str);
}

extern sgx_status_t ecall_iwasm_main(
	sgx_enclave_id_t eid, uint8_t* wasm_file_buf, size_t wasm_file_size);

} // extern "C"

static std::vector<uint8_t> ReadFile2Buffer(const std::string& filename)
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

	std::vector<uint8_t> buffer(file_size);

	read_size = fread(buffer.data(), 1, file_size, file);
	fclose(file);

	if (read_size < file_size)
	{
		throw std::runtime_error(
			"Read file " + filename + " to buffer failed: read file content failed");
	}

	return buffer;
}

static void WriteBuffer2File(
	const std::string& filename, const std::vector<uint8_t>& buffer)
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

static void enclave_init(sgx_enclave_id_t *p_eid)
{
	sgx_launch_token_t token = { 0 };
	sgx_status_t ret = SGX_ERROR_UNEXPECTED;
	int updated = 0;

	std::vector<uint8_t> tokenBuf;
	try
	{
		tokenBuf = ReadFile2Buffer(INTEL_SGX_ECTOKEN);
	}
	catch(const std::runtime_error&)
	{}

	ret = sgx_create_enclave(
		INTEL_SGX_TRUSTED_LIB, SGX_DEBUG_FLAG, &token, &updated,
		p_eid, nullptr);

	if (ret != SGX_SUCCESS) {
		throw std::runtime_error("Failed to create enclave");
	}

	if (updated == 1)
	{
		tokenBuf.resize(std::distance(std::begin(token), std::end(token)));
		std::copy(std::begin(token), std::end(token), tokenBuf.begin());
		WriteBuffer2File(INTEL_SGX_ECTOKEN, tokenBuf);
	}
}

int main(int argc, char**argv)
{
	if (argc < 2)
	{
		std::cerr << "ERROR: "
			<< "Please provide a path to WASM file." << std::endl;
		return -1;
	}

	std::string wasmFilename = argv[1];

	std::cout << "INFO: "
		<< "Specified WASM file @ " << wasmFilename << std::endl;

	auto wasmBuf = ReadFile2Buffer(wasmFilename);

	// init enclave
	sgx_enclave_id_t eid = 0;
	enclave_init(&eid);

	// iwasm main
	auto ret = ecall_iwasm_main(eid, wasmBuf.data(), wasmBuf.size());
	if(ret != SGX_SUCCESS)
	{
		std::cerr << "ERROR: "
			<< "Failed to run iwasm main." << std::endl;
	}

	// destroy enclave
	sgx_destroy_enclave(eid);

	return 0;
}
