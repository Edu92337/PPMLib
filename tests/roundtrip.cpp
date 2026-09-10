#include "ppm.hpp"

#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace {

using Bytes = std::vector<uint8_t>;

bool read_file(const std::string& path, Bytes& data) {
    std::ifstream file(path, std::ios::binary);
    if (!file) return false;

    file.seekg(0, std::ios::end);
    const std::streamoff size = file.tellg();
    if (size < 0) return false;
    file.seekg(0, std::ios::beg);

    data.resize(static_cast<std::size_t>(size));
    if (!data.empty()) {
        file.read(reinterpret_cast<char*>(data.data()), size);
        if (!file) return false;
    }
    return true;
}

template <typename T>
bool read_value(std::ifstream& file, T& value) {
    file.read(reinterpret_cast<char*>(&value), sizeof(value));
    return static_cast<bool>(file);
}

bool compress_file(const std::string& input_path,
                   const std::string& archive_path,
                   const Bytes& original) {
    Ppm encoder(5, true);
    for (uint8_t byte : original) {
        encoder.process_symbol(byte);
    }
    encoder.aritmetico.finalize_encoding();

    ArquivoInfo info{input_path, static_cast<uintmax_t>(original.size())};
    return encoder.aritmetico.save_archive(
        archive_path, {info}, original.size());
}

bool decompress_file(const std::string& archive_path,
                     const Bytes& original,
                     Bytes& restored) {
    std::ifstream archive(archive_path, std::ios::binary);
    if (!archive) return false;

    uint64_t file_count = 0;
    if (!read_value(archive, file_count) || file_count != 1) return false;

    uint16_t name_size = 0;
    if (!read_value(archive, name_size)) return false;
    archive.seekg(name_size, std::ios::cur);
    if (!archive) return false;

    uint64_t original_size = 0;
    uint32_t compressed_bits = 0;
    if (!read_value(archive, original_size) ||
        !read_value(archive, compressed_bits)) {
        return false;
    }
    if (original_size != original.size() || compressed_bits == 0) {
        return original.empty() && compressed_bits == 0;
    }

    Ppm decoder(5, true);
    decoder.aritmetico.prepare_decoding(archive);
    restored.reserve(static_cast<std::size_t>(original_size));
    for (uint64_t i = 0; i < original_size; ++i) {
        restored.push_back(decoder.decode_symbol(archive));
    }
    return true;
}

} // namespace

int main(int argc, char** argv) {
    std::string input_path;
    std::string archive_path;
    Bytes original;
    if (argc == 1) {
        input_path = "generated-roundtrip-input";
        archive_path = "generated-roundtrip.ppm";
        original.reserve(4096);
        for (int repeat = 0; repeat < 16; ++repeat) {
            for (int byte = 0; byte < 256; ++byte) {
                original.push_back(static_cast<uint8_t>(byte));
            }
        }
    } else if (argc == 3) {
        input_path = argv[1];
        archive_path = argv[2];
        if (!read_file(input_path, original)) {
            std::cerr << "failed to read input: " << input_path << '\n';
            return 1;
        }
    } else {
        std::cerr << "usage: ppm_roundtrip [input compressed_archive]\n";
        return 2;
    }

    if (!compress_file(input_path, archive_path, original)) {
        std::cerr << "failed to compress input\n";
        return 1;
    }

    Bytes restored;
    if (!decompress_file(archive_path, original, restored)) {
        std::cerr << "failed to decompress generated archive\n";
        return 1;
    }
    if (restored != original) {
        std::cerr << "FAILURE: decompression did not reproduce the input\n";
        return 1;
    }

    std::cout << "OK: compression and decompression reproduced "
              << original.size() << " bytes\n";
    return 0;
}
