/*
 * CS 261 PA3: Mini-ELF disassembler
 *
 * Name: Sayemum Hassan
 */

#include "p3-disas.h"

/**********************************************************************
 *                         REQUIRED FUNCTIONS
 *********************************************************************/

/* NOTES
1. First phase of Von Neuman Y86 Instruction
2. Pull data (bytes) from memory and populate the y86_inst_t struct
3. cpu is just a container for the program counter, which tells us where in memory we need to fetch the instruction from
*/
y86_inst_t fetch (y86_t *cpu, byte_t *memory)
{
    y86_inst_t ins;

    // Check for NULL pointers
    if (cpu == NULL || memory == NULL) {
        ins.icode = INVALID;
        cpu->stat = INS;
        return ins;
    }

    // ERROR CHECK: out of bounds location
    if (cpu->pc > 0xfff) {
        ins.icode = INVALID;
        cpu->stat = ADR;
        return ins;
    }

    // First byte (icode/ifun)
    ins.icode = memory[cpu -> pc] >> 4;     // High order 4 bits
    ins.ifun.b = memory[cpu -> pc] & 0xf;  // Low order 4 bits

    // Switch statements for updating valP
    switch (ins.icode) {
        case HALT:
        case NOP:
        case RET:
        case IOTRAP:
            ins.valP = cpu->pc + 1;
            break;
        case CMOV:
        case OPQ:
        case PUSHQ:
        case POPQ:
            ins.valP = cpu->pc + 2;
            break;
        case JUMP:
        case CALL:
            ins.valP = cpu->pc + 9;
            break;
        case IRMOVQ:
        case RMMOVQ:
        case MRMOVQ:
            ins.valP = cpu->pc + 10;
            break;
        default:
            ins.icode = INVALID;
            cpu->stat = INS;
            return ins;
    }

    // ERROR CHECK: All first-byte instructions having "0" low order 4 bits ifun (except for the cases that don't have 0 ifun)
    if (ins.ifun.b != 0) {
        switch (ins.icode) {
            case HALT:
            case NOP:
            case RET:
            case IRMOVQ:
            case RMMOVQ:
            case MRMOVQ:
            case CALL:
            case PUSHQ:
            case POPQ:
                ins.icode = INVALID;
                cpu->stat = INS;
                return ins;
            case CMOV:
            case OPQ:
            case JUMP:
            case IOTRAP:
            case INVALID:
                break;
        }
    }

    // ERROR CHECK: specific fn ifun like cmovXX and jXX
    switch (ins.icode) {
        case HALT:
        case NOP:
        case RET:
        case IRMOVQ:
        case RMMOVQ:
        case MRMOVQ:
        case CALL:
        case PUSHQ:
        case POPQ:
            break;
        case CMOV:
            if (ins.ifun.b < 0 || ins.ifun.b > 6) {
                ins.icode = INVALID;
                cpu->stat = INS;
                return ins;
            }
            break;
        case OPQ:
            if (ins.ifun.b < 0 || ins.ifun.b > 3) {
                ins.icode = INVALID;
                cpu->stat = INS;
                return ins;
            }
            break;
        case JUMP:
            if (ins.ifun.b < 0 || ins.ifun.b > 6) {
                ins.icode = INVALID;
                cpu->stat = INS;
                return ins;
            }
            break;
        case IOTRAP:
            if (ins.ifun.b < 0 || ins.ifun.b > 5) {
                ins.icode = INVALID;
                cpu->stat = INS;
                return ins;
            }
            break;
        case INVALID:
            ins.icode = INVALID;
            cpu->stat = INS;
            return ins;
    }

    //ERROR CHECK: out of bounds for second byte
    if (ins.icode != HALT && ins.icode != NOP && ins.icode != RET && ins.icode != IOTRAP) {
        if (cpu->pc + 1 > 0xfff) {
            ins.icode = INVALID;
            cpu->stat = ADR;
            return ins;
        }
    }
    

    // Second byte (rA and rB) (or fn/valC depending on context) [for jump and call, the next 9 bytes are the second byte]
    switch (ins.icode) {
        case CMOV:
        case RMMOVQ:
        case MRMOVQ:
        case OPQ:
        case PUSHQ:
        case POPQ:
        case IRMOVQ:
            ins.ra = memory[cpu->pc + 1] >> 4;
            ins.rb = memory[cpu->pc + 1] & 0xf;
            break;
        case JUMP:
        case CALL:
            memcpy(&ins.valC.dest, &memory[cpu->pc + 1], 8);
            break;
        case HALT:
        case NOP:
        case RET:
        case IOTRAP:
        case INVALID:
            break;
    }


    // ERROR CHECK: out of bounds location for rest of bytes
    switch (ins.icode) {
        case HALT:
        case NOP:
        case RET:
        case IOTRAP:
        case INVALID:
            break;
        case CMOV:
        case IRMOVQ:
        case RMMOVQ:
        case MRMOVQ:
        case OPQ:
        case JUMP:
        case CALL:
        case PUSHQ:
        case POPQ:
            if (cpu->pc + 2 > 0xfff) {
                ins.icode = INVALID;
                cpu->stat = ADR;
                return ins;
            }
            break;
    }

    // ERROR CHECK: 0xf for second byte on some instructions (irmovq, pushq, popq)
    switch (ins.icode) {
        case HALT:
        case NOP:
        case RET:
        case IOTRAP:
        case INVALID:
        case CMOV:
        case RMMOVQ:
        case MRMOVQ:
        case OPQ:
        case JUMP:
        case CALL:
            break;
        case IRMOVQ:
            if (ins.ra != 0xf) {
                ins.icode = INVALID;
                cpu->stat = INS;
                return ins;
            }
            break;
        case PUSHQ:
        case POPQ:
            if (ins.rb != 0xf) {
                ins.icode = INVALID;
                cpu->stat = INS;
                return ins;
            }
            break;
    }

    // valC Third byte (irmovq, rmmovq, mrmovq)
    switch (ins.icode) {
        case IRMOVQ:
            // ins.valC.v = memory[cpu->pc + 2];
            memcpy(&ins.valC.v, &memory[cpu->pc + 2], 8);
            break;
        case RMMOVQ:
        case MRMOVQ:
            memcpy(&ins.valC.d, &memory[cpu->pc + 2], 8);
            break;
        case NOP:
        case CMOV:
        case OPQ:
        case JUMP:
        case CALL:
        case RET:
        case PUSHQ:
        case POPQ:
        case IOTRAP:
        case INVALID:
        case HALT:
            break;
    }

    cpu->stat = AOK;

    // Check if instruction is a halt and update cpu status
    if (ins.icode == HALT) {
        cpu->stat = HLT;
    }

    return ins;
}

