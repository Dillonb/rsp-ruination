#ifndef RSP_RUINATION_RSP_STATE_H
#define RSP_RUINATION_RSP_STATE_H

#include "rsp_types.h"

extern rsp_t rsp;
#define N64RSP rsp

INLINE void set_rsp_register(byte r, word value) {
    if (r != 0) {
        N64RSP.gpr[r] = value;
    }
}

#define FLAGREG_BOOL(x) ((x) ? 0xFFFF : 0)

INLINE word get_rsp_register(byte r) {
        return N64RSP.gpr[r];
}

INLINE sdword get_rsp_accumulator(int e) {
    sdword val = (sdword)N64RSP.acc.h.elements[e] << 32;
    val       |= (sdword)N64RSP.acc.m.elements[e] << 16;
    val       |= (sdword)N64RSP.acc.l.elements[e] << 0;
    if ((val & 0x0000800000000000) != 0) {
        val |= 0xFFFF000000000000;
    }
    return val;
}

INLINE void set_rsp_accumulator(int e, dword val) {
    N64RSP.acc.h.elements[e] = (val >> 32) & 0xFFFF;
    N64RSP.acc.m.elements[e] = (val >> 16) & 0xFFFF;
    N64RSP.acc.l.elements[e] = val & 0xFFFF;
}

INLINE half rsp_get_vco() {
    half value = 0;
    for (int i = 0; i < 8; i++) {
        bool h = N64RSP.vco.h.elements[VU_ELEM_INDEX(i)] != 0;
        bool l = N64RSP.vco.l.elements[VU_ELEM_INDEX(i)] != 0;
        word mask = (l << i) | (h << (i + 8));
        value |= mask;
    }
    return value;
}

INLINE half rsp_get_vcc() {
    half value = 0;
    for (int i = 0; i < 8; i++) {
        bool h = N64RSP.vcc.h.elements[VU_ELEM_INDEX(i)] != 0;
        bool l = N64RSP.vcc.l.elements[VU_ELEM_INDEX(i)] != 0;
        word mask = (l << i) | (h << (i + 8));
        value |= mask;
    }
    return value;
}

INLINE byte rsp_get_vce() {
    byte value = 0;
    for (int i = 0; i < 8; i++) {
        bool l = N64RSP.vce.elements[VU_ELEM_INDEX(i)] != 0;
        value |= (l << i);
    }
    return value;
}

INLINE void rsp_set_vcc(half vcc) {
    for (int i = 0; i < 8; i++) {
        N64RSP.vcc.l.elements[VU_ELEM_INDEX(i)] = FLAGREG_BOOL(vcc & 1);
        vcc >>= 1;
    }

    for (int i = 0; i < 8; i++) {
        N64RSP.vcc.h.elements[VU_ELEM_INDEX(i)] = FLAGREG_BOOL(vcc & 1);
        vcc >>= 1;
    }
}

INLINE void rsp_set_vco(half vco) {
    for (int i = 0; i < 8; i++) {
        N64RSP.vco.l.elements[VU_ELEM_INDEX(i)] = FLAGREG_BOOL(vco & 1);
        vco >>= 1;
    }

    for (int i = 0; i < 8; i++) {
        N64RSP.vco.h.elements[VU_ELEM_INDEX(i)] = FLAGREG_BOOL(vco & 1);
        vco >>= 1;
    }
}

INLINE void rsp_set_vce(half vce) {
    for (int i = 0; i < 8; i++) {
        N64RSP.vce.elements[VU_ELEM_INDEX(i)] = FLAGREG_BOOL(vce & 1);
        vce >>= 1;
    }
}

#endif //RSP_RUINATION_RSP_STATE_H
