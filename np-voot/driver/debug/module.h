/*  module.h

    $Id: module.h,v 1.6 2003/03/06 07:37:56 quad Exp $

*/

#ifndef __DRIVER_MODULE_H__
#define __DRIVER_MODULE_H__

typedef enum
{
    VR_DORDRAY,
    VR_BALBADOS,
    VR_CYPHER,
    VR_GRYSVOK,
    VR_APHARMDB,
    VR_BALKEROS_BAD,
    VR_RAIDEN,
    VR_TEMJIN,
    VR_FEIYEN,
    VR_ANGELAN,
    VR_SPECINEFF,
    VR_APHARMDS,
    VR_AJIM,
    VR_BALBAROS_BAD,
    VR_BRADTOS,
    VR_FEIYEN_BAD,
    VR_SENTINEL
} voot_vr_id;

/* NOTE: Module definitions. */

void    module_initialize   (void);
void    module_configure    (void);

#endif
