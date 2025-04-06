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

#ifndef SERIALIZE_CORE_SERIALIZABLE_H
#define SERIALIZE_CORE_SERIALIZABLE_H
#include "packio/core/version.h"
#include "packio/core/compression.h"
#include <iostream>
#include <vector>
#include <array>
#include <typeinfo>
#include <sstream>
#include <string>
#include <zstd.h>
#include <zstd_errors.h>
#include <stdint.h>

#ifdef __GNUC__
#define PACKIO_PACKED(...) __VA_ARGS__ __attribute__((__packed__))
#elif _MSC_VER
#define PACKIO_PACKED(...) __pragma( pack(push, 1) ) __VA_ARGS__ __pragma( pack(pop))
#else
    #error "Unsupported compiler"
#endif

namespace packio
{
    enum class VERSION_COMPARISON{GREATER=-1, EQUAL, LESS};

    struct Version
    {
        int major, minor, patch;

        [[nodiscard]] std::string to_string() const {
            return std::to_string(major)+"."+std::to_string(minor)+"."+std::to_string(patch);
        }
    };

    inline auto operator>(const Version &v1, const Version &v2) {
        if (v1.major > v2.major)
            return true;
        if (v1.major == v2.major) {
            if (v1.minor > v2.minor)
                return true;
            if (v1.minor == v2.minor)
                if (v1.patch > v2.patch)
                    return true;
        }
        return false;
    }

    inline auto operator==(const Version &v1, const Version &v2) {
        return v1.major == v2.major && v1.minor == v2.minor && v1.patch == v2.patch;
    }

    // Function to parse version strings into a tuple of integers
    inline Version parseVersion(const std::string& version) {
        int major, minor, patch;
        char dot;
        std::istringstream(version) >> major >> dot >> minor >> dot >> patch;
        return {major, minor, patch};
    }

    // Function to compare two version strings
    inline VERSION_COMPARISON compareVersion(const Version& v1, const Version& v2) {
        if (v1 == v2) return VERSION_COMPARISON::EQUAL;
        if (v1 > v2) return VERSION_COMPARISON::GREATER;
        return VERSION_COMPARISON::LESS;
    }

    // ************************************************************************************************************
    // Function to implement for serializable instances
    // ************************************************************************************************************
    struct SerializableVersion {
        uint16_t versionMajor{}, versionMinor{}, versionPatch{};
    };

    /**
     * @brief Serialize a serializable signature to identify the serializable implementation.
     *
     * This function serializes a serializable signature to be able to replicate it on deserialization.
     * The format of the signature is 16 bytes, which are used to uniquely identify
     * the serializable implementation.
     *
     * @tparam T The serializable type.
     * @param serializable The serializable implementation.
     * @return The serializable signature as an array of 16 bytes.
     */
    template<typename T>
    constexpr std::array<char, 16> serializeSignature();

    /**
     * @brief Serialize the body of a Serializable object to the output stream.
     *
     * This function serializes the body of the given Serializable object to the output stream.
     * It writes the tau parameter and the adjusted lengths vector to the stream in a format suitable
     * for deserialization.
     *
     * @param serializable The serializable implementation to serialize.
     * @param stream The output stream to write to.
     */
    template<typename T>
    void serializeBody(const T &serializable, std::ostream &stream);

    /**
     * @brief Deserialize the body of a PenaltyStrategy object from the input stream.
     *
     * This function deserializes the body of a PenaltyStrategy object from the input stream.
     * It reads the tau parameter and the adjusted lengths vector from the stream and
     * constructs a new PenaltyStrategy object with these values.
     *
     * @param stream The input stream to read from.
     * @return The deserialized penalty object.
     */
    template<typename U, typename T=U, int MAJOR=-1, int MINOR=-1, int PATCH=-1>
    U deserializeBody(std::istream &stream);

