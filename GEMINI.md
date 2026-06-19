# Project Context: C++ Desktop Application

## Technical Stack
- **Language:** C++17 (Strictly follow C++17 standards)
- **UI Framework:** wxWidgets 3.3.1
- **Testing:** googletest (Internal logic only)
- **Build System:** Makefile

## Development Rules
- Use modern C++17 features (e.g., structured bindings, `std::optional`, `if constexpr`).
- Avoid C++20/23 features as they will break my build.
- UI code must be compatible with wxWidgets 3.3.1.
- All code must be cross-platform (Windows/Linux) unless specified.

## Search & Context
- I have excluded `googletest/`, and `wxWidgets-3.3.1_References/` from your index via .aiexclude.
- Do not attempt to read or suggest changes inside those excluded directories.
- Focus strictly on the logic in my local project files and the `Makefile`.

## Language Rules
- All code comments must be in English.
- All user-facing hard-coded strings must be in English.
