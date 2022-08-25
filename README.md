# net++

A small, dependency-free, header-only, cross-platform C++ library wrapping socket calls for:
- Windows (Windows Sockets 2)
- UNIX (Sockets + UNIX)

This project is developed on a x86_64 machine running Windows 10 and GNU/Linux.

This project is also *tested* on ARMv8 (64 Bit).

## Getting Started

### General

- choose between [the newest version](https://github.com/GereonV/NETpp/tree/master) or [the most recent release](https://github.com/GereonV/NETpp/releases)
- make the header-files excessable via your include-path
- include them whereever you need

### Compatibility

- Windows:
    - make sure to create a `net::context` **before** using anything
    - link against `Ws2_32`
- UNIX:
    - `net::udp::server::connect()` may work multiple times

## Help

- first-time users are encouraged to look at [the examples](https://github.com/GereonV/NETpp/tree/master/doc).
- ask for anything in the discussion-section

## Contributing

PRs are open and help is much appreciated - just follow the commit-style and use common sense
