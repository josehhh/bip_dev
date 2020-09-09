/*
	Application template for Amazfit Bip BipOS
	(C) Maxim Volkov  2019 <Maxim.N.Volkov@ya.ru>
	
	Application template, header file

*/

#ifndef __APP_TEMPLATE_H__
#define __APP_TEMPLATE_H__

#define COLORS_COUNT	4

// the data structure for our screen
struct app_data_ {
			void* 	ret_f;					//	the address of the return function
			int		col;					//	the current color of the font
			int     counter;               // counter of the square position.
			int word_index;
			int row_index;
};



// template.c
void 	show_screen (void *return_screen);
void 	key_press_screen();
int 	dispatch_screen (void *param);
void 	screen_job();
void	draw_screen(int word_index);
void 	reset_status(int* word_index);
void	next_state(int *row_index, int *word_index);
#endif