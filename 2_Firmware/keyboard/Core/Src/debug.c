#include "stdio.h"
#include "stm32f1xx_hal.h"

extern UART_HandleTypeDef huart1;

#ifdef __cplusplus  
extern "C" {  
#endif //__cplusplus  
   
int fputc(int ch, FILE* f)  
{  
	HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, 0xFFFF );
	return ch;
}  
   
#ifdef __cplusplus  
}  
#endif //__cplusplus  