/**********************************************************************
 *                         OPTIONAL FUNCTIONS
 *********************************************************************/

void usage_p3 (char **argv)
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
    printf("  -d      Disassemble code contents\n");
    printf("  -D      Disassemble data contents\n");
}

bool parse_command_line_p3 (int argc, char **argv,
        bool *print_header, bool *print_phdrs,
        bool *print_membrief, bool *print_memfull,
        bool *disas_code, bool *disas_data, char **filename)
{
    // Check for null pointers
    if (print_header == NULL || print_phdrs == NULL
        || print_membrief == NULL || print_memfull == NULL
        || disas_code == NULL || disas_data == NULL || filename == NULL) {
        return false;
    }

    // parameter parsing w/ getopt()
    int c;
    while ((c = getopt(argc, argv, "hHafsmMdD")) != -1) {
        switch (c) {
            case 'h':
                usage_p3(argv);
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
            case 'd':
                *disas_code = true;
                break;
            case 'D':
                *disas_data = true;
                break;
            default:
                usage_p3(argv);
                return false;
        }
    }

    // Check if both -m and -M was called. It's invalid if its both.
   if (*print_membrief && *print_memfull) {
        usage_p3(argv);
        return false;
   }

    if (optind != argc-1) {
        // no filename (or extraneous input)
        usage_p3(argv);
        return false;
    }
    *filename = argv[optind];   // save filename

    return true;
}

