#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <stdint.h>

// CMake passes -DNDEBUG to release builds - this would disable ISViewer logging in libdragon
#ifdef NDEBUG
#undef NDEBUG
#define NDEBUG_
#endif
#include <libdragon.h>
#include <rsp.h>
#ifdef NDEBUG_
#define NDEBUG
#endif

#include "rsp_vector_instructions.h"
#include "rsp_funct.h"
#include "rsp_state.h"
#include "mips_instruction_decode.h"
#include "testcases.h"

extern const void __basic_ucode_data_start;
extern const void __basic_ucode_start;
extern const void __basic_ucode_end;

static resolution_t res = RESOLUTION_320x240;
static bitdepth_t bit = DEPTH_32_BPP;

static volatile bool rsp_execution_complete = false;

static void sp_handler() {
    rsp_execution_complete = true;
}

typedef uint64_t dword;
typedef uint32_t word;
typedef uint16_t half;
typedef uint8_t  byte;

typedef union v128 {
    word words[4];
    half elements[8];
    byte bytes[16];
} v128_t;
_Static_assert(sizeof(v128_t) == sizeof(word) * 4, "v128_t needs to be 128 bits");

typedef struct v_result {
    v128_t res;
    v128_t acch;
    v128_t accm;
    v128_t accl;
} v_result_t;

typedef union flag_result {
    struct {
        half vcc;
        half vco;
        half vce;
        half padding;
    };
    dword packed;
} flag_result_t;
_Static_assert(sizeof(flag_result_t) == sizeof(dword), "flag_result_t should be 64 bits");

typedef struct testcase {
    v128_t zero;

    v128_t arg1;
    v128_t arg2;

    v_result_t result_elements[16];
    flag_result_t flag_elements[16];
}  testcase_t;

testcase_t* dmem_results;
testcase_t* testcase_emulated;

_Static_assert(sizeof(testcase_t) ==
                       (16 * 3) +      // zero, arg1, arg2
                       (16 * 4 * 16) + // 4x res, acch, accm, accl * 16
                       (8 * 16), // 16x flag_result_t (5 bytes with 3 bytes padding)
                       "Testcase blob is expected size");

void print_v128(volatile v128_t* r) {
    printf("%04X%04X%04X%04X%04X%04X%04X%04X",
           r->elements[0], r->elements[1], r->elements[2], r->elements[3],
           r->elements[4], r->elements[5], r->elements[6], r->elements[7]);
}

void print_v128_ln(volatile v128_t* r) {
    print_v128(r);
    printf("\n");
}

word element_instruction(word instruction, int element) {
    return instruction | (element << 21);
}

void load_replacement_ucode(word instruction, int* replacement_indices, unsigned long ucode_size) {
    word* ucode = malloc(ucode_size);
    word* uncached_ucode = UncachedAddr(ucode);
    memcpy(uncached_ucode, &__basic_ucode_start, ucode_size);
    for (int i = 0; i < 16; i++) {
        uncached_ucode[replacement_indices[i]] = instruction == 0x00000000 ? instruction : element_instruction(instruction, i);
    }
    rsp_load_code(uncached_ucode, ucode_size, 0);
    for (int i = 0; i < 0x1000; i++) {
        N64RSP.sp_imem[i] = uncached_ucode[i];
    }
    free(ucode);
}

typedef void(*rspinstr_handler_t)(mips_instruction_t);

typedef struct rsp_testable_instruction {
    rspinstr_handler_t handler;
    int funct;
    const char* name;
    bool random_vs;
} rsp_testable_instruction_t;

#define INSTR(_handler, _funct, _name, _random_vs) { .handler = _handler, .funct = _funct, .name = _name, .random_vs = _random_vs}

