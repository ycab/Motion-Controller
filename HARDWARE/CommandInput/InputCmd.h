#ifndef __InputCmd_H
#define __InputCmd_H 
#include "sys.h"
#define X1 PCin(6)      
#define X2 PCin(7) 
#define X3 PCin(8) 
#define X4 PCin(9) 
#define X5 PAin(11) 
#define X6 PAin(12) 
#define RunDirection PBout(10)
void GPIO_Input_Init(void);

		 				    
#endif
