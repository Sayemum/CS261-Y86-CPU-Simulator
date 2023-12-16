/*
 * CS 261 PA1: Mini-ELF header verifier
 *
 * Name: Sayemum Hassan
 */

#include "p1-check.h"

/**********************************************************************
 *                         REQUIRED FUNCTIONS
 *********************************************************************/

bool read_header (FILE *file, elf_hdr_t *hdr)
{
    if (hdr == NULL) {
        return false;
    }

    size_t bytes_read = fread(hdr, sizeof(elf_hdr_t), 1, file);

    // Check if there was a problem reading the header (valid magic number and file big enough), if not, then return true
    return (bytes_read == 1) && (hdr -> magic == 4607045);
}

/**********************************************************************
 *                         OPTIONAL FUNCTIONS
 *********************************************************************/

void usage_p1 (char **argv)
{
    printf("Usage: %s <option(s)> mini-elf-file\n", argv[0]);
    printf(" Options are:\n");
    printf("  -h      Display usage\n");
    printf("  -H      Show the Mini-ELF header\n");
}

bool parse_command_line_p1 (int argc, char **argv, bool *print_header, char **filename)
{
   // Check for null pointers
    if (print_header == NULL || filename == NULL) {
        return false;
    }

    // Parse command flags using getopt()
   bool exit_successful = true;
   int c;
   while ((c = getopt(argc, argv, "hH")) != -1) {
    switch (c) {
        case 'h':
            return exit_successful;
        case 'H':
            *print_header = true;
            break;
        default:
            break;
    }
   }

   // Check for only one additional argument, else return invalid
    if (optind == argc - 1) {
        *filename = argv[optind];
    } else {
        exit_successful = false;
    }

    return exit_successful;
}

void dump_header (elf_hdr_t *hdr)
{
    // Print first 16 bytes of the file
    for (int i = 0; i < 16; i++) {
        if (i == 8) {
            printf(" ");
        }

        printf("%02x", ((uint8_t *)hdr)[i]);

        if (i != 15) {
            printf(" ");
        }
    }
    printf("\n");

    // Print version
    printf("Mini-ELF version %x\n", hdr -> e_version);

    // Print entry point
    printf("Entry point 0x%x\n", hdr -> e_entry);

    // Print program headers
    printf("There are %x program headers, starting at offset %d (0x%x)\n", hdr -> e_num_phdr, hdr -> e_phdr_start, hdr -> e_phdr_start);

    // Check and print symbol table
    if (hdr -> e_symtab != 0) {
        printf("There is a symbol table starting at offset %d (0x%x)\n", hdr -> e_symtab, hdr -> e_symtab);
    } else {
        printf("There is no symbol table present\n");
    }

    // Check and print string table
    if (hdr -> e_strtab != 0) {
        printf("There is a string table starting at offset %d (0x%x)\n", hdr -> e_strtab, hdr -> e_strtab);
    } else {
        printf("There is no string table present\n");
    }
}