rsp_testable_instruction_t instrs[] = {
        INSTR(rsp_vec_vabs, FUNCT_RSP_VEC_VABS, "vabs", false),
        INSTR(rsp_vec_vadd, FUNCT_RSP_VEC_VADD, "vadd", false),
        INSTR(rsp_vec_vaddc, FUNCT_RSP_VEC_VADDC, "vaddc", false),
        INSTR(rsp_vec_vand, FUNCT_RSP_VEC_VAND, "vand", false),
        INSTR(rsp_vec_vch, FUNCT_RSP_VEC_VCH, "vch", false),
        INSTR(rsp_vec_vcl, FUNCT_RSP_VEC_VCL, "vcl", false),
        INSTR(rsp_vec_vcr, FUNCT_RSP_VEC_VCR, "vcr", false),
        INSTR(rsp_vec_veq, FUNCT_RSP_VEC_VEQ, "veq", false),
        INSTR(rsp_vec_vge, FUNCT_RSP_VEC_VGE, "vge", false),
        INSTR(rsp_vec_vlt, FUNCT_RSP_VEC_VLT, "vlt", false),
        INSTR(rsp_vec_vmacf, FUNCT_RSP_VEC_VMACF, "vmacf", false),
        // unimplemented
        //INSTR(rsp_vec_vmacq, FUNCT_RSP_VEC_VMACQ, "vmacq", false),
        INSTR(rsp_vec_vmacu, FUNCT_RSP_VEC_VMACU, "vmacu", false),
        INSTR(rsp_vec_vmadh, FUNCT_RSP_VEC_VMADH, "vmadh", false),
        INSTR(rsp_vec_vmadl, FUNCT_RSP_VEC_VMADL, "vmadl", false),
        INSTR(rsp_vec_vmadm, FUNCT_RSP_VEC_VMADM, "vmadm", false),
        INSTR(rsp_vec_vmadn, FUNCT_RSP_VEC_VMADN, "vmadn", false),
        INSTR(rsp_vec_vmov, FUNCT_RSP_VEC_VMOV, "vmov", true),
        INSTR(rsp_vec_vmrg, FUNCT_RSP_VEC_VMRG, "vmrg", false),
        INSTR(rsp_vec_vmudh, FUNCT_RSP_VEC_VMUDH, "vmudh", false),
        INSTR(rsp_vec_vmudl, FUNCT_RSP_VEC_VMUDL, "vmudl", false),
        INSTR(rsp_vec_vmudm, FUNCT_RSP_VEC_VMUDM, "vmudm", false),
        INSTR(rsp_vec_vmudn, FUNCT_RSP_VEC_VMUDN, "vmudn", false),
        INSTR(rsp_vec_vmulf, FUNCT_RSP_VEC_VMULF, "vmulf", false),
        // unimplemented
        //INSTR(rsp_vec_vmulq, FUNCT_RSP_VEC_VMULQ, "vmulq", false),
        INSTR(rsp_vec_vmulu, FUNCT_RSP_VEC_VMULU, "vmulu", false),
        INSTR(rsp_vec_vnand, FUNCT_RSP_VEC_VNAND, "vnand", false),
        INSTR(rsp_vec_vne, FUNCT_RSP_VEC_VNE, "vne", false),
        INSTR(rsp_vec_vnop, FUNCT_RSP_VEC_VNOP, "vnop", false),
        INSTR(rsp_vec_vnor, FUNCT_RSP_VEC_VNOR, "vnor", false),
        INSTR(rsp_vec_vnxor, FUNCT_RSP_VEC_VNXOR, "vnxor", false),
        INSTR(rsp_vec_vor, FUNCT_RSP_VEC_VOR, "vor", false),
        INSTR(rsp_vec_vrcp, FUNCT_RSP_VEC_VRCP, "vrcp", true),
        INSTR(rsp_vec_vrcph_vrsqh, FUNCT_RSP_VEC_VRCPH, "vrcph", true),
        INSTR(rsp_vec_vrcph_vrsqh, FUNCT_RSP_VEC_VRSQH, "vrsqh", true),
        INSTR(rsp_vec_vrcpl, FUNCT_RSP_VEC_VRCPL, "vrcpl", true),
        // unimplemented
        //INSTR(rsp_vec_vrndn, FUNCT_RSP_VEC_VRNDN, "vrndn", false),
        // unimplemented
        //INSTR(rsp_vec_vrndp, FUNCT_RSP_VEC_VRNDP, "vrndp", false),
        INSTR(rsp_vec_vrsq, FUNCT_RSP_VEC_VRSQ, "vrsq", true),
        INSTR(rsp_vec_vrsql, FUNCT_RSP_VEC_VRSQL, "vrsql", true),
        INSTR(rsp_vec_vsar, FUNCT_RSP_VEC_VSAR, "vsar", false),
        INSTR(rsp_vec_vsub, FUNCT_RSP_VEC_VSUB, "vsub", false),
        INSTR(rsp_vec_vsubc, FUNCT_RSP_VEC_VSUBC, "vsubc", false),
        INSTR(rsp_vec_vxor, FUNCT_RSP_VEC_VXOR, "vxor", false),
};

const int vs = 1;
const int vt = 2;
const int vd = 3;

mips_instruction_t vec_instr(int funct) {

    mips_instruction_t instr;
    instr.raw = OPC_CP2 << 26 | 1 << 25 | vt << 16 | vs << 11 | vd << 6 | funct;
    return instr;
}

