/* This file is part of the libdream Dreamcast function library.
 * Please see libdream.c for further details.
 *
 * (c)2000 Dan Potter
 */

/* There are still potentially some Issues with turning on transparency
   in this... *sigh* */


#include "dream.h"
#include "ta.h"

/* These values are used during TA render completion; the values
   are swapped out to perform page flipping. */
struct pv_str ta_page_values[2];

/* Global background data structure; this is used during the
   rendering process. */
bkg_poly ta_bkg;

/* Current page */
int ta_curpage = 0;

/* Submitted polygons since the last EOL */
int ta_submitted_polys;

/* 3d-specific parameters; these are all about rendering and nothing
   to do with setting up the video, although these do currently assume
   a 640x480x16bit screen. Some stuff in here is still unknown. */
uint32 three_d_parameters[] = {
	0x80a8, 0x15d1c951,	/* M (Unknown magic value) */
	0x80a0, 0x00000020,	/* M */
	0x8008, 0x00000000,	/* TA out of reset */
	0x8048, 0x00000009,	/* alpha config */
	0x8068, 0x02800000,	/* pixel clipping x */
	0x806c, 0x01e00000,	/* pixel clipping y */
	0x8110, 0x00093f39,	/* M */
	0x8098, 0x00800408,	/* M */
	0x804c, 0x000000a0,	/* display align (640*2)/8 */
	0x8078, 0x3f800000,	/* polygon culling (1.0f) */
	0x8084, 0x00000000,	/* M */
	0x8030, 0x00000101,	/* M */
	0x80b0, 0x007f7f7f,	/* Fog table color */
	0x80b4, 0x007f7f7f,	/* Fog vertex color */
	0x80c0, 0x00000000,	/* color clamp min */
	0x80bc, 0xffffffff,	/* color clamp max */
	0x8080, 0x00000007,	/* M */
	0x8074, 0x00000001,	/* cheap shadow */
	0x807c, 0x0027df77,	/* M */
	0x8008, 0x00000001,	/* TA reset */
	0x8008, 0x00000000,	/* TA out of reset */
	0x80e4, 0x00000000,	/* stride width */
	0x6884, 0x00000000,	/* Disable all interrupt events */
	0x6930, 0x00000000,
	0x6938, 0x00000000,
	0x6900, 0xffffffff,	/* Clear all pending int events */
	0x6908, 0xffffffff,
	0x6930, 0x002807ec,	/* Re-enable some events */
	0x6938, 0x0000000e,
	0x80b8, 0x0000ff07,	/* fog density */
	0x80b4, 0x007f7f7f,	/* fog vertex color */
	0x80b0, 0x007f7f7f	/* fog table color */
};

/* We wait for vertical blank (to make it nicer looking) and then
   set these screen parameters. These are equivalent to libdream's
   vid_setup() plus a few extras. */
uint32 scrn_parameters[] = {
//	0x80e8, 0x00160008,	/* screen control */
//	0x8044, 0x00000000,	/* pixel mode (vb+0x11) */
//	0x805c, 0x00000000,	/* Size modulo and display lines (vb+0x17) */
//	0x80d0, 0x00000150,	/* interlace flags */
//	0x80d8, 0x020c0359,	/* M */
	0x80cc, 0x00150104,	/* M */
	0x80d4, 0x007e0345,	/* horizontal border */
//	0x80dc, 0x00240204,	/* vertical position */
	0x80e0, 0x07d6c63f,	/* sync control */
//	0x80ec, 0x000000a4,	/* horizontal position */
//	0x80f0, 0x00120012,	/* vertical border */
	0x80c8, 0x03450000,	/* set to same as border H in 80d4 */
	0x8068, 0x027f0000,	/* (X resolution - 1) << 16 */
	0x806c, 0x01df0000,	/* (Y resolution - 1) << 16 */
//	0x805c, 0x1413b93f,	/* Size modulo and display lines (vb+0x17) */
	0x804c, 0x000000a0,	/* display align */
	0x8118, 0x00008040,	/* M */
	0x80f4, 0x00000401,	/* anti-aliasing */
	0x8048, 0x00000009,	/* alpha config */
//	0x8044, 0x00000004,	/* pixel mode (vb+0x11) */
	0x7814, 0x00000000,	/* More interrupt control stuff (so it seems) */
	0x7834, 0x00000000,
	0x7854, 0x00000000,
	0x7874, 0x00000000,
	0x78bc, 0x4659404f,
	0x8040, 0x00000000	/* border color */
};



/* Initialize fog tables; we don't use these right now but
   it's part of the proper setup. */
