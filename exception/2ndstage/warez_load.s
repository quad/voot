	.globl	_warez_load
	.text

_warez_load:
	sts.l	pr, @-r15

!	mov.l   data_size,r3
	mov		r4,r3
	mov	#0,r4
	mov	#0,r5
	add	#1,r5
	mov.l	data_start,r0
	mov.l	dst,r1
loop:
	mov.b	@r0,r2
	mov.b	r2,@r1
	add	#1,r0
	add	#1,r1
	sub	r5,r3
	cmp/eq	r4,r3
	bf	loop
	nop		
	nop
	bsr	ResetGD				! setup drive
	nop
	bsr	Init_drive
	nop
	
self:

	mova	p1search,r0
	mov	r0,r4
	mova	p1replace,r0
	mov	r0,r5
	bsr	patch
	nop
	
	mova	p2search,r0     
	mov	r0,r4           
	mova	p2replace,r0    
	mov	r0,r5           
	bsr	patch           
	nop                     

   
        mov	#0x18,r4
        mov	#0,r5
        bsr	Exec_Cmd
        nop
        
	mov	#0,r1
	mov	r1,r2
	mov	r2,r3
	mov	r3,r4
	mov	r4,r5
	mov	r5,r6
	mov	r6,r7
	mov	r7,r8
	mov	r8,r9
	mov	r9,r10
	mov	r10,r11
	mov	r11,r12
	mov	r12,r13
	mov	r13,r14

	lds.l	@r15+,pr

	mov.l	goexe,r0
	jmp	@r0
	mov	#0,r0
	
.align 4
goexe:		.long 0xac010000

p1search:	.byte	0x00, 0x0e, 0xa0, 0x00, 0xa4, 0x00, 0x00, 0x24, 0x00, 0x08    ,0,0
p1replace:	.byte   0x00, 0x0e, 0xa0, 0x00, 0xa4, 0x00, 0x00, 0x28, 0x00, 0x08    ,0,0

p2search:	.byte	0x7c, 0x67, 0x20, 0x35, 0x01, 0x8d, 0x0f, 0xe3, 0x73, 0x65    ,0,0
p2replace:	.byte   0x73, 0x60, 0x0f, 0xc9, 0x80, 0xcb, 0x0f, 0xe3, 0x03, 0x65    ,0,0

!data_size:
!	.long	_binary_1st_read_bin_size
data_start:
!	.long	_binary_1st_read_bin_start
	.long	0x8C300000
	
ResetGD:
	sts.l	pr, @-r15
	
	mova	GDreset,r0		
	mov.l	@r0,r4
	mova	GDresetval,r0
	mov.l	@r0,r5
	
	mov.l	r5,@r4
	
	mova	Aflushstart,r0
	mov.l	@r0,r4
	mova	Aflushend,r0
	mov.l	@r0,r5
		
aflushlp:				! some flushing?	
	mov.l	@r4,r0
	add	#4,r4
	
	cmp/eq	r4,r5
	bf	aflushlp

	bsr	GdInitSystem		! do a gdGdcinitsystem.
	nop
	lds.l	@r15+,pr
	nop
	rts
	nop
	
	.align 	4

GDreset:	.long	0xa05f74e4
GDresetval:	.long	0x1fffff
Aflushstart:	.long	0xa0000000
Aflushend:	.long	0xa0200000
!-------------------------------------------------------------------------------

Ascihex:

	mov	#0,r0		
	mov	#8,r1
	mov	#9,r3
	mov	#0xf,r5
	mov	#5,r6
	
AH_loop:		
	mov.b	@r4+,r2		! get byte
	
	extu.b	r2,r2		! extend unsigned
	
	add	#-0x30,r2	! substract 0x30
	
	cmp/pz	r2
	bf	AH_invalid
	
	cmp/gt	r3,r2
	bt	AH_high
	
	bra	AH_next
	nop

AH_invalid:			! zero the invalid value.
	bra	AH_next
	mov	#0,r2
	
AH_high:
	
	and	r5,r2
	add	#-1,r2
	
	cmp/pz	r2
	bf	AH_invalid
	
	cmp/gt	r6,r2
	bt	AH_invalid
	
	add	#0xa,r2

AH_next:

	shll2	r0		! shift 4 bits
	shll2	r0
	or	r2,r0		! insert new value
	
	add	#-1,r1
	cmp/pl  r1
	bt	AH_loop
	
	rts
	nop
	


!------------------------------------------------------------------------------
!r4 , r5, r6 args.
	.align	4
	
Exec_Cmd:

	sts.l	pr, @-r15		! save return register
	
	bsr	GdReqCmd		! Issue command
	nop
	
	mov	r0,r4
	mova	cmdnum, r0		! save return value
	mov.l	r4, @r0
	
		
	mova	cmdnum,r0
	mov.l	@r0,r4
	

