/*
 * CS 261 PA4: Mini-ELF interpreter
 *
 * Name: Sayemum Hassan
 */

#include "p4-interp.h"

/**********************************************************************
 *                         REQUIRED FUNCTIONS
 *********************************************************************/

void check_cc (y86_inst_t inst, y86_t *cpu, y86_reg_t valA, y86_reg_t valB) {
    int64_t result = 0;
    
    switch (inst.ifun.b) {
        case ADD:
            result = valB + valA;
            break;
        case SUB:
            result = valB - valA;
            break;
        case AND:
            result = valB & valA;
            break;
        case XOR:
            result = valB ^ valA;
            break;
        default:
            break;
    }



    // check for overflow ONLY for ADD and SUB, else set to 0
    switch (inst.ifun.b) {
        case ADD:
        case SUB:
            // check for zero or negative
            if (result == 0) {
                cpu->zf = 1;
            }
            if (result < 0) {
                cpu->sf = 1;
            }
            if ( (valA < 0) == (valB < 0) &&
                 (result < 0) != (valA < 0)) {
                cpu->of = 1;
            }
            break;
        default:
            // cpu->of = 0;
            break;
    }
}

bool get_cnd (y86_inst_t inst, y86_t *cpu) {
    bool result = 0;

    switch (inst.icode) {
        case JUMP:
            switch (inst.ifun.b) {
                case JMP:       result = 1;                                     break;
                case JLE:       result = (cpu->sf ^ cpu->of) | cpu->zf;         break;
                case JL:        result = cpu->sf ^ cpu->of;                     break;
                case JE:        result = cpu->zf;                               break;
                case JNE:       result = !cpu->zf;                              break;
                case JGE:       result = !(cpu->sf ^ cpu->of);                  break;
                case JG:        result = !(cpu->sf ^ cpu->of) & !cpu->zf;       break;
                // case BADJUMP:       result = 1;     break;
            }
            break;
        case CMOV:
            break;
        default:
            break;
    }

    return result;
}

y86_reg_t decode_execute (y86_t *cpu, y86_inst_t inst, bool *cnd, y86_reg_t *valA)
{
    // ERROR CHECK: invalid pointers
    if (cpu == NULL || cnd == NULL || valA == NULL) {
        // return invalid
        cpu->stat = INS;
        return 0;
    }

    // ERROR CHECK: out of bounds location
    if (cpu->pc > 0xfff) {
        cpu->stat = ADR;
        return 0;
    }

    y86_reg_t valB = 0;

    /* decode */
    switch (inst.icode) {
        case CMOV:
            *valA = cpu->reg[inst.ra];
            break;
        case OPQ:
        case RMMOVQ:
            *valA = cpu->reg[inst.ra];
            valB = cpu->reg[inst.rb];
            break;
        case MRMOVQ:
            valB = cpu->reg[inst.rb];
            break;
        case CALL:
            valB = cpu->reg[RSP];
            break;
        case PUSHQ:
            *valA = cpu->reg[inst.ra];
            valB = cpu->reg[RSP];
            break;
        case RET:
        case POPQ:
            *valA = cpu->reg[RSP];
            valB = cpu->reg[RSP];
            break;
        default:
            break;
    }


    y86_reg_t valE = 0;

    /* execute */
    switch (inst.icode) {
        case HALT:
            cpu->stat = HLT;
            break;
        case IRMOVQ:
            valE = inst.valC.v;
            break;
        case OPQ:
            // calculate valE
            switch (inst.ifun.b) {
                case ADD:
                    // valE = (int)valB + (int)*valA;
                    valE = valB + *valA;
                    check_cc(inst, cpu, *valA, valB);
                    break;
                case SUB:
                    valE = valB - *valA;
                    check_cc(inst, cpu, *valA, valB);
                    break;
                case AND:
                    valE = valB & *valA;
                    check_cc(inst, cpu, *valA, valB);
                    break;
                case XOR:
                    valE = valB ^ *valA;
                    check_cc(inst, cpu, *valA, valB);
                    break;
                case BADOP:
                    cpu->stat = INS;
                default:
                    break;
            }
            break;
        case RMMOVQ:
        case MRMOVQ:
            valE = valB + inst.valC.d;
            break;
        case CALL:
        case PUSHQ:
            valE = valB - 8;
            break;
        case RET:
        case POPQ:
            valE = valB + 8;
            break;
        case JUMP:
            *cnd = get_cnd(inst, cpu);
            // set cnd
            break;
        case CMOV:
            valE = *valA;
            *cnd = get_cnd(inst, cpu);
            // set cnd
            break;
        default:
            break;
    }

    return valE;
}

/*bool memory_out_of_bounds(byte_t *memory) {
    return 
}*/

