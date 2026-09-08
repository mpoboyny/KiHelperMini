#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <elf.h>
#include <libgen.h>
#include <list>
#include <string>

#define PATH_BUFFER_SIZE 1024

std::list<std::string> visited;
std::list<std::string> missing;
std::list<std::string> full_output;

// Standard search paths for Debian Trixie (64-bit)
const char* standard_paths[] = {
    "/lib/x86_64-linux-gnu/",
    "/usr/lib/x86_64-linux-gnu/",
    "/usr/local/lib/",
    "/lib/",
    "/usr/lib/"
};

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

void scan_elf(const std::string& filename, int level) {
    if (contains(visited, filename)) {
        return;
    }
    visited.push_back(filename);

    FILE* f = fopen(filename.c_str(), "rb");
    if (!f) {
        return;
    }

    // Read the main ELF header
    Elf64_Ehdr ehdr;
    if (fread(&ehdr, 1, sizeof(ehdr), f) != sizeof(ehdr)) {
        fclose(f);
        return;
    }
    
    // Verify magic numbers to ensure it is a valid ELF executable/library
    if (memcmp(ehdr.e_ident, ELFMAG, SELFMAG) != 0) {
        fclose(f);
        return;
    }

    // Search through sections to locate the Dynamic Section
    fseek(f, ehdr.e_shoff, SEEK_SET);
    Elf64_Shdr shdr;
    Elf64_Shdr dyn_shdr = {0}, str_shdr = {0};

    for (int i = 0; i < ehdr.e_shnum; i++) {
        if (fread(&shdr, 1, sizeof(shdr), f) != sizeof(shdr)) break;
        if (shdr.sh_type == SHT_DYNAMIC) dyn_shdr = shdr;
    }

    if (dyn_shdr.sh_type == SHT_DYNAMIC) {
        // Retrieve the linked string table section for dynamic entries
        fseek(f, ehdr.e_shoff + dyn_shdr.sh_link * sizeof(shdr), SEEK_SET);
        if (fread(&str_shdr, 1, sizeof(str_shdr), f) == sizeof(str_shdr)) {
            
            int entries = dyn_shdr.sh_size / sizeof(Elf64_Dyn);
            Elf64_Dyn* dyn_table = (Elf64_Dyn*)malloc(dyn_shdr.sh_size);
            char* str_table = (char*)malloc(str_shdr.sh_size);

            if (dyn_table && str_table) {
                fseek(f, dyn_shdr.sh_offset, SEEK_SET);
                fread(dyn_table, 1, dyn_shdr.sh_size, f);
                fseek(f, str_shdr.sh_offset, SEEK_SET);
                fread(str_table, 1, str_shdr.sh_size, f);

                // Extract the directory of the current binary to check for local libs
                char* filename_copy = strdup(filename.c_str());
                char* binary_dir = dirname(filename_copy);

                // Iterate through all entries looking for DT_NEEDED dependencies
                for (int i = 0; i < entries; i++) {
                    if (dyn_table[i].d_tag == DT_NEEDED) {
                        const char* lib_name_ptr = str_table + dyn_table[i].d_un.d_val;
                        std::string lib_name = lib_name_ptr;
                        
                        char found_path[PATH_BUFFER_SIZE];
                        int found = 0;

                        // 1. Try to find the library in the same directory as the executable
                        snprintf(found_path, sizeof(found_path), "%s/%s", binary_dir, lib_name.c_str());
                        FILE* t = fopen(found_path, "rb");
                        if (t) { fclose(t); found = 1; }

                        // 2. Try global standard paths if not found locally
                        if (!found) {
                            size_t num_paths = sizeof(standard_paths) / sizeof(standard_paths[0]);
                            for (size_t p = 0; p < num_paths; p++) {
                                snprintf(found_path, sizeof(found_path), "%s%s", standard_paths[p], lib_name.c_str());
                                t = fopen(found_path, "rb");
                                if (t) { fclose(t); found = 1; break; }
                            }
                        }

                        if (found) {
                            add_output_line(level, "-> " + lib_name + " (" + found_path + ")");
                            // Recursive step down into the sub-dependencies of the found library
                            scan_elf(found_path, level + 1);
                        } else {
                            add_output_line(level, "-> " + lib_name + " (NOT FOUND)");
                            add_missing(lib_name);
                        }
                    }
                }
                free(filename_copy);
            }
            free(dyn_table);
            free(str_table);
        }
    }
    fclose(f);
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <path_to_binary>\n", argv[0]);
        return 1;
    }
    full_output.push_back(std::string("Static dependency analysis for: ") + argv[1]);
    scan_elf(argv[1], 0);
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
