#pragma once
#include <iostream>
#include <fstream>
#include <cstddef>
#include <unordered_map>
#include <array>
#include <optional>
#include <memory>

struct Node {
    std::optional<uint64_t> file_byte;
    uint64_t byte_freq{ 0 };
    std::unique_ptr<Node> left_child{ nullptr };
    std::unique_ptr<Node> right_child{ nullptr };


    Node(std::unique_ptr<Node> left, std::unique_ptr<Node> right)
        : byte_freq{ left->byte_freq + right->byte_freq }, left_child{ std::move(left) }, right_child{ std::move(right) } {
    }

    Node(uint64_t file_byte, uint64_t freq)
        : file_byte{ file_byte }, byte_freq{ freq } {
    }

    bool operator<(const Node& otherNode) {
        return this->byte_freq < otherNode.byte_freq;
    }
};

class HuffmanEncoder
{
public:
    HuffmanEncoder(std::string input_filepath, std::string output_filepath);
    void encode();
    void decode();

private:
    std::string input_filepath;
    std::string output_filepath;
    std::vector<std::byte> readFile();
    void buildHuffmanMapping(std::unique_ptr<Node>&, std::string,
        std::unordered_map<uint64_t, std::string>&);
    std::unordered_map<uint64_t, std::string> buildHuffmanMap(const std::array<uint64_t, 256>&);
    void handle_unique_bytes(std::array<uint64_t, 256>&);
    std::unique_ptr<Node> getHuffmanTree(const std::array<uint64_t, 256>&);
    std::string get_encoded_binary_string(const std::unordered_map<uint64_t, std::string>&, const std::vector<std::byte>&);
    std::vector<std::byte> get_encoded_bytes(const std::string&);
    std::vector<std::byte> get_encoded_bytes(const std::array<uint64_t, 256>&);
    std::vector<std::byte> get_encoded_bytes(const uint64_t);
    std::vector<std::byte> parse_encoded_file(const std::vector<std::byte>&, std::unique_ptr<Node>, uint64_t);
    std::array<uint64_t, 256> parse_frequency_array(const std::vector<std::byte>&);
    uint64_t parse_encoded_bit_length(const std::vector<std::byte>&);
    uint64_t parse_bytes_to_int(const std::byte&, const std::byte&, const std::byte&, const std::byte&);
    void save_encoded_file(const std::vector<std::byte>&, const std::vector<std::byte>&, const std::vector<std::byte>&);
    void save_decoded_file(const std::vector<std::byte>&);
};

