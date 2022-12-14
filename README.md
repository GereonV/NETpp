# net++

A small, dependency-free, header-only, cross-platform C++ library wrapping socket calls for:
- Windows (Windows Sockets 2)
- UNIX (Sockets + UNIX)

This project:
- is developed on a x86_64 machine running Windows 10 and GNU/Linux
- is also *tested* on ARMv8 (64 Bit)
- compiles flawlessly with `-Wpedantic -Wall -Wextra -Wconversion`
- [uses the permissive MIT license](https://github.com/GereonV/NETpp/blob/master/LICENSE)

## Getting Started

### General

- get [the most recent version](https://github.com/GereonV/NETpp/releases/latest)
- make the header-files accessible via your include-path
- include them whereever you need

### Compatibility

- Windows:
    - make sure to create a `net::context` **before** using anything (available as macro `NET_INIT()`)
    - link against `Ws2_32`
- UNIX:
    - `net::udp::server::connect()` may work multiple times

## Help

- first-time users are encouraged to look at [the examples](https://github.com/GereonV/NETpp/tree/master/doc)
- ask for anything in the discussion-section ([go to guide](#discussions))

## Contributing

**Please use the appropriate feature! :)**

### [Issues](https://github.com/GereonV/NETpp/issues)
- bug-reports (will be labeled *bug*)
- concrete request about changes to the code (will be labeled *enhancement*)
- shortcomings of the documentation/examples (will be labeled *documentation*)

### [Discussions](https://github.com/GereonV/NETpp/discussions)

- **Q&A**: ask questions on how to use the library
- **Ideas**: present ideas about new features
- **Polls**: survey opinions
- **Show and tell**: share experiences using the library
- **General**: anything else

### [PRs](https://github.com/GereonV/NETpp/pulls)

PRs are open and help is much appreciated - just follow the commit-style and use common sense
