/*  main.c

    $Id: main.c,v 1.1 2002/06/06 19:57:09 quad Exp $

DESCRIPTION

    A full graphical loader for Netplay VOOT Extensions.

    This is the main initialization module.

CHANGELOG

    Tue May 28 09:28:32 PDT 2002    Scott Robinson <scott_vo@quadhome.com>
        First revision of the graphical loader. Imported sources from the
        debug driver.

*/

#include <kos.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include "gdrom.h"
#include "boot.h"
#include "anim.h"

#include "main.h"

extern uint8 romdisk[];
KOS_INIT_ROMDISK (romdisk);

GLuint texture[1];

static void graphic_render (void)
{
    glKosBeginFrame ();

    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glLoadIdentity ();

    glScalef (512.0f, 154.0f, 0.0f);

    glDisable (GL_TEXTURE_2D);

    glBegin (GL_TRIANGLE_STRIP);

        glColor3f (0.0f, 1.0f, 0.0f);

        glVertex3f (0.0f, 1.0f, 0.5f);

        glVertex3f (1.0f, 1.0f, 0.5f);

        glVertex3f (0.0f, 0.0f, -0.5f);

        glVertex3f (1.0f, 0.0f, -0.5f);

    glEnd ();

    glEnable (GL_TEXTURE_2D);

    glBindTexture (GL_TEXTURE_2D, texture[0]);

    glBegin (GL_TRIANGLE_STRIP);

        glColor3f (1.0f, 1.0f, 1.0f);

        glTexCoord2f (0.0f, (154.0f/256.0f));
        glVertex3f (0.0f, 1.0f, 0.0f);

        glTexCoord2f (1.0f, (154.0f/256.0f));
        glVertex3f (1.0f, 1.0f, 0.0f);

        glTexCoord2f (0.0f, 0.0f);
        glVertex3f (0.0f, 0.0f, 0.0f);

        glTexCoord2f (1.0f, 0.0f);
        glVertex3f (1.0f, 0.0f, 0.0f);

    glEnd ();

    glKosFinishFrame ();
}

int main (void)
{
    /* STAGE: Initialize the animation system. */

    anim_init ();

    /* STAGE: Load our single texture. */

    gl_load_texture_png ("/rd/netplay.png", &texture[0]);

    /* STAGE: If the START button is pressed, exit out of the loader completely. */

    cont_btn_callback (maple_first_controller (), CONT_START, (cont_btn_callback_t) arch_exit);

    /* STAGE: Keep us looping until a valid disc is inserted. */

    for (;;)
    {
        /* STAGE: Render the current scene. */

        graphic_render ();
    }

    /* STAGE: Deinitialize the entire animation system. */

    anim_shutdown ();
}
