/*
 * Dreamcast function library (libdream)
 * Portions (c)2000 Dan Potter, Jordan DeLong
 *
 * Many thanks to and helpful info from these guys:
 *  Marcus Comstedt (some cd, sound, video, and maple info)
 *  Mike Brent (early libdream help and serial cable help)
 *  Bitmaster (video info)
 *  HeroZero (waitvbl info)
 *  HitMen-DC (help on gcc)
 *  Chuck Mason (320x240 example code)
 *  Maiwe (TA help)
 *  The DCDev list on eGroups 
 *
 * This library is licensed under the MIT/X style license. For more
 * information, please see COPYING.
 */

#include "dream.h"

/* This pretty much does everything you need to do -- it grabs
   the cable spec, sets up the video, and initializes the serial
   port (for debugging). Returns 0 on success. */
int dc_setup_real(int video_mode, int pixel_mode, int quiet) {
	/* Initialize SCIF */
	serial_init(57600);
	if (!quiet)
		serial_printf("SCIF debug interface initialized\r\n");
	
	/* Init video */
	vid_init(vid_check_cable(), video_mode, pixel_mode);
	vid_clear(0,0,0);
	
	/* Init sound */
	snd_init();
	
	/* Init CD-ROM */
	cdrom_init();
	iso_init();
	/* cd_init(); */

	/* Init input devices (Maple Bus) */
	maple_init(quiet);

	/* Init timers */
	timer_init();

	return 0;
}

/* Non-Quiet version */
int dc_setup(int video_mode, int pixel_mode) {
	return dc_setup_real(video_mode, pixel_mode, 0);
}

/* Quiet version */
int dc_setup_quiet(int video_mode, int pixel_mode) {
	return dc_setup_real(video_mode, pixel_mode, 1);
}

