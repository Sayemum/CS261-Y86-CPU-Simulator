/*
 * CS 261: Main driver
 *
 * Name: Sayemum Hassan
 */

#include "p1-check.h"
#include "p2-load.h"
#include "p3-disas.h"
#include "p4-interp.h"

int main (int argc, char **argv)
{
    //<<-- PART 1 -->>
    // parse command-line options
    bool print_header = false;
    bool print_phdrs = false;
    bool print_membrief = false;
    bool print_memfull = false;
    bool disas_code = false;
    bool disas_data = false;
    bool exec_normal = false;
    bool exec_debug = false;
    char *filename = NULL;

    if (!parse_command_line_p4(argc, argv, &print_header, &print_phdrs, 
                                &print_membrief, &print_memfull, &disas_code, &disas_data, &exec_normal, &exec_debug, &filename)) {
        exit(EXIT_FAILURE);
    }

    if (filename != NULL) {
        // open Mini-ELF binary
        FILE *openFile = fopen(filename, "r");
        if (!openFile) {
            printf("Failed to read file\n");
            exit(EXIT_FAILURE);
        }

        // P1: load and check Mini-ELF header
        elf_hdr_t hdr;
        if (!read_header(openFile, &hdr)) {
            printf("Failed to read file\n");
            exit(EXIT_FAILURE);
        }

        // P1 output
        if (print_header) {
            dump_header(&hdr);
        }


        //<<-- PART 2 -->>
        // Read all phdrs
        elf_phdr_t phdrs[hdr.e_num_phdr];

        for (uint16_t i = 0; i < hdr.e_num_phdr; i++) {
            
            // Read each phdr from file and load into elements
            if (!read_phdr(openFile, hdr.e_phdr_start + (i * 20), &phdrs[i])) {
                printf("Failed to read file\n");
                exit(EXIT_FAILURE);
            }
        }

        // Print Headers
        if (print_phdrs) {
            dump_phdrs(hdr.e_num_phdr, phdrs);
        }

        // Load Segments after Creating Memory
        byte_t* memory = (byte_t*)calloc(MEMSIZE, 1);

        for (uint16_t i = 0; i < hdr.e_num_phdr; i++) {
            // Load each phdr into memory
            if (!load_segment(openFile, memory, &phdrs[i])) {
                printf("Failed to read file\n");
                exit(EXIT_FAILURE);
            }
        }

        // Print contents of memory (if requested)
        if (print_membrief) {

            for (uint16_t i = 0; i < hdr.e_num_phdr; i++) {
                dump_memory(memory, phdrs[i].p_vaddr, phdrs[i].p_vaddr + phdrs[i].p_size);
            }

        } else if (print_memfull) {
            dump_memory(memory, 0, MEMSIZE);
        }


        //<<-- PART 3 -->>
        // Disasemble code contents (if requested)
        if (disas_code) {
            printf("Disassembly of executable contents:\n");
            for (uint16_t i = 0; i < hdr.e_num_phdr; i++) {
                disassemble_code(memory, &phdrs[i], &hdr);
            }
        }
        if (disas_data) {
            printf("Disassembly of data contents:\n");
            for (uint16_t i = 0; i < hdr.e_num_phdr; i++) {
                disassemble_data(memory, &phdrs[i]);
            }
        }


        //<<-- PART 4 -->>
        y86_t cpu;              /* main cpu struct */
        bool cnd = 0;           /* condition signal */
        y86_reg_t valA = 0;     /* intermediate register */
        y86_reg_t valE = 0;     /* intermediate register */
        uint64_t totalExecutionCount = 0;

        /* initilize cpu */
        memset(&cpu, 0, sizeof(y86_t));
        cpu.stat = AOK;
        cpu.pc = hdr.e_entry;
        cpu.zf = 0;
        cpu.sf = 0;
        cpu.of = 0;

        // initilize each register as 0
        for (uint64_t i = 0; i < NUMREGS; i++) {
            cpu.reg[i] = 0;
        }

        // Print beginning cpu state if exec_debug
        if (exec_debug) {
            printf("Beginning execution at 0x%04x\n", hdr.e_entry);
            dump_cpu_state(&cpu);
            printf("\n");
        }

        /* main cpu cycle */
        while (cpu.stat == AOK) {
            y86_inst_t ins = fetch(&cpu, memory);

            if (cpu.stat != INS && cpu.stat != ADR && ins.icode != INVALID) {
                valE = decode_execute(&cpu, ins, &cnd, &valA);

                // ERROR CHECK: decode/execute failed

                memory_wb_pc(&cpu, ins, memory, cnd, valA, valE);

                totalExecutionCount++;

                /* trace output (if requested) */
                if (exec_debug) {
                    printf("Executing: ");
                    disassemble(&ins);
                    printf("\n");
                    dump_cpu_state(&cpu);
                }

                if (exec_debug && cpu.stat == AOK) {
                    printf("\n");
                }

            } else {
                if (exec_debug) {
                    printf("Invalid instruction at 0x%04x\n", memory[cpu.pc]);
                }
            }

            
        }

        if (exec_normal) {
            printf("Beginning execution at 0x%04x\n", hdr.e_entry);
            dump_cpu_state(&cpu);
            printf("Total execution count: %lu\n", totalExecutionCount);
        }
        if (exec_debug) {
            printf("Total execution count: %lu\n\n", totalExecutionCount);
            dump_memory(memory, 0, MEMSIZE);
        }
        
        // cleanup
        free(memory);
        fclose(openFile);
    }

    return EXIT_SUCCESS;
}