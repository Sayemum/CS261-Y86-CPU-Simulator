#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <check.h>

#include "../p4-interp.h"

/* check decoding and execution of the HALT instruction */
START_TEST (D_func_decode_halt)
{
    y86_t cpu;
    y86_inst_t inst;
    bool cnd = false;
    y86_reg_t valA = 0;
    y86_reg_t valE = 0;

    memset (&inst, 0, sizeof (inst));
    inst.icode = HALT;

    memset (&cpu, 0, sizeof (cpu));
    cpu.stat = AOK;

    valE = decode_execute (&cpu, inst, &cnd, &valA);
    ck_assert (valE == 0 || valE != 0);
    ck_assert (cpu.stat == HLT);
}
END_TEST

/* check decoding and execution of the NOP instruction */
START_TEST (D_func_decode_nop)
{
    y86_t cpu;
    y86_inst_t inst;
    bool cnd = false;
    y86_reg_t valA = 0;
    y86_reg_t valE = 0;

    memset (&inst, 0, sizeof (inst));
    inst.icode = NOP;

    memset (&cpu, 0, sizeof (cpu));
    cpu.stat = AOK;

    valE = decode_execute (&cpu, inst, &cnd, &valA);
    ck_assert (valE == 0 || valE != 0);
    ck_assert (cpu.stat == AOK);
}
END_TEST

/* check memory, writeback, and PC update of the HALT instruction */
START_TEST (D_func_memory_halt)
{
    y86_t cpu;
    byte_t *memory = malloc (MEMSIZE);
    y86_inst_t inst;
    bool cnd = false;
    y86_reg_t valA;
    y86_reg_t valE;

    memset (&cpu, 0, sizeof (cpu));
    memset (memory, 0, MEMSIZE);
    memset (&inst, 0, sizeof (inst));
    cpu.stat = AOK;
    valA = valE = 0;

    inst.icode = HALT;
    inst.valP = rand() % MEMSIZE;
    memory_wb_pc (&cpu, inst, memory, cnd, valA, valE);
    ck_assert (cpu.pc == inst.valP);
}
END_TEST

/* check memory, writeback, and PC update of the NOP instruction */
START_TEST (D_func_memory_nop)
{
    y86_t cpu;
    byte_t *memory = malloc (MEMSIZE);
    y86_inst_t inst;
    bool cnd = false;
    y86_reg_t valA;
    y86_reg_t valE;

    memset (&cpu, 0, sizeof (cpu));
    memset (memory, 0, MEMSIZE);
    memset (&inst, 0, sizeof (inst));
    cpu.stat = AOK;
    valA = valE = 0;

    inst.icode = NOP;
    inst.valP = rand() % MEMSIZE;
    memory_wb_pc (&cpu, inst, memory, cnd, valA, valE);
    ck_assert (cpu.pc == inst.valP);
}
END_TEST

/* check for null-pointer handling */
START_TEST (D_error_decode_NULL_cnd_valA)
{
    y86_t cpu;
    y86_inst_t inst;
    y86_reg_t valE = 0;

    memset (&inst, 0, sizeof (inst));
    memset (&cpu, 0, sizeof (cpu));
    cpu.stat = AOK;

    inst.icode = NOP;
    valE = decode_execute (&cpu, inst, NULL, NULL);
    ck_assert (valE == 0 || valE != 0);
    ck_assert (cpu.stat == INS);
}
END_TEST

/* helper function: extract the given register value from a CPU state */
static y86_reg_t _check_get_reg (y86_t *cpu, y86_regnum_t num)
{
    return cpu->reg[num];
}

