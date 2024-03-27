#ifndef SERIALIZE_TESTUTILS_SERIALIZABLEINSTANCES_H
#define SERIALIZE_TESTUTILS_SERIALIZABLEINSTANCES_H
#include "packio/core/serializable.h"
#include "packio/core/version.h"
#include <variant>

namespace packio::testutils {
    class TestMock1 : public SerializableInstance {
    public:
        TestMock1() = default;
    };

    class TestMock2 : public SerializableInstance
    {
    public:
        TestMock2() = default;
    };

    using TestMockVariant = std::variant<TestMock1, TestMock2>;
}

namespace packio
{
    // ######################################################################################
    // TestMock1
    // ######################################################################################
    template<>
    constexpr std::array<char, 16> serializeSignature<testutils::TestMock1>()
    {
        return {'T', 'e', 's', 't', 'M', 'o', 'c', 'k', 'e', 'r', '1', ' ', ' ', ' ', ' ', ' '};
    }

    template<>
    void serializeBody(const testutils::TestMock1 &serializable, std::ostream &stream)
    {
        (void) serializable, stream;
    }

    template<>
    testutils::TestMock1 deserializeBody<testutils::TestMock1>(std::istream &stream)
    {
        return testutils::TestMock1{};
    }

    // ######################################################################################
    // TestMock2
    // ######################################################################################
    template<>
    constexpr std::array<char, 16> serializeSignature<testutils::TestMock2>()
    {
        return {'T', 'e', 's', 't', 'M', 'o', 'c', 'k', 'e', 'r', '2', ' ', ' ', ' ', ' ', ' '};
    }

    template<>
    void serializeBody(const testutils::TestMock2 &serializable, std::ostream &stream)
    {
        (void) serializable, stream;
    }

    template<>
    testutils::TestMock2 deserializeBody<testutils::TestMock2>(std::istream &stream)
    {
        return testutils::TestMock2{};
    }
} //namespace serialize::testutils
#endif //SERIALIZE_TESTUTILS_SERIALIZABLEINSTANCES_H
