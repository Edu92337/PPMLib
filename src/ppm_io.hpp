#pragma once
#include <string>

namespace ppm_io {

/** Compresses input_path into archive_path using a Ppm model with the given Kmax. */
bool compress_file(const std::string& input_path,
                    const std::string& archive_path,
                    int kmax = 5);

/** Decompresses archive_path into output_path. kmax must match the value
 *  used during compression — the current archive format does not persist it. */
bool decompress_file(const std::string& archive_path,
                      const std::string& output_path,
                      int kmax = 5);

} // namespace ppm_io