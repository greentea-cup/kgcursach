#include <fstream>

#include "util.hpp"

// https://stackoverflow.com/a/524843/20935957
std::string read_file(char const *path) {
	std::ifstream ifs(path, std::ios::in | std::ios::binary | std::ios::ate);
	std::ifstream::pos_type fileSize = ifs.tellg();
	ifs.seekg(0, std::ios::beg);
	// std::vector<char> bytes(fileSize);
	char *bytes = new char[fileSize];
	ifs.read(bytes, fileSize);
	std::string res(bytes, fileSize);
	delete[] bytes;
	return res;
}

