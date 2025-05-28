#include <iostream>
#include <fstream>
#include <cstddef>
#include <vector>
#include "HuffmanEncoder.h"

std::tuple<bool, std::string, std::string> parse_cmd_line(int argc, char* argv[]) {
	if (argc != 4) {
		throw std::runtime_error("incorrect number of cmdline arguments");
	}
	
	std::string encode_mode = std::string(argv[1]);
	if (encode_mode != "-e" && encode_mode != "-d") {
		throw std::runtime_error("compression mode is not present");
	}

	bool is_encode_mode = encode_mode == "-e";
	return std::make_tuple(is_encode_mode, std::string(argv[2]), std::string(argv[3]));
}

int main(int argc, char* argv[])
{
	auto [encode_mode, input_file_path, output_file_path] = parse_cmd_line(argc, argv);
	HuffmanEncoder huffmanEncoder{ input_file_path, output_file_path };

	if (encode_mode) {
		huffmanEncoder.encode();
	}
	else {
		huffmanEncoder.decode();
	}
}