Execlp1:
	bsr	GdExecServer		! Get result
	nop
	
	mova	cmdnum, r0
	mov.l	@r0, r4
	mova	stat, r0
	mov	r0,r5
	
	bsr	GdGetCmdStat
	nop
	
	cmp/eq	#1, r0			! busy? -> loop
	bt	Execlp1
		
	cmp/eq	#2, r0 
	bt	ExecSuccess
	
	mova	stat,r0
	mov.l	@r0,r0
	
	lds.l	@r15+, pr
	nop
	
	rts
	nop
	
ExecSuccess:

	mov	#0,r0
	lds.l	@r15+,pr
	nop
	rts
	nop	
	
	.align 4
	
cmdnum:		.long 0	
	
stat:		.long 0,0,0,0
!------------------------------------------------------------------------------

Init_drive:

	sts.l	pr,@-r15			! save return value	
	
	mov	#0,r4
	mov.l	r4,@-r15
	
inilp:	
	mov	#24,r4				! Init disc
	mov	#0,r5
	
	bsr	Exec_Cmd
	nop
	
	cmp/eq	#0,r0
	bt	No_Error
	
	
	mov	r0,r1
	mov.l	@r15+,r0
	add	#1,r0
	
	cmp/eq	#8,r0
	bt	Init_Error2
	
	mov.l	r0,@-r15
	bra 	inilp
	nop
	
Init_Error2:
	mov	r1,r0
	bra	Init_Error
	nop	
	
	
	
No_Error:	
	mov.l	@r15+,r0

	mova	iniparam,r0
	mov	r0,r4
	
	bsr	GdGetDrvStat			! Get status
	nop
	
						! Get disc type
	mova	iniparam,r0
	add	#4,r0
	mov.l	@r0,r4	
	nop	
	mov	r4,r0
	nop
	
	cmp/eq	#32,r0				!0x10 for mode 1 CD, 0x20 for mode 2 CD, 0x80 for GD
	bt	id_mode2
	
id_mode1:

	mova	id_m1val,r0
	mov	r0,r5
	mova	id_par2,r0
	mov	r0,r4
	mov.l	@r5,r6
	mov.l	r6,@r4
	bra	id_cont1
	nop
		
id_mode2:	

	mova	id_m2val,r0
	mov	r0,r5
	mova	id_par2,r0
	mov	r0,r4
	mov.l	@r5,r6
	mov.l	r6,@r4

id_cont1:

	mova	id_par0,r0			! setup disctype.
	mov	r0,r4
	bsr	GdChangeDataType
	nop

	lds.l	@r15+,pr
	nop
	rts
	nop

Init_Error:

	lds.l	@r15+,pr
	nop
	rts
	nop

	.align 4
id_m1val:	.long	1024
id_m2val:	.long	2048

id_par0:	.long	0
id_par1:	.long	8192
id_par2:	.long	0
id_par3:	.long	2048

iniparam:	.long	0,0,0,0

!------------------------------------------------------------------------------
	.align 	4
GdReqCmd:			! Perform GD command
	bra	do_syscall
	mov	#0,r7

GdGetCmdStat:			! Get Results from buffer
	bra	do_syscall
	mov	#1,r7
	
GdExecServer:			! Fill buffer with status of current command.
	bra	do_syscall
	mov	#2,r7

GdInitSystem:			! Initialize GD system.
	bra	do_syscall
	mov	#3,r7

GdGetDrvStat:			! Get status of drive/media type
	bra	do_syscall
	mov	#4,r7

GdChangeDataType:		! Prepare GD for GD/CD type.
	mov	#10,r7

do_syscall:
	mov.l	sysvec_bc,r0
	mov	#0,r6
	mov.l	@r0,r0
	jmp	@r0
	nop

	.align	4
sysvec_bc:
	.long	0x8c0000bc


	.align	4
	
patch:

	mov.l 	patchstart, r6             	
        mov.l 	patchend, r7               
        dt 	r6   
        
patchloop1:                                     ! search for matching string           
	add 	#0x01, r6                                    
	cmp/eq 	r6, r7                                  
        bt/s 	patchquit
        sub 	r0, r0     
patchloop2:                                         
        mov.b 	@(r0, r6), r1                             
        mov.b	@(r0, r4), r2                             
        add 	#0x01, r0                                    
        cmp/eq 	r1, r2                                   
        bf/s 	patchloop1
        cmp/eq 	#0x0a, r0                                 
        bf 	patchloop2  
        
patchloop3:                                     ! replace with patch value
        mov.b 	@r5+, r1                                  
        mov.b 	r1, @r6                                   
        dt 	r0                                           
        bf/s 	patchloop3                                
        add 	#0x01, r6                                    
        mov 	#0x01, r0                                    
        
patchquit:  
        rts                                             
        nop                                            
        
        .align 4
        
patchstart:	.long	0xac000000
patchend:       .long	0xac003ff9
     	.align	4    	
dst:
	.long	0xac010000
