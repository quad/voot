/* This file is part of the libdream Dreamcast function library.
 * Please see libdream.c for further details.
 *
 * (c)2000 Dan Potter
 */


#ifndef __3D_H
#define __3D_H

/* Yeah, this breaks the normal libdream naming convention.. I'm finally
   sick of it =). Also the OS doesn't use dc_* so I'm getting ready
   for moving to that. */

/* These values are used during TA render completion; the values
   are swapped out to perform page flipping. */
extern struct pv_str {
	uint32	view_address;	/* Display start address */
	uint32	ta_struct;	/* TA structure address */
	uint32	ta_scratch;	/* TA scratch space address */
	uint32	output_address;	/* Render output address */
	uint32	zclip;		/* Min Z clip coord (IEEE float) */
	uint32	pclip_x;	/* Pixel Clipping X (IEEE float) */
	uint32	pclip_y;	/* Pixel Clippnig Y (IEEE float) */
	uint32	disp_align;	/* Display align (xwidth*bpp/8) */
	uint32	alpha_mode;	/* Alpha pixel mode */

	/* Two of these aren't precisely known, but they are all related
	   to the TA buffer structure. The two unknowns are currently
	   mostly/entirely irrelevant but I suspect they deal with
	   further polygon/volume types. */
	uint32	next_ta_buf;	/* Next frame's TA buffer space */
	uint32	unknown2;	/* Next frame's ? */
	uint32	next_ta_scratch;/* Next frame's TA scratch space address */
	uint32	unknown3;	/* Next frame's ? */
} ta_page_values[2];

/* Background plane data: this is a sort of truncated normal
   polygon data set. I'm assuming it's stored in internalized
   PVR format. It's a three-vertex polygon strip. */
typedef struct {
	uint32		flags1, flags2;
	uint32		dummy;
	float		x1, y1, z1;
	uint32		argb1;
	float		x2, y2, z2;
	uint32		argb2;
	float		x3, y3, z3;
	uint32		argb3;
} bkg_poly;

/* Global background data structure; this is used during the
   rendering process. */
extern bkg_poly ta_bkg;


/* Polygon header; each polygon you send to the TA needs to have
   this as its first structure. For flags info, see maiwe's doc. */
typedef struct {
	uint32	flags1, flags2, flags3, flags4;
	uint32	dummy1, dummy2, dummy3, dummy4;
} poly_hdr_t;

/* Vertex structure; each polygon has three or more of these
   arranged in a structure called a strip. If bit 28 is set
   in the flags word, it signifies the end of a strip. Note that
   technically multiple strips don't have to be adjacent but
   they are still part of the same polygon. */

/* Opaque Colored vertex */
typedef struct {
	uint32	flags;
	float	x, y, z;
	float	a, r, g, b;
} vertex_oc_t;

/* Opaque Textured vertex */
typedef struct {
	uint32	flags;
	float	x, y, z, u, v;
	uint32	dummy1, dummy2;
	float	a, r, g, b;
	float	oa, or, og, ob;
} vertex_ot_t;

/* Current page */
extern int ta_curpage;

/* Prepare the TA for page flipped 3D */
void ta_init();

/* Send a store queue full of data to the TA */
void ta_send_queue(void *sql, int size);

/* Begin the rendering process for one frame */
void ta_begin_render();

/* Commit a polygon header to the TA */
void ta_commit_poly_hdr(poly_hdr_t *polyhdr);

/* Commit a vertex to the TA; include sizeof() parameter */
void ta_commit_vertex(void *vertex, int size);

/* Commit an end-of-list to the TA */
void ta_commit_eol();

/* Finish rendering a frame; this assumes you have written
   a completed display list to the TA. It sets everything up and
   waits for the next vertical blank period to switch buffers. */
void ta_finish_frame();

/* Build a polygon header from the given parameters */
void ta_build_poly_hdr(poly_hdr_t *target, int translucent,
	int textureformat, int tw, int th, uint32 textureaddr,
	int filtering);

/* Load texture data into the PVR ram */
void ta_load_texture(uint32 dest, void *src, int size);

/* Return a pointer to write to the texture ram directly */
void *ta_texture_map(uint32 loc);

/* Vertex constants */
#define TA_VERTEX_NORMAL 0xe0000000
#define TA_VERTEX_EOL 0xf0000000

/* Translucency constants */
#define TA_OPAQUE		0
#define TA_TRANSLUCENT		1

/* Texture format constants */
#define TA_NO_TEXTURE		0

#define TA_ARGB1555		((0<<1) | 1)	/* Flat versions */
#define TA_RGB565		((1<<1) | 1)
#define TA_ARGB4444		((2<<1) | 1)
#define TA_YUV422		((3<<1) | 1)
#define TA_BUMP			((4<<1) | 1)

#define TA_ARGB1555_TWID	(0<<1)		/* Twiddled versions */
#define TA_RGB565_TWID		(1<<1)
#define TA_ARGB4444_TWID	(2<<1)
#define TA_YUV422_TWID		(3<<1)
#define TA_BUMP_TWID		(4<<1)

/* Filtering constants */
#define TA_NO_FILTER		0
#define TA_BILINEAR_FILTER	1


#endif	/* __3D_H */
