#ifndef _HAL_KEY_H
#define _HAL_KEY_H

#include "stdint.h"
  
#define IO_NUMBER  					(8*11) // 11->74HC165D
#define KEY_NUMBER 					(87)
#define SPI_BUFFER_SIZE 		((IO_NUMBER) / 8 + 1)    // 1->defauft state
#define KEY_REPORT_SIZE     (16)
#define HID_REPORT_SIZE     (KEY_REPORT_SIZE)

typedef enum { 
			/*------------------------- HID report data -------------------------*/
			LEFT_CTRL = -8, LEFT_SHIFT = -7, LEFT_ALT = -6, LEFT_GUI = -5,
			RIGHT_CTRL = -4, RIGHT_SHIFT = -3, RIGHT_ALT = -2, RIGHT_GUI = -1,
			RESERVED = 0, ERROR_ROLL_OVER, POST_FAIL, ERROR_UNDEFINED,
			A, B, C, D, E, F, G, H, I, J, K, L, M,
			N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
			NUM_1/*1!*/, NUM_2/*2@*/, NUM_3/*3#*/, NUM_4/*4$*/, NUM_5/*5%*/,
			NUM_6/*6^*/, NUM_7/*7&*/, NUM_8/*8**/, NUM_9/*9(*/, NUM_0/*0)*/,
			ENTER, ESC, BACKSPACE, TAB, SPACE,
			MINUS/*-_*/, EQUAL/*=+*/, LEFT_U_BRACE/*[{*/, RIGHT_U_BRACE/*]}*/,
			BACKSLASH/*\|*/, NONE_US/**/, SEMI_COLON/*;:*/, QUOTE/*'"*/,
			GRAVE_ACCENT/*`~*/, COMMA/*,<*/, PERIOD/*.>*/, SLASH/*/?*/,
			CAP_LOCK, F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,
			PRINT, SCROLL_LOCK, PAUSE, INSERT, HOME, PAGE_UP, DELETE, END, PAGE_DOWN,
			RIGHT_ARROW, LEFT_ARROW, DOWN_ARROW, UP_ARROW, PAD_NUM_LOCK,
			PAD_SLASH, PAD_ASTERISK, PAD_MINUS, PAD_PLUS, PAD_ENTER,
			PAD_NUM_1, PAD_NUM_2, PAD_NUM_3, PAD_NUM_4, PAD_NUM_5,
			PAD_NUM_6, PAD_NUM_7, PAD_NUM_8, PAD_NUM_9, PAD_NUM_0,
			PAD_PERIOD = 99, // followed with 20 bits 0, total 16 Bytes report data
			/*------------------------- HID report data -------------------------*/
			/*--------------------------- Smart keys ----------------------------*/
			FN = 1000,
			/*--------------------------- Smart keys ----------------------------*/
}key_code_t;

uint8_t* hal_key_scan(void);
void hal_key_debounce( uint32_t us );
void hal_key_init(void);
int8_t is_fn_key_press(void);
uint8_t * key_remap( void );

#endif /*_HAL_KEY_H*/
