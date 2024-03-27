#ifndef SERIALIZE_CORE_SERIALIZABLE_H
#define SERIALIZE_CORE_SERIALIZABLE_H
#include "serialpack/core/version.h"
#include <iostream>
#include <vector>
#include <array>
#include <typeinfo>

namespace packio
{
    class Serializable;

    // ************************************************************************************************************
    // Child registration mechanism
    // ************************************************************************************************************
    class SerializableInstance {};


    // ************************************************************************************************************
    // Concepts
    // ************************************************************************************************************
    template<typename T>
    concept IsSerializableInstance = std::is_base_of_v<SerializableInstance, T>;

    template<typename T, typename... Args>
    concept ConvertibleToT = (std::is_convertible_v<Args, T> && ...);

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
    template<IsSerializableInstance T>
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
    template<IsSerializableInstance T>
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
    template<IsSerializableInstance T>
    T deserializeBody(std::istream &stream);

    // ************************************************************************************************************
    // Available function for serializable instances
    // ************************************************************************************************************
    // Define Deserializer class template
    template<typename U>
    class Deserializer {
        // Helper struct to handle deserialization
        template<typename... Args>
        requires ConvertibleToT<U, Args...>
        struct DeserializeHelper;

    public:
        // Function to deserialize a Serializable type
        template<IsSerializableInstance... Args>
        requires (sizeof...(Args) > 0)
        static auto deserialize(std::istream& stream) {
            std::array<char, 16> signature{};
            stream.read(signature.data(), sizeof(char) * 16);

            SerializableVersion version{};
            stream.read(reinterpret_cast<char*>(&version), sizeof(version));

            return static_cast<U>(DeserializeHelper<Args...>::deserialize(stream, signature));
        }
    };

    // Define helper struct DeserializeHelper outside of Deserializer
    template<typename U>
    template<typename... Args>
    requires ConvertibleToT<U, Args...>
    struct Deserializer<U>::DeserializeHelper {
        static U deserialize(std::istream& stream, const std::array<char, 16>& signature) {
            // Implementation of DeserializeHelper
            return U{}; // Placeholder return value
        }
    };

    template<IsSerializableInstance T>
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
