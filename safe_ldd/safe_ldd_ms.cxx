#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <windows.h>
#include <vector>
#include <list>
#include <string>

#define PATH_BUFFER_SIZE 1024

std::list<std::string> visited;
std::list<std::string> missing;
std::list<std::string> full_output;

// Standard Windows DLL search directories (system dir, windows dir, PATH, cwd).
std::list<std::string> standard_dirs;

int contains(const std::list<std::string>& values, const std::string& value) {
    for (const std::string& current : values) {
        if (current == value) {
            return 1;
        }
    }
    return 0;
}

void add_output_line(int level, const std::string& line) {
    std::string indented_line;
    for (int i = 0; i < level; i++) {
        indented_line += "  ";
    }
    indented_line += line;
    full_output.push_back(indented_line);
}

void add_missing(const std::string& lib) {
    if (!contains(missing, lib)) {
        missing.push_back(lib);
    }
}

// Split a PATH-style list into its directories (only non-empty entries).
static void split_paths(const char* path_value, std::list<std::string>& dirs) {
    const char* start = path_value;
    for (const char* p = path_value;; p++) {
        if (*p == ';' || *p == '\0') {
            if (p > start) {
                dirs.push_back(std::string(start, static_cast<size_t>(p - start)));
            }
            if (*p == '\0') {
                break;
            }
            start = p + 1;
        }
    }
}

// Collect the standard Windows search directories once at startup.
void init_standard_dirs() {
    char buf[PATH_BUFFER_SIZE];
    if (GetSystemDirectoryA(buf, sizeof(buf)) > 0 && buf[0] != '\0') {
        standard_dirs.push_back(buf);
    }
    if (GetWindowsDirectoryA(buf, sizeof(buf)) > 0 && buf[0] != '\0') {
        standard_dirs.push_back(buf);
    }
    const char* path_value = getenv("PATH");
    if (path_value) {
        split_paths(path_value, standard_dirs);
    }
    if (GetCurrentDirectoryA(sizeof(buf), buf) > 0 && buf[0] != '\0') {
        standard_dirs.push_back(buf);
    }
}

std::string join_path(const std::string& dir, const std::string& name) {
    if (dir.empty()) {
        return name;
    }
    if (dir.back() == '\\' || dir.back() == '/') {
        return dir + name;
    }
    return dir + "\\" + name;
}

bool file_exists(const std::string& path) {
    DWORD attrs = GetFileAttributesA(path.c_str());
    return (attrs != INVALID_FILE_ATTRIBUTES) && ((attrs & FILE_ATTRIBUTE_DIRECTORY) == 0);
}

// Extract the directory of the given path through the last separator.
std::string get_binary_dir(const std::string& filename) {
    size_t pos = filename.find_last_of("/\\");
    if (pos == std::string::npos) {
        return std::string();
    }
    return filename.substr(0, pos);
}

// Convert a PE relative virtual address (RVA) into a file offset using the
// section headers. RVAs inside the header area map to the same offset.
DWORD rva_to_offset(const std::vector<IMAGE_SECTION_HEADER>& sections, DWORD rva) {
    if (sections.empty()) {
        return rva;
    }
    if (rva < sections[0].VirtualAddress) {
        return rva;
    }
    for (const IMAGE_SECTION_HEADER& sec : sections) {
        if (rva >= sec.VirtualAddress && rva < sec.VirtualAddress + sec.SizeOfRawData) {
            return rva - sec.VirtualAddress + sec.PointerToRawData;
        }
    }
    return 0;
}

// Read a NUL-terminated ASCII string located at the given RVA.
std::string read_rva_string(const std::vector<BYTE>& image, DWORD file_size,
                            const std::vector<IMAGE_SECTION_HEADER>& sections, DWORD rva) {
    DWORD offset = rva_to_offset(sections, rva);
    if (offset == 0 || offset >= file_size) {
        return std::string();
    }
    std::string result;
    for (DWORD i = offset; i < file_size; i++) {
        char c = static_cast<char>(image[i]);
        if (c == '\0') {
            break;
        }
        result += c;
        if (result.size() > 4096) {
            return std::string();
        }
    }
    return result;
}

