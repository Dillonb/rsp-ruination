#include <ucode.S>

.macro cfc2 reg, flagreg
    .long (0b010010 << 26 | 0b00010 << 21 | \reg << 16 | \flagreg << 11)
.endm

.text

lqv $v00, 0, 0, $zero

// Load first argument
la $2, arg1
lqv $v01, 0, 0, $2

// Load second argument
la $2, arg2
lqv $v02, 0, 0, $2

.macro runOp element, resAddr, acchAddr, accmAddr, acclAddr, vccAddr, vcoAddr, vceAddr
    .word 0xFFFFFFFF
    la $2, \resAddr
    la $3, \acchAddr
    la $4, \accmAddr
    la $5, \acclAddr
    la $6, \vccAddr
    la $7, \vcoAddr
    la $8, \vceAddr
    jal saveResults
    nop
.endm

// Run instruction once for each broadcast modifier
runOp 0, res_e0, acc_h_e0, acc_m_e0, acc_l_e0, vcc_e0, vco_e0, vce_e0
runOp 1, res_e1, acc_h_e1, acc_m_e1, acc_l_e1, vcc_e1, vco_e1, vce_e1
runOp 2, res_e2, acc_h_e2, acc_m_e2, acc_l_e2, vcc_e2, vco_e2, vce_e2
runOp 3, res_e3, acc_h_e3, acc_m_e3, acc_l_e3, vcc_e3, vco_e3, vce_e3
runOp 4, res_e4, acc_h_e4, acc_m_e4, acc_l_e4, vcc_e4, vco_e4, vce_e4
runOp 5, res_e5, acc_h_e5, acc_m_e5, acc_l_e5, vcc_e5, vco_e5, vce_e5
runOp 6, res_e6, acc_h_e6, acc_m_e6, acc_l_e6, vcc_e6, vco_e6, vce_e6
runOp 7, res_e7, acc_h_e7, acc_m_e7, acc_l_e7, vcc_e7, vco_e7, vce_e7
runOp 8, res_e8, acc_h_e8, acc_m_e8, acc_l_e8, vcc_e8, vco_e8, vce_e8
runOp 9, res_e9, acc_h_e9, acc_m_e9, acc_l_e9, vcc_e9, vco_e9, vce_e9
runOp 10, res_e10, acc_h_e10, acc_m_e10, acc_l_e10, vcc_e10, vco_e10, vce_e10
runOp 11, res_e11, acc_h_e11, acc_m_e11, acc_l_e11, vcc_e11, vco_e11, vce_e11
runOp 12, res_e12, acc_h_e12, acc_m_e12, acc_l_e12, vcc_e12, vco_e12, vce_e12
runOp 13, res_e13, acc_h_e13, acc_m_e13, acc_l_e13, vcc_e13, vco_e13, vce_e13
runOp 14, res_e14, acc_h_e14, acc_m_e14, acc_l_e14, vcc_e14, vco_e14, vce_e14
runOp 15, res_e15, acc_h_e15, acc_m_e15, acc_l_e15, vcc_e15, vco_e15, vce_e15

break

deadloop:
	j deadloop
	nop

saveResults:
    sqv $v03, 0, 0, $2
    // acc_h
    vsar $v04, $v00, $v00, 0x8
    sqv $v04, 0, 0, $3
    // acc_m
    vsar $v04, $v00, $v00, 0x9
    sqv $v04, 0, 0, $4
    // acc_l
    vsar $v04, $v00, $v00, 0xA
    sqv $v04, 0, 0, $5
    // VCC
    cfc2 9, 1
    sh $9, 0($6)
    // VCO
    cfc2 9, 0
    sh $9, 0($7)
    // VCE
    cfc2 9, 2
    sh $9, 0($8)
    jr $ra
    nop

.data
zero:  .word 0,0,0,0
arg1:  .word 0,0,0,0
arg2:  .word 0,0,0,0

res_e0:   .word 0,0,0,0
acc_h_e0: .word 0,0,0,0
acc_m_e0: .word 0,0,0,0
acc_l_e0: .word 0,0,0,0

res_e1:   .word 0,0,0,0
acc_h_e1: .word 0,0,0,0
acc_m_e1: .word 0,0,0,0
acc_l_e1: .word 0,0,0,0

