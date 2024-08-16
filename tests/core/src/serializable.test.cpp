#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_template_test_macros.hpp>
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


