# AI Agent Guidelines for Cross-Platform C++17 Development (wxWidgets)

## 0. Critical Operational Constraint (Rule Zero)
- **STRICT DIRECTIVE:** Do only what I explicitly say. 
- **STRICT DIRECTIVE:** Pay close attention to the current implementation.
- **NO UNSOLICITED ACTIONS:** Do NOT perform unsolicited analyses of the whole project.
- **NO UNRELATED CHANGES:** Do NOT modify, refactor, or "improve" any files or code blocks outside the immediate scope of the requested change. Fix only what you are told to fix.

### Target Standard & UI Framework
- **Language Standard:** C++17 (`-std=c++17` on GCC / `/std:c++17` on MSVC)
- **Linkage & Scope:** Do not create anonymous namespaces. Always use static functions if helper functions with internal linkage are needed.
- **GUI Framework:** wxWidgets 3.3.1 (Do not use deprecated pre-3.0 APIs)

### Environment A: Linux
- **Compiler:** `g++` (GCC)
- **Build System:** `GNU Make` (Makefile)
- **Build Command:** `make release`

### Environment B: Windows
- **Compiler:** MSVC (Visual Studio via Developer Command Prompt)
- **Build System:** `nmake` (NMakefile)
- **Build Command:** `nmake /f NMakefile`
