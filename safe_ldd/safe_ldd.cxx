#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <elf.h>
#include <libgen.h>

#define MAX_LIBS 500
#define PATH_BUFFER_SIZE 1024

// Arrays for tracking processed paths and missing libraries
char visited[MAX_LIBS][PATH_BUFFER_SIZE];
int visited_count = 0;

char missing[MAX_LIBS][PATH_BUFFER_SIZE];
int missing_count = 0;

// Standard search paths for Debian Trixie (64-bit)
const char* standard_paths[] = {
    "/lib/x86_64-linux-gnu/",
    "/usr/lib/x86_64-linux-gnu/",
    "/usr/local/lib/",
    "/lib/",
    "/usr/lib/"
};

// Checks if a library has already been processed to prevent circular dependency infinite loops
int is_visited(const char* lib) {
    for (int i = 0; i < visited_count; i++) {
        if (strcmp(visited[i], lib) == 0) return 1;
    }
    return 0;
}

// Tracks missing libraries without duplicates
void add_missing(const char* lib) {
    for (int i = 0; i < missing_count; i++) {
        if (strcmp(missing[i], lib) == 0) return;
    }
    if (missing_count < MAX_LIBS) {
        strncpy(missing[missing_count++], lib, PATH_BUFFER_SIZE - 1);
    }
}

void scan_elf(const char* filename, int level) {
    if (is_visited(filename)) return;
    if (visited_count < MAX_LIBS) {
        strncpy(visited[visited_count++], filename, PATH_BUFFER_SIZE - 1);
    }

    FILE* f = fopen(filename, "rb");
    if (!f) return;

    // Read the main ELF header
    Elf64_Ehdr ehdr;
    if (fread(&ehdr, 1, sizeof(ehdr), f) != sizeof(ehdr)) { fclose(f); return; }
    
    // Verify magic numbers to ensure it is a valid ELF executable/library
    if (memcmp(ehdr.e_ident, ELFMAG, SELFMAG) != 0) { fclose(f); return; }

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
                char* filename_copy = strdup(filename);
                char* binary_dir = dirname(filename_copy);

                // Iterate through all entries looking for DT_NEEDED dependencies
                for (int i = 0; i < entries; i++) {
                    if (dyn_table[i].d_tag == DT_NEEDED) {
                        char* lib_name = str_table + dyn_table[i].d_un.d_val;
                        
                        // Print indentation for the visual tree hierarchy
                        for (int j = 0; j < level; j++) printf("  ");
                        
                        char found_path[PATH_BUFFER_SIZE];
                        int found = 0;

                        // 1. Try to find the library in the same directory as the executable
                        snprintf(found_path, sizeof(found_path), "%s/%s", binary_dir, lib_name);
                        FILE* t = fopen(found_path, "rb");
                        if (t) { fclose(t); found = 1; }

                        // 2. Try global standard paths if not found locally
                        if (!found) {
                            size_t num_paths = sizeof(standard_paths) / sizeof(standard_paths[0]);
                            for (size_t p = 0; p < num_paths; p++) {
                                snprintf(found_path, sizeof(found_path), "%s%s", standard_paths[p], lib_name);
                                t = fopen(found_path, "rb");
                                if (t) { fclose(t); found = 1; break; }
                            }
                        }

                        if (found) {
                            printf("-> %s (%s)\n", lib_name, found_path);
                            // Recursive step down into the sub-dependencies of the found library
                            scan_elf(found_path, level + 1);
                        } else {
                            printf("-> %s (NOT FOUND)\n", lib_name);
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
    printf("Static dependency analysis for: %s\n", argv[1]);
    scan_elf(argv[1], 0);

    // Print summary report at the very end
    printf("\n========================================\n");
    printf("ANALYSIS SUMMARY:\n");
    printf("========================================\n");
    printf("Total unique dependencies scanned: %d\n", visited_count);
    
    if (missing_count > 0) {
        printf("Status: CRITICAL - %d missing libraries found!\n\n", missing_count);
        printf("The following libraries are required but could not be located:\n");
        for (int i = 0; i < missing_count; i++) {
            printf("  [!] %s\n", missing[i]);
        }
    } else {
        printf("Status: OK - All analyzed dependencies are present.\n");
        printf("The binary should be able to start safely.\n");
    }
    printf("========================================\n");

    return missing_count > 0 ? 1 : 0;
}
