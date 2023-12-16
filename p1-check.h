#ifndef __CS261_P1__
#define __CS261_P1__

#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "elf.h"

/**
 * @brief Load a Mini-ELF header from an open file stream
 *
 * @param file File stream to use for input
 * @param hdr Pointer to region where the Mini-ELF header should be loaded
 * @returns True if the header was successfully loaded and verified, false otherwise
 */
bool read_header (FILE *file, elf_hdr_t *hdr);

/**
 * @brief Print the program usage text
 *
 * @param argv Array of command-line options
 */
void usage_p1 (char **argv);

/**
 * @brief Parse the command line options
 *
 * @param argc Number of command-line options
 * @param argv Array of command-line options
 * @param print_header Pointer to boolean flag for printing the file header
 * @param filename Pointer to string buffer for Mini-ELF filename
 * @returns True if the command-line options were valid, false if not
 */
bool parse_command_line_p1 (int argc, char **argv, bool *print_header, char **filename);

/**
 * @brief Print Mini-ELF header information to standard out
 *
 * @param hdr Header with info to print
 */
void dump_header (elf_hdr_t *hdr);

#endif
