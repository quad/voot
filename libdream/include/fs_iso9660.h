/* This file is part of the libdream Dreamcast function library.
   Please see libdream.c for further details.

   (c)2000 Dan Potter

   Ported from KallistiOS (Dreamcast OS) for libdream by Dan Potter
*/

#ifndef __FS_ISO9660_H
#define __FS_ISO9660_H

/* Stuff ported over from KOS vfs */
typedef struct dirent {
	int	size;
	char	name[256];
} dirent_t;

#define O_RDONLY	1
#define O_MODE_MASK	7
#define O_DIR		0x1000

#define SEEK_SET	0
#define SEEK_CUR	1
#define SEEK_END	2

/* Other */
#define MAX_ISO_FILES 8
#define MAX_OPEN_FILES MAX_ISO_FILES

/* Prototypes */
uint32	iso_open(const char *path, int flags);
uint32	iso_open_select(const char *path, int flags, uint32 session_base_select);
uint32 iso_open_gdrom(const char *fn, int mode);
void	iso_close(uint32 fd);
int	iso_read(uint32 fd, void *buf, int count);
long	iso_seek(uint32 fd, long offset, int whence);
long	iso_tell(uint32 fd);
int	iso_total(uint32 fd);

dirent_t *iso_readdir(uint32 fd);

int iso_init();

#endif	/* __FS_ISO9660_H */
