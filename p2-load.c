/*
 * CS 261 PA2: Mini-ELF loader
 *
 * Name: Sayemum Hassan
 */

#include "p2-load.h"

/**********************************************************************
 *                         REQUIRED FUNCTIONS
 *********************************************************************/

bool read_phdr (FILE *file, uint16_t offset, elf_phdr_t *phdr)
{
    if (file == NULL || phdr == NULL) {
        return false;
    }

    int seek_success = fseek(file, offset, SEEK_SET);
    if (seek_success != 0) {
        return false;
    }

    size_t bytes_read = fread(phdr, sizeof(elf_phdr_t), 1, file);

    // Check if there was a problem reading the header (valid magic number and file big enough), if not, then return true
    return (bytes_read == 1) && (phdr -> magic == 3735928559);
}

bool load_segment (FILE *file, byte_t *memory, elf_phdr_t *phdr)
{
    if (file == NULL || memory == NULL || phdr == NULL) {
        return false;
    }

    // Illegal virtual address
    if (phdr -> p_vaddr < 0 || phdr -> p_vaddr + phdr -> p_size >= MEMSIZE) {
        return false;
    }

    int seek_success = fseek(file, phdr -> p_offset, SEEK_SET);
    if (seek_success != 0) {
        return false;
    }

    size_t bytes_read = fread(&memory[phdr -> p_vaddr], phdr -> p_size, 1, file);
    if (bytes_read != 1 && phdr->p_size != 0) {
        return false;
    }

    // Check if there was a problem reading the header (valid magic number and file big enough), if not, then return true
    return (phdr -> magic == 3735928559);
}

/**********************************************************************
 *                         OPTIONAL FUNCTIONS
 *********************************************************************/

void usage_p2 (char **argv)
{
    printf("Usage: %s <option(s)> mini-elf-file\n", argv[0]);
    printf(" Options are:\n");
    printf("  -h      Display usage\n");
    printf("  -H      Show the Mini-ELF header\n");
    printf("  -a      Show all with brief memory\n");
    printf("  -f      Show all with full memory\n");
    printf("  -s      Show the program headers\n");
    printf("  -m      Show the memory contents (brief)\n");
    printf("  -M      Show the memory contents (full)\n");
}

bool parse_command_line_p2 (int argc, char **argv,
        bool *print_header, bool *print_phdrs,
        bool *print_membrief, bool *print_memfull,
        char **filename)
{
    // Check for null pointers
    if (print_header == NULL || print_phdrs == NULL
        || print_membrief == NULL || print_memfull == NULL
        || filename == NULL) {
        return false;
    }

    // parameter parsing w/ getopt()
    int c;
    while ((c = getopt(argc, argv, "hHafsmM")) != -1) {
        switch (c) {
            case 'h':
                usage_p2(argv);
                return true;
            case 'H':
                *print_header = true;
                break;
            case 'a':
                *print_header = true;
                *print_phdrs = true;
                *print_membrief = true;
                break;
            case 'f':
                *print_header = true;
                *print_phdrs = true;
                *print_memfull = true;
            case 's':
                *print_phdrs = true;
                break;
            case 'm':
                *print_membrief = true;
                break;
            case 'M':
                *print_memfull = true;
                break;
            default:
                usage_p2(argv);
                return false;
        }
    }

    // Check if both -m and -M was called. It's invalid if its both.
   if (*print_membrief && *print_memfull) {
        usage_p2(argv);
        return false;
   }

    if (optind != argc-1) {
        // no filename (or extraneous input)
        usage_p2(argv);
        return false;
    }
    *filename = argv[optind];   // save filename

    return true;
}

void dump_phdrs (uint16_t numphdrs, elf_phdr_t *phdrs)
{
    // Print header labels
    printf(" Segment   Offset    Size      VirtAddr  Type      Flags\n");

    // Print content of each header
    for (uint16_t i = 0; i < numphdrs; i++) {

        printf("  %02d       ", i);
        printf("0x%04x    ", phdrs[i].p_offset);
        printf("0x%04x    ", phdrs[i].p_size);
        printf("0x%04x    ", phdrs[i].p_vaddr);

        // Print Type
        if (phdrs[i].p_type == 0x1) {
            printf("CODE      ");

        } else if (phdrs[i].p_type == 0x0) {
            printf("DATA      ");
        } else if (phdrs[i].p_type == 0x2) {
            printf("STACK     ");
        }

        // Print Flag
        uint16_t flag_num = phdrs[i].p_flags;
        if (flag_num - 4 >= 0) {printf("R"); flag_num -= 4;} else {printf(" ");}
        if (flag_num - 2 >= 0) {printf("W"); flag_num -= 2;} else {printf(" ");}
        if (flag_num - 1 >= 0) {printf("X"); flag_num -= 1;} else {printf(" ");}

        printf("\n");
    }
}

void dump_memory (byte_t *memory, uint16_t start, uint16_t end)
{
    printf("Contents of memory from %04x to %04x:\n", start, end);
    for (uint16_t i = start; i < end; i++) {
        // Print memory address of every mutliple of 16 bytes
        if (i % 16 == 0 || i == start) {
            printf("  %04x  ", i);
        }

        printf("%02x", memory[i]);

        if (i % 16 == 7) { printf(" "); }

        if (i % 16 == 15 || i == end-1) {
            printf("\n");
        } else {
            printf(" ");
        }
    }
}

