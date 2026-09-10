# PPM Lib C++

C++17 library for PPM (Prediction by Partial Matching) compression with
32-bit arithmetic coding.

The project contains the PPM model, context tree, arithmetic encoder and
arithmetic decoder, together with a round-trip test that verifies whether the
decompressed bytes match the original bytes.

## Current status

- Static library generated as `libppm.a`.
- CMake target exported as `PPM::ppm`.
- PPM context size configurable through `Kmax`.
- Binary file encoding and decoding.
- Compression followed by decompression test.

The library provides a high-level file API through `ppm_io`, as well as the
lower-level `Ppm`, `Codificador_aritmetico` and `ArquivoInfo` types.

## Requirements

- CMake 3.16 or newer.
- A compiler with C++17 support.
- A system with binary file support.

## Project structure

```text
.
├── CMakeLists.txt
├── cmake/
│   └── PPMLibConfig.cmake.in
├── include/                 # created during installation
├── src/
│   ├── codificador_aritmetico.hpp
│   ├── estrutura_contexto.cpp
│   ├── estrutura_contexto.hpp
│   ├── ppm.cpp
│   ├── ppm.hpp
│   ├── ppm_io.cpp
│   └── ppm_io.hpp
└── tests/
    └── roundtrip.cpp
```

## Building

Default configuration, including the test target:

```bash
cmake -S . -B build
cmake --build build --parallel
```

The static library is generated at:

```text
build/libppm.a
```

To build only the library:

```bash
cmake -S . -B build -DPPM_BUILD_TESTS=OFF
cmake --build build --parallel
```

## Testing

The automated test uses a deterministic binary sample:

```bash
ctest --test-dir build --output-on-failure
```

The executable also accepts a real file. It compresses the file, reads the
compressed archive, decompresses the bytes in memory and compares the result
with the input:

```bash
./build/ppm_roundtrip \
  "/path/to/input" \
  "/tmp/archive.ppm"
```

Example using the Silesia `dickens` file:

```bash
./build/ppm_roundtrip \
	"/path/to/Silesia/dickens" \
	/tmp/dickens-roundtrip.ppm
```

A successful run reports:

```text
OK: compression and decompression reproduced N bytes
```

## Installation

After building:

```bash
cmake --install build --prefix install
```

The installation includes:

- `install/lib/libppm.a`;
- headers under `install/include/ppm`;
- CMake configuration files under `install/lib/cmake/PPMLib`.

## Using the library from another CMake project

After installing the library:

```cmake
cmake_minimum_required(VERSION 3.16)
project(example LANGUAGES CXX)

find_package(PPMLib CONFIG REQUIRED)

add_executable(example main.cpp)
target_link_libraries(example PRIVATE PPM::ppm)
target_compile_features(example PRIVATE cxx_std_17)
```

When configuring the consumer project, provide the installation prefix:

```bash
cmake -S . -B build -DCMAKE_PREFIX_PATH=/path/to/PPM_lib_cpp/install
cmake --build build
```

## Current API usage

The recommended file-level API is provided by `ppm_io`:

```cpp
#include "ppm_io.hpp"

int main() {
	const int kmax = 5;
	if (!ppm_io::compress_file("input.bin", "output.ppm", kmax)) {
		return 1;
	}
	return ppm_io::decompress_file("output.ppm", "restored.bin", kmax) ? 0 : 1;
}
```

The `kmax` value must be the same during compression and decompression because
the current archive format does not store it. The high-level API currently
compresses and decompresses one file per operation.

For direct access to the model and arithmetic coder, the lower-level encoding
flow is:

The basic encoding flow is:

```cpp
#include "ppm.hpp"

#include <cstdint>
#include <fstream>

Ppm compressor(5, true);

std::ifstream input("input.bin", std::ios::binary);
char byte = 0;
uint64_t size = 0;
while (input.get(byte)) {
		compressor.process_symbol(
				static_cast<uint8_t>(static_cast<unsigned char>(byte)));
		++size;
}

compressor.aritmetico.finalize_encoding();

ArquivoInfo info{"input.bin", size};
compressor.aritmetico.save_archive(
		"output.ppm", {info}, size);
```

The `Kmax` value controls the largest context order used by the model. Larger
values may improve compression for some data, but increase memory usage and
processing cost.

## Current file format

The `save_archive` method writes data in the following order:

```text
uint64_t file_count
for each file:
		uint16_t name_length
		name bytes
		uint64_t original_size
uint32_t bit_count
arithmetic stream bytes, MSB first
```

The current format uses the platform's native binary representation. As a
result, files produced on platforms with different byte order may not be
portable.

The archive currently stores the original file name and size, but the high-level
decompression function writes to the output path supplied by the caller.

## License

No license has been defined for this repository yet.
# PPMLib
