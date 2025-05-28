#include "HuffmanEncoder.h"
#include <cstddef>
#include <fstream>
#include <iostream>
#include <memory>
#include <queue>
#include <unordered_map>
#include <utility>
#include <vector>
#include <bitset>

struct NodeComparator {
	bool operator()(const std::unique_ptr<Node>& a, const std::unique_ptr<Node>& b) const {
		return a->byte_freq > b->byte_freq;
	}
};


HuffmanEncoder::HuffmanEncoder(std::string input_filepath,
	std::string output_filepath)
	: input_filepath{ input_filepath }, output_filepath{ output_filepath } {
};

void HuffmanEncoder::encode() {
	std::vector<std::byte> file_bytes = readFile();
	std::array<uint64_t, 256> frequencyArray{};

	for (std::byte& b : file_bytes) {
		uint64_t index = static_cast<uint64_t>(b);
		frequencyArray[index]++;
	}

	handle_unique_bytes(frequencyArray);

	std::unordered_map<uint64_t, std::string> huffman_map =
		buildHuffmanMap(frequencyArray);
	std::string encoded_string =
		get_encoded_binary_string(huffman_map, file_bytes);
	int encoded_bits_length = encoded_string.size();

	std::vector<std::byte> encoded_bytes = get_encoded_bytes(encoded_string);
	std::vector<std::byte> encoded_frequency_array =
		get_encoded_bytes(frequencyArray);

	save_encoded_file(encoded_frequency_array,
		get_encoded_bytes(encoded_bits_length), encoded_bytes);
}

void HuffmanEncoder::decode() {
	std::vector<std::byte> encoded_bytes = readFile();

	std::array<uint64_t, 256> frequency_array = parse_frequency_array(encoded_bytes);
	uint64_t encoded_bits_length = parse_encoded_bit_length(encoded_bytes);
	std::vector<std::byte> decoded_bytes = parse_encoded_file(encoded_bytes, std::move(getHuffmanTree(frequency_array)), encoded_bits_length);

	save_decoded_file(decoded_bytes);
}

void HuffmanEncoder::handle_unique_bytes(std::array<uint64_t, 256>& frequencyArray) {
	int distinctByteIndex = -1;
	int uniqueBytes = 0;

	for (int i = 0; i < 256; i++) {
		if (frequencyArray[i] > 0) {
			distinctByteIndex = i;
			uniqueBytes++;
		}
	}

	if (uniqueBytes > 1) {
		return;
	}

	frequencyArray[(distinctByteIndex + 1) % frequencyArray.size()]++;
}

std::array<uint64_t, 256>
HuffmanEncoder::parse_frequency_array(const std::vector<std::byte>& encoded_bytes) {
	std::array<uint64_t, 256> decoded_frequency_array;
	uint64_t encoded_frequency_array_ending_index = 256 * 4;

	for (int i = 0; i < encoded_frequency_array_ending_index; i += 4) {
		decoded_frequency_array[i / 4] =
			parse_bytes_to_int(encoded_bytes[i], encoded_bytes[i + 1],
				encoded_bytes[i + 2], encoded_bytes[i + 3]);
	}

	return decoded_frequency_array;
}

uint64_t HuffmanEncoder::parse_encoded_bit_length(
	const std::vector<std::byte>& encoded_bytes) {
	uint64_t encoded_bit_index = 4 * 256;
	uint64_t decoded_bit_length = parse_bytes_to_int(
		encoded_bytes[encoded_bit_index], encoded_bytes[encoded_bit_index + 1],
		encoded_bytes[encoded_bit_index + 2],
		encoded_bytes[encoded_bit_index + 3]);

	return decoded_bit_length;
}

uint64_t HuffmanEncoder::parse_bytes_to_int(const std::byte& first_byte,
	const std::byte& second_byte,
	const std::byte& third_byte,
	const std::byte& fourth_byte) {

	uint64_t value = (static_cast<uint64_t>(first_byte) << 24) |
		(static_cast<uint64_t>(second_byte) << 16) |
		(static_cast<uint64_t>(third_byte) << 8) |
		(static_cast<uint64_t>(fourth_byte));


	return value;
}

std::vector<std::byte> HuffmanEncoder::parse_encoded_file(
	const std::vector<std::byte>& encoded_bytes,
	std::unique_ptr<Node> root_node,
	uint64_t encoded_bits_length) {
	std::vector<std::byte> decoded_file_contents;

	const int encoded_file_starting_index = (4 * 256) + 4;
	uint64_t current_encoded_bits_length = 0;
	Node* root_pointer = root_node.get();

	for (int i = encoded_file_starting_index; i < encoded_bytes.size() && current_encoded_bits_length < encoded_bits_length; i++) {
		uint64_t encoded_val = std::to_integer<uint64_t>(encoded_bytes[i]);
		std::string encoded_string = std::bitset<8>(encoded_val).to_string();

		for (int j = 0; j < encoded_string.size() && current_encoded_bits_length < encoded_bits_length; j++) {
			char encoded_bit_val = encoded_string[j];
			current_encoded_bits_length++;

			if (encoded_bit_val == '0') {
				root_pointer = root_pointer->left_child.get();
			}
			else {
				root_pointer = root_pointer->right_child.get();
			}

			if (root_pointer->file_byte.has_value()) {
				decoded_file_contents.push_back(static_cast<std::byte>(root_pointer->file_byte.value()));
				root_pointer = root_node.get();
			}
		}
	}

	return decoded_file_contents;
}