half next_half() {
    static half h = 0;
    return h++;
}

#define hang(message,...) do { printf(message, ##__VA_ARGS__); while(1) { console_render(); } } while(0)

void dump_rsp(int e) {
    printf("Input (broadcast modifier %d):\n", e);
    print_v128_ln(&dmem_results->arg1);
    print_v128_ln(&dmem_results->arg2);

    printf("\nSoft RSP vd/acch/accm/accl\n");
    print_v128_ln(&testcase_emulated->result_elements[e].res);

    print_v128_ln(&testcase_emulated->result_elements[e].acch);
    print_v128_ln(&testcase_emulated->result_elements[e].accm);
    print_v128_ln(&testcase_emulated->result_elements[e].accl);

    printf("\nVCO 0x%04X VCC 0x%04X VCE 0x%02X\n", testcase_emulated->flag_elements[e].vco, testcase_emulated->flag_elements[e].vcc, testcase_emulated->flag_elements[e].vce);

    printf("\nReal RSP vd/acch/accm/accl\n");
    print_v128_ln(&dmem_results->result_elements[e].res);
    print_v128_ln(&dmem_results->result_elements[e].acch);
    print_v128_ln(&dmem_results->result_elements[e].accm);
    print_v128_ln(&dmem_results->result_elements[e].accl);
    printf("VCO 0x%04X VCC 0x%04X VCE 0x%02X\n\n", dmem_results->flag_elements[e].vco, dmem_results->flag_elements[e].vcc, dmem_results->flag_elements[e].vce);
}


int arg1_index = 0;
int arg2_index = 0;
void testcases_reset() {
    arg1_index = 0;
    arg2_index = 0;
}

void next_testcase(volatile half* a1, volatile half* a2) {
    *a1 = testcase_numbers[arg1_index];
    *a2 = testcase_numbers[arg2_index];

    if (++arg1_index == NUM_TESTCASE_NUMBERS) {
        arg1_index = 0;
        if (++arg2_index == NUM_TESTCASE_NUMBERS) {
            arg2_index = 0;
        }
    }
}

void run_test(rsp_testable_instruction_t* testable_instruction, mips_instruction_t instruction) {
    for (int e = 0; e < 16; e++) {
        for (int i = 0; i < 4; i++) {
            dmem_results->result_elements[e].res.words[i] = 0;
            dmem_results->result_elements[e].acch.words[i] = 0;
            dmem_results->result_elements[e].accm.words[i] = 0;
            dmem_results->result_elements[e].accl.words[i] = 0;
        }
        dmem_results->flag_elements[e].packed = 0;
    }

    for (int i = 0; i < 8; i++) {
        next_testcase(&dmem_results->arg1.elements[i], &dmem_results->arg2.elements[i]);
    }

    for (int i = 0; i < 8; i++) {
        N64RSP.vu_regs[vs].elements[i] = dmem_results->arg1.elements[i];
        N64RSP.vu_regs[vt].elements[i] = dmem_results->arg2.elements[i];
    }

    rsp_load_data(dmem_results, sizeof(testcase_t), 0);
    rsp_execution_complete = false;
    rsp_run_async();

    for (int e = 0; e < 16; e++) {
        instruction.cp2_vec.e = e;
        testable_instruction->handler(instruction);
        for (int i = 0; i < 8; i++) {
            testcase_emulated->result_elements[e].res.elements[i] = N64RSP.vu_regs[vd].elements[i];
            testcase_emulated->result_elements[e].accl.elements[i] = N64RSP.acc.l.elements[i];
            testcase_emulated->result_elements[e].accm.elements[i] = N64RSP.acc.m.elements[i];
            testcase_emulated->result_elements[e].acch.elements[i] = N64RSP.acc.h.elements[i];

            testcase_emulated->flag_elements[e].vcc = rsp_get_vcc();
            testcase_emulated->flag_elements[e].vce = rsp_get_vce();
            testcase_emulated->flag_elements[e].vco = rsp_get_vco();
        }
    }

    while (!rsp_execution_complete) {
        printf("Waiting on the RSP, if you're seeing this you probably have timing issues\n");
        console_render();
    }

    rsp_read_data(dmem_results, sizeof(testcase_t), 0);


    for (int e = 0; e < 16; e++) {
        for (int i = 0; i < 8; i++) {
            if (testcase_emulated->result_elements[e].res.elements[i] != dmem_results->result_elements[e].res.elements[i]) {
                dump_rsp(e);
                hang("vd Mismatch!");
            }
            if (testcase_emulated->result_elements[e].accl.elements[i] != dmem_results->result_elements[e].accl.elements[i]) {
                dump_rsp(e);
                hang("accl mismatch!\n");
            }
            if (testcase_emulated->result_elements[e].accm.elements[i] != dmem_results->result_elements[e].accm.elements[i]) {
                dump_rsp(e);
                hang("accm mismatch!\n");
            }
            if (testcase_emulated->result_elements[e].acch.elements[i] != dmem_results->result_elements[e].acch.elements[i]) {
                dump_rsp(e);
                hang("acch mismatch!\n");
            }

            if (testcase_emulated->flag_elements[e].vcc != dmem_results->flag_elements[e].vcc) {
                dump_rsp(e);
                hang("vcc mismatch!");
            }
            if (testcase_emulated->flag_elements[e].vce != dmem_results->flag_elements[e].vce) {
                dump_rsp(e);
                hang("vce mismatch!");
            }
            if (testcase_emulated->flag_elements[e].vco != dmem_results->flag_elements[e].vco) {
                dump_rsp(e);
                hang("vco Mismatch!");
            }
        }
    }
}