    // ************************************************************************************************************
    // Available function for serializable instances
    // ************************************************************************************************************
    // Define helper struct DeserializeHelper outside of Deserializer
    template<typename U, int MAJOR, int MINOR, int PATCH, typename T, typename... Args>
    struct DeserializeHelper {
        static U deserialize(std::istream& stream, const std::array<char, 16>& signature) {
            if (std::equal(std::begin(signature), std::end(signature), std::begin(serializeSignature<T>()))) {
                return deserializeBody<U, T, MAJOR, MINOR, PATCH>(stream);
            } else {
                return DeserializeHelper<U, MAJOR, MINOR, PATCH, Args...>::deserialize(stream, signature);
            }
        }
    };

    // Base case for DeserializeHelper
    /**
     * Deserialize the given serialized data from the input stream into type U.
     *
     * @tparam U      The type we expect to deserialize into.
     * @tparam MAJOR  Expected major version.
     * @tparam MINOR  Expected minor version.
     * @tparam PATCH  Expected patch version.
     * @tparam T      The original type (usually same as U, but can vary if you have custom expansions).
     */
    template<typename U, int MAJOR, int MINOR, int PATCH, typename T>
    struct DeserializeHelper<U, MAJOR, MINOR, PATCH, T> {
        static U deserialize(std::istream& stream, const std::array<char, 16>& signature)
        {
            if(stream.bad()){
                throw std::runtime_error("Attempt to deserialize with a bad stream");
            }

            // 1) Check signature
            std::array<char, 16> expectedSig = serializeSignature<T>();
            if (!std::equal(signature.begin(), signature.end(), expectedSig.begin())) {
                throw std::runtime_error("Attempt to deserialize an unrecognized serializable");
            }

            // 2) Read compression flag
            bool isCompressed;
            readAll(stream, &isCompressed, sizeof(isCompressed),
                    "Failed to read compression flag");

            if (isCompressed) {
                // ---- COMPRESSION BRANCH ----

                // A) Read sizes + checksum
                size_t uncompressedSize = 0;
                readAll(stream, &uncompressedSize, sizeof(uncompressedSize),
                        "Failed to read uncompressed size");
                size_t compressedSize = 0;
                readAll(stream, &compressedSize, sizeof(compressedSize),
                        "Failed to read compressed size");

                uint64_t checksumStored;
                readAll(stream, &checksumStored, sizeof(checksumStored),
                        "Failed to read checksum");

                // B) Read compressed data
                std::vector<char> compressedData(compressedSize);
                readAll(stream, compressedData.data(), compressedSize,
                        "Failed to read compressed data");

                // C) Verify checksum
                uint64_t checksumComputed = computeChecksum(compressedData.data(), compressedSize);
                if (checksumComputed != checksumStored) {
                    throw std::runtime_error("Checksum verification failed: Data corruption detected");
                }

                // D) Decompress
                std::vector<char> uncompressedData = decompressZstd(
                        compressedData.data(), compressedSize, uncompressedSize
                );

                // E) Deserialize from uncompressed data
                std::stringstream tempBuffer(std::string(uncompressedData.begin(), uncompressedData.end()),
                                             std::ios::binary | std::ios::in);
                return deserializeBody<T>(tempBuffer);

            } else {
                // ---- NO COMPRESSION ----

                // A) Figure out how much data is left excluding the checksum
                std::streampos startPos = stream.tellg();
                stream.seekg(0, std::ios::end);
                std::streampos endPos = stream.tellg();
                // We expect the last 8 bytes to be the XXH64 checksum
                if (endPos < startPos + static_cast<std::streamoff>(sizeof(uint64_t))) {
                    throw std::runtime_error("Stream is too short (no space for checksum)");
                }

                std::streamsize dataSize = static_cast<std::streamsize>(endPos - startPos - sizeof(uint64_t));

                // B) Read uncompressed data
                std::vector<char> serializedData(dataSize);
                stream.seekg(startPos);
                readAll(stream, serializedData.data(), dataSize,
                        "Failed to read uncompressed data");

                // C) Read checksum
                uint64_t checksumStored;
                readAll(stream, &checksumStored, sizeof(checksumStored),
                        "Failed to read checksum");

                // D) Verify checksum
                uint64_t checksumComputed = computeChecksum(serializedData.data(), dataSize);
                if (checksumComputed != checksumStored) {
                    throw std::runtime_error("Checksum verification failed: Data corruption detected");
                }

                // E) Deserialize from the validated data
                std::istringstream validatedStream(
                        std::string(serializedData.begin(), serializedData.end()),
                        std::ios::binary
                );

                return deserializeBody<T>(validatedStream);
            }
        }
    };

