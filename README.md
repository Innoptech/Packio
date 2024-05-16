# Packio: General Purpose Serializer/Deserializer

Packio is a C++ library for serialization and deserialization of data types. It provides a flexible and efficient way to convert data into a binary format for storage or transmission and vice versa.

## Features
<ul>
<li style="list-style-type: '✅'">Serialize data to a binary stream
<li style="list-style-type: '✅'">Deserialize data from a binary stream
<li style="list-style-type: '✅'">Support for custom serialization and deserialization methods
<li style="list-style-type: '✅'">Efficient handling of complex data structures
<li style="list-style-type: '✅'">Flexible and extensible design
</ul>

## Usage
To use Packio in your C++ projects, include the necessary headers and link against the Packio library. Then, you can use the provided serializer and deserializer classes to convert your data types to and from binary format.

### Example
Suppose you have the following Serializable types:

```cpp
#include "packio/core/serializable.h"

struct Foo1 {};
struct Foo2 {};
using FooVariant = std::variant<Foo1, Foo2>; // Define a convertible like std::variant or Type Erasure

namespace packio {
    // Implement requirements for Foo1
    template<>
    constexpr std::array<char, 16> serializeSignature<Foo1>() {
        return {'1'};
    }
    
    template<>
    void serializeBody(const Foo1 &serializable, std::ostream &stream) {
        (void) serializable, stream;
    }
    
    template<>
    FooVariant deserializeBody<FooVariant, Foo1>(std::istream &stream) {
        return Foo1{};
    }
    //Or 
    template<>
    Foo1 deserializeBody<Foo1>(std::istream &stream) {
        return Foo1{};
    }
    
    // Implement requirements for Foo2
    template<>
    constexpr std::array<char, 16> serializeSignature<Foo2>() {
        return {'2'};
    }
    
    template<>
    void serializeBody(const Foo2 &serializable, std::ostream &stream) {
        (void) serializable, stream;
    }
    
    template<>
    FooVariant deserializeBody<FooVariant, Foo2>(std::istream &stream) {
        return Foo2{};
    }
    //Or 
    template<>
    Foo2 deserializeBody<Foo2>(std::istream &stream) {
        return Foo2{};
    }
} // namespace packio

// Serialize
packio::serialize(Foo1{}, std::cout); // Could be any ostream

// Deserialize
auto result = packio::Deserializer<FooVariant>::deserialize<Foo1, Foo2>(std::cin); // Could be any istream
```
# Integrate to your codebase
### Smart method
Include this repository with CMAKE Fetchcontent and link your executable/library to `packio` library.   
Choose if you want to fetch a specific branch or tag using `GIT_TAG`. Use the `master` branch to keep updated with the latest improvements.
```cmake
include(FetchContent)
FetchContent_Declare(
    libpackio
    GIT_REPOSITORY https://github.com/Innoptech/Packio.git
    GIT_TAG master
    GIT_SHALLOW TRUE
    GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(libpackio)
```
### Naïve method
Simply add [serializable.h](modules/core/include/packio/core/serializable.h) to your codebase.

## Contributing
If you'd like to contribute to Packio, feel free to fork the repository and submit a pull request with your changes. Contributions are always welcome!

## License
Packio is licensed under the MIT License. See the LICENSE file for details.
