#ifndef __IMAGE_H
#define __IMAGE_H
#include "sys.h"

#define BACK_COL 0xe73c
#define TOP_0 0x1bba
#define TOP_1 0x24be

extern const u8 gImage_top_down_0[108],gImage_top_down_1[28],gImage_top_down_2[108],gImage_top_down_3[18],
								gImage_top_down_4[208],gImage_top_down_5[208],gImage_top_down_6[18],gImage_top_shadow_0[108],
								gImage_top_shadow_1[28],gImage_top_shadow_2[108],gImage_top_start[7208],gImage_buttom_0[170],
								gImage_buttom_1[170],gImage_buttom_2[18],gImage_buttom_3[242],gImage_buttom_4[28],
								gImage_buttom_5[242],gImage_buttom_6[18];//图片数据(包含信息头)

#endif