    // Define Deserializer class template
    template<typename U, int MAJOR=-1, int MINOR=-1, int PATCH=-1>
    class Deserializer {
    public:
        // Function to deserialize a Serializable type
        template<typename... Args>
        requires (sizeof...(Args) > 0)
        static auto deserialize(std::istream& stream, const std::string &minVersion="") {
            std::array<char, 16> signature{};
            stream.read(signature.data(), sizeof(char) * 16);

            SerializableVersion version{};
            stream.read(reinterpret_cast<char*>(&version), sizeof(version));

            return DeserializeHelper<U, MAJOR, MINOR, PATCH, Args...>::deserialize(stream, signature);
        }
    };

    /**
     * Deserialize a serializable object of type T from the input stream.
     *
     * @tparam T The type of the serializable object to deserialize.
     * @param stream The input stream to deserialize from.
     * @return An instance of type T deserialized from the input stream.
     */
    template<typename T>
    inline T deserialize(std::istream &stream)
    {
        return packio::Deserializer<T>::template deserialize<T>(stream);
    }


    /**
     * Serialize the given object to the output stream.
     *
     * @tparam T Type of the object to serialize.
     * @tparam EnableCompression If true, uses Zstd compression.
     */
    template<typename T, bool EnableCompression = true>
    inline void serialize(const T &serializable, std::ostream &stream)
    {
        if(stream.bad()){
            throw std::runtime_error("Attempt to serialize with a bad stream");
        }

        // 1) Write out "signature" (unique ID for type T)
        auto signature = serializeSignature<T>();
        writeAll(stream, signature.data(), signature.size(),
                 "Failed to write signature");

        // 2) Write out version info
        SerializableVersion version {PACKIO_VER_MAJOR, PACKIO_VER_MINOR, PACKIO_VER_PATCH};
        writeAll(stream, &version, sizeof(version),
                 "Failed to write version");

        // 3) Write compression flag
        constexpr bool isCompressionEnabled = EnableCompression;
        writeAll(stream, &isCompressionEnabled, sizeof(isCompressionEnabled),
                 "Failed to write compression flag");

        // 4) Convert object to a raw byte buffer (uncompressed)
        std::stringstream tempBuffer(std::ios::binary | std::ios::out);
        serializeBody(serializable, tempBuffer);
        std::string uncompressedData = tempBuffer.str();
        const size_t uncompressedSize = uncompressedData.size();

        if constexpr (EnableCompression) {
            // ---- COMPRESSION BRANCH ----

            // A) Compress data
            std::vector<char> compressedData = compressZstd(
                    uncompressedData.data(),
                    uncompressedSize
            );
            const size_t compressedSize = compressedData.size();

            // B) Compute checksum on compressed data
            uint64_t checksum = computeChecksum(compressedData.data(), compressedSize);

            // C) Write sizes + checksum + compressed bytes
            writeAll(stream, &uncompressedSize, sizeof(uncompressedSize),
                     "Failed to write uncompressed size");
            writeAll(stream, &compressedSize, sizeof(compressedSize),
                     "Failed to write compressed size");
            writeAll(stream, &checksum, sizeof(checksum),
                     "Failed to write checksum");
            writeAll(stream, compressedData.data(), compressedSize,
                     "Failed to write compressed data");
        } else {
            // ---- NO COMPRESSION ----

            // A) Compute checksum on uncompressed data
            uint64_t checksum = computeChecksum(uncompressedData.data(), uncompressedSize);

            // B) Write uncompressed bytes + checksum
            writeAll(stream, uncompressedData.data(), uncompressedSize,
                     "Failed to write uncompressed data");
            writeAll(stream, &checksum, sizeof(checksum),
                     "Failed to write checksum");
        }
    }
} //namespace serialize::core
#endif //SERIALIZE_CORE_SERIALIZABLE_H
