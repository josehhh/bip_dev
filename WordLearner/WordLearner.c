/*
	Application template for Amazfit Bip BipOS
	(C) Maxim Volkov  2019 <Maxim.N.Volkov@ya.ru>
	
	Application template loader for the BipOS
	
*/

#include <libbip.h>
#include "WordLearner.h"
#include "content.words"

#define MODE_GUESS_MEANING 0
#define MODE_GUESS_FROM_MEANING 1
#define N_MODES 2


void reset_status(int * word_index)
{
	(*word_index) = _rand() / N_WORDS;
	(*word_index) = 0;
}


//	screen menu structure - each screen has its own
struct regmenu_ screen_data = {
						55,							//	main screen number, value 0-255, for custom windows it is better to take from 50 and above
						1,							//	auxiliary screen number (usually 1)
						0,							//	0
						dispatch_screen,			//	pointer to the function handling touch, swipe, long press
						key_press_screen, 			//	pointer to the function handling pressing the button
						screen_job,					//	pointer to the callback function of the timer  
						0,							//	0
						show_screen,				//	pointer to the screen display function
						0,							//	
						0							//	long press of the button
					};
int main(int param0, char** argv){	//	here the variable argv is not defined
	show_screen((void*) param0);
}

void show_screen (void *param0){
struct app_data_** 	app_data_p = get_ptr_temp_buf_2(); 	//	pointer to a pointer to screen data
struct app_data_ *	app_data;					//	pointer to screen data

Elf_proc_* proc;

// check the source at the procedure launch
if ( (param0 == *app_data_p) && get_var_menu_overlay()){ // return from the overlay screen (incoming call, notification, alarm, target, etc.)


	app_data = *app_data_p;					//	the data pointer must be saved for the deletion 
											//	release memory function reg_menu
	*app_data_p = NULL;						//	reset the pointer to pass it to the function reg_menu		

	// 	create a new screen when the pointer temp_buf_2 is equal to 0 and the memory is not released		
	reg_menu(&screen_data, 0); 				// 	menu_overlay=0
	
	*app_data_p = app_data;						//	restore the data pointer after the reg_menu function
	
	//   here we perform actions when returning from the overlay screen: restore data, etc.
	reset_status(&app_data->word_index);

	
} else { // if the function is started for the first time i.e. from the menu 


	// create a screen (register in the system) 
	reg_menu(&screen_data, 0);

	// allocate the necessary memory and place the data in it (the memory by the pointer stored at temp_buf_2 is released automatically by the function reg_menu of another screen)
	*app_data_p = (struct app_data_ *)pvPortMalloc(sizeof(struct app_data_));
	app_data = *app_data_p;		//	data pointer
	
	// clear the memory for data
	_memclr(app_data, sizeof(struct app_data_));
	
	//	param0 value contains a pointer to the data of the running process structure Elf_proc_
	proc = param0;
	
	// remember the address of the pointer to the function you need to return to after finishing this screen
	if ( param0 && proc->ret_f ) 			//	if the return pointer is passed, then return to it
		app_data->ret_f = proc->elf_finish;
	else					//	if not, to the watchface
		app_data->ret_f = show_watchface;
	
	// here we perform actions that are necessary if the function is launched for the first time from the menu: filling all data structures, etc.
	
	app_data->col=0;
	app_data->counter=0;
	reset_status(&app_data->word_index);

}

// here we do the interface drawing, there is no need to update (move to video memory) the screen

draw_screen(app_data->word_index);

// if necessary, set the call timer screen_job in ms
set_update_period(1, 5000);
}

void key_press_screen(){
struct app_data_** 	app_data_p = get_ptr_temp_buf_2(); 	//	pointer to a pointer to screen data 
struct app_data_ *	app_data = *app_data_p;				//	pointer to screen data

// call the return function (usually this is the start menu), specify the address of the function of our application as a parameter
show_menu_animate(app_data->ret_f, (unsigned int)show_screen, ANIMATE_RIGHT);	
};

void screen_job(){
// if necessary, you can use the screen data in this function
struct app_data_** 	app_data_p = get_ptr_temp_buf_2(); 	//	pointer to pointer to screen data  
struct app_data_ *	app_data = *app_data_p;				//	pointer to screen data

// do periodic action: animation, counter increase, screen update,
// rendering the interface, update (transfer to video memory) the screen

app_data->col = (app_data->col+1)%COLORS_COUNT;
next_state(&app_data->row_index, &app_data->word_index);

// transfer screen lines that have been redrawn to video memory
repaint_screen_lines(0, 176);

// if necessary, set the screen_job call timer again
set_update_period(1, 5000);
}

void next_state(int *row_index, int *word_index)
{
	int r;
	(*row_index)++;
	if ((*word_index) == 2)
	{
		(*word_index) = 0;
	}

	r = (*word_index);
	while (r == (*word_index))
		{
			r = _rand() % N_WORDS;
		}
		(*word_index) = r;
}



int dispatch_screen (void *param){
struct app_data_** 	app_data_p = get_ptr_temp_buf_2(); 	//	pointer to a pointer to screen data 
struct app_data_ *	app_data = *app_data_p;				//	pointer to screen data

// in case of rendering the interface, update (transfer to video memory) the screen

struct gesture_ *gest = param;
int result = 0;





switch (gest->gesture){
	case GESTURE_CLICK: {			
			next_state(&app_data->row_index, &app_data->word_index);
			break;
		};
		case GESTURE_SWIPE_RIGHT: {	//	swipe to the right

			

			// usually this is the exit from the application
			// show_menu_animate(app_data->ret_f, (unsigned int)show_screen, ANIMATE_RIGHT);	
			break;
		};
		case GESTURE_SWIPE_LEFT: {	// swipe to the left

			
			// actions when swiping left	
			break;
		};
		case GESTURE_SWIPE_UP: {	// swipe up
			next_state(&app_data->row_index, &app_data->word_index);
			// actions when swiping up
			break;
		};
		case GESTURE_SWIPE_DOWN: {	// swipe down
			next_state(&app_data->row_index, &app_data->word_index);
			// actions when swiping down
			break;
		};		
		default:{	// something went wrong ...
			
			break;
		};		
		
	}
	draw_screen(app_data->word_index);

	return result;
};











// custom function
void draw_screen(int word_index){

set_bg_color(COLOR_BLACK);
set_fg_color(COLOR_WHITE);
fill_screen_bg();

text_out_center(words[word_index*2], VIDEO_X / 2, VIDEO_Y / 4);
draw_horizontal_line(VIDEO_Y / 2, 0, VIDEO_X);
text_out_center(words[word_index*2+1], VIDEO_X / 2, VIDEO_Y * 3 / 4);	



repaint_screen_lines(0, 176);


};