void scan_pe(const std::string& filename, int level) {
    if (contains(visited, filename)) {
        return;
    }
    visited.push_back(filename);

    // Read the whole image into memory to walk the PE structures safely.
    HANDLE hFile = CreateFileA(filename.c_str(), GENERIC_READ, FILE_SHARE_READ,
                               NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        return;
    }
    DWORD file_size = GetFileSize(hFile, NULL);
    if (file_size < 64) {
        CloseHandle(hFile);
        return;
    }
    std::vector<BYTE> image(file_size);
    DWORD total = 0;
    while (total < file_size) {
        DWORD chunk = 0;
        if (!ReadFile(hFile, image.data() + total, file_size - total, &chunk, NULL) || chunk == 0) {
            break;
        }
        total += chunk;
    }
    CloseHandle(hFile);
    if (total != file_size) {
        return;
    }

    // Verify the DOS header magic.
    IMAGE_DOS_HEADER dos;
    memcpy(&dos, image.data(), sizeof(dos));
    if (dos.e_magic != IMAGE_DOS_SIGNATURE) {
        return;
    }

    // Verify the "PE\0\0" signature.
    if (dos.e_lfanew <= 0 || static_cast<DWORD>(dos.e_lfanew) + 4 > file_size) {
        return;
    }
    DWORD pe_signature;
    memcpy(&pe_signature, image.data() + dos.e_lfanew, sizeof(pe_signature));
    if (pe_signature != IMAGE_NT_SIGNATURE) {
        return;
    }

    // Read the COFF file header.
    IMAGE_FILE_HEADER file_header;
    memcpy(&file_header, image.data() + dos.e_lfanew + 4, sizeof(file_header));

    // Read the optional header start to determine whether it is PE32 or PE32+.
    const DWORD opt_offset = static_cast<DWORD>(dos.e_lfanew) + 4 + sizeof(IMAGE_FILE_HEADER);
    if (opt_offset + sizeof(WORD) > file_size) {
        return;
    }
    WORD opt_magic;
    memcpy(&opt_magic, image.data() + opt_offset, sizeof(opt_magic));

    // Read the data directories (clamped to the standard 16 entries).
    DWORD num_rvas = 0;
    if (opt_magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC) {
        IMAGE_OPTIONAL_HEADER32 oh;
        if (opt_offset + sizeof(oh) > file_size) {
            return;
        }
        memcpy(&oh, image.data() + opt_offset, sizeof(oh));
        num_rvas = oh.NumberOfRvaAndSizes;
    } else if (opt_magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC) {
        IMAGE_OPTIONAL_HEADER64 oh;
        if (opt_offset + sizeof(oh) > file_size) {
            return;
        }
        memcpy(&oh, image.data() + opt_offset, sizeof(oh));
        num_rvas = oh.NumberOfRvaAndSizes;
    } else {
        return;
    }
    if (num_rvas > IMAGE_NUMBEROF_DIRECTORY_ENTRIES) {
        num_rvas = IMAGE_NUMBEROF_DIRECTORY_ENTRIES;
    }

    // Read the section headers for RVA -> file offset translation.
    std::vector<IMAGE_SECTION_HEADER> sections;
    DWORD sec_offset = opt_offset + file_header.SizeOfOptionalHeader;
    if (sec_offset + static_cast<DWORD>(file_header.NumberOfSections) * sizeof(IMAGE_SECTION_HEADER) > file_size) {
        return;
    }
    for (int i = 0; i < file_header.NumberOfSections; i++) {
        IMAGE_SECTION_HEADER sec;
        memcpy(&sec, image.data() + sec_offset + i * sizeof(sec), sizeof(sec));
        sections.push_back(sec);
    }

    // Locate the import directory (data directory index 1).
    IMAGE_DATA_DIRECTORY import_dir = {0, 0};
    if (num_rvas > IMAGE_DIRECTORY_ENTRY_IMPORT) {
        DWORD dd_offset = (opt_magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC)
                              ? offsetof(IMAGE_OPTIONAL_HEADER64, DataDirectory)
                              : offsetof(IMAGE_OPTIONAL_HEADER32, DataDirectory);
        memcpy(&import_dir, image.data() + opt_offset + dd_offset +
                                IMAGE_DIRECTORY_ENTRY_IMPORT * sizeof(IMAGE_DATA_DIRECTORY),
               sizeof(import_dir));
    }
    if (import_dir.VirtualAddress == 0 || import_dir.Size == 0) {
        return;
    }

    // Extract the directory of the current binary to check for local DLLs.
    std::string binary_dir = get_binary_dir(filename);

    // Walk the list of import descriptors until the all-zero terminator.
    DWORD import_offset = rva_to_offset(sections, import_dir.VirtualAddress);
    for (int guard = 0; guard < 65536; guard++) {
        if (import_offset == 0 || import_offset + sizeof(IMAGE_IMPORT_DESCRIPTOR) > file_size) {
            break;
        }
        IMAGE_IMPORT_DESCRIPTOR desc;
        memcpy(&desc, image.data() + import_offset, sizeof(desc));
        if (desc.Name == 0) {
            break; // Terminator descriptor.
        }

        std::string lib_name = read_rva_string(image, file_size, sections, desc.Name);
        if (!lib_name.empty()) {
            char found_path[PATH_BUFFER_SIZE];
            int found = 0;

            // 1. Try to find the library in the same directory as the binary.
            snprintf(found_path, sizeof(found_path), "%s", join_path(binary_dir, lib_name).c_str());
            if (file_exists(found_path)) {
                found = 1;
            }

            // 2. Try the standard Windows search directories.
            if (!found) {
                for (const std::string& dir : standard_dirs) {
                    snprintf(found_path, sizeof(found_path), "%s", join_path(dir, lib_name).c_str());
                    if (file_exists(found_path)) {
                        found = 1;
                        break;
                    }
                }
            }

            if (found) {
                add_output_line(level, "-> " + lib_name + " (" + found_path + ")");
                // Recurse into the dependencies of the found DLL.
                scan_pe(found_path, level + 1);
            } else {
                add_output_line(level, "-> " + lib_name + " (NOT FOUND)");
                add_missing(lib_name);
            }
        }
        import_offset += sizeof(IMAGE_IMPORT_DESCRIPTOR);
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <path_to_binary>\n", argv[0]);
        return 1;
    }
    init_standard_dirs();
    full_output.push_back(std::string("Static dependency analysis for: ") + argv[1]);
    scan_pe(argv[1], 0);
    printf("========================================\n");
    printf("\nWHOLE OUTPUT:\n");
    for (const std::string& line : full_output) {
        printf("%s\n", line.c_str());
    }

    printf("========================================\n");
    printf("ANALYSIS SUMMARY:\n");
    printf("========================================\n");
    printf("Total unique dependencies scanned: %zu\n", visited.size());
    printf("Missing libraries: %zu\n", missing.size());

    if (!missing.empty()) {
        printf("Status: CRITICAL - Missing libraries found!\n");
        printf("========================================\n");
        printf("NOT FOUND:\n");
        printf("========================================\n");
        std::list<std::string> missing_sorted = missing;
        missing_sorted.sort();
        for (const std::string& lib : missing_sorted) {
            printf("  [!] %s\n", lib.c_str());
        }
        printf("========================================\n");
    } else {
        printf("Status: OK - All analyzed dependencies are present.\n");
    }

    return missing.empty() ? 0 : 1;
}