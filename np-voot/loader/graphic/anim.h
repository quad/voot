/*  anim.h

    $Id: anim.h,v 1.1 2002/06/05 06:46:01 quad Exp $

*/

#ifndef __LOADER_ANIM_H__
#define __LOADER_ANIM_H__

typedef enum
{
    ANIM_INIT,
    ANIM_START,
    ANIM_GO,
    ANIM_STOP,
    ANIM_STOPPED,
    ANIM_DONE
} anim_status_e;

typedef void anim_control_f (void *);

typedef struct
{
    GLuint          x;
    GLuint          y;
    GLuint          z;

    anim_status_e   status;

    anim_control_f  *func;
} anim_t;

int gl_load_texture_png (const char *filename, GLuint *texture);
void anim_init (void);
void anim_shutdown (void);

#endif