int main(void) {
    debug_init(DEBUG_FEATURE_LOG_ISVIEWER);
    debug_init_isviewer();
    dmem_results = UncachedAddr(malloc(sizeof(testcase_t)));
    testcase_emulated = UncachedAddr(malloc(sizeof(testcase_t)));

    /* Initialize peripherals */
    display_init( res, bit, 2, GAMMA_NONE, ANTIALIAS_RESAMPLE );
    console_init();
    console_set_render_mode(RENDER_MANUAL);
    rsp_init();

    /* Attach SP handler and enable interrupt */
    register_SP_handler(&sp_handler);
    set_SP_interrupt(1);

    // Size must be multiple of 8 and start & end must be aligned to 8 bytes
    unsigned long data_size = (unsigned long) (&__basic_ucode_start - &__basic_ucode_data_start);
    unsigned long ucode_size = (unsigned long) (&__basic_ucode_end - &__basic_ucode_start);
    rsp_load_data((void*)&__basic_ucode_data_start, data_size, 0);

    int replacement_indices[16];
    int found = 0;
    word* ucodeptr = (word*)&__basic_ucode_start;
    for (int i = 0; i < (ucode_size / 4); i++) {
        if (ucodeptr[i] == 0xFFFFFFFF) {
            replacement_indices[found++] = i;
        }
    }

    if (found != 16) {
        hang("Didn't find exactly 16 instances, found %d instead. Bad!\n", found);
    }

    console_clear();

    printf("Ready!\n");

    console_render();

    word old_instruction = 0xFFFFFFFF;


    // Get initial state by "testing" with NOPs
    load_replacement_ucode(0x00000000, replacement_indices, ucode_size);
    memset(dmem_results, 0x00, sizeof(testcase_t));
    rsp_load_data(dmem_results, sizeof(testcase_t), 0);
    rsp_execution_complete = false;
    rsp_run_async();
    while (!rsp_execution_complete) {
        printf("Grabbing initial state from RSP...\n");
        console_render();
    }
    rsp_read_data(dmem_results, sizeof(testcase_t), 0);
    rsp_set_vcc(dmem_results->flag_elements[0].vcc);
    rsp_set_vce(dmem_results->flag_elements[0].vce);
    rsp_set_vco(dmem_results->flag_elements[0].vco);

    for (int i = 0; i < 4; i++) {
        N64RSP.acc.h.words[i] = dmem_results->result_elements[0].acch.words[i];
        N64RSP.acc.m.words[i] = dmem_results->result_elements[0].accm.words[i];
        N64RSP.acc.l.words[i] = dmem_results->result_elements[0].accl.words[i];
    }

    while (1) {

        for (int instr = 0; instr < (sizeof(instrs) / sizeof(rsp_testable_instruction_t)); instr++) {
            printf("Fuzzing %s %d times...\n", instrs[instr].name, FUZZES_PER_INSTRUCTION);
            console_render();

            mips_instruction_t instruction = vec_instr(instrs[instr].funct);

            if (instruction.raw != old_instruction) {
                load_replacement_ucode(instruction.raw, replacement_indices, ucode_size);
                old_instruction = instruction.raw;
            }

            testcases_reset();
            for (int test = 0; test < FUZZES_PER_INSTRUCTION; test++) {
                run_test(&instrs[instr], instruction);
                if ((test & 0x3FFF) == 0) {
                    int remaining = FUZZES_PER_INSTRUCTION - test;
                    printf("%d done, %d remaining\n", test, remaining);
                    console_render();
                }
            }
        }
        console_render();
    }
}
