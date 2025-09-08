# tag-invoke

A C++ header-only library implementing the `tag_invoke` customization point mechanism. This library provides utilities and traits for the tag_invoke pattern, which allows elegant generic programming by providing a standard way to customize behavior for different types through ADL (Argument-Dependent Lookup).

## What is tag_invoke?

The `tag_invoke` pattern is a customization point mechanism that has been proposed for the C++ standard. It allows libraries to define customizable operations that users can specialize for their own types without modifying the library code. This pattern relies on Argument-Dependent Lookup (ADL) to find the appropriate customization.

### Benefits

- **Non-intrusive customization**: Add behavior to types without modifying their definition
- **ADL-based dispatch**: Leverages C++'s lookup rules for clean, efficient dispatch
- **Type safety**: Compile-time checking ensures only valid customizations are called
- **Standard approach**: Based on proposed C++ standard mechanisms

## Installation

This is a header-only library. Simply include the header in your project:

```cpp
#include "tag_invoke.hpp"
```

Requirements: C++17 or later

## Basic Usage

### 1. Define a Tag Type

First, define a tag type that represents the operation you want to customize:

```cpp
#include "tag_invoke.hpp"
#include <iostream>
#include <string>

// Define a tag for a "serialize" operation
struct serialize_tag {};
inline constexpr serialize_tag serialize{};
```

### 2. Provide Default Implementation

Create a generic function that uses `tag_invoke`:

```cpp
template<typename T>
std::string do_serialize(const T& obj) {
    if constexpr (svh::is_tag_invocable_v<serialize_tag, const T&>) {
        return tag_invoke(serialize, obj);
    } else {
        return "No serialization available";
    }
}
```

### 3. Customize for Your Types

Provide customizations for specific types by defining `tag_invoke` functions:

```cpp
// Built-in types
std::string tag_invoke(serialize_tag, int value) {
    return std::to_string(value);
}

std::string tag_invoke(serialize_tag, const std::string& str) {
    return "\"" + str + "\"";
}

// Custom types
struct Person {
    std::string name;
    int age;
};

std::string tag_invoke(serialize_tag, const Person& p) {
    return "{\"name\": \"" + p.name + "\", \"age\": " + std::to_string(p.age) + "}";
}
```

### 4. Use the Customized Operation

```cpp
int main() {
    std::cout << do_serialize(42) << std::endl;           // "42"
    std::cout << do_serialize(std::string("hello")) << std::endl; // "\"hello\""
    
    Person p{"Alice", 30};
    std::cout << do_serialize(p) << std::endl;            // "{\"name\": \"Alice\", \"age\": 30}"
    
    // Example with a type that has no customization
    struct UnsupportedType { int value; };
    UnsupportedType unsupported{123};
    std::cout << do_serialize(unsupported) << std::endl;  // "No serialization available"
    
    return 0;
}
```

## Advanced Examples

### Example 1: Mathematical Operations with Custom Types

```cpp
#include "tag_invoke.hpp"
#include <iostream>

// Tag for addition operation
struct add_tag {};
inline constexpr add_tag add{};

// Generic addition function
template<typename T, typename U>
auto do_add(const T& lhs, const U& rhs) {
    static_assert(svh::is_tag_invocable_v<add_tag, const T&, const U&>, 
                  "Addition not supported for these types");
    return tag_invoke(add, lhs, rhs);
}

// Custom Vector type
struct Vector2D {
    double x, y;
    
    Vector2D(double x = 0, double y = 0) : x(x), y(y) {}
    
    friend std::ostream& operator<<(std::ostream& os, const Vector2D& v) {
        return os << "Vector2D(" << v.x << ", " << v.y << ")";
    }
};

// Customize addition for Vector2D
Vector2D tag_invoke(add_tag, const Vector2D& lhs, const Vector2D& rhs) {
    return Vector2D(lhs.x + rhs.x, lhs.y + rhs.y);
}

// Customize scalar multiplication
Vector2D tag_invoke(add_tag, const Vector2D& vec, double scalar) {
    return Vector2D(vec.x + scalar, vec.y + scalar);
}

int main() {
    Vector2D v1{1.0, 2.0};
    Vector2D v2{3.0, 4.0};
    
    auto result1 = do_add(v1, v2);      // Vector addition
    auto result2 = do_add(v1, 5.0);     // Scalar addition
    
    std::cout << "v1 + v2 = " << result1 << std::endl;  // Vector2D(4, 6)
    std::cout << "v1 + 5 = " << result2 << std::endl;   // Vector2D(6, 7)
    
    return 0;
}
```