void printASMFromREGNUM(y86_regnum_t operand) {
    switch (operand) {
        case RAX:       printf("%%rax"); break;
        case RCX:       printf("%%rcx"); break;
        case RDX:       printf("%%rdx"); break;
        case RBX:       printf("%%rbx"); break;
        case RSP:       printf("%%rsp"); break;
        case RBP:       printf("%%rbp"); break;
        case RSI:       printf("%%rsi"); break;
        case RDI:       printf("%%rdi"); break;
        case R8:        printf("%%r8"); break;
        case R9:        printf("%%r9"); break;
        case R10:       printf("%%r10"); break;
        case R11:       printf("%%r11"); break;
        case R12:       printf("%%r12"); break;
        case R13:       printf("%%r13"); break;
        case R14:       printf("%%r14"); break;
        default:        printf("NOREG"); break;
    }
}


/* NOTES
1. used to disassemble one instruction, so this needs to be repeatedly called for multiple instructions
2. This ONLY PRINTS THE ASSEMBLY TRANSLATION AND NOTHING ELSE (ex. noop, halt)
*/
void disassemble (y86_inst_t *inst)
{
    // Check for NULL pointers
    if (inst == NULL) {
        return;
    }

    // Print appropriate icode
    switch (inst->icode) {
        case HALT:      printf("halt"); break;
        case NOP:       printf("nop"); break;
        case RET:       printf("ret"); break;
        case CMOV:
            // figure out what kind of cmov this is
            switch(inst->ifun.b) {
                case RRMOVQ:    printf("rrmovq"); break;
                case CMOVLE:    printf("cmovle"); break;
                case CMOVL:     printf("cmovl"); break;
                case CMOVE:     printf("cmove"); break;
                case CMOVNE:    printf("cmovne"); break;
                case CMOVGE:    printf("cmovge"); break;
                case CMOVG:     printf("cmovg"); break;
                case BADCMOV:   printf("badcmov"); break;
            }

            break;
        case IRMOVQ:    printf("irmovq"); break;
        case RMMOVQ:    printf("rmmovq"); break;
        case MRMOVQ:    printf("mrmovq"); break;
        case OPQ:
            // figure out the operation and put it before the q
            switch(inst->ifun.b) {
                case ADD:       printf("addq"); break;
                case SUB:       printf("subq"); break;
                case AND:       printf("andq"); break;
                case XOR:       printf("xorq"); break;
                case BADOP:     printf("badop"); break;
            }
            
            break;
        case JUMP:
            // figure out what kind of jump this is
            switch(inst->ifun.b) {
                case JMP:       printf("jmp"); break;
                case JLE:       printf("jle"); break;
                case JL:        printf("jl"); break;
                case JE:        printf("je"); break;
                case JNE:       printf("jne"); break;
                case JGE:       printf("jge"); break;
                case JG:        printf("jg"); break;
                case BADJUMP:   printf("badjump"); break;
            }
            break;
        case CALL:      printf("call"); break;
        case PUSHQ:     printf("pushq"); break;
        case POPQ:      printf("popq"); break;
        case IOTRAP:    printf("iotrap"); break;
        case INVALID:   printf("invalid"); break;
    }


    // Print appropriate ifunctions
    switch (inst->icode) {
        case CMOV:
            printf(" ");
            printASMFromREGNUM(inst->ra);
            printf(", ");
            printASMFromREGNUM(inst->rb);
            break;
        case IOTRAP:
            printf(" %d", inst->ifun.trap);
            break;
        case RMMOVQ:
            // check if register is NOREG, if so then don't print register
            printf(" ");
            printASMFromREGNUM(inst->ra);
            printf(", ");
            printf("0x%lx", inst->valC.d);
            if (inst->rb != NOREG) {
                printf("(");
                printASMFromREGNUM(inst->rb);
                printf(")");
            }
            break;
        case MRMOVQ:
            printf(" ");
            printf("0x%lx", inst->valC.d);
            if (inst->rb != NOREG) {
                printf("(");
                printASMFromREGNUM(inst->rb);
                printf(")");
            }
            printf(", ");
            printASMFromREGNUM(inst->ra);
            break;
        case OPQ:
            printf(" ");
            printASMFromREGNUM(inst->ra);
            printf(", ");
            printASMFromREGNUM(inst->rb);
            break;
        case IRMOVQ:
            //f, rB
            printf(" ");
            printf("0x%lx", inst->valC.v);
            printf(", ");
            printASMFromREGNUM(inst->rb);
            break;
        case PUSHQ:
        case POPQ:
            //rA, f
            printf(" ");
            printASMFromREGNUM(inst->ra);
            break;
        case CALL:
        case JUMP:
            printf(" ");
            printf("0x%lx", inst->valC.dest);
            break;
        case HALT:
        case NOP:
        case RET:
        case INVALID:
            break;
    }
}

