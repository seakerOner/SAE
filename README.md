# SAE (Seak An Engine)

**SAE** is a game engine being built from scratch, designed to be modular, lightweight, and cross-platform.
Currently, the project is in its early development stage, focusing on input handling, core systems, 
and laying the foundation for rendering and game logic.

# Planned Features

- Multi-platform support: Linux (first target), Windows, macOS
- Input Layer:
    - Keyboard, mouse, and gamepad support
    - Event system to decouple input from game logic
- **Game Loop** with multiple threads:
    - Input / event thread
    - Game logic / math / entities thread
    - Rendering thread
- Entity system planned
- Minimalist core with extensibility in mind

# Examples

- Examples of usability will be added as more functionality is estabilished.
- **To run an example** simply do:

```bash
make examples NAME_OF_FILE.c
```

# Dependencies

## SEAKCUTILS — general utility library

A collection of low-level, performance-oriented utilities written in C, designed to be reused across my own systems projects.
<https://github.com/seakerOner/seakcutils>

## RGFW -  A cross platform lightweight single-header window abstraction library for creating graphical programs or libraries.

RGFW is a focused general windowing framework for creating and handling windows and graphics contexts.
The API is a mix between GLFW and SDL while maintaining a minminalistic and easy to modify implementation.
<https://github.com/ColleagueRiley/RGFW>

# Status

**Early development**: currently focused on the input system and core engine foundation.

Building

No complex build instructions yet.
Include `core_base.h` in your project and compile the SAE modules. (Look at main.c)

# Dependencies

- SEAKCUTILS (general utilities library) <https://github.com/seakerOner/seakcutils>