/* check decoding and execution of the IRMOVQ instruction */
START_TEST (C_func_decode_irmovq)
{
    y86_t cpu;
    y86_inst_t inst;
    bool cnd = false;
    y86_reg_t valA = 0;
    y86_reg_t valE = 0;

    /* assemble test instruction and initial CPU state */
    memset (&inst, 0, sizeof (inst));
    inst.icode = IRMOVQ;
    memset (&cpu, 0, sizeof (cpu));
    cpu.stat = AOK;
    inst.valC.v = 0 + rand();
    inst.rb = (y86_regnum_t)(rand() % NOREG);		/* random register */

    /* test: load a positive value */
    valE = decode_execute (&cpu, inst, &cnd, &valA);
    ck_assert (valE == (y86_reg_t)inst.valC.v);
    ck_assert (cpu.sf == 0);
    ck_assert (cpu.zf == 0);
    ck_assert (_check_get_reg (&cpu, inst.rb) == 0);

    /* test: load a negative value */
    inst.valC.v = -inst.valC.v;
    valE = decode_execute (&cpu, inst, &cnd, &valA);
    ck_assert (valE == (y86_reg_t)inst.valC.v);
    ck_assert (cpu.sf == 0);
    ck_assert (cpu.zf == 0);
    ck_assert (_check_get_reg (&cpu, inst.rb) == 0);

    /* test: load a zero value */
    inst.valC.v = 0;
    valE = decode_execute (&cpu, inst, &cnd, &valA);
    ck_assert (valE == (y86_reg_t)inst.valC.v);
    ck_assert (cpu.sf == 0);
    ck_assert (cpu.zf == 0);
    ck_assert (_check_get_reg (&cpu, inst.rb) == 0);
}
END_TEST

/* helper function: set the given register value in a CPU state */
static void _check_set_reg (y86_t *cpu, y86_regnum_t num, y86_reg_t val)
{
    cpu->reg[num] = val;
}

/* check decoding and execution of the OPq instructions (no CC testing) */
START_TEST (C_func_decode_opq)
{
    y86_t cpu;
    y86_inst_t inst;
    bool cnd = false;
    y86_reg_t valA = 0;
    y86_reg_t valE = 0;
    y86_reg_t expected_valE = 0;
    y86_reg_t regs[NOREG];
    uint8_t i = 0;

    memset (&inst, 0, sizeof (inst));
    inst.icode = OPQ;

    memset (&cpu, 0, sizeof (cpu));
    cpu.stat = AOK;
    for (i = RAX; i < NOREG; i++)
    {
        /* set registers to random values */
        regs[i] = (((int64_t)rand()) << 32) | rand();
        _check_set_reg (&cpu, i, regs[i]);
    }

    /* addition */
    inst.ra = (y86_regnum_t)(rand() % NOREG);
    inst.rb = (y86_regnum_t)(rand() % NOREG);
    inst.ifun.op = ADD;
    expected_valE = _check_get_reg (&cpu, inst.ra) + _check_get_reg (&cpu, inst.rb);
    valE = decode_execute (&cpu, inst, &cnd, &valA);
    ck_assert (valE == expected_valE);
    ck_assert (regs[inst.ra] == _check_get_reg (&cpu, inst.ra));
    ck_assert (regs[inst.rb] == _check_get_reg (&cpu, inst.rb));

    /* subtraction */
    inst.ra = (y86_regnum_t)(rand() % NOREG);
    inst.rb = (y86_regnum_t)(rand() % NOREG);
    inst.ifun.op = SUB;
    expected_valE = _check_get_reg (&cpu, inst.rb) - _check_get_reg (&cpu, inst.ra);
    valE = decode_execute (&cpu, inst, &cnd, &valA);
    ck_assert (valE == expected_valE);
    ck_assert (regs[inst.ra] == _check_get_reg (&cpu, inst.ra));
    ck_assert (regs[inst.rb] == _check_get_reg (&cpu, inst.rb));

    /* bitwise AND */
    inst.ra = (y86_regnum_t)(rand() % NOREG);
    inst.rb = (y86_regnum_t)(rand() % NOREG);
    inst.ifun.op = AND;
    expected_valE = _check_get_reg (&cpu, inst.ra) & _check_get_reg (&cpu, inst.rb);
    valE = decode_execute (&cpu, inst, &cnd, &valA);
    ck_assert (valE == expected_valE);
    ck_assert (regs[inst.ra] == _check_get_reg (&cpu, inst.ra));
    ck_assert (regs[inst.rb] == _check_get_reg (&cpu, inst.rb));

    /* bitwise XOR */
    inst.ra = (y86_regnum_t)(rand() % NOREG);
    inst.rb = (y86_regnum_t)(rand() % NOREG);
    inst.ifun.op = XOR;
    expected_valE = _check_get_reg (&cpu, inst.ra) ^ _check_get_reg (&cpu, inst.rb);
    valE = decode_execute (&cpu, inst, &cnd, &valA);
    ck_assert (valE == expected_valE);
    ck_assert (regs[inst.ra] == _check_get_reg (&cpu, inst.ra));
    ck_assert (regs[inst.rb] == _check_get_reg (&cpu, inst.rb));
}
END_TEST