void print_format_raw_bytes(byte_t* memory, y86_t cpu, y86_inst_t ins) {
    // Check for NULL pointers
    if (memory == NULL) {
        return;
    }

    // Print all bytes
    switch (ins.icode) {
        case HALT:
        case NOP:
        case RET:
        case IOTRAP:
            printf("%02x                            ", memory[cpu.pc]);
            break;
        case CMOV:
        case OPQ:
        case PUSHQ:
        case POPQ:
            printf("%02x ", memory[cpu.pc]);
            printf("%02x                         ", memory[cpu.pc+1]);
            break;
        case JUMP:
        case CALL:
            for (uint64_t offset = 0; offset <= 8; offset++) {
                printf("%02x ", memory[cpu.pc + offset]);
            }
            printf("   ");
            break;
        case IRMOVQ:
        case RMMOVQ:
        case MRMOVQ:
            for (uint64_t offset = 0; offset <= 9; offset++) {
                printf("%02x ", memory[cpu.pc + offset]);
            }
            break;
        case INVALID:
            break;
    }

    printf("|   ");
}

/* NOTES
1. used to disassemble EVERYTHING and prints an entire code segment with memory on the left and assembly on the right
2. memory is a pointer to all virtual address memory space
3. phdr is pointer to program header for the segment that we need to print
4. hdr is pointer to main mini-elf header
5. This will have a loop that prints every instruction inside the phdr segment
6. USE the TEMPLATE provided
*/
void disassemble_code (byte_t *memory, elf_phdr_t *phdr, elf_hdr_t *hdr)
{
    // Check for NULL pointers
    if (memory == NULL || phdr == NULL || hdr == NULL) {
        printf("Failed to disassemble code\n");
        exit(EXIT_FAILURE);
    }

    y86_t cpu;          // CPU struct to store "fake" PC
    y86_inst_t ins;     // struct to hold fetched instruction

    // start at beginning of segment
    cpu.pc = phdr -> p_vaddr;
    printf("  0x%03lx:                               | .pos 0x%03lx code\n", cpu.pc, cpu.pc);

    // iterate through the segment one instruction at a time
    while (cpu.pc < phdr->p_vaddr + phdr->p_size) {     // while pc < end_of_segment
        ins = fetch(&cpu, memory);      // stage 1: fetch instruction

        // abort with error if instruction was invalid
        if (cpu.stat == INS || ins.icode == INVALID) {
            printf("Invalid opcode: 0x%02x\n", memory[cpu.pc]);
            break;
        }

        if (cpu.pc == hdr->e_entry) {
            printf("  0x%03lx:                               | _start:\n", cpu.pc);
        }

        // print current address and raw bytes of instruction
        printf("  0x%03lx: ", cpu.pc);
        print_format_raw_bytes(memory, cpu, ins);

        disassemble(&ins);               // stage 2: print disassembly
        printf("\n");
        cpu.pc = ins.valP;              // stage 3: update PC (go to next instruction)
    }
    printf("\n");
}