void memory_wb_pc (y86_t *cpu, y86_inst_t inst, byte_t *memory,
        bool cnd, y86_reg_t valA, y86_reg_t valE)
{
    // ERROR CHECK: invalid pointers
    if (cpu == NULL || memory == NULL) {
        // return invalid
        cpu->stat = INS;
        return;
    }

    // ERROR CHECK: out of bounds location
    if (cpu->pc > 0xfff) {
        cpu->stat = ADR;
        return;
    }

    y86_reg_t valM = 0;

    /* memory */
    switch (inst.icode) {
        case RMMOVQ:
            memcpy(&memory[valE], &valA, 8);
            break;
        case MRMOVQ:
            if (valE + 8 > MEMSIZE) {
                cpu->stat = ADR;
                break;
            }
            memcpy(&valM, &memory[valE], 8);
            break;
        case CALL:
            memcpy(&memory[valE], &inst.valP, 8);
            break;
        case PUSHQ:
            memcpy(&memory[valE], &valA, 8);
            break;
        case RET:
        case POPQ:
            memcpy(&valM, &memory[valA], 8);
            break;
        default:
            break;
    }


    /* write back */
    switch (inst.icode) {
        case IRMOVQ:
            cpu->reg[inst.rb] = valE;
            break;
        case OPQ:
            cpu->reg[inst.rb] = valE;
            break;
        case MRMOVQ:
            if (valE + 8 > MEMSIZE) {
                break;
            }
            cpu->reg[inst.ra] = valM;
            break;
        case CALL:
        case RET:
        case PUSHQ:
            cpu->reg[RSP] = valE;
            break;
        case POPQ:
            cpu->reg[RSP] = valE;
            cpu->reg[inst.ra] = valM;
            break;
        case CMOV:
            // set cnd
            // (cnd) ? cpu->reg[inst.rb] = valE;
            break;
        default:
            break;
    }

    /* pc update */
    switch (inst.icode) {
        case JUMP:
            // condition either valC or valP
            // (cnd) ? cpu->pc = inst.valC.dest : cpu->pc = inst.valP;
            break;
        case CALL:
            // valC
            cpu->pc = inst.valC.dest;
            break;
        case RET:
            // valM
            cpu->pc = valM;
            break;
        default:
            cpu->pc = inst.valP;
            break;
    }
}

/**********************************************************************
 *                         OPTIONAL FUNCTIONS
 *********************************************************************/

void usage_p4 (char **argv)
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
    printf("  -e      Execute program\n");
    printf("  -E      Execute program (trace mode)\n");
}

bool parse_command_line_p4 (int argc, char **argv,
        bool *print_header, bool *print_phdrs,
        bool *print_membrief, bool *print_memfull,
        bool *disas_code, bool *disas_data,
        bool *exec_normal, bool *exec_debug, char **filename)
{
    // ERROR CHECK: invalid pointers
    if (print_header == NULL || print_phdrs == NULL
        || print_membrief == NULL || print_memfull == NULL
        || disas_code == NULL || disas_data == NULL
        || exec_normal == NULL || exec_debug == NULL || filename == NULL) {
        return false;
    }

    // parameter parsing w/ getopt()
    int c;
    while ((c = getopt(argc, argv, "hHafsmMdDeE")) != -1) {
        switch (c) {
            case 'h':
                usage_p4(argv);
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
            case 'e':
                *exec_normal = true;
                break;
            case 'E':
                *exec_debug = true;
                break;
            default:
                usage_p4(argv);
                return false;
        }
    }

    // Check if both -m and -M was called. It's invalid if its both.
   if (*print_membrief && *print_memfull) {
        usage_p4(argv);
        return false;
   }

    // Check if both -e and -E was called. It's invalid if its both.
   if (*exec_normal && *exec_debug) {
        usage_p4(argv);
        return false;
   }

   if (optind != argc-1) {
        // no filename (or extraneous input)
        usage_p4(argv);
        return false;
    }
    *filename = argv[optind];   // save filename

    return true;
}

void dump_cpu_state (y86_t *cpu)
{
    // ERROR CHECK: invalid pointers
    if (cpu == NULL) {
        // return invalid
        printf("Invalid arguments.");
        exit(EXIT_FAILURE);
    }

    printf("Y86 CPU state:\n");

    printf("    PC: %016lx   flags: Z%d S%d O%d     ", cpu->pc, cpu->zf, cpu->sf, cpu->of);

    // Print Y86 CPU Status
    switch (cpu->stat) {
        case HLT:
            printf("HLT\n");
            break;
        case ADR:
            printf("ADR\n");
            break;
        case INS:
            printf("INS\n");
            break;
        default:
            printf("AOK\n");
            break;
    }
    
    printf("  %%rax: %016lx    %%rcx: %016lx\n", cpu->reg[RAX], cpu->reg[RCX]);
    printf("  %%rdx: %016lx    %%rbx: %016lx\n", cpu->reg[RDX], cpu->reg[RBX]);
    printf("  %%rsp: %016lx    %%rbp: %016lx\n", cpu->reg[RSP], cpu->reg[RBP]);
    printf("  %%rsi: %016lx    %%rdi: %016lx\n", cpu->reg[RSI], cpu->reg[RDI]);
    printf("   %%r8: %016lx     %%r9: %016lx\n", cpu->reg[R8], cpu->reg[R9]);
    printf("  %%r10: %016lx    %%r11: %016lx\n", cpu->reg[R10], cpu->reg[R11]);
    printf("  %%r12: %016lx    %%r13: %016lx\n", cpu->reg[R12], cpu->reg[R13]);
    printf("  %%r14: %016lx\n", cpu->reg[R14]);
}