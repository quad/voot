/* This file is part of the libdream Dreamcast function library.
   Please see libdream.c for further details.

   (c)2000 Dan Potter

   Ported from KallistiOS (Dreamcast OS) for libdream by Dan Potter
*/

#ifndef __CDROM_H
#define __CDROM_H

/* Command responses */
#define ERR_OK		0
#define ERR_NO_DISC	1
#define ERR_DISC_CHG	2
#define ERR_SYS		3

/* TOC structure returned by the BIOS */
typedef struct {
	uint32	entry[99];
	uint32	first, last;
	uint32	dunno;
} CDROM_TOC;

/* TOC access macros */
#define TOC_LBA(n) ((n) & 0x00ffffff)
#define TOC_ADR(n) ( ((n) & 0x0f000000) >> 24 )
#define TOC_CTRL(n) ( ((n) & 0xf0000000) >> 28 )
#define TOC_TRACK(n) ( ((n) & 0x00ff0000) >> 16 )

/* Command execution sequence */
int cdrom_exec_cmd(int cmd, void *param);

/* Re-init the drive, e.g., after a disc change, etc */
int cdrom_reinit();

/* Read the table of contents */
int cdrom_read_toc(CDROM_TOC *toc_buffer, int session);

/* Read one or more sectors */
int cdrom_read_sectors(void *buffer, int sector, int cnt);

/* Locate the LBA sector of the data track */
uint32 cdrom_locate_data_track(CDROM_TOC *toc, int start);

/* Initialize */
int cdrom_init();


#endif	/* __CDROM_H */