static void ta_fog_init() {
	volatile uint32 *regs = (uint32*)0xa05f0000;
	uint32	idx;
	uint32	value;
	
	for (idx=0x8200, value=0xfffe; idx<0x8400; idx+=4) {
		regs[idx/4] = value;
		value -= 0x101;
	}
}

/* Set up TA buffers. This function takes a base address and sets up
   the TA rendering structures there. Each tile of the screen (32x32) receives
   a small buffer space. This function currently assumes 640x480. */
static void ta_create_buffers(uint32 strbase, uint32 bufbase) {
	int x, y;
	volatile uint32 *vr = (uint32*)0xa5000000;
	
	for (x=0; x<0x48; x+=4)
		vr[(strbase+x)/4] = 0;
	vr[(strbase+0x48)/4] = 0x10000000;
	vr[(strbase+0x4c)/4] = 0x80000000;
	vr[(strbase+0x50)/4] = 0x80000000;
	vr[(strbase+0x54)/4] = 0x80000000;
	vr[(strbase+0x58)/4] = 0x80000000;
	vr[(strbase+0x5c)/4] = 0x80000000;
	vr += (strbase+0x60)/4;
	for (x=0; x<(640/32); x++) {
		for (y=0; y<(480/32); y++) {
			/* Tile index; note: end-of-list on the last tile! */
			if (x == (640/32)-1 && y == (480/32)-1)
				vr[0] = 0x80000000 | (y << 8) | (x << 2);
			else
				vr[0] = (y << 8) | (x << 2);

			/* Opaque poly buffer */
			vr[1] = bufbase + 0x500 * y + 0x40 * x;
			/* Opaque volume mod buffer */
			vr[2] = (bufbase+0x4b00) | 0x80000000;
			/* Translucent poly buffer */
			vr[3] = bufbase+0x4b00 + 0x500 * y + 0x40 * x;
			/* Translucent volume mod buffer */
			vr[4] = strbase | 0x80000000;
			/* Punch-thru poly buffer */
			vr[5] = strbase | 0x80000000;
			vr += 6;
		}
	}
}

/* Take a series of register/value pairs and set the values */
static void set_regs(uint32 *values, uint32 cnt) {
	volatile uint32 long *regs = (uint32*)0xa05f0000;
	int i;
	uint32 r, v;

	for (i=0; i<cnt; i+=2) {
		r = values[i];
		v = values[i+1];
		regs[r/4] = v;
	}
}

/* Set a TA structure buffer; after this completes, all rendering
   and setup operations will reference the given buffer. */
static void ta_select_buffer(int which) {
	volatile uint32	*regs = (uint32*)0xa05f0000;
	struct pv_str	*pv = ta_page_values+which;

	regs[0x8008/4] = 1;		/* Reset TA */
	regs[0x8008/4] = 0;
	regs[0x8124/4] = pv->next_ta_buf;
	regs[0x812c/4] = pv->unknown2;
	regs[0x8128/4] = pv->next_ta_scratch;
	regs[0x8130/4] = pv->unknown3;
	regs[0x813c/4] = 0x000e0013;	/* Tile count: (480/32-1) << 16 | (640/32-1) */
	regs[0x8164/4] = pv->next_ta_buf;
	regs[0x8140/4] = 0x00100002 | 0x200;	/* Translucent enable */
	regs[0x8144/4] = 0x80000000;	/* Confirm settings */
	(void)regs[0x8144/4];
}


