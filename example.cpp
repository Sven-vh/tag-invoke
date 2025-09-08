// Example usage of the tag-invoke library
// Compile with: g++ -std=c++17 -I. -o example example.cpp

#include "tag_invoke.hpp"
#include <iostream>
#include <string>

// Define a serialization tag
struct serialize_tag {};
inline constexpr serialize_tag serialize{};

// Generic serialization function
template<typename T>
std::string do_serialize(const T& obj) {
    if constexpr (svh::is_tag_invocable_v<serialize_tag, const T&>) {
        return tag_invoke(serialize, obj);
    } else {
        return "No serialization available";
    }
}

// Customizations for built-in types
std::string tag_invoke(serialize_tag, int value) {
    return std::to_string(value);
}

std::string tag_invoke(serialize_tag, const std::string& str) {
    return "\"" + str + "\"";
}

// Custom type
struct Person {
    std::string name;
    int age;
};

// Customization for Person
std::string tag_invoke(serialize_tag, const Person& p) {
    return "{\"name\": \"" + p.name + "\", \"age\": " + std::to_string(p.age) + "}";
}

int main() {
    std::cout << "tag-invoke example\n";
    std::cout << "==================\n\n";
    
    // Test with various types
    std::cout << "Serializing int: " << do_serialize(42) << std::endl;
    std::cout << "Serializing string: " << do_serialize(std::string("hello")) << std::endl;
    
    Person alice{"Alice", 30};
    std::cout << "Serializing Person: " << do_serialize(alice) << std::endl;
    
    // Type without customization
    struct UnsupportedType { int value; };
    UnsupportedType unsupported{123};
    std::cout << "Serializing UnsupportedType: " << do_serialize(unsupported) << std::endl;
    
    return 0;
}