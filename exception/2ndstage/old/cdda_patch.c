/* cdda_patch.c

DESCRIPTION

    CDDA access patching and reporting.

*/

#include "vars.h"
#include "exception.h"
#include "exception-lowlevel.h"
#include "voot.h"
#include "util.h"
#include "adx_tlk2.h"
#include "asic.h"

#include "cdda_patch.h"

#define GDFSDA_BASE     ((uint8 *) 0x8c01e6d0)

static uint8 *gdFsDaPlay_ref = GDFSDA_BASE;
extern uint8 gdFsDaPlay_patch_start[];
extern uint8 gdFsDaPlay_patch_end[];

static uint8 *gdFsDaStop_ref = GDFSDA_BASE + 0x30;
extern uint8 gdFsDaStop_patch_start[];
extern uint8 gdFsDaStop_patch_end[];

static uint8 *gdFsDaPause_ref = GDFSDA_BASE + 0x48;
extern uint8 gdFsDaPause_patch_start[];
extern uint8 gdFsDaPause_patch_end[];

asm("
_gdFsDaPlay_patch_start:

_gdFsDaPlay:
    nop
    nop
    mov.l   r0, @-r15
    mov.l   gdFsDaPlay_patch_location, r0
    jmp     @r0
    mov.l   @r15+, r0

gdFsDaPlay_patch_location:
    .long   _patch_gdFsDaPlay

_gdFsDaPlay_patch_end:

_gdFsDaStop_patch_start:

_gdFsDaStop:
    nop
    nop
    mov.l   r0, @-r15
    mov.l   gdFsDaStop_patch_location, r0
    jmp     @r0
    mov.l   @r15+, r0

gdFsDaStop_patch_location:
    .long   _patch_gdFsDaStop

_gdFsDaStop_patch_end:

_gdFsDaPause_patch_start:

_gdFsDaPause:
    nop
    nop
    mov.l   r0, @-r15
    mov.l   gdFsDaPause_patch_location, r0
    jmp     @r0
    mov.l   @r15+, r0

gdFsDaPause_patch_location:
    .long   _patch_gdFsDaPause

_gdFsDaPause_patch_end:
");

static ADXT     music_handle = NULL;
static uint8    *music_work = NULL;

static char *cdda_track_table[] = {
    "",         /* Track 00: Doesn't exist. */
    "",         /* Track 01: Session 1 audio track. */
    "",         /* Track 02: Session 1 data track. */
    "",         /* Track 03: Session 2 data track. */
    "04.adx",   /* Track 04: transition '99 - oratorio tangram */
    "05.adx",   /* Track 05: jingle */
    "06.adx",   /* Track 06: into the crimson */
    "07.adx",   /* Track 07: bloody sorrow */
    "08.adx",   /* Track 08: above and beyond */
    "09.adx",   /* Track 09: sunshine generator */
    "10.adx",   /* Track 10: triplet repeat */
    "11.adx",   /* Track 11: high on hope */
    "12.adx",   /* Track 12: zodiac empathy */
    "13.adx",   /* Track 13: ocean whispers */
    "14.adx",   /* Track 14: sweet tragedy of dreams */
    "15.adx",   /* Track 15: movin' melodies */
    "16.adx",   /* Track 16: monstrous beat */
    "17.adx",   /* Track 17: free radical */
    "18.adx",   /* Track 18: 13 seconds warning */
    "19.adx",   /* Track 19: among fields of crystal */
    "20.adx",   /* Track 20: encoded final */
    "21.adx",   /* Track 21: winner */
    "22.adx",   /* Track 22: winner ver.1 */
    "23.adx",   /* Track 23: winner ver.2 */
    "24.adx",   /* Track 24: loser 1 */
    "25.adx",   /* Track 25: loser 2 */
    "26.adx",   /* Track 26: loser */
    "27.adx",   /* Track 27: draw game */
    "28.adx",   /* Track 28: reincarnation */
    "29.adx",   /* Track 29: regeneration */
    "30.adx",   /* Track 30: subconscious awareness */
    "31.adx",   /* Track 31: withdrawal #1 */
    "32.adx",   /* Track 32: withdrawal #2 */
    "33.adx",   /* Track 33: energy flux */
    "34.adx",   /* Track 34: intermission */
    "35.adx",   /* Track 35: fatal option #1 */
    "36.adx",   /* Track 36: msbs ver5.4(DNA custom) */
    "37.adx",   /* Track 37: sally (DNA) */
    "38.adx",   /* Track 38: radio sprite */
    "39.adx",   /* Track 39: close to me */
    "40.adx",   /* Track 40: out of sight and mind */
    "41.adx",   /* Track 41: my lost love hunting your lost face */
    "42.adx",   /* Track 42: new way home */
    "43.adx",   /* Track 43: no more tears */
    "44.adx",   /* Track 44: burning inside */
    "45.adx",   /* Track 45: coral flanger */
    "46.adx",   /* Track 46: solider blue */
    "47.adx",   /* Track 47: fatal option #2 */
    "48.adx",   /* Track 48: msbs ver5.4(RNA custom) */
    "49.adx",   /* Track 49: sally (RNA) */
    "50.adx",   /* Track 50: encounter */
    "51.adx",   /* Track 51: encounter 2 */
    "52.adx",   /* Track 52: THE WIND IS BLOWING */
    "53.adx",   /* Track 53: EVERYTHING MERGES WITH THE NIGHT */
    "54.adx",   /* Track 54: EARTH LIGHT */
    "55.adx",   /* Track 55: FADE TO BLACK */
    "56.adx",   /* Track 56: Rank UP */
    "57.adx",   /* Track 57: Rank DOWN */
};

void handle_ADXT_error(void *obj, char *msg)
{
    voot_printf(VOOT_PACKET_TYPE_DEBUG, "ADXT Error: '%s'", msg);
}

static void adxt_heartbeat(void)
{
    voot_printf(VOOT_PACKET_TYPE_DEBUG, "ADXT heartbeat start.");
    ADXT_ExecServer();
    voot_printf(VOOT_PACKET_TYPE_DEBUG, "ADXT heartbeat end.");
}

void* handle_adxt_heartbeat(void *passer, register_stack *stack, void *current_vector)
{
    ((asic_lookup_table_entry *) passer)->clear_irq = FALSE;

    adxt_heartbeat();

    return current_vector;
}


static void maybe_init_adx(void)
{
    asic_lookup_table_entry new_irq;

    /* STAGE: See if we should first ... */
    if (music_work || music_handle)
        return;

    /* STAGE: Initialize the ADXT system. */
    //ADXT_Init();
    voot_printf(VOOT_PACKET_TYPE_DEBUG, "Adding V-Sync callback for ADXT...");
    new_irq.irq = EXP_CODE_INT9;
    new_irq.mask0 = ASIC_MASK0_VSYNC;
    new_irq.handler = handle_adxt_heartbeat;
    add_asic_handler(&new_irq);

    /* STAGE: Receive errors. */
    voot_printf(VOOT_PACKET_TYPE_DEBUG, "Passing error callback function...");
    ADXT_EntryErrFunc(handle_ADXT_error, NULL);

    /* STAGE: Allocate sound memory and create the handle. */
    music_work = malloc(ADXT_WORKSIZE);
    if (music_work)
    {
        voot_printf(VOOT_PACKET_TYPE_DEBUG, "Creating ADXT handle...");

        music_handle = ADXT_Create(ADXT_USE_NCH, music_work, ADXT_WORKSIZE);
    }
}

void cdda_enable(void)
{
    /* STAGE: Install the various function patches. */
    memcpy(gdFsDaPlay_ref, gdFsDaPlay_patch_start, (gdFsDaPlay_patch_end - gdFsDaPlay_patch_start));
    memcpy(gdFsDaStop_ref, gdFsDaStop_patch_start, (gdFsDaStop_patch_end - gdFsDaStop_patch_start));
    memcpy(gdFsDaPause_ref, gdFsDaPause_patch_start, (gdFsDaPause_patch_end - gdFsDaPause_patch_start));
}

int32 patch_gdFsDaPlay(int32 start_track, int32 end_track, int32 repeat_mode)
{
    maybe_init_adx();

    if (music_handle && cdda_track_table[start_track])
    {
        voot_printf(VOOT_PACKET_TYPE_DEBUG, "[play] Playing track %d. (%s)", start_track, cdda_track_table[start_track]);

        ADXT_StartFname(music_handle, cdda_track_table[start_track]);

#if 0
        for (;;)
        {
            int32 x;

            voot_printf(VOOT_PACKET_TYPE_DEBUG, "[play] I'm still alive.");

            for (x = 0; x < 60; x++)
                vid_waitvbl();
        }
#endif

        voot_printf(VOOT_PACKET_TYPE_DEBUG, "[play] ADXT_StartFname returned.");
    }
    else if (music_handle)
    {
        voot_printf(VOOT_PACKET_TYPE_DEBUG, "[play] Couldn't locate file for track %d.", start_track);
    }
    else
    {
        voot_printf(VOOT_PACKET_TYPE_DEBUG, "[play] Handle never initialized.");
    }

    return 0;
}

int32 patch_gdFsDaStop(void)
{
    maybe_init_adx();

    if (music_handle)
    {
        voot_printf(VOOT_PACKET_TYPE_DEBUG, "[stop] Stopping.");

        ADXT_Stop(music_handle);
    }
    else
    {
        voot_printf(VOOT_PACKET_TYPE_DEBUG, "[stop] Handle never initialized.");
    }

    return 0;
 }

int32 patch_gdFsDaPause(void)
{
    maybe_init_adx();

    if (music_handle)
    {
        voot_printf(VOOT_PACKET_TYPE_DEBUG, "[pause] We can't pause the music yet!");
    }
    else
    {
        voot_printf(VOOT_PACKET_TYPE_DEBUG, "[pause] Handle never initialized.");
    }

    return 0;
}
