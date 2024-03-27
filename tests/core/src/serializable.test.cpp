#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_template_test_macros.hpp>
#include "testutils/serializableinstances.h"
#include <sstream>

using namespace serialpack::core;
using namespace serialpack::testutils;

TEMPLATE_TEST_CASE("PenaltyStrategy Test", "[penalty]", TestMock1, TestMock2)
{
    SECTION("Serialize mockers")
    {
        TestType mocker{};

        std::stringstream stream{};
        serialize(mocker, stream);

        stream.seekg(0);
        auto const restitutedMock = deserialize<TestMockVariant, TestMock1, TestMock2>(stream);
        //STATIC_REQUIRE(std::is_same_v<decltype(restitutedMock), TestMockVariant>, "deserialize type");
    }
}

