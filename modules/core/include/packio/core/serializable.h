/*
MIT License

Copyright (c) 2024 Innoptech

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
#include <iostream>
#include <vector>
#include <array>
#include <typeinfo>
#include <sstream>
#include <string>
#include <zlib.h>
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
    template<typename U, int MAJOR, int MINOR, int PATCH, typename T>
    struct DeserializeHelper<U, MAJOR, MINOR, PATCH, T> {
        static U deserialize(std::istream& stream, const std::array<char, 16>& signature) {
            if (std::equal(std::begin(signature), std::end(signature), std::begin(serializeSignature<T>()))) {
                // Read the compression flag
                bool header;
                stream.read(reinterpret_cast<char*>(&header), sizeof(bool));

                if (header) {
                    // Step 3: Read uncompressed and compressed size
                    uLongf uncompressedSize;
                    stream.read(reinterpret_cast<char*>(&uncompressedSize), sizeof(uncompressedSize));
                    uLongf compressedSize;
                    stream.read(reinterpret_cast<char*>(&compressedSize), sizeof(compressedSize));

                    // Extract checksum (last 4 bytes in the stream)
                    uLong crc32ChecksumStored;
                    stream.read(reinterpret_cast<char*>(&crc32ChecksumStored), sizeof(crc32ChecksumStored));

                    // Step 4: Read compressed data
                    std::vector<char> compressedData(compressedSize);
                    stream.read(compressedData.data(), compressedSize);

                    // Compute checksum for the serialized data
                    uLong crc32ChecksumComputed = crc32(0L, Z_NULL, 0);
                    crc32ChecksumComputed = crc32(crc32ChecksumComputed,
                                                  reinterpret_cast<const Bytef*>(compressedData.data()),compressedSize);

                    // Verify checksum
                    if (crc32ChecksumComputed != crc32ChecksumStored) {
                        throw std::runtime_error("Checksum verification failed: Data corruption detected");
                    }

                    // Step 5: Decompress the data
                    std::vector<char> uncompressedData(uncompressedSize);
                    int result = uncompress(reinterpret_cast<Bytef*>(uncompressedData.data()), &uncompressedSize,
                                            reinterpret_cast<const Bytef*>(compressedData.data()), compressedSize);

                    if (result != Z_OK) {
                        throw std::runtime_error("Decompression failed");
                    }

                    // Step 6: Deserialize from the decompressed data
                    std::stringstream tempBuffer(std::ios::binary | std::ios::in | std::ios::out);
                    tempBuffer.write(uncompressedData.data(), uncompressedSize);

                    return deserializeBody<T>(tempBuffer);
                } else {
                    // No compression: read the serialized data (excluding the checksum)
                    std::string serializedData;
                    std::streampos startPos = stream.tellg();
                    stream.seekg(0, std::ios::end);
                    std::streampos endPos = stream.tellg();
                    std::streamsize dataSize = endPos - startPos - sizeof(uLong);

                    serializedData.resize(dataSize);
                    stream.seekg(startPos);
                    stream.read(&serializedData[0], dataSize);

                    // Extract checksum (last 4 bytes in the stream)
                    uLong crc32ChecksumStored;
                    stream.read(reinterpret_cast<char*>(&crc32ChecksumStored), sizeof(crc32ChecksumStored));

                    // Compute checksum for the serialized data
                    uLong crc32ChecksumComputed = crc32(0L, Z_NULL, 0);
                    crc32ChecksumComputed = crc32(crc32ChecksumComputed,
                                                  reinterpret_cast<const Bytef*>(serializedData.data()),
                                                  serializedData.size());

                    // Verify checksum
                    if (crc32ChecksumComputed != crc32ChecksumStored) {
                        throw std::runtime_error("Checksum verification failed: Data corruption detected");
                    }

                    // Deserialize from validated data
                    std::istringstream validatedStream(serializedData);
                    return deserializeBody<T>(validatedStream);
                }
                return deserializeBody<U, T, MAJOR, MINOR, PATCH>(stream);
            } else {
                throw std::runtime_error{"Attempt to deserialize an unrecognised serializable"};
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
     * Serialize the given serializable object to the output stream.
     *
     * @tparam T The type of the serializable object to serialize.
     * @param serializable The serializable object to serialize.
     * @param stream The output stream to serialize to.
     */
    template<typename T, bool EnableCompression=true>
    inline void serialize(const T &serializable, std::ostream &stream)
    {
        auto signature = serializeSignature<T>();
        SerializableVersion version{PACKIO_VER_MAJOR, PACKIO_VER_MINOR, PACKIO_VER_PATCH};

        stream.write(signature.data(), sizeof(signature));
        stream.write(reinterpret_cast<const char*>(&version), sizeof(version));

        // Write the compression flag
        constexpr bool isCompressionEnabled{EnableCompression};
        stream.write(reinterpret_cast<const char*>(&isCompressionEnabled), sizeof(isCompressionEnabled));

        if constexpr (EnableCompression) {
            // Serialize to a temporary buffer
            std::stringstream tempBuffer(std::ios::binary | std::ios::out);
            serializeBody(serializable, tempBuffer);

            // Get uncompressed data
            std::string uncompressedData = tempBuffer.str();
            const auto uncompressedSize = static_cast<uLongf>(uncompressedData.size());

            // Step 4: Compress the data
            std::vector<char> compressedData(compressBound(uncompressedSize));
            uLongf compressedSize = compressedData.size();
            int result = compress(reinterpret_cast<Bytef*>(compressedData.data()), &compressedSize,
                                  reinterpret_cast<const Bytef*>(uncompressedData.data()), uncompressedSize);

            if (result != Z_OK) {
                throw std::runtime_error("Compression failed");
            }

            // Write the uncompressed size, the compressed size and the compressed data
            stream.write(reinterpret_cast<const char*>(&uncompressedSize), sizeof(uncompressedSize));
            stream.write(reinterpret_cast<const char*>(&compressedSize), sizeof(compressedSize));

            // Compute checksum (CRC32)
            uLong crc32Checksum = crc32(0L, Z_NULL, 0);
            crc32Checksum = crc32(crc32Checksum, reinterpret_cast<const Bytef*>(compressedData.data()), compressedSize);
            stream.write(reinterpret_cast<const char*>(&crc32Checksum), sizeof(crc32Checksum));

            stream.write(compressedData.data(), static_cast<long>(compressedSize));
        } else {
            // No compression: directly serialize the object body to the stream
            std::stringstream tempBuffer(std::ios::binary | std::ios::out);
            serializeBody(serializable, tempBuffer);
            const std::string serializedData = tempBuffer.str();

            stream.write(serializedData.data(), static_cast<long>(serializedData.size()));

            // Compute checksum (CRC32)
            uLong crc32Checksum = crc32(0L, Z_NULL, 0);
            crc32Checksum = crc32(crc32Checksum, reinterpret_cast<const Bytef*>(serializedData.data()), serializedData.size());
            stream.write(reinterpret_cast<const char*>(&crc32Checksum), sizeof(crc32Checksum));
        }
    }
} //namespace serialize::core
#endif //SERIALIZE_CORE_SERIALIZABLE_H
