#ifndef __GDROM_H__
#define __GDROM_H__

#define GD_TYPE_CDDA    0x00
#define GD_TYPE_CDROM   0x10
#define GD_TYPE_CDXA    0x20
#define GD_TYPE_CDI     0x30
#define GD_TYPE_GDROM   0x80

int gdc_get_drv_stat(void *param);
void wait_for_gdrom(void);
unsigned int gdrom_disc_type(void);

#endif