std::vector<std::byte>
HuffmanEncoder::get_encoded_bytes(const std::string& encoded_string) {
	std::vector<std::byte> encoded_bytes;

	int current_bit_count = 0;
	int current_byte_value = 0;

	for (char bit : encoded_string) {
		current_byte_value <<= 1;
		current_bit_count++;

		if (bit == '1') {
			current_byte_value |= 1;
		}

		if (current_bit_count == 8) {
			encoded_bytes.push_back(static_cast<std::byte>(current_byte_value));
			current_byte_value = 0;
			current_bit_count = 0;
		}
	}

	if (current_bit_count > 0) {
		current_byte_value <<= (8 - current_bit_count);
		encoded_bytes.push_back(static_cast<std::byte>(current_byte_value));
	}

	return encoded_bytes;
}

std::vector<std::byte>
HuffmanEncoder::get_encoded_bytes(const std::array<uint64_t, 256>& frequency_array) {
	std::vector<std::byte> encoded_bytes;

	for (uint64_t freq_int : frequency_array) {
		uint64_t freq = static_cast<uint64_t>(freq_int);
		std::vector<std::byte> frequency_bytes = get_encoded_bytes(freq);
		encoded_bytes.insert(encoded_bytes.end(), frequency_bytes.begin(),
			frequency_bytes.end());
	}

	return encoded_bytes;
}

std::vector<std::byte> HuffmanEncoder::get_encoded_bytes(const uint64_t value) {
	return {
		(static_cast<std::byte>((value >> 24) & 0xFF)),
		(static_cast<std::byte>((value >> 16) & 0xFF)),
		(static_cast<std::byte>((value >> 8) & 0xFF)),
		(static_cast<std::byte>(value & 0xFF)),
	};
}

void HuffmanEncoder::save_encoded_file(
	const std::vector<std::byte>& encoded_frequency_array,
	const std::vector<std::byte>& encoded_bit_length,
	const std::vector<std::byte>& encoded_bytes) {
	std::ofstream outfile(output_filepath, std::ios::binary | std::ios::trunc);

	if (!outfile.is_open()) {
		std::cerr << "failed to open file, filepath: " << output_filepath << '\n';
		throw std::runtime_error("failed to open filepath");
	}

	outfile.write(reinterpret_cast<const char*>(encoded_frequency_array.data()),
		encoded_frequency_array.size());

	outfile.write(reinterpret_cast<const char*>(encoded_bit_length.data()),
		encoded_bit_length.size());

	outfile.write(reinterpret_cast<const char*>(encoded_bytes.data()),
		encoded_bytes.size());

	outfile.close();
}

void HuffmanEncoder::save_decoded_file(const std::vector<std::byte>& decoded_bytes) {
	std::ofstream outfile(output_filepath, std::ios::binary | std::ios::trunc);

	if (!outfile.is_open()) {
		std::cerr << "failed to open file, filepath: " << output_filepath << '\n';
		throw std::runtime_error("failed to open filepath");
	}

	for (std::byte b : decoded_bytes) {
		outfile << static_cast<char>(b);
	}
}

std::string HuffmanEncoder::get_encoded_binary_string(
	const std::unordered_map<uint64_t, std::string>& huffmanMap,
	const std::vector<std::byte>& file_bytes) {
	std::string encoded_string;
	for (std::byte file_byte : file_bytes) {
		uint64_t index = std::to_integer<uint64_t>(file_byte);
		encoded_string += huffmanMap.at(index);
	}

	return encoded_string;
}

std::unique_ptr<Node>
HuffmanEncoder::getHuffmanTree(const std::array<uint64_t, 256>& frequencyArray) {
	std::priority_queue<std::unique_ptr<Node>, std::vector<std::unique_ptr<Node>>, NodeComparator> byte_queue;

	for (int i = 0; i < 256; i++) {
		if (frequencyArray[i] == 0) {
			continue;
		}

		byte_queue.push(std::make_unique<Node>(i, frequencyArray[i]));
	}

	while (byte_queue.size() > 1) {
		auto firstNode =
			std::move(const_cast<std::unique_ptr<Node> &>(byte_queue.top()));
		byte_queue.pop();

		auto secondNode =
			std::move(const_cast<std::unique_ptr<Node> &>(byte_queue.top()));
		byte_queue.pop();

		byte_queue.push(
			std::make_unique<Node>(std::move(firstNode), std::move(secondNode)));
	}

	return std::move(const_cast<std::unique_ptr<Node> &>(byte_queue.top()));
}

std::unordered_map<uint64_t, std::string>
HuffmanEncoder::buildHuffmanMap(const std::array<uint64_t, 256>& frequencyArray) {
	std::unique_ptr<Node> rootNode = getHuffmanTree(frequencyArray);
	std::unordered_map<uint64_t, std::string> huffmanMapping;
	buildHuffmanMapping(rootNode, "", huffmanMapping);
	return huffmanMapping;
}

void HuffmanEncoder::buildHuffmanMapping(
	std::unique_ptr<Node>& rootNode, std::string encoded_string,
	std::unordered_map<uint64_t, std::string>& huffmanMapping) {
	if (rootNode->file_byte.has_value()) {
		huffmanMapping[rootNode->file_byte.value()] = encoded_string;
		return;
	}
	buildHuffmanMapping(rootNode->left_child, encoded_string + "0",
		huffmanMapping);
	buildHuffmanMapping(rootNode->right_child, encoded_string + "1",
		huffmanMapping);
}

std::vector<std::byte> HuffmanEncoder::readFile() {
	std::ifstream file(input_filepath, std::ios::binary | std::ios::ate);

	if (!file) {
		throw std::runtime_error("failed to open file");
	}

	std::streamsize size = file.tellg();
	file.seekg(0, std::ios::beg);

	std::vector<std::byte> buffer(size);
	if (!file.read(reinterpret_cast<char*>(buffer.data()), size)) {
		throw std::runtime_error("failed to read file");
	}

	return buffer;
}