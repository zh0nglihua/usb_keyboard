#include "hal_key.h"
#include "gpio.h"
#include "string.h"
#include "spi.h"
#include "stdio.h"
#include "main.h"

#define  KEY_DEBUGx

#define  FN_KEY_COM					(3)			
#define  FN_KEY_SEG					(0)

SPI_HandleTypeDef *hspi_hanlde;

static uint8_t spi_buffer[SPI_BUFFER_SIZE];
static uint8_t remap_buffer[IO_NUMBER/8];
static uint8_t hid_buffer[HID_REPORT_SIZE];

const int16_t key_map[3][KEY_NUMBER] = 
{
        // The first layer, used for aligning 74HC165 IO pins to PCB key layout
        {		8,   9, 10, 11, 15, 14, 13, 12,  0, 1,   2,  3,  4,  5,  6,  7,
            83, 82, 75, 65, 64, 59, 63, 48, 52, 32, 36, 37, 38, 20, 21, 22, 23,
            81, 74, 77, 66, 71, 58, 62, 49, 40, 33, 34, 35, 39, 16, 17, 18, 19,
            80, 73, 78, 67, 70, 57, 61, 50, 53, 41, 44, 45, 28,
            84, 72, 79, 69, 68, 56, 51, 55, 54, 42, 46, 25, 29,
            85, 86, 76, 60, 43, 47, 24, 26, 27, 31, 30,
				},

        // Other layer, used for mapping default key layout to custom layout
        {ESC, F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12, PRINT, SCROLL_LOCK,PAUSE,
            GRAVE_ACCENT, NUM_1, NUM_2, NUM_3, NUM_4, NUM_5, NUM_6, NUM_7, NUM_8, NUM_9, NUM_0, MINUS, EQUAL, BACKSPACE, INSERT, HOME, PAGE_UP,
            TAB, Q,W,E,R,T,Y,U,I,O,P,LEFT_U_BRACE, RIGHT_U_BRACE, BACKSLASH, DELETE, END, PAGE_DOWN,
            CAP_LOCK, A,S,D,F,G,H,J,K,L,SEMI_COLON, QUOTE, ENTER,
            LEFT_SHIFT, Z,X,C,V,B,N,M,COMMA, PERIOD, SLASH, RIGHT_SHIFT, UP_ARROW,
            LEFT_CTRL, LEFT_GUI, LEFT_ALT, SPACE, RIGHT_ALT, RIGHT_GUI, FN, RIGHT_CTRL, LEFT_ARROW, DOWN_ARROW, RIGHT_ARROW
				},

        {ESC, F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12, PAUSE, SCROLL_LOCK,PAUSE,
            GRAVE_ACCENT, NUM_1, NUM_2, NUM_3, NUM_4, NUM_5, NUM_6, NUM_7, NUM_8, NUM_9, NUM_0, MINUS, EQUAL, BACKSPACE, INSERT,HOME, PAGE_UP,
            TAB, Q,W,E,R,T,Y,U,I,O,P,LEFT_U_BRACE, RIGHT_U_BRACE, BACKSLASH, DELETE,END, PAGE_DOWN,
            CAP_LOCK, A,S,D,F,G,H,J,K,L,SEMI_COLON, QUOTE, ENTER,
            LEFT_SHIFT, Z,X,C,V,B,N,M,COMMA, PERIOD, SLASH, RIGHT_SHIFT, UP_ARROW,
            LEFT_CTRL, LEFT_GUI, LEFT_ALT, SPACE, RIGHT_ALT, RIGHT_GUI, FN, RIGHT_CTRL, LEFT_ARROW, DOWN_ARROW, RIGHT_ARROW,
				}
				
};


inline void hal_delay_us(uint32_t _us)
{
    for (int i = 0; i < _us; i++)
        for (int j = 0; j < 8; j++)  // ToDo: tune this for different chips
            __NOP();
}

/*
 *   key scan
 */
