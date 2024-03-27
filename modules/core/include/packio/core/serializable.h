#ifndef SERIALIZE_CORE_SERIALIZABLE_H
#define SERIALIZE_CORE_SERIALIZABLE_H
#include "packio/core/version.h"
#include <iostream>
#include <vector>
#include <array>
#include <typeinfo>
#include <sstream>
#include <string>

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

    template<typename T>
    void serialize(const T &serializable, std::ostream &stream)
    {
        auto signature = serializeSignature<T>();
        SerializableVersion version{SERIALPACK_VER_MAJOR, SERIALPACK_VER_MINOR, SERIALPACK_VER_PATCH};
        stream.write(signature.data(), sizeof(decltype(signature)));
        stream.write((char*)&version, sizeof(decltype(version)));
        serializeBody(serializable, stream);
    }
} //namespace serialize::core
#endif //SERIALIZE_CORE_SERIALIZABLE_H
