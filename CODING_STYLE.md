# Coding Style

We recommend to follow these guidelines when writing code for Net64. They aren't very strict rules since we want to be flexible and we understand that under certain circumstances some of them can be counterproductive. Just try to follow as many of them as possible.

------

## Contents

- [General Rules](#general-rules)
- [Naming Rules](#naming-rules)
- [Comments](#comments)
- [Formatting](#formatting)


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
    - Appreviations should be in all upper case with with a '_' for clarity: `struct IPC_Queue`

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


## Formatting

Follow the indentation/whitespace style of existing files. Do not use tabs, use 4-spaces instead. Run clang-format on modified files.
