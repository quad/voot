/*  controller.h

    $Id: controller.h,v 1.1 2002/06/11 20:40:47 quad Exp $

*/

#ifndef __COMMON_CONTROLLER_H__
#define __COMMON_CONTROLLER_H__

typedef struct
{
    uint32  unk_a;
    uint32  unk_b;

    uint32  button_on;
    uint32  button_off;

    uint32  button_pressed;
    uint32  button_released;

    uint16  trigger_r;
    uint16  trigger_l;    

    int16   axis_x1;
    int16   axis_y1;

    int16   axis_x2;
    int16   axis_y2;

    char   *label;
} controller_status;

typedef enum
{
    CONTROLLER_PORT_A0,
    CONTROLLER_PORT_A1,
    CONTROLLER_PORT_A2,
    CONTROLLER_PORT_A3,
    CONTROLLER_PORT_A4,
    CONTROLLER_PORT_A5,

    CONTROLLER_PORT_B0,
    CONTROLLER_PORT_B1,
    CONTROLLER_PORT_B2,
    CONTROLLER_PORT_B3,
    CONTROLLER_PORT_B4,
    CONTROLLER_PORT_B5,

    CONTROLLER_PORT_C0,
    CONTROLLER_PORT_C1,
    CONTROLLER_PORT_C2,
    CONTROLLER_PORT_C3,
    CONTROLLER_PORT_C4,
    CONTROLLER_PORT_C5,

    CONTROLLER_PORT_D0,
    CONTROLLER_PORT_D1,
    CONTROLLER_PORT_D2,
    CONTROLLER_PORT_D3,
    CONTROLLER_PORT_D4,
    CONTROLLER_PORT_D5
} controller_port;

#define CONTROLLER_MASK_BUTTON_C        (1 << 0)
#define CONTROLLER_MASK_BUTTON_B        (1 << 1)
#define CONTROLLER_MASK_BUTTON_A        (1 << 2)
#define CONTROLLER_MASK_BUTTON_START    (1 << 3)
#define CONTROLLER_MASK_A_UP            (1 << 4)
#define CONTROLLER_MASK_A_DOWN          (1 << 5)
#define CONTROLLER_MASK_A_LEFT          (1 << 6)
#define CONTROLLER_MASK_A_RIGHT         (1 << 7)
#define CONTROLLER_MASK_BUTTON_Z        (1 << 8)
#define CONTROLLER_MASK_BUTTON_Y        (1 << 9)
#define CONTROLLER_MASK_BUTTON_X        (1 << 10)
#define CONTROLLER_MASK_BUTTON_D        (1 << 11)
#define CONTROLLER_MASK_B_UP            (1 << 12)
#define CONTROLLER_MASK_B_DOWN          (1 << 13)
#define CONTROLLER_MASK_B_LEFT          (1 << 14)
#define CONTROLLER_MASK_B_RIGHT         (1 << 15)
#define CONTROLLER_MASK_TRIGGER_L       (1 << 16)
#define CONTROLLER_MASK_TRIGGER_R       (1 << 17)

/* NOTE: Module definitions */

void                controller_init         (void);
controller_status*  check_controller_info   (controller_port port);
uint32              check_controller_press  (controller_port port);

#endif