/* check handling of invalid OPq instruction */
START_TEST (C_error_decode_invalid_opq)
{
    y86_t cpu;
    y86_inst_t inst;
    bool cnd = false;
    y86_reg_t valA = 0;
    y86_reg_t valE = 0;

    memset (&inst, 0, sizeof (inst));
    memset (&cpu, 0, sizeof (cpu));
    cpu.stat = AOK;

    inst.icode = OPQ;
    inst.ifun.op = BADOP;
    valE = decode_execute (&cpu, inst, &cnd, &valA);
    ck_assert (valE == 0 || valE != 0);
    ck_assert (cpu.stat == INS);
}
END_TEST

/* check memory, writeback, and PC update of the IRMOVQ instruction */
START_TEST (C_func_memory_irmovq)
{
    y86_t cpu;
    byte_t *memory = malloc (MEMSIZE);
    y86_inst_t inst;
    bool cnd = false;
    y86_reg_t valA;
    y86_reg_t valE;

    for (y86_regnum_t reg = RAX; reg < NOREG; reg++) {
        valA = 0;
        memset (&cpu, 0, sizeof (cpu));
        memset (memory, 0, MEMSIZE);
        memset (&inst, 0, sizeof (inst));
        cpu.stat = AOK;

        inst.icode = IRMOVQ;
        inst.rb = reg;
        inst.valP = rand() % MEMSIZE;
        valE = rand();
        memory_wb_pc (&cpu, inst, memory, cnd, valA, valE);
        ck_assert (cpu.stat == AOK);
        ck_assert (cpu.reg[reg] == valE);
        ck_assert (cpu.pc == inst.valP);
    }
}
END_TEST

/* check memory, writeback, and PC update of the OPQ instruction */
START_TEST (C_func_memory_opq)
{
    y86_t cpu;
    byte_t *memory = malloc (MEMSIZE);
    y86_inst_t inst;
    bool cnd = false;
    y86_reg_t valA;
    y86_reg_t valE;

    for (y86_regnum_t reg = RAX; reg < NOREG; reg++) {
        valA = 0;
        memset (&cpu, 0, sizeof (cpu));
        memset (memory, 0, MEMSIZE);
        memset (&inst, 0, sizeof (inst));
        cpu.stat = AOK;

        inst.icode = OPQ;
        inst.rb = reg;
        inst.valP = rand() % MEMSIZE;
        valE = rand();
        memory_wb_pc (&cpu, inst, memory, cnd, valA, valE);
        ck_assert (cpu.stat == AOK);
        ck_assert (cpu.reg[reg] == valE);
        ck_assert (cpu.pc == inst.valP);
    }
}
END_TEST

void public_tests (Suite *s)
{
    TCase *tc_public = tcase_create ("Public");
    tcase_add_test (tc_public, D_func_decode_halt);
    tcase_add_test (tc_public, D_func_decode_nop);
    tcase_add_test (tc_public, D_func_memory_halt);
    tcase_add_test (tc_public, D_func_memory_nop);
    tcase_add_test (tc_public, D_error_decode_NULL_cnd_valA);
    tcase_add_test (tc_public, C_func_decode_irmovq);
    tcase_add_test (tc_public, C_func_decode_opq);
    tcase_add_test (tc_public, C_func_memory_irmovq);
    tcase_add_test (tc_public, C_func_memory_opq);
    tcase_add_test (tc_public, C_error_decode_invalid_opq);
    suite_add_tcase (s, tc_public);
}