res_e2:   .word 0,0,0,0
acc_h_e2: .word 0,0,0,0
acc_m_e2: .word 0,0,0,0
acc_l_e2: .word 0,0,0,0

res_e3:   .word 0,0,0,0
acc_h_e3: .word 0,0,0,0
acc_m_e3: .word 0,0,0,0
acc_l_e3: .word 0,0,0,0

res_e4:   .word 0,0,0,0
acc_h_e4: .word 0,0,0,0
acc_m_e4: .word 0,0,0,0
acc_l_e4: .word 0,0,0,0

res_e5:   .word 0,0,0,0
acc_h_e5: .word 0,0,0,0
acc_m_e5: .word 0,0,0,0
acc_l_e5: .word 0,0,0,0

res_e6:   .word 0,0,0,0
acc_h_e6: .word 0,0,0,0
acc_m_e6: .word 0,0,0,0
acc_l_e6: .word 0,0,0,0

res_e7:   .word 0,0,0,0
acc_h_e7: .word 0,0,0,0
acc_m_e7: .word 0,0,0,0
acc_l_e7: .word 0,0,0,0

res_e8:   .word 0,0,0,0
acc_h_e8: .word 0,0,0,0
acc_m_e8: .word 0,0,0,0
acc_l_e8: .word 0,0,0,0

res_e9:   .word 0,0,0,0
acc_h_e9: .word 0,0,0,0
acc_m_e9: .word 0,0,0,0
acc_l_e9: .word 0,0,0,0

res_e10:   .word 0,0,0,0
acc_h_e10: .word 0,0,0,0
acc_m_e10: .word 0,0,0,0
acc_l_e10: .word 0,0,0,0

res_e11:   .word 0,0,0,0
acc_h_e11: .word 0,0,0,0
acc_m_e11: .word 0,0,0,0
acc_l_e11: .word 0,0,0,0

res_e12:   .word 0,0,0,0
acc_h_e12: .word 0,0,0,0
acc_m_e12: .word 0,0,0,0
acc_l_e12: .word 0,0,0,0

res_e13:   .word 0,0,0,0
acc_h_e13: .word 0,0,0,0
acc_m_e13: .word 0,0,0,0
acc_l_e13: .word 0,0,0,0

res_e14:   .word 0,0,0,0
acc_h_e14: .word 0,0,0,0
acc_m_e14: .word 0,0,0,0
acc_l_e14: .word 0,0,0,0

res_e15:   .word 0,0,0,0
acc_h_e15: .word 0,0,0,0
acc_m_e15: .word 0,0,0,0
acc_l_e15: .word 0,0,0,0

vcc_e0: .half 0
vco_e0: .half 0
vce_e0: .half 0
pad_e0: .half 0

vcc_e1: .half 0
vco_e1: .half 0
vce_e1: .half 0
pad_e1: .half 0

vcc_e2: .half 0
vco_e2: .half 0
vce_e2: .half 0
pad_e2: .half 0

vcc_e3: .half 0
vco_e3: .half 0
vce_e3: .half 0
pad_e3: .half 0

vcc_e4: .half 0
vco_e4: .half 0
vce_e4: .half 0
pad_e4: .half 0

vcc_e5: .half 0
vco_e5: .half 0
vce_e5: .half 0
pad_e5: .half 0

vcc_e6: .half 0
vco_e6: .half 0
vce_e6: .half 0
pad_e6: .half 0

vcc_e7: .half 0
vco_e7: .half 0
vce_e7: .half 0
pad_e7: .half 0

vcc_e8: .half 0
vco_e8: .half 0
vce_e8: .half 0
pad_e8: .half 0

vcc_e9: .half 0
vco_e9: .half 0
vce_e9: .half 0
pad_e9: .half 0

vcc_e10: .half 0
vco_e10: .half 0
vce_e10: .half 0
pad_e10: .half 0

vcc_e11: .half 0
vco_e11: .half 0
vce_e11: .half 0
pad_e11: .half 0

vcc_e12: .half 0
vco_e12: .half 0
vce_e12: .half 0
pad_e12: .half 0

vcc_e13: .half 0
vco_e13: .half 0
vce_e13: .half 0
pad_e13: .half 0

vcc_e14: .half 0
vco_e14: .half 0
vce_e14: .half 0
pad_e14: .half 0

vcc_e15: .half 0
vco_e15: .half 0
vce_e15: .half 0
pad_e15: .half 0