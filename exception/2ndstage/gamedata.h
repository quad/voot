#ifndef __GAMEDATA_H__
#define __GAMEDATA_H__

/*

    Gamedata structure starts at: 0x8CCF9ECC

    01-XXXXXX   8-bit constant write
    000000YY

    01-CF9F58   Debug Mode
    00000001

    01-CF9EFD
    00000001    Dumb AI Mode (only in single player)

*/

typedef struct
{
    uint16 arcade;          /* 0x1A-0x1B: FD 0F is 5.2 enabled. */

    uint8 difficulty;       /* 0x1D: 1-8 legal values. */
    uint8 match_time_1p;    /* 0x1E: 0-4 legal values. 0 is 60 seconds. 4 is Deathmatch. */
    uint8 match_time_vs;    /* 0x1F: 0-4 legal values. 0 is 60 seconds. 4 is Deathmatch. */
    uint8 match_count_1p;   /* 0x20: 0-3 legal values. (1/2/3/5) */
    uint8 match_count_vs;   /* 0x21: 0-3 legal values. (1/2/3/5) */

    uint8 muteki_p1;        /* 0x2E: 0 is off. 1 is on - non single player */
    uint8 muteki_p2;        /* 0x2F: 0 is off. 1 is on - non single player */

    uint8 enemy_shoot;      /* 0x31: 0 is shooting. 1 is no shooting. */
    uint8 slow;             /* 0x32: 0 is off. 1 is on. */
    uint8 move_vr;          /* 0x33: 0 is 1P. 1 is 2P. */

    uint8 controller_a;     /* 0x37: 0x0 - 0x5 (A/B/C/D/E/F) */
    uint8 controller_b;     /* 0x38: 0x0 - 0x5 (A/B/C/D/E/F) */

uint8 unknown_a;            /* 0x3B: 0x1 */

    uint8 paused;           /* 0x3C: 0 is unpaused. 1 is paused. */

uint8 unknown_b;            /* 0x45: Game complete, temjin in fog. */

    uint8 survival;         /* 0x48: 0 is off. 1 is on. */
    uint8 emb_p1;           /* 0x49: 0 is off. 1 is on. */
    uint8 emb_p2;           /* 0x4A: 0 is off. 1 is on. */
    uint8 com_p1;           /* 0x4B: 0 is off. 1 is Gold. 2 is Silver. */
    uint8 com_p2;           /* 0x4C: 0 is off. 1 is Gold. 2 is Silver. */

    uint8 attract_cycle;    /* 0x4F: 0x0 - 0x4 which attract cycle. */

    uint8 fog_mode;         /* 0x5D: 0 is off. 1 is on. */

    uint8 proto_p1;         /* 0x6A: 0 is off. 1 is on. */
    uint8 proto_p2;         /* 0x6B: 0 is off. 1 is on. */

    uint8 menu_side;        /* 0x62: 0 is DNA. 1 is RNA. Not game side! */

    uint8 select_main_menu; /* 0x67: 0x0-0xC selects main menu option. */

    uint8 proto_temjin;     /* 0x86: 0 is off. 1 is on. */
    uint8 proto_raiden;     /* 0x87: 0 is off. 1 is on. */

    uint8 debug_mode;       /* 0x8C: 0 is hidden. 1 is active. */
    uint8 versus_cable_us;  /* 0x8D: 0 is hidden. 1 is active. */
} voot_gamedata_t;

/*

Total Guesstimated System Memory:
    8CCF9C00 - 8CCFAFFF (0x13ff / 5119d)

Initial Focus:
    8CCF9EE0 - 8CCF9EFF

*/

#define VOOT_MEM_START      0x8CCF9ECC
#define VOOT_MEM_END        0x8CCFA2CC

void gamedata_enable_debug(void);

#endif
