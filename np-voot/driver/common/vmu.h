/*  vmu.h

    $Id: vmu.h,v 1.1 2002/06/11 21:56:22 quad Exp $

*/

#ifndef __COMMON_VMU_H__
#define __COMMON_VMU_H__

typedef uint8 vmu_port;

#define VMU_PORT_A1             0
#define VMU_PORT_A2             1
#define VMU_PORT_B1             2
#define VMU_PORT_B2             3
#define VMU_PORT_C1             4
#define VMU_PORT_C2             5
#define VMU_PORT_D1             6
#define VMU_PORT_D2             7
#define VMU_PORT_NONE           8

#define VMU_MAX_FILENAME        20

#define VMU_BUFFER_SIZE         0xf420

/*
    NOTE: Possible copyright violation here. The VMU error codes were passed
    to me by a guy on #dcdev. If it's bad for me to have this, I could just
    remove it. Quite frankly, I figured out most of them by myself.
*/

#define VMU_ERR_OK              0x00000000  /* No error */
#define VMU_ERR_BUSY            0xffffffff  /* Busy */
#define VMU_ERR_INVALID_PARAM   0xffffff00  /* Invalid function parameter */
#define VMU_ERR_ILLEGAL_DISK    0xffffff81  /* Illegal disk */
#define VMU_ERR_UNKNOWN_DISK    0xffffff83  /* Not supported disk */
#define VMU_ERR_NO_DISK         0xffffff01  /* Disk is not connected */
#define VMU_ERR_UNFORMAT        0xffffff03  /* Disk is not formatted */
#define VMU_ERR_DISK_FULL       0xffffff04  /* Disk full */
#define VMU_ERR_FILE_NOT_FOUND  0xffffff05  /* File not found */
#define VMU_ERR_FILE_EXIST      0xffffff06  /* File already exists */
#define VMU_ERR_CANNOT_OPEN     0xffffff07  /* Cannot open file */
#define VMU_ERR_CANNOT_CREATE   0xffffff08  /* Cannot create executable */
#define VMU_ERR_EXECFILE_EXIST  0xffffff09  /* Executable file exists */
#define VMU_ERR_CANNOT_DELETE   0xffffff0a  /* Cannot delete file */
#define VMU_ERR_ACCESS_DENIED   0xffffff0b  /* Access is refused */
#define VMU_ERR_VERIFY          0xffffff10  /* Verify error */
#define VMU_ERR_WRITE_ERROR     0xffffff40  /* Write error */
#define VMU_ERR_FILE_BROKEN     0xffffff41  /* File is broken */
#define VMU_ERR_BUPFILE_CRC     0xffffff20  /* CRC Error */
#define VMU_ERR_BUPFILE_ILLEGAL 0xffffff21  /* File is not backup file */
#define VMU_ERR_GENERIC         0xffff0000  /* Unknown error */

/* NOTE: Since they have a common theme. */

#define VMU_STATUS_READY        0x00000000
#define VMU_STATUS_BUSY         0xffffffff

/* NOTE: Module declarations */

void    vmu_init        (void);
uint32  vmu_load_file   (vmu_port port, char *filename, uint8 *obuffer, uint32 num_blocks);
uint32  vmu_status      (vmu_port port);
uint32  vmu_exists_file (vmu_port port, char *filename);
uint32  vmu_mount       (vmu_port port);

#endif
