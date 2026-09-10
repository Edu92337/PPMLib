#include "ppm_io.hpp"
#include "ppm.hpp"

#include <fstream>
#include <iostream>
#include <iterator>
#include <vector>

namespace ppm_io {

bool compress_file(const std::string& input_path,
                    const std::string& archive_path,
                    int kmax) {
    std::ifstream input(input_path, std::ios::binary);
    if (!input) {
        std::cerr << "[ERROR] Could not open input file: " << input_path << std::endl;
        return false;
    }

    std::vector<uint8_t> original(
        (std::istreambuf_iterator<char>(input)),
        std::istreambuf_iterator<char>());
    input.close();

    Ppm encoder(kmax, /*treinando=*/true);
    for (uint8_t byte : original) {
        encoder.process_symbol(byte);
    }
    encoder.aritmetico.finalize_encoding();

    ArquivoInfo info{input_path, static_cast<uintmax_t>(original.size())};
    return encoder.aritmetico.save_archive(archive_path, {info}, original.size());
}

bool decompress_file(const std::string& archive_path,
                      const std::string& output_path,
                      int kmax) {
    std::ifstream archive(archive_path, std::ios::binary);
    if (!archive) {
        std::cerr << "[ERROR] Could not open archive file: " << archive_path << std::endl;
        return false;
    }

    uint64_t file_count = 0;
    archive.read(reinterpret_cast<char*>(&file_count), sizeof(file_count));
    if (!archive || file_count != 1) {
        std::cerr << "[ERROR] Unsupported or corrupted archive" << std::endl;
        return false;
    }

    uint16_t name_size = 0;
    archive.read(reinterpret_cast<char*>(&name_size), sizeof(name_size));
    archive.seekg(name_size, std::ios::cur); // pula o nome do arquivo original

    uint64_t original_size = 0;
    uint32_t compressed_bits = 0;
    archive.read(reinterpret_cast<char*>(&original_size), sizeof(original_size));
    archive.read(reinterpret_cast<char*>(&compressed_bits), sizeof(compressed_bits));
    if (!archive) {
        std::cerr << "[ERROR] Truncated archive header" << std::endl;
        return false;
    }

    std::ofstream output(output_path, std::ios::binary);
    if (!output) {
        std::cerr << "[ERROR] Could not open output file: " << output_path << std::endl;
        return false;
    }

    if (original_size == 0) return true; // arquivo original vazio

    Ppm decoder(kmax, /*treinando=*/true);
    decoder.aritmetico.prepare_decoding(archive);
    for (uint64_t i = 0; i < original_size; ++i) {
        output.put(static_cast<char>(decoder.decode_symbol(archive)));
    }

    return true;
}

} // namespace ppm_io