/* Prepare the TA for page flipped 3D */
void ta_init() {
	volatile unsigned long *regs = (unsigned long*)0xa05f0000;

	/* Prepare TA value structure */
#define pv ta_page_values[0]
	pv.view_address	= 0x00600000;
	pv.ta_struct	= 0x005683c8;
	pv.ta_scratch	= 0x00400000;
	pv.output_address = 0x00200000;
	pv.zclip	= 0x3e4cccc0;		/* 0.2f */
	pv.pclip_x	= 0x027f0000;
	pv.pclip_y	= 0x01df0000;
	pv.disp_align	= 0x000000a0;		/* (640*2)/8 */
	pv.alpha_mode	= 0x00000009;
	pv.next_ta_buf	= 0x0015ed80;		/* Opaque poly buffers */
	pv.unknown2	= 0x0010e800;		/* ? buffers */
	pv.next_ta_scratch = 0x00000000;
	pv.unknown3	= 0x0010e740;		/* ? buffers */
#undef pv
#define pv ta_page_values[1]
	pv.view_address	= 0x00200000;
	pv.ta_struct	= 0x001683c8;
	pv.ta_scratch	= 0x00000000;
	pv.output_address = 0x00600000;
	pv.zclip	= 0x3e4cccc0;		/* 0.2f */
	pv.pclip_x	= 0x027f0000;
	pv.pclip_y	= 0x01df0000;
	pv.disp_align	= 0x000000a0;		/* (640*2)/8 */
	pv.alpha_mode	= 0x00000009;
	pv.next_ta_buf	= 0x0055ed80;		/* Opaque poly buffers */
	pv.unknown2	= 0x0050e800;		/* ? buffers */
	pv.next_ta_scratch = 0x00400000;
	pv.unknown3	= 0x0050e740;		/* ? buffers */
#undef pv

	/* Blank screen and reset display enable */
	regs[0x80e8/4] |= 0x00000008;	/* Blank */
	regs[0x8044/4] &= ~0x00000001;	/* Display disable */

	/* Fully reset the TA */
	regs[0x8008/4] = 0xffffffff;
	regs[0x8008/4] = 0;

	/* Clear out video memory */
	vid_empty();

	/* Setup basic 3D parameters */
	set_regs(three_d_parameters, sizeof(three_d_parameters)/4);
	ta_fog_init();

	/* Set screen mode parameters */
	vid_waitvbl();
	set_regs(scrn_parameters, sizeof(scrn_parameters)/4);	

	/* Point at the second set of buffer structures, 
	   and build said structures. */
	ta_select_buffer(1);
	// ta_create_buffers(0x568380, 0x563880);
	ta_create_buffers(0x568380, 0x55ed80);

	/* Now setup the first frame */
	ta_select_buffer(0);
	// ta_create_buffers(0x168380, 0x163880);
	ta_create_buffers(0x168380, 0x15ed80);

	/* Set starting render output addresses */
	regs[0x8060/4] = 0x00200000;	/* render output address */
	regs[0x8064/4] = 0x00600000;	/* render to texture output address */
	
	/* Set display start address */
	vid_set_start(0x00200000);

	/* Point back at the second output buffer */
	ta_select_buffer(1);

	/* Unblank screen and set display enable */
	regs[0x80e8/4] &= ~0x00000008;	/* Unblank */
	regs[0x8044/4] |= 0x00000001;	/* Display enable */

	/* Set current page */
	ta_curpage = 0;

	/* And submitted polys */
	ta_submitted_polys = 0;
}


/* Copy data 4 bytes at a time */
static void copy4(uint32 *dest, uint32 *src, int bytes) {
	bytes = bytes / 4;
	while (bytes-- > 0) {
		*dest++ = *src++;
	}
}

/* Send a store queue full of data to the TA */
void ta_send_queue(void *sql, int size) {
	volatile unsigned long *regs = (unsigned long*)0xff000038;

	/* Set store queue destination == tile accelerator */
	regs[0] = regs[1] = 0x10;

	/* Post the first queue */
	copy4((uint32*)0xe0000000, (uint32*)sql, size);
	asm("mov	#0xe0,r0");
	asm("shll16	r0");
	asm("shll8	r0");
	asm("pref	@r0");

	/* If there was a second queue... */
	if (size == 64) {
		asm("mov	#0xe0,r0");
		asm("shll16	r0");
		asm("shll8	r0");
		asm("or		#0x20,r0");
		asm("pref	@r0");
	}
}

/* Begin the rendering process for one frame */
void ta_begin_render() {
	/* Clear all pending events */
	volatile unsigned long *pvrevt = (unsigned long*)0xa05f6900;
	*pvrevt = 0xffffffff;
}

/* Commit a polygon header to the TA */
void ta_commit_poly_hdr(poly_hdr_t *polyhdr) {
	ta_send_queue(polyhdr, 32);
	ta_submitted_polys++;
}

/* Commit a vertex to the TA */
void ta_commit_vertex(void *vertex, int size) {
	ta_send_queue(vertex, size);
}

/* Commit an end-of-list to the TA */
void ta_commit_eol() {
	uint32	words[8] = { 0 };
	ta_send_queue(words, 32);
	ta_submitted_polys = 0;
}

/* Finish rendering a frame; this assumes you have written
   a completed display list to the TA. It sets everything up and
   waits for the next vertical blank period to switch buffers. */
