/*  gamedata.h

    $Id: gamedata.h,v 1.6 2002/10/18 19:52:19 quad Exp $

TODO

    Remove the old (non-working) voot_gamedata_t structure and replace it
    with a series of definitions which allow proper access to the various
    gamedata variables.

*/

#ifndef __COMMON_GAMEDATA_H__
#define __COMMON_GAMEDATA_H__

/*

    voot_gamedata_t structure starts at: 0x8CCF9ECC (8ccf0000 is the index in the VOOT code)

    Gameshark code format:

    01-XXXXXX   8-bit constant write
    000000YY

    01-CF9F58   Debug Mode
    00000001

    01-CF9EFD
    00000001    Dumb AI Mode (only in single player)

    uint16 *arcade = (uint16 *) (0x8CCF9ECC + 0x1A);
    uint8 *enemy_shoot = (uint8 *) (0x8CCF9ECC + 0x31);
    uint16 *proto_on = (uint16 *) (0x8CCF9ECC + 0x6A);

*/

#define _GAMEDATA_BASE          (0x8ccf0000)

#define GAMEDATA_ANIM_MODE_A    ((uint16 *) (_GAMEDATA_BASE + 0x0228))
#define GAMEDATA_ANIM_MODE_B    ((uint16 *) (_GAMEDATA_BASE + 0x022a))

#define GAMEDATA_P1_HANDICAP    ((uint8 *)  (_GAMEDATA_BASE + 0x6220))
#define GAMEDATA_P1_VR_SELECT   ((uint8 *)  (_GAMEDATA_BASE + 0x6236))
#define GAMEDATA_P1_HEALTH_REAL ((uint16 *) (_GAMEDATA_BASE + 0x6284))
#define GAMEDATA_P1_HEALTH_CALC ((uint16 *) (_GAMEDATA_BASE + 0x6286))
#define GAMEDATA_P1_RW_AMMO     ((uint16 *) (_GAMEDATA_BASE + 0x63b4))
#define GAMEDATA_P1_VA_CALC     ((uint16 *) (_GAMEDATA_BASE + 0x63ec))
#define GAMEDATA_P1_VA_REAL     ((uint16 *) (_GAMEDATA_BASE + 0x63ee))
#define GAMEDATA_P1_CC_TIME     ((uint16 *) (_GAMEDATA_BASE + 0x96b0))

#define GAMEDATA_GAME_SIDE      ((uint8 *)  (_GAMEDATA_BASE + 0x96f8))

