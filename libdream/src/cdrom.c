/* This file is part of the libdream Dreamcast function library.
   Please see libdream.c for further details.

   keyboard.c
   (c)2000 Dan Potter
*/

#include "dream.h"

/*

This module contains low-level primitives for accessing the CD-Rom (I
refer to it as a CD-Rom and not a GD-Rom, because this code will not
access the GD area, by design). Whenever a file is accessed and a new
disc is inserted, it reads the TOC for the disc in the drive and
gets everything situated. After that it will read raw sectors from 
the data track on a standard DC bootable CDR (one audio track plus
one data track in xa1 format).

Most of the information/algorithms in this file are thanks to
Marcus Comstedt.

Note that these functions may be affected by changing compiler options...
they require their parameters to be in certain registers, which is 
normally the case with the default options. If in doubt, decompile the
output and look to make sure.

Ported from KallistiOS (Dreamcast OS) for libdream by Dan Potter

*/


/* GD-Rom BIOS calls... named mostly after Marcus' code. None have more
   than two parameters; R7 (fourth parameter) needs to describe
   which syscall we want. */

#define MAKE_SYSCALL(rs, p1, p2, idx) \
	uint32 *syscall_bc = (uint32*)0x8c0000bc; \
	int (*syscall)() = (int (*)())(*syscall_bc); \
	rs syscall((p1), (p2), 0, (idx));

/* Reset system functions */
static void gdc_init_system() {	MAKE_SYSCALL(/**/, 0, 0, 3); }

/* Submit a command to the system */
static int gdc_req_cmd(int cmd, void *param) { MAKE_SYSCALL(return, cmd, param, 0); }

/* Check status on an executed command */
static int gdc_get_cmd_stat(int f, void *status) { MAKE_SYSCALL(return, f, status, 1); }

/* Execute submitted commands */
static void gdc_exec_server() { MAKE_SYSCALL(/**/, 0, 0, 2); }

/* Check drive status and get disc type */
static int gdc_get_drv_stat(void *param) { MAKE_SYSCALL(return, param, 0, 4); }

/* Set disc access mode */
static int gdc_change_data_type(void *param) { MAKE_SYSCALL(return, param, 0, 10); }


/* Command execution sequence */
int cdrom_exec_cmd(int cmd, void *param) {
	int status[4];
	int f, n;

	/* Submit the command and wait for it to finish */
	f = gdc_req_cmd(cmd, param);
	do {
		gdc_exec_server();
	} while ((n = gdc_get_cmd_stat(f, status)) == 1);

	if (n == 2)
		return ERR_OK;
	else {
		switch(status[0]) {
			case 2: return ERR_NO_DISC;
			case 6: return ERR_DISC_CHG;
			default:
				return ERR_SYS;
		}
	}
}

/* Re-init the drive, e.g., after a disc change, etc */
int cdrom_reinit() {
	int	rv = ERR_OK;
	int	i, r = -1, cdxa;
	uint32	params[4];

	/* Try a few times; it might be busy. If it's still busy
	   after this loop then it's probably really dead. */
	for (i=0; i<8; i++) {
		if (!(r = cdrom_exec_cmd(24, NULL)))
			break;
	}
	if (i >= 8) { rv = r; goto exit; }
	
	/* Check disc type and set parameters */
	gdc_get_drv_stat(params);
	cdxa = params[1] == 32;
	params[0] = 0;				/* 0 = set, 1 = get */
	params[1] = 8192;			/* ? */
	params[2] = cdxa ? 2048 : 1024;		/* CD-XA mode 1/2 */
	params[3] = 2048;			/* sector size */
	if (gdc_change_data_type(params) < 0) { rv = ERR_SYS; goto exit; }

exit:
	return rv;
}

/* Read the table of contents */
int cdrom_read_toc(CDROM_TOC *toc_buffer, int session) {
	struct {
		int	session;
		void	*buffer;
	} params;
	int rv;
	
	params.session = session;
	params.buffer = toc_buffer;
	rv = cdrom_exec_cmd(19, &params);
	
	return rv;
}

/* Read one or more sectors */
int cdrom_read_sectors(void *buffer, int sector, int cnt) {
	struct {
		int	sec, num;
		void	*buffer;
		int	dunno;
	} params;
	int rv;

	params.sec = sector;	/* Starting sector */
	params.num = cnt;	/* Number of sectors */
	params.buffer = buffer;	/* Output buffer */
	params.dunno = 0;	/* ? */
	rv = cdrom_exec_cmd(16, &params);
	
	return rv;
}

/* Locate the LBA sector of the data track; use after reading TOC */
uint32 cdrom_locate_data_track(CDROM_TOC *toc, int start) {
	int i, first, last, work_from, work_end, mod;
	
	first = TOC_TRACK(toc->first);
	last = TOC_TRACK(toc->last);
	
	if (first < 1 || last > 99 || first > last)
		return 0;

    work_from = (start) ? first : last;
    work_end = (start) ? last : first;
    mod = (start) ? 1 : -1;

	/* Find the last track which as a CTRL of 4 */
	for (i=work_from; ((start) ? (i<=work_end) : (i>=work_end)); i+=mod) {
		if (TOC_CTRL(toc->entry[i - 1]) == 4)
			return TOC_LBA(toc->entry[i - 1]);
	}

	return 0;
}

/* Initialize: assume no threading issues */
int cdrom_init() {
	uint32 p, x;
	volatile uint32 *react = (uint32*)0xa05f74e4,
		*waits = (uint32*)0xa0000000;

	/* Reactivate drive: send the command and then cause
	   some wait states for it to finish. */
	*react = 0x1fffff;
	for (p=0; p<0x200000/4; p++) { x = waits[p]; }

	/* Reset system functions */
	gdc_init_system();

	return 0;
}

