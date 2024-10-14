#ifndef __BOOTLOADER_H
#define __BOOTLOADER_H

#define ENABLE_INT()    __set_PRIMASK(0)        /*使能全局中断*/
#define DISABLE_INT()   __set_PRIMASK(1)        /*禁止全局中断*/

void JumpToBootLoader(void);

#endif
