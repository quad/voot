/*  anim.c

DESCRIPTION

    Animation control system for the graphical loader.

CHANGELOG

    Tue Jun  4 20:30:08 PDT 2002    Scott Robinson <scott_vo@quadhome.com>
        Began prototyping the system interface.

*/

#include <kos.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <png/png.h>

#include "anim.h"

int gl_load_texture_png (const char *filename, GLuint *texture)
{
    uint32 width, height;
    uint32 texture_address;
    
    /* STAGE: Load the texture from the specified PNG. */

    if (png_load_texture (filename, &texture_address, PNG_NO_ALPHA, &width, &height))
    {
        printf ("Unable to load '%s' as a PNG for texturing.", filename);

        return 0;
    }

    /* STAGE: Allocate a kGL texture. */

    glGenTextures (1, texture);
    glBindTexture (GL_TEXTURE_2D, *texture);
    glKosTex2D (GL_RGB565_TWID, width, height, (pvr_ptr_t) texture_address);

    return 1;
}

static void gl_configure (void)
{
    /* STAGE: Initialize the kGL sub-system. */

    glKosInit ();

    /* STAGE: Configure the screen for 640x480 with the origin being in the upper left. */

    glMatrixMode (GL_PROJECTION);
    glLoadIdentity ();
    glOrtho (0.0f, 640.0f, 480.0f, 0.0f, 0.0f, 1.0f);

    /* STAGE: Reset for drawing. */

    glMatrixMode (GL_MODELVIEW);
    glLoadIdentity ();

    /* STAGE: Configure kGL for textures. */

    glEnable (GL_TEXTURE_2D);

    /* STAGE: Configure the Z Buffer. */

    glEnable (GL_DEPTH_TEST);
    glDepthFunc (GL_LEQUAL);
}

void anim_init (void)
{
    /* STAGE: Configure the PVR. */

    pvr_init_defaults ();

    /* STAGE: Configure the KOS OpenGL engine. */

    gl_configure ();
}

void anim_shutdown (void)
{
    /* STAGE: We don't do anything yet. */
}