void printReadWriteDataContents(byte_t *memory, y86_t cpu, y86_inst_t ins) {
    // Print little-endian so start from smallest to largest bytes
    switch (ins.icode) {
        case HALT:
        case NOP:
        case RET:
        case IOTRAP:
            printf("%02x", memory[cpu.pc]);
            break;
        case CMOV:
        case OPQ:
        case PUSHQ:
        case POPQ:
            printf("%02x", memory[cpu.pc+1]);
            printf("%02x", memory[cpu.pc]);
            break;
        case JUMP:
        case CALL:
            for (int64_t offset = 8; offset >= 0; offset--) {
                if (memory[cpu.pc + offset] != 0) {
                    printf("%02x", memory[cpu.pc + offset]);
                }
            }
            break;
        case IRMOVQ:
        case RMMOVQ:
        case MRMOVQ:
            for (int64_t offset = 9; offset >= 0; offset--) {
                if (memory[cpu.pc + offset] != 0) {
                    printf("%02x", memory[cpu.pc + offset]);
                }
            }
            break;
        case INVALID:
            break;
    }
}
void disassemble_data (byte_t *memory, elf_phdr_t *phdr)
{
    // Check for NULL pointers
    if (memory == NULL || phdr == NULL) {
        printf("Failed to disassemble data\n");
        exit(EXIT_FAILURE);
    }

    y86_t cpu;          // CPU struct to store "fake" PC
    y86_inst_t ins;     // struct to hold fetched instruction

    // start at beginning of segment
    cpu.pc = phdr -> p_vaddr;

    printf("  0x%03x:                               | .pos 0x%03x data\n", phdr->p_vaddr, phdr->p_vaddr);

    // iterate through the segment one instruction at a time
    while (cpu.pc < phdr->p_vaddr + phdr->p_size) {     // while pc < end_of_segment
        ins = fetch(&cpu, memory);      // stage 1: fetch instruction

        // abort with error if instruction was invalid
        if (cpu.stat == INS || ins.icode == INVALID) {
            printf("Invalid opcode: 0x%02x\n", memory[cpu.pc]);
            break;
        }

        // print current address and raw bytes of instruction
        printf("  0x%03lx: ", cpu.pc);
        print_format_raw_bytes(memory, cpu, ins);

        // disassemble(&ins);               // stage 2: print disassembly
        printf(".quad 0x");
        printReadWriteDataContents(memory, cpu, ins);
        printf("\n");
        cpu.pc = ins.valP;              // stage 3: update PC (go to next instruction)
    }
    printf("\n");
}

void disassemble_rodata (byte_t *memory, elf_phdr_t *phdr)
{
    // Check for NULL pointers
    if (memory == NULL || phdr == NULL) {
        printf("Failed to disassemble rodata\n");
        exit(EXIT_FAILURE);
    }

    y86_t cpu;          // CPU struct to store "fake" PC
    y86_inst_t ins;     // struct to hold fetched instruction

    // start at beginning of segment
    cpu.pc = phdr -> p_vaddr;

    printf("Disassembly of data contents:\n");
    printf("  0x%03x:                               | .pos 0x%03x rodata\n", phdr->p_vaddr, phdr->p_vaddr);

    // iterate through the segment one instruction at a time
    while (cpu.pc < phdr->p_vaddr + phdr->p_size) {     // while pc < end_of_segment
        ins = fetch(&cpu, memory);      // stage 1: fetch instruction

        // abort with error if instruction was invalid
        if (cpu.stat == INS || ins.icode == INVALID) {
            printf("Invalid opcode: 0x%02x\n", memory[cpu.pc]);
            break;
        }
        
        printf("  0x%03lx: ", cpu.pc);
        print_format_raw_bytes(memory, cpu, ins);

        printf(".string \"");
        printReadWriteDataContents(memory, cpu, ins);
        printf("\n");
        cpu.pc = ins.valP;              // stage 3: update PC (go to next instruction)
    }
    printf("\n");
}