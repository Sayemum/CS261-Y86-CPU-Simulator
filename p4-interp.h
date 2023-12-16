#ifndef __CS261_P4__
#define __CS261_P4__

#include <getopt.h>
#include <stdbool.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "elf.h"
#include "y86.h"

/**
 * @brief Read register values and execute ALU operation
 *
 * @param cpu Y86 CPU structure
 * @param inst Y86 instruction structure for currently-executing instruction
 * @param cnd Pointer to boolean flag to be set for conditional jumps and moves
 * @param valA Pointer to Y86 register for passing valA to later stages
 * @returns Result of execute phase (valE)
 */
y86_reg_t decode_execute (y86_t *cpu, y86_inst_t inst, bool *cnd, y86_reg_t *valA);

/**
 * @brief Perform any memory read/write, write register values, and update the PC
 *
 * @param cpu Y86 CPU structure
 * @param inst Y86 instruction structure for currently-executing instruction
 * @param memory Pointer to beginning of the Y86 address space
 * @param cnd Flag that indicates whether a conditional jumps or move should happen
 * @param valA Register with valA from earlier stages
 * @param valE Register with valE from earlier stages
 */

void memory_wb_pc (y86_t *cpu, y86_inst_t inst, byte_t *memory,
        bool cnd, y86_reg_t valA, y86_reg_t valE);

/**
 * @brief Print the program usage text
 *
 * @param argv Array of command-line options
 */
void usage_p4 (char **argv);

/**
 * @brief Parse the command line options
 *
 * @param argc Number of command-line options
 * @param argv Array of command-line options
 * @param print_header Pointer to boolean flag for printing the file header
 * @param print_phdrs Pointer to boolean flag for printing the program headers
 * @param print_membrief Pointer to boolean flag for printing the memory briefly
 * @param print_memfull Pointer to boolean flag for printing the memory in full
 * @param disas_code Pointer to boolean flag for disassembling code segments
 * @param disas_data Pointer to boolean flag for disassembling data segments
 * @param exec_normal Pointer to boolean flag for executing the program normally
 * @param exec_debug Pointer to boolean flag for executing the program w/ debug tracing
 * @param filename Pointer to string buffer for Mini-ELF filename
 * @returns True if the command-line options were valid, false if not
 */
bool parse_command_line_p4 (int argc, char **argv,
        bool *print_header, bool *print_phdrs,
        bool *print_membrief, bool *print_memfull,
        bool *disas_code, bool *disas_data,
        bool *exec_normal, bool *exec_debug, char **filename);

/**
 * @brief Print info about a Y86 CPU to standard out
 *
 * @param cpu Pointer to Y86 CPU structure to print
 */
void dump_cpu_state (y86_t *cpu);

#endif