typedef struct
{
    char module_name[16]    __attribute__ ((packed));   /* 0x00 - 0x0a: Module Name (maybe not all!) */

    uint32 adjust_x         __attribute__ ((packed));   /* 0x10 - 0x13: +0 == 0x0 */
    uint32 adjust_y         __attribute__ ((packed));   /* 0x14 - 0x17: +0 == 0x8 */
    uint16 defeat_perfect   __attribute__ ((packed));   /* 0x18 - 0x19: FD 0F is movie unlock. Bitfield of defeat against 5.45 wo/ continue. */
    uint16 defeat           __attribute__ ((packed));   /* 0x1A - 0x1B: FD 0F is 5.2 enabled. Bitfield of defeat against 5.45. */

    uint8 unknown_a         __attribute__ ((packed));   /* 0x1C: UNKNOWN (searched) */

    uint8 difficulty        __attribute__ ((packed));   /* 0x1D: 1-8 legal values. */
    uint8 match_time_1p     __attribute__ ((packed));   /* 0x1E: 0-4 legal values. 0 is 60 seconds. 4 is Deathmatch. */
    uint8 match_time_vs     __attribute__ ((packed));   /* 0x1F: 0-4 legal values. 0 is 60 seconds. 4 is Deathmatch. */
    uint8 match_count_1p    __attribute__ ((packed));   /* 0x20: 0-3 legal values. (1/2/3/5) */
    uint8 match_count_vs    __attribute__ ((packed));   /* 0x21: 0-3 legal values. (1/2/3/5) */

    uint8 unknown_ba[3]     __attribute__ ((packed));   /* 0x22 - 0x24: UNKNOWN */

    uint8 unknown_b         __attribute__ ((packed));   /* 0x25: UNKNOWN */

    uint8 unknown_bb[8]     __attribute__ ((packed));   /* 0x26 - 0x2D: UNKNOWN */

    uint8 muteki_p1         __attribute__ ((packed));   /* 0x2E: 0 is off. 1 is on - non single player */
    uint8 muteki_p2         __attribute__ ((packed));   /* 0x2F: 0 is off. 1 is on - non single player */
    uint8 enemy_move        __attribute__ ((packed));   /* 0x30: 0 is moving. 1 is no moving. */
    uint8 enemy_shoot       __attribute__ ((packed));   /* 0x31: 0 is shooting. 1 is no shooting. */
    uint8 slow              __attribute__ ((packed));   /* 0x32: 0 is off. 1 is on. */
    uint8 move_vr           __attribute__ ((packed));   /* 0x33: 0 is 1P. 1 is 2P. */
    uint8 engine_run        __attribute__ ((packed));   /* 0x34: 0 if engine stopped. 1 if engine running. */
    uint8 frame_delay       __attribute__ ((packed));   /* 0x35: Number of frames delay between each render. */
    uint8 perform_load      __attribute__ ((packed));   /* 0x36: 0 if running. 1 is loading. */
    uint8 controller_a      __attribute__ ((packed));   /* 0x37: 0x0 - 0x5 (A/B/C/D/E/F) */
    uint8 controller_b      __attribute__ ((packed));   /* 0x38: 0x0 - 0x5 (A/B/C/D/E/F) */
    uint8 ajim              __attribute__ ((packed));   /* 0x39: 0 is no Ajim. 1 is Ajim accessable. */
    uint8 data_port         __attribute__ ((packed));   /* 0x3A: (vmu_port) which port the active VMU is on. */
    uint8 funky_control     __attribute__ ((packed));   /* 0x3B: In game mode, 0 in causes funky control. 1 is normal. */
    uint8 paused            __attribute__ ((packed));   /* 0x3C: 0 is unpaused. 1 is paused. */
    uint8 force_unpause     __attribute__ ((packed));   /* 0x3D: 0 is normal. 1 forces an unpause. */
    uint8 watch_mode        __attribute__ ((packed));   /* 0x3E: 0 is off. 1 is on. (able to change in all modes) */
    uint8 quick_select      __attribute__ ((packed));   /* 0x3F: 0 is off. 1 is on. */
    uint8 quick_continue    __attribute__ ((packed));   /* 0x40: 0 is off. 1 is on. */
    uint8 infight_mode      __attribute__ ((packed));   /* 0x41: 0 is off. 1 is on. */

    uint8 unknown_ca[2]     __attribute__ ((packed));   /* 0x42 - 0x43: UNKNOWN (searched) */

    uint8 watch_view        __attribute__ ((packed));   /* 0x44: 0x0 - 0xe */

    uint8 unknown_cb        __attribute__ ((packed));   /* 0x45: UNKNOWN (searched) */

    uint8 controller_req    __attribute__ ((packed));   /* 0x46: Determines number of controllers required? (maybe? 0x0-0x2 is normal. 0x3 is required two.) */
    uint8 mode_52           __attribute__ ((packed));   /* 0x47: 0 is 5.2. 1 is 5.45. */
    uint8 survival          __attribute__ ((packed));   /* 0x48: 0 is off. 1 is on. */

    union
    {
        uint8   index[2]    __attribute__ ((packed));

        union
        {
            uint8   p1      __attribute__ ((packed));   /* 0x49: 0 is off. 1 is on. */
            uint8   p2      __attribute__ ((packed));   /* 0x4A: 0 is off. 1 is on. */
        } player;
    } cust_emb              __attribute__ ((packed));

    union
    {
        uint8   index[2]    __attribute__ ((packed));

        union
        {
            uint8   p1      __attribute__ ((packed));   /* 0x4B: 0 is off. 1 is Gold. 2 is Silver. */
            uint8   p2      __attribute__ ((packed));   /* 0x4C: 0 is off. 1 is Gold. 2 is Silver. */
        } player;
    } cust_head             __attribute__ ((packed));

    uint8 unknown_d         __attribute__ ((packed));   /* 0x4D: UNKNOWN */

    uint8 control_port      __attribute__ ((packed));   /* 0x4E: 0 is port A. 1 is port B. */
    uint8 attract_cycle     __attribute__ ((packed));   /* 0x4F: 0x0 - 0x4 which attract cycle. */

    uint8 unknown_e[13]     __attribute__ ((packed));   /* 0x50 - 0x5C: UNKNOWN */

    uint8 fog_mode          __attribute__ ((packed));   /* 0x5D: 0 is off. 1 is on. */
    uint8 rumble_a          __attribute__ ((packed));   /* 0x5E: 0 is off. 1 is on. */
    uint8 rumble_b          __attribute__ ((packed));   /* 0x5F: 0 is off. 1 is on. */
    uint8 audio_mono        __attribute__ ((packed));   /* 0x60: 0 is stereo. 1 is mono. */

    uint8 unknown_f         __attribute__ ((packed));   /* 0x61: UNKNOWN */

    uint8 menu_side         __attribute__ ((packed));   /* 0x62: 0 is DNA. 1 is RNA. Not game side! */
    uint8 bgm_volume        __attribute__ ((packed));   /* 0x63: 0x0 - 0x9 */
    uint8 sfx_volume        __attribute__ ((packed));   /* 0x64: 0x0 - 0x9 */
    
    uint8 unknown_g[2]      __attribute__ ((packed));   /* 0x65 - 0x66: UNKNOWN */

    uint8 select_main_menu  __attribute__ ((packed));   /* 0x67: 0x0-0xC selects main menu option. */

    uint16 unknown_h        __attribute__ ((packed));   /* 0x68 - 0x69: UNKNOWN */

    uint8 proto_p1          __attribute__ ((packed));   /* 0x6A: 0 is off. 1 is on. */
    uint8 proto_p2          __attribute__ ((packed));   /* 0x6B: 0 is off. 1 is on. */

    uint8 unknown_i[16]     __attribute__ ((packed));   /* 0x6c - 0x7b: UNKNOWN */

    uint8 quick_select_1p   __attribute__ ((packed));   /* 0x7c: Quick-selected 1P VR */
    uint8 quick_select_2p   __attribute__ ((packed));   /* 0x7d: Quick-selected 2P VR */
    uint8 level_select      __attribute__ ((packed));   /* 0x7e: Level selected (DNA/1P) */

    uint8 unknown_j[7]      __attribute__ ((packed));   /* 0x7f - 0x85: UNKNOWN */

    uint8 proto_temjin      __attribute__ ((packed));   /* 0x86: 0 is off. 1 is on. */
    uint8 proto_raiden      __attribute__ ((packed));   /* 0x87: 0 is off. 1 is on. */

    uint8 unknown_k[4]      __attribute__ ((packed));   /* 0x88 - 0x8B: UNKNOWN. */

    uint8 debug_mode        __attribute__ ((packed));   /* 0x8C: 0 is hidden. 1 is active. */
    uint8 versus_cable_us   __attribute__ ((packed));   /* 0x8D: 0 is hidden. 1 is active. */
} gamedata_opt_t;

#define GAMEDATA_OPT        ((gamedata_opt_t *) (_GAMEDATA_BASE + 0x9ecc))

#define GAME_MEM_START      0x8c010000
#define GAME_MEM_END        0x8c24ffff

#define OVERLAY_MEM_START   0x8c270000

#define VOOT_MEM_START      0x8ccf0000
#define VOOT_MEM_END        0x8ccfffff

/* NOTE: Module definitions. */

void    gamedata_enable_debug   (void);

#endif
