#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_template_test_macros.hpp>
#include "testutils/serializableinstances.h"
#include <sstream>

using namespace packio;
using namespace packio::testutils;

TEMPLATE_TEST_CASE("PenaltyStrategy Test", "[penalty]", TestMock1, TestMock2)
{
    SECTION("Serialize mockers")
    {
        TestType mocker{};

        std::stringstream stream{};
        serialize(mocker, stream);

        stream.seekg(0);
        auto const restitutedMock = Deserializer<TestMockVariant>::deserialize<TestMock1, TestMock2>(stream);
        //STATIC_REQUIRE(std::is_same_v<decltype(restitutedMock), TestMockVariant>, "deserialize type");
    }
}

