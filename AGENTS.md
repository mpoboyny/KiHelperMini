# AI Agent Guidelines for Cross-Platform C++17 Development (wxWidgets)

## 0. Critical Operational Constraint (Rule Zero)
- **STRICT DIRECTIVE:** Do only what I explicitly say. 
- **NO UNSOLICITED ACTIONS:** Do NOT perform unsolicited analyses of the whole project.
- **NO UNRELATED CHANGES:** Do NOT modify, refactor, or "improve" any files or code blocks outside the immediate scope of the requested change. Fix only what you are told to fix.

## 1. System Environment & Toolchains
This project targets both Linux and Windows. Code MUST compile on both toolchains using the respective build tools.

### Target Standard & UI Framework
- **Language Standard:** C++17 (`-std=c++17` on GCC / `/std:c++17` on MSVC)
- **GUI Framework:** wxWidgets 3.3.1 (Do not use deprecated pre-3.0 APIs)

### Environment A: Linux
- **Compiler:** `g++` (GCC)
- **Build System:** `GNU Make` (Makefile)
- **Build Command:** `make -j$(nproc) DEBUG=1`

### Environment B: Windows
- **Compiler:** MSVC (Visual Studio 2022 Community via Developer Command Prompt)
- **Build System:** `nmake` (Makefile.vc)
- **Build Command:** `nmake /f NMAkefile`

## 2. Mandatory Diagnostics & Warning Gates
Before declaring a task complete, you must verify that the code compiles with zero warnings and zero errors on both platforms.

## 3. Strict wxWidgets 3.3.1 & Cross-Platform Rules

1. **Modern wxWidgets 3.3+ Event Handling:**
   - NEVER use legacy event tables (`BEGIN_EVENT_TABLE`, `EVT_BUTTON`, etc.).
   - ALWAYS use `Bind()` for event handling (e.g., `button->Bind(wxEVT_BUTTON, &MyFrame::OnButton, this);`). This ensures compile-time type safety.
2. **Memory Management (wxWidgets & C++17):**
   - **wxWidgets Windows/Controls:** Child windows/controls owned by a parent window (e.g., `wxButton` inside a `wxPanel`) are managed internally by wxWidgets. Do NOT wrap them in `std::unique_ptr` or call `delete` manually. Use raw pointers via `new`.
   - **Non-widget Objects:** Use `std::make_unique` or `std::make_shared` for any standard heap allocations not bound to the wxWidgets window hierarchy.
3. **Strings and Encoding:**
   - Always use `wxString` when interacting with the wxWidgets API.
   - For standard logic, prefer `std::string` or `std::string_view` (C++17) and convert via `wxString::FromUTF8()` or `wxString::ToStdString()`. Avoid `wxT()` or `_T()` macros; string literals should be standard UTF-8.
4. **No Compiler/OS-Specific Extensions:**
   - Never use Variable-Length Arrays (VLAs) like `int arr[n];`. Use `std::vector`.
   - Use `std::filesystem` for OS-agnostic path and file handling; do not hardcode backslashes (`\\`) for Windows paths.

## 4. Definition of Done (DoD)
1. Code compiles without warnings/errors via `make` (Linux) and `nmake` (Windows).
