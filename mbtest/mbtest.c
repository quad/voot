#include <dream.h>

void wait_for_controller (void)
{
    unsigned char cmaddr;
    int press_ok;

    vid_clear(0,100,0);

    bfont_draw_str(vram_s+10*640+20, 640, "Initializing Maple Bus subsystem...");
    maple_init(1);

    /* Wait for a keypress from a controller */
    bfont_draw_str(vram_s+30*640+20, 640, "Press Start to continue.");

    cmaddr = maple_controller_addr();
    if(!cmaddr)
    {
        bfont_draw_str(vram_s+50*640+20, 640, "Sucks to be controllerless?");
        return;
    }

    press_ok = 0;
    while(!press_ok)
    {
        cont_cond_t cond;

        cont_get_cond(cmaddr, &cond);
        if (!(cond.buttons & CONT_START))
        {
            bfont_draw_str(vram_s+70*640+20, 640, "Someone pressed Start!");
            press_ok = 1;
        }

        vid_waitvbl();
    }

    bfont_draw_str(vram_s+50*640+20, 640, "Let's go, Gekigangar Punch!\n");
    maple_shutdown();
}

void dc_main(void)
{
    vid_init(vid_check_cable(), DM_640x480, PM_RGB565);

    vid_clear(0,0,0);

    wait_for_controller();

    bfont_draw_str(vram_s+300*640+20, 640, "We're finished. I guess something bad happened.");
    while(1);
}