void ta_finish_frame() {
	int i, taend;
	volatile unsigned long *regs = (unsigned long*)0xa05f0000;
	volatile unsigned long *vrl = (unsigned long *)0xa5000000;
	struct pv_str	*pv;
	uint32 *bkgdata = (uint32*)&ta_bkg;

	/* Wait for TA to finish munching (including translucent data) */
	while (!(regs[0x6900/4] & 0x80))
		;
	regs[0x6900/4] = 0x80;

	/* Throw the background data on the end of the TA's list */
	taend = regs[0x8138/4];
	for (i=0; i<0x40; i+=4)
		vrl[(i+taend)/4] = bkgdata[i/4];
	vrl[(0x44+taend)/4] = 0;	/* not sure if this is required */
	
	/* Wait for the VB to get here */
	regs[0x6900/4] = 0x08;
	while (!(regs[0x6900/4] & 0x08))
		;
	regs[0x6900/4] = 0x08;

	/* Find the register values for the current page */
	pv = ta_page_values + ta_curpage;

	/* Calculate background value for below */
	/* Small side note: during setup, the value is originally
	   0x01203000... I'm thinking that the upper word signifies
	   the length of the background plane list in dwords
	   shifted up by 4. */
	taend = 0x01000000 | ((taend - pv->ta_scratch) << 1);
	
	/* Switch start address to the "good" buffer
	   Ok this is REALLY weird.. but it works.. whatever =) */
	if (vid_cable_type == 0)
		vid_set_start(ta_page_values[ta_curpage ^ 1].view_address);
	else
		vid_set_start(pv->view_address);
		
	/* Finish up rendering the current frame (into the other buffer) */
	regs[0x802c/4] = pv->ta_struct;
	regs[0x8020/4] = pv->ta_scratch;
	regs[0x8060/4] = pv->output_address;
	regs[0x808c/4] = taend;			/* Bkg plane location */
	regs[0x8088/4] = pv->zclip;
	regs[0x8068/4] = pv->pclip_x;
	regs[0x806c/4] = pv->pclip_y;
	regs[0x804c/4] = pv->disp_align;
	regs[0x8048/4] = pv->alpha_mode;
	regs[0x8014/4] = 0xffffffff;		/* Go! */
	
	/* Reset TA, switch buffers */
	ta_select_buffer(ta_curpage);

	/* Swap out pages */
	ta_curpage ^= 1;
}

/* Build a polygon header from the given parameters; this is pretty
   incomplete right now but it's better than having to do it by hand. */
void ta_build_poly_hdr(poly_hdr_t *target, int translucent,
		int textureformat, int tw, int th, uint32 textureaddr,
		int filtering) {
	if (textureformat) {
		int i, ts = 8, n = 3;
		for (i=0; i<8 && n; i++) {
			if ((n&1) && tw == ts) {
				tw = i;
				n &= ~1;
			}
			if ((n&2) && th == ts) {
				th = i;
				n &= ~2;
			}
			ts <<= 1;
		}
		textureformat <<= 26;		
	}
	
	if (!translucent && !textureformat) {
		target->flags1 = 0x80870012;
		target->flags2 = 0x90800000;
		target->flags3 = 0x20800440;
		target->flags4 = 0x00000000;
		target->dummy1 = target->dummy2
			= target->dummy3 = target->dummy4 = 0xffffffff;
	} else if (translucent && !textureformat) {
		target->flags1 = 0x82840012;
		target->flags2 = 0x90800000;
		target->flags3 = 0x949004c0;
		target->flags4 = 0x00000000;
		target->dummy1 = target->dummy2
			= target->dummy3 = target->dummy4 = 0xffffffff;
	} else if (!translucent && textureformat) {
		target->flags1 = 0x8084001a;
		target->flags2 = 0x90800000;
		target->flags3 = 0x20800440 | (tw << 3) | th;
		if (filtering)
			target->flags3 |= 0x2000;
		target->flags4 = textureformat | (textureaddr >> 3);
	} else if (translucent && textureformat) {
		target->flags1 = 0x8284001a;
		target->flags2 = 0x92800000;
		target->flags3 = 0x949004c0 | (tw << 3) | th;
		if (filtering)
			target->flags3 |= 0x2000;
		target->flags4 = textureformat | (textureaddr >> 3);
	}
}

/* Load texture data into the PVR ram */
void ta_load_texture(uint32 dest, void *src, int size) {
	volatile uint32 *destl = (uint32*)(0xa4000000 | dest);
	uint32 *srcl = (uint32*)src;
	
	if (size % 4)
		size = (size/4)+1;
	else
		size = size/4;

	while (size-- > 0)
		*destl++ = *srcl++;
}

/* Return a pointer to write to the texture ram directly */
/* Danger, DANGER WILL ROBINSON: Compiling this with -O2 makes
   it "optimize out" the addition! Unless we take special steps.. 
   Bug in GCC? */
void *ta_texture_map(uint32 loc) {
	uint32 final = 0xa4000000 + loc;
	return (void *)final;
}









