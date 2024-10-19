#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_template_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include "testutils/serializableinstances.h"
#include <sstream>

using namespace packio;
using namespace packio::testutils;

TEMPLATE_TEST_CASE("Serializing Test", "[packio]", TestMock1, TestMock2)
{
    SECTION("With compression (default)")
    {
        TestType mocker{};

        std::stringstream stream{};
        serialize(mocker, stream);

        stream.seekg(0);
        auto const restitutedMock = Deserializer<TestMockVariant>::deserialize<TestMock1, TestMock2>(stream);
        REQUIRE(std::holds_alternative<TestType>(restitutedMock));
        REQUIRE(std::get<TestType>(restitutedMock).getId() == mocker.getId());
    }
    SECTION("Without compression")
    {
        TestType mocker{};

        std::stringstream stream{};
        serialize<TestType, false>(mocker, stream);

        stream.seekg(0);
        auto const restitutedMock = Deserializer<TestMockVariant>::deserialize<TestMock1, TestMock2>(stream);
        REQUIRE(std::holds_alternative<TestType>(restitutedMock));
        REQUIRE(std::get<TestType>(restitutedMock).getId() == mocker.getId());
    }
}

TEST_CASE("Serializing Test for trivial type", "[packio]")
{
    TestMock1 mocker{};

    std::stringstream stream{};
    serialize(mocker, stream);

    stream.seekg(0);
    auto const restitutedMock = deserialize<TestMock1>(stream);
    REQUIRE(restitutedMock.getId() == mocker.getId());
}

TEST_CASE("Data corruption in signature", "[packio]")
{
    TestMock1 mocker{};

    std::stringstream stream{};
    serialize(mocker, stream);

    stream.seekg(0);

    // Corrupt the serialized stream by changing some bytes
    std::string serializedData = stream.str();
    serializedData[0] = 0xFF;  // Arbitrarily corrupting byte in signature
    std::stringstream corruptedStream(serializedData);

    // Attempt to deserialize the corrupted data
    REQUIRE_THROWS_WITH(packio::deserialize<TestMock1>(corruptedStream),
                        Catch::Matchers::ContainsSubstring("Attempt to deserialize an unrecognised serializable"));
}

TEST_CASE("Data corruption in body", "[packio]")
{
    TestMock1 mocker{};

    std::stringstream stream{};
    serialize(mocker, stream);

    stream.seekg(0);

    // Corrupt the serialized stream by changing some bytes
    std::string serializedData = stream.str();
    *std::rbegin(serializedData) = 0xFF;  // Arbitrarily corrupting byte out of signature
    std::stringstream corruptedStream(serializedData);

    // Attempt to deserialize the corrupted data
    REQUIRE_THROWS_WITH(packio::deserialize<TestMock1>(corruptedStream),
                        Catch::Matchers::ContainsSubstring("Checksum verification failed: Data corruption detected"));
}
