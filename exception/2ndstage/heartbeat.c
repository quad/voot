/*  heartbeat.c

    Essentially the core of any code I might write from here on. Every UBC
    exception (thus around 60 times a second - we're breaking on pageflip)
    we receive a call here.
*/

#include "vars.h"
#include "exception.h"
#include "exception-lowlevel.h"
#include "system.h"
#include "biudp.h"
#include "heartbeat.h"

void init_heartbeat(void)
{
    exception_table_entry new;

    /* STAGE: Catch the pageflip exceptions. */
    new.type = EXP_TYPE_GEN;
    new.code = EXP_CODE_UBC;
    new.handler = heartbeat;

    add_exception_handler(&new);

    /* STAGE: !!! There needs to be a better method of notification. */
}

/*
    SCIF Write:
        WRITE on 0xFFE8000C in R2   (PC: 8c0397f4)

    SCIF Read:
        READ  on 0xFFE80014 in R3   (PC: 8c039b58)
*/

void init_ubc_b_serial(void)
{
    exception_table_entry new;

    /* STAGE: Configure UBC Channel B for breakpoint on serial port access */
    *UBC_R_BARB = 0xFFE80014;
    *UBC_R_BAMRB = UBC_BAMR_NOASID;
    *UBC_R_BBRB = UBC_BBR_READ | UBC_BBR_OPERAND;

    ubc_wait();

#if NO_HANDLE_RXI
    /* STAGE: Add exception handler for RXI interrupts. */
    new.type = EXP_TYPE_INT;
    new.code = EXP_CODE_RXI;
    new.handler = rxi_handler;

    add_exception_handler(&new);
#endif NO_HANDLE_RXI
} 

void* rxi_handler(register_stack *stack, void *current_vector)
{
    biudp_write_str("#Received RXI interrupt.\r\n");

    return current_vector;
}

void* heartbeat(register_stack *stack, void *current_vector)
{
    static bool done_once = FALSE;

    /* STAGE: Run this section of code only once. */
    if (!done_once)
    {
        /* STAGE: !!! Check the timer chip. See if VOOT is using it. */

        done_once = TRUE;
    }

    /* STAGE: Enable features in gamedata structure. */
    {
        /*
        volatile uint16 *arcade = (volatile uint16 *) (0x8CCF9ECC + 0x1A);
        volatile uint8 *enemy_shoot = (volatile uint8 *) (0x8CCF9ECC + 0x31);
        volatile uint16 *proto_on = (volatile uint16 *) (0x8CCF9ECC + 0x6A);
        */
        volatile uint16 *proto_ok = (volatile uint16 *) (0x8CCF9ECC + 0x86);
        volatile uint16 *menus = (volatile uint16 *) (0x8CCF9ECC + 0x8C);

        *proto_ok = 0x0101;
        *menus = 0x0101;
    }

    /* STAGE: Display serial information. */
    if (*UBC_R_BRCR & UBC_BRCR_CMFB)
    {
        biudp_write('#');
        biudp_write(stack->r3);
        biudp_write('#');
        biudp_write_hex(spc());

        /* STAGE: Be sure to clear the proper bit. */
        *UBC_R_BRCR &= ~(UBC_BRCR_CMFB);
    }
    
    /* STAGE: Track at what address the pageflip is occuring. */
    if (*UBC_R_BRCR & UBC_BRCR_CMFA)
    {
        static uint32 saved_spc;
        static uint32 save_count = 0;

        if (saved_spc != spc())
        {
            biudp_write_str("[UBC] Pageflip 0x");
            biudp_write_hex(saved_spc);
            biudp_write_str(" lasted 0x");
            biudp_write_hex(save_count);
            biudp_write_str("\r\n");

            saved_spc = spc();
            save_count = 0;

            biudp_write_str("[UBC] Pageflip @ 0x");
            biudp_write_hex(saved_spc);
            biudp_write_str("\r\n");
        }

        save_count++;

        /* STAGE: Be sure to clear the proper bit. */
        *UBC_R_BRCR &= ~(UBC_BRCR_CMFA);
    }

    return current_vector;
}
