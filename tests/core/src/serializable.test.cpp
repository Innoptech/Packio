#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_template_test_macros.hpp>
#include "testutils/serializableinstances.h"
#include <sstream>

using namespace packio;
using namespace packio::testutils;

TEMPLATE_TEST_CASE("Serializing Test", "[packio]", TestMock1, TestMock2)
{
    SECTION("Serialize mockers")
    {
        TestType mocker{};

        std::stringstream stream{};
        serialize(mocker, stream);

        stream.seekg(0);
        auto const restitutedMock = Deserializer<TestMockVariant>::deserialize<TestMock1, TestMock2>(stream);
        REQUIRE(std::holds_alternative<TestType>(restitutedMock));
        REQUIRE(std::get<TestType>(restitutedMock).getId() == mocker.getId());
    }
    SECTION("Serialize and Deserialize Different Types") {
        // Serialize a different type of mocker
        TestType mocker{};
        std::stringstream stream{};
        serialize(mocker, stream);

        // Deserialize using a different type
        stream.seekg(0);
        auto const restitutedMock = Deserializer<TestMockVariant>::deserialize<TestMock2, TestMock1>(stream);

        // Check if the deserialized object is of the correct type
        REQUIRE(std::holds_alternative<TestType>(restitutedMock));
    }
}

