/*
MIT License

Copyright (c) 2025 Innoptech

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#ifndef PACKIO_CORE_COMPRESSION_H
#define PACKIO_CORE_COMPRESSION_H
#include <zstd.h>
#include <sstream>
#include <vector>
#include <stdexcept>
#include <array>
#include <algorithm> // std::equal


namespace packio
{
    // Helper to ensure stream reads the exact amount requested
    inline void readAll(std::istream& stream, void* buffer, std::streamsize size, const char* errorMsg) {
        stream.read(reinterpret_cast<char*>(buffer), size);
        if (!stream.good() || stream.gcount() != size) {
            throw std::runtime_error(errorMsg);
        }
    }

    // Helper to ensure stream writes the exact amount requested
    inline void writeAll(std::ostream& stream, const void* buffer, std::streamsize size, const char* errorMsg) {
        stream.write(reinterpret_cast<const char*>(buffer), size);
        if (!stream.good()) {
            throw std::runtime_error(errorMsg);
        }
    }

    inline uint64_t fnv1a_64(const void* data, size_t length) {
        static constexpr uint64_t FNV_OFFSET_BASIS = 1469598103934665603ULL;
        static constexpr uint64_t FNV_PRIME        = 1099511628211ULL;

        uint64_t hash = FNV_OFFSET_BASIS;
        const auto* bytes = static_cast<const unsigned char*>(data);

        for (size_t i = 0; i < length; ++i) {
            hash ^= (uint64_t)bytes[i];
            hash *= FNV_PRIME;
        }
        return hash;
    }

    inline uint64_t computeChecksum(const void* data, size_t size) {
        return fnv1a_64(data, size);
    }

    // Compress data using Zstd with highest-level + multi-threading
    inline std::vector<char> compressZstd(const void* data, size_t dataSize) {
        // Create a Zstd context
        ZSTD_CCtx* cctx = ZSTD_createCCtx();
        if (!cctx) {
            throw std::runtime_error("Failed to create Zstd compression context");
        }

        // Set compression parameters
        ZSTD_CCtx_setParameter(cctx, ZSTD_c_compressionLevel, ZSTD_maxCLevel()); // highest compression
        ZSTD_CCtx_setParameter(cctx, ZSTD_c_nbWorkers, 4); // multi-threading

        // Allocate output buffer
        size_t maxCompressedSize = ZSTD_compressBound(dataSize);
        std::vector<char> compressed(maxCompressedSize);

        // Perform compression
        size_t compressedSize = ZSTD_compress2(
                cctx,
                compressed.data(), maxCompressedSize,
                data, dataSize
        );

        ZSTD_freeCCtx(cctx);

        // Check for errors
        if (ZSTD_isError(compressedSize)) {
            throw std::runtime_error(
                    std::string("Zstd compression failed: ") + ZSTD_getErrorName(compressedSize));
        }

        // Resize vector to actual size
        compressed.resize(compressedSize);
        return compressed;
    }

    // Decompress data using Zstd
    inline std::vector<char> decompressZstd(const void* data, size_t compressedSize, size_t expectedDecompressedSize) {
        std::vector<char> decompressed(expectedDecompressedSize);

        size_t actualSize = ZSTD_decompress(
                decompressed.data(), expectedDecompressedSize,
                data, compressedSize
        );

        if (ZSTD_isError(actualSize)) {
            throw std::runtime_error(
                    std::string("Zstd decompression failed: ") + ZSTD_getErrorName(actualSize));
        }
        if (actualSize != expectedDecompressedSize) {
            throw std::runtime_error("Decompression size mismatch (possible data corruption).");
        }

        return decompressed;
    }
} //namespace packio::core
#endif //PACKIO_CORE_COMPRESSION_H