uint8_t* hal_key_scan(void)
{
		if(hspi_hanlde == NULL)
			return NULL;
		
	  memset(spi_buffer, 0xFF, SPI_BUFFER_SIZE);
    //SCAN_PL_GPIO_Port->BSRR = SCAN_PL_Pin; // Latch
		HAL_GPIO_WritePin(SCAN_PL_GPIO_Port, SCAN_PL_Pin, GPIO_PIN_SET); // Latch
    hspi_hanlde->pRxBuffPtr = (uint8_t*) spi_buffer;
    hspi_hanlde->RxXferCount = SPI_BUFFER_SIZE;
    __HAL_SPI_ENABLE(hspi_hanlde);
    while (hspi_hanlde->RxXferCount > 0U)
    {
        if (__HAL_SPI_GET_FLAG(hspi_hanlde, SPI_FLAG_RXNE))
        {
            /* read the received data */
            (*(uint8_t*) hspi_hanlde->pRxBuffPtr) = *(__IO uint8_t*) &hspi_hanlde->Instance->DR;
            hspi_hanlde->pRxBuffPtr += sizeof(uint8_t);
            hspi_hanlde->RxXferCount--;
        }
    }
    __HAL_SPI_DISABLE(hspi_hanlde);
    //SCAN_PL_GPIO_Port->BRR = SCAN_PL_Pin; // Sample
    HAL_GPIO_WritePin(SCAN_PL_GPIO_Port, SCAN_PL_Pin, GPIO_PIN_RESET); 
		
#ifdef KEY_DEBUG
		//printf("key sacn buffer \r\n");
		for( uint8_t  i = 0; i < SPI_BUFFER_SIZE; i++  )
		{
				printf("%02x,",spi_buffer[i]);
		}
		printf("\r\n");
#endif		
		
    return (uint8_t*)(spi_buffer + 1);
}

/*key debounce*/ 
void hal_key_debounce( uint32_t us )
{
		uint8_t debounce_buffer[SPI_BUFFER_SIZE] = {0};
		uint8_t *scan_buffer = NULL;
		hal_delay_us(us);
		scan_buffer =	hal_key_scan();
		if(scan_buffer == NULL)
			return ;
		uint8_t mask;

    for (int i = 0; i < SPI_BUFFER_SIZE; i++)
    {
        mask = scan_buffer[i] ^ debounce_buffer[i];
        scan_buffer[i] |= mask;
    }
		
#ifdef KEY_DEBUG
//		for( uint8_t  i = 0; i < SPI_BUFFER_SIZE -1; i++  )
//		{
//				printf("%02x,",scan_buffer[i]);
//		}
//		printf("\r\n");
#endif		
}

uint8_t * key_remap( void )
{
	  int16_t index, bit_index;
		uint8_t *scan_buffer = spi_buffer + 1;
    memset(remap_buffer, 0, IO_NUMBER / 8);
    for (int16_t i = 0; i < IO_NUMBER / 8; i++)
    {
        for (int16_t j = 0; j < 8; j++)
        {
            index = (int16_t) (key_map[0][i * 8 + j] / 8); 
            bit_index = (int16_t) (key_map[0][i * 8 + j] % 8);
						if( i*8 + j >= KEY_NUMBER || index*8 + bit_index >= KEY_NUMBER )
						{
							break;
						}							
							 
            if ( ~scan_buffer[index] & (1 << bit_index))
						{
								remap_buffer[i] |= 1 << j;
								//printf("i :%d, j :%d, index :%d bit_idx :%d, val :%02x\r\n", i, j , index, bit_index, scan_buffer[index] );
						}  
						
        }
			
        //remap_buffer[i] = ~remap_buffer[i];
    }
#ifdef KEY_DEBUG
//		printf("Remap buffer \r\n");
//		for( uint8_t  i = 0; i < IO_NUMBER / 8; i++  )
//		{
//				printf("%02x,",remap_buffer[i]);
//		}
//		printf("\r\n");
#endif				
		uint8_t layer = is_fn_key_press() ? 2: 1;
		//printf("layer :%d\r\n", layer );
    memset(hid_buffer, 0, KEY_REPORT_SIZE);
    for (int i = 0; i < IO_NUMBER / 8; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            index = (int16_t) (key_map[layer][i * 8 + j] / 8 + 1); // +1 for modifier
            bit_index = (int16_t) (key_map[layer][i * 8 + j] % 8);
						//printf("index :%d, bit idex %d\r\n", index, bit_index);
            if (bit_index < 0)
            {
                index -= 1;
                bit_index += 8;
            } else if (index > 100)
                continue;

            if (remap_buffer[i] & (1 << j))
						{
							 //printf("i :%d, j :%d, idx :%d, bit idx  :%d, val :%d\r\n", i, j , index, bit_index, key_map[layer][i * 8 + j] );
							 hid_buffer[index] |= 1 << (bit_index); // +1 for Report-ID
						}
               
        }
    }
#ifdef KEY_DEBUG
//			printf("HID buffer \r\n");
//		for( uint8_t  i = 0; i < HID_REPORT_SIZE; i++  )
//		{
//				printf("%02x,",hid_buffer[i]);
//		}
//		printf("\r\n");
#endif		
    return hid_buffer;
}

int8_t is_fn_key_press(void)
{
		return (remap_buffer[FN_KEY_COM]&(1<<FN_KEY_SEG));
}

void hal_key_init(void)
{
	hspi_hanlde = &hspi1;
	HAL_GPIO_WritePin(SCAN_CE_GPIO_Port, SCAN_CE_Pin, GPIO_PIN_RESET);
}