### Example 2: Visitation Pattern

```cpp
#include "tag_invoke.hpp"
#include <iostream>
#include <variant>
#include <string>

// Tag for visiting operation
struct visit_tag {};
inline constexpr visit_tag visit{};

// Generic visit function
template<typename Visitor, typename Variant>
auto do_visit(Visitor&& visitor, Variant&& var) {
    return tag_invoke(visit, std::forward<Visitor>(visitor), std::forward<Variant>(var));
}

// Custom variant-like type
template<typename... Types>
struct MyVariant {
    std::variant<Types...> data;
    
    template<typename T>
    MyVariant(T&& value) : data(std::forward<T>(value)) {}
    
    template<typename T>
    T& get() { return std::get<T>(data); }
    
    template<typename T>
    const T& get() const { return std::get<T>(data); }
    
    template<typename T>
    bool holds() const { return std::holds_alternative<T>(data); }
};

// Customize visitation for MyVariant
template<typename Visitor, typename... Types>
auto tag_invoke(visit_tag, Visitor&& visitor, const MyVariant<Types...>& var) {
    return std::visit(std::forward<Visitor>(visitor), var.data);
}

int main() {
    MyVariant<int, std::string, double> var1{42};
    MyVariant<int, std::string, double> var2{std::string("hello")};
    
    auto printer = [](const auto& value) {
        std::cout << "Value: " << value << std::endl;
    };
    
    do_visit(printer, var1);  // Value: 42
    do_visit(printer, var2);  // Value: hello
    
    return 0;
}
```

## API Reference

### Core Components

#### `tag_invoke()`
```cpp
void tag_invoke();  // ADL anchor - no definition provided
```
The foundational ADL anchor for the tag_invoke pattern. This function is never defined and serves only to enable ADL lookup for user-provided customizations.

#### `tag_invoke_result_t<Tag, Args...>`
```cpp
template<class Tag, class... Args>
using tag_invoke_result_t = decltype(tag_invoke(std::declval<Tag>(), std::declval<Args>()...));
```
Type alias that determines the result type of a `tag_invoke` call. Useful for SFINAE and return type deduction.

#### `is_tag_invocable<Tag, Args...>`
```cpp
template<class Tag, class... Args>
using is_tag_invocable = /* implementation-defined */;
```
Type trait that determines whether `tag_invoke` can be called with the given tag and arguments.

#### `is_tag_invocable_v<Tag, Args...>`
```cpp
template<class Tag, class... Args>
inline constexpr bool is_tag_invocable_v = is_tag_invocable<Tag, Args...>::value;
```
Convenience variable template for `is_tag_invocable`.

### Usage Patterns

#### 1. Conditional Invocation
```cpp
template<typename T>
auto maybe_serialize(const T& obj) {
    if constexpr (svh::is_tag_invocable_v<serialize_tag, const T&>) {
        return tag_invoke(serialize, obj);
    } else {
        return default_behavior(obj);
    }
}
```

#### 2. SFINAE-Based Overloading
```cpp
template<typename T>
std::enable_if_t<svh::is_tag_invocable_v<serialize_tag, const T&>, std::string>
serialize_if_possible(const T& obj) {
    return tag_invoke(serialize, obj);
}
```

#### 3. Concept-Based Constraints (C++20)
```cpp
template<typename T>
concept Serializable = svh::is_tag_invocable_v<serialize_tag, const T&>;

template<Serializable T>
std::string serialize(const T& obj) {
    return tag_invoke(serialize, obj);
}
```

## Best Practices

1. **Use descriptive tag names**: Make your intent clear with names like `serialize_tag`, `hash_tag`, etc.
2. **Provide meaningful defaults**: Consider what should happen when no customization exists
3. **Document your customization points**: Clear documentation helps users understand how to extend your library
4. **Use SFINAE or concepts**: Provide clear error messages when customizations are missing
5. **Consider performance**: The pattern has minimal runtime overhead due to ADL resolution

## Compiler Support

This library requires C++17 or later and has been tested with:
- GCC 7+
- Clang 6+
- MSVC 2017+

## Related Work

- [P1895: tag_invoke: A general pattern for supporting customizable functions](https://wg21.link/P1895)
- [P2300: std::execution](https://wg21.link/P2300) - Uses tag_invoke extensively
- [C++23 Standard Proposals](https://en.cppreference.com/w/cpp/23)

## License

This project is available under the same terms as the repository license.

## Contributing

Contributions are welcome! Please ensure any examples you add are clear, well-documented, and demonstrate practical use cases.