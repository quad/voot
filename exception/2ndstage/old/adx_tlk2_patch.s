    .section .text

    .global _ADXT_Init
    .global _ADXT_Create
    .global _ADXT_Destroy
    .global _adxt_start_sj
    .global _adxt_start_stm
    .global _ADXT_Stop
    .global _ADXCRS_Lock
    .global _ADXCRS_Unlock
    .global _ADXERR_CallErrFunc
    .global _ADXSTM_OpenRange
    .global _ADXSTM_GetFad
    .global _ADXSTM_GetFsizeSct
    .global _ADXF_GetFileRange
    .global _ADXT_ExecServer
    .global _ADXT_EntryErrFunc

!
! adx_inis.o
!

_ADXT_Init:
    nop
    mov.l   r0, @-r15
    mov.l   _ADXT_Init_ref, r0
    jmp     @r0
    mov.l   @r15+, r0
    nop

_ADXT_Init_ref:
    .long   0x8c010000 + 0x32004 + 0x6

!
! adx_tlk.o
!

_ADXT_Create:
    nop
    mov.l   r0, @-r15
    mov.l   _ADXT_Create_ref, r0
    jmp     @r0
    mov.l   @r15+, r0
    nop

_ADXT_Destroy:
    nop
    mov.l   r0, @-r15
    mov.l   _ADXT_Destroy_ref, r0
    jmp     @r0
    mov.l   @r15+, r0
    nop

_adxt_start_sj:
    nop
    mov.l   r0, @-r15
    mov.l   _adxt_start_sj_ref, r0
    jmp     @r0
    mov.l   @r15+, r0
    nop

_adxt_start_stm:
    nop
    mov.l   r0, @-r15
    mov.l   _adxt_start_stm_ref, r0
    jmp     @r0
    mov.l   @r15+, r0
    nop

_ADXT_Stop:
    nop
    mov.l   r0, @-r15
    mov.l   _ADXT_Stop_ref, r0
    jmp     @r0
    mov.l   @r15+, r0
    nop

_ADXT_ExecServer:
    nop
    mov.l   r0, @-r15
    mov.l   _ADXT_ExecServer_ref, r0
    jmp     @r0
    mov.l   @r15+, r0
    nop

_ADXT_EntryErrFunc:
    nop
    mov.l   r0, @-r15
    mov.l   _ADXT_EntryErrFunc_ref, r0
    jmp     @r0
    mov.l   @r15+, r0
    nop

_ADXT_Create_ref:
    .long   0x8c010000 + 0x32140 + 0x0
_ADXT_Destroy_ref:
    .long   0x8c010000 + 0x32140 + 0x1c6
_adxt_start_sj_ref:
    .long   0x8c010000 + 0x32140 + 0x2ce
_adxt_start_stm_ref:
    .long   0x8c010000 + 0x32140 + 0x2fe
_ADXT_Stop_ref:
    .long   0x8c010000 + 0x32140 + 0x3a4
_ADXT_ExecServer_ref:
    .long   0x8c010000 + 0x32140 + 0x63a
_ADXT_EntryErrFunc_ref:
    .long   0x8c010000 + 0x32140 + 0x800

!
! adx_crs.o
!

_ADXCRS_Lock:
    nop
    mov.l   r0, @-r15
    mov.l   _ADXCRS_Lock_ref, r0
    jmp     @r0
    mov.l   @r15+, r0
    nop

_ADXCRS_Unlock:
    nop
    mov.l   r0, @-r15
    mov.l   _ADXCRS_Unlock_ref, r0
    jmp     @r0
    mov.l   @r15+, r0
    nop

_ADXCRS_Lock_ref:
    .long   0x8c010000 + 0x42d34 + 0x8
_ADXCRS_Unlock_ref:
    .long   0x8c010000 + 0x42d34 + 0x30

!
! adx_errs.o
!

_ADXERR_CallErrFunc:
    nop
    mov.l   r0, @-r15
    mov.l   _ADXERR_CallErrFunc_ref, r0
    jmp     @r0
    mov.l   @r15+, r0
    nop

_ADXERR_CallErrFunc_ref:
    .long   0x8c010000 + 0x42d94 + 0x3e

!
! adx_stms.o
!

_ADXSTM_OpenRange:
    nop
    mov.l   r0, @-r15
    mov.l   _ADXSTM_OpenRange_ref, r0
    jmp     @r0
    mov.l   @r15+, r0
    nop

_ADXSTM_GetFad:
    nop
    mov.l   r0, @-r15
    mov.l   _ADXSTM_GetFad_ref, r0
    jmp     @r0
    mov.l   @r15+, r0
    nop

_ADXSTM_GetFsizeSct:
    nop
    mov.l   r0, @-r15
    mov.l   _ADXSTM_GetFsizeSct_ref, r0
    jmp     @r0
    mov.l   @r15+, r0
    nop

_ADXSTM_OpenRange_ref:
    .long   0x8c010000 + 0x4446c + 0x140
_ADXSTM_GetFad_ref:
    .long   0x8c010000 + 0x4446c + 0x540
_ADXSTM_GetFsizeSct_ref:
    .long   0x8c010000 + 0x4446c + 0x55e

!
! ads_fs.o
!

_ADXF_GetFileRange:
    nop
    mov.l   r0, @-r15
    mov.l   _ADXF_GetFileRange_ref, r0
    jmp     @r0
    mov.l   @r15+, r0
    nop

_ADXF_GetFileRange_ref:
    .long   0x8c010000 + 0x42ec0 + 0xc4e
