	.section .text
	.global	_vbr
    .global _vbr_set
	.global	_dbr
    .global _dbr_set
    .global _r15
    .global _spc

_vbr:
	stc     VBR, r0
	rts
	nop

_vbr_set:
	ldc     r4, VBR
	rts
	nop

_dbr:
	stc     DBR, r0
	rts
	nop

_dbr_set:
	ldc     r4, DBR
	rts
	nop

_r15:
    mov     r15, r0
    rts
    nop

_spc:
    stc     SPC, r0
    rts
    nop
