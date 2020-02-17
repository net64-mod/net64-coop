# Coding Style

We recommend to follow these guidelines when writing code for Net64. They aren't very strict rules since we want to be flexible and we understand that under certain circumstances some of them can be counterproductive. Just try to follow as many of them as possible.

------

## Contents

- [General Rules](#general-rules)
- [Naming Rules](#naming-rules)
- [Comments](#comments)
- [Indentation](#indentation-style)


## General Rules

- C++ code base currently uses C++17.
- Line width is typically 120 characters.
- Do not introduce new dependencies.
- Avoid platform specific code.
- Use namespaces often.
- Avod #defines, use constants instead.
- Prefer `static_cast` and `reinterpret_cast` over C-style casts.
- Ensure that every source file you modify has the newline at the end of file. Every line ends with "newline" and the end of file must have "newline" too, GitHub usually warns about it.
- Don't use `using namespace [x]` in header files.
- Prefer smart pointers and other RAII containers over `new` & `delete`


## Naming Rules

- Files and folders should be lower snake_case, use `*.hpp` for C++ headers and `*.cpp` for C++ source files:
    - message_queue.cpp
    - message_queue.hpp
    - memory/
        - memory_handle.hpp
        - mem_ptr.hpp

- Struct, class, namespace and enum names should be upper CamelCase:
    - `struct GenericBinObject`
    - `enum struct SectionType`
    - `namespace Memory`

- POD-types and typedefs to PODs should be lower snake_case with a `_t` postfix:
    - `struct my_pod_t`
    - `using addr_t = std::uint32_t`

- Function, variable and namespace identifiers should be lower snake_case:
    - `int gather_symbols(const SymbolTable&)`
    - `std::size_t current_offset`

- Additionally please postfix **variables** in these cases:
    - Global variables: `_g` - `unsigned my_global_g`
    - Static variables: `_s` - `std::size_t instances_s`
    - Private & protected member variables: `_` - `addr_t offset_`
    
- Compile time constants should be all upper SNAKE_CASE:
    - `const NUM_SECTIONS{18}`
    - `static constexpr MAGIC_NUMBER{0x7F}`
    - `template<unsigned OPCODE_WIDTH>`
    
- Template type paramters are upper CamelCase or just T, U, V...:
    - `template<typename Identifier>`
    
- defines should be all upper case with a '_' postfix:
    - `#define USE_PROCESS_HANDLE_LINUX_`
    - `#define CHECK_(expr) assert(expr)`


## Comments

- For regular comments, use C++ style (`//`) comments, even for multi-line ones.
- For doc-comments (Doxygen comments), use `///` if it's a single line, else use the `/** */` style featured in the example. Start the text on the second line, not the first containing `/**`.
- For items that are both defined and declared in two separate files, put the doc-comment only next to the associated declaration. (In a header file, usually.) Otherwise, put it next to the implementation. Never duplicate doc-comments in both places.


## Indentation Style

Follow the indentation/whitespace style shown below. Do not use tabs, use 4-spaces instead.

```cpp
// Add this header to every new file you create:
//
// Copyright 2019 Net64 Coop Project
// Licensed under GPLv3
// Refer to the LICENSE file included.
//

// Use pragma directives instead of #ifndef... #define... #endif
#pragma once

// Includes should be sorted lexicographically
// Corresponding header file, STD includes, then library includes, and finally net64 includes
// No blank line between #includes (unless conditional #include presents or after the corresponding header file)
#include "memory_handle.hpp"

#include <map>
#include <memory>
#include <spdlog/spdlog.h>
#include <wx/wx.h>
#include "core/memory/memory_handle.hpp"
#include "core/memory/n64ref.hpp"
#include "core/memory/util.hpp"
#include "util/is_instantiation.hpp"


namespace Example
{

// Namespace contents are not indented

// Declare globals at the top
int foo_g{};            // {} can be used to initialize types as 0, false, or nullptr
char* some_pointer_g{}; // Pointer * and reference & stick to the type name, and make sure to
                        // initialize as nullptr!

/// A colorful enum.
enum struct SomeEnum   // Prefer scoped enums
{
    COLOR_RED,   ///< The color of fire.
    COLOR_GREEN, ///< The color of grass.
    COLOR_BLUE,  ///< Not actually the color of water.
};

/**
 * Very important struct that does a lot of stuff.
 * Note that the asterisks are indented by one space to align to the first line.
 */
struct RWHandle : RHandle   // Always use struct instead of class
{
    // Access order: public, protected, private - if possible

    // Within each group try to use this order:
    // 1. Friend declarations and typedefs
    // 2. Member functions
    // 3. Static functions
    // 4. Member variables
    // 5. Static variables


    RWHandle(Emulator& hdl): // Prefer references over pointers
        RHandle(hdl)
    {
    }

    // Avoid get_* set_* member functions
    bool valid() const      // note the space after ()
    {
        return hdl_ != nullptr;
    }

    // An operator overload
    std::ostream& opeator<<(std::ostream& strm)
    {
        // Mark this function as not implemented
        NOT_IMPLEMENTED_
    }

    static constexpr addr_t MAX_ADDR{Emulator::RAM_SIZE - 1},
                           INVALID_ADDR{std::numeric_limits<addr_t>::max()};

private:    // No indentation here
    void a_secret_function();

    int m_my_happy_little_int{}; // Always initialize members!

    static float some_static_value_s{3.141f};
};

// Use "typename" rather than "class" here
template<typename T>
void foo_bar()
{
    const std::string some_string{"prefer uniform initialization"};

    int some_array[]{
        5, 25, 7, 42
    };

    if(no == space_after_the_if)
    {
        call_function();
    }
    else
    {
        // Use a space after the // when commenting
    }

    // Place a single space after the for loop semicolons, prefer pre-increment
    for(int i{}; i != 25; ++i)
    {
        // This is how we write loops
    }

    do_stuff(this, function, call, takes, many, many, arguments, and_becomes_very_very_long, so,
             break, it, like, this);

    if(this ||
        condition_is_also_very_very_long && and_takes_up_multiple && lines && like && this ||
        everything || alright || then)
    {
        // Creating an object
        n64m::CPtr<u8> queue_size_ptr{memory_handle, 0xFF4300};
    }

    switch(var)
    {
    // No indentation for case label
    case 1:{
        int case_var{var + 3};
        do_something(case_var);
        break;
    }
    case 3:
        do_something(var);
        return;
    default:
        // Yes, even break for the last case
        break;
    }
}

} // Example

```
 
