# Packio: General Purpose Serializer/Deserializer

Packio is a C++ library for serialization and deserialization of data types. It provides a flexible and efficient way to convert data into a binary format for storage or transmission and vice versa.

## Features
<html lang="en">
<head>
    <meta charset="UTF-8">
    <style>
        /* Remove default list style */
        ul {
            list-style: none;
            padding-left: 0;
        }
    </style>
</head>
<body>
    <h1>Packio: General Purpose Serializer/Deserializer</h1>
    <p>Packio is a C++ library for serialization and deserialization of data types. It provides a flexible and efficient way to convert data into a binary format for storage or transmission and vice versa.</p>

<h2>Features</h2>
<ul>
    <li>&#10003; Serialize data to a binary stream</li>
    <li>&#10003; Deserialize data from a binary stream</li>
    <li>&#10003; Support for custom serialization and deserialization methods</li>
    <li>&#10003; Efficient handling of complex data structures</li>
    <li>&#10003; Flexible and extensible design</li>
</ul>
</body>
</html>

## Usage
To use Packio in your C++ projects, include the necessary headers and link against the Packio library. Then, you can use the provided serializer and deserializer classes to convert your data types to and from binary format.

### Example
Suppose you have the following Serializable types:

```cpp
struct Foo1 {};
struct Foo2 {};

// Implement requirements for Foo1
template<>
constexpr auto serializeSignature<Foo1>() {
    return std::array<char, 16>{'1'};
}

template<>
void serializeBody(const Foo1 &serializable, std::ostream &stream) {
    (void) serializable, stream;
}

template<>
Foo1 deserializeBody<Foo1>(std::istream &stream) {
    return Foo1{};
}

// Implement requirements for Foo2
template<>
constexpr auto serializeSignature<Foo2>() {
    return std::array<char, 16>{'2'};
}

template<>
void serializeBody(const Foo2 &serializable, std::ostream &stream) {
    (void) serializable, stream;
}

template<>
Foo1 deserializeBody<Foo2>(std::istream &stream) {
    return Foo2{};
}

// Serialize
serialize(Foo1{}, std::cout); // Could be any ostream

// Deserialize
using FooVariant = std::variant<Foo1, Foo2>; // Define a convertible
auto result = Deserializer<FooVariant>::deserialize<Foo1, Foo2>(std::cin); // Could be any istream
```
## Installation
Currently, Packio is a header-only library, so you just need to include the necessary headers in your project.

## Contributing
If you'd like to contribute to Packio, feel free to fork the repository and submit a pull request with your changes. Contributions are always welcome!

## License
Packio is licensed under the MIT License. See the LICENSE file for details.