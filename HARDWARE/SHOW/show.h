#ifndef __SHOW_H
#define __SHOW_H
#include "sys.h"

void show_top(void);
void show_tip(u16 num,u16 len);
void show_buttom(u16 buttom_x,u16 buttom_y,u16 size_x,u16 size_y);
void key_back(void);
extern u8 flag_page;

#endif
