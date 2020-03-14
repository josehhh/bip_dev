/*
(C) Volkov Maxim 2019 (Maxim.N.Volkov@ya.ru)

Calendar v1.0
Simple calendar app.
The day of the week calculation algorithm works for any date of the Gregorian calendar later than 1583.
The Gregorian calendar began to operate in 1582 - after October 4, it immediately arrived on October 15.

Calendar 1600 to 3000
Functions of turning the calendar up and down - month, year arrows
By clicking on the name of the month, the current date is set.


v.1.1
- fixed transitions at startup from the quick menu
	
*/

#include <libbip.h>
#include "calend.h"
#define DEBUG_LOG

//	calendar screen menu structure
struct regmenu_ menu_calend_screen = {
						55,
						1,
						0,
						dispatch_calend_screen,
						key_press_calend_screen, 
						calend_screen_job,
						0,
						show_calend_screen,
						0,
						0
					};

int main(int param0, char** argv){	//argv variable not defined
	show_calend_screen((void*) param0);
}

void show_calend_screen (void *param0){
struct calend_** 	calend_p = get_ptr_temp_buf_2(); 	//	pointer to screen data pointer 
struct calend_ *	calend;								//	pointer to screen data
struct calend_opt_ 	calend_opt;							//	calendar options


#ifdef DEBUG_LOG
log_printf(5, "[show_calend_screen] param0=%X; *temp_buf_2=%X; menu_overlay=%d", (int)param0, (int*)get_ptr_temp_buf_2(), get_var_menu_overlay());
log_printf(5, " #calend_p=%X; *calend_p=%X", (int)calend_p, (int)*calend_p);
#endif

if ( (param0 == *calend_p) && get_var_menu_overlay()){ // return from the overlay screen (incoming call, notification, alarm, target, etc.)

#ifdef DEBUG_LOG
	log_printf(5, "  #from overlay");
	log_printf(5, "\r\n");
#endif	

	calend = *calend_p;						//	 pointer to data must be saved for exception
											//	memory release function reg_menu
	*calend_p = NULL;						//	nullify the pointer to pass to the reg_menu function	

	// 	create a new screen, while the pointers temp_buf_1 and temp_buf_2 were 0 and the memory was not freed
	reg_menu(&menu_calend_screen, 0); 		// 	menu_overlay=0
	
	*calend_p = calend;						//	we restore the pointer to the data after the reg_menu function
	
	draw_month(0, calend->month, calend->year);
	
} else { 			// if the function started from the menu,

#ifdef DEBUG_LOG
	log_printf(5, "  #from menu");
	log_printf(5, "\r\n");
#endif
	// create a screen
	reg_menu(&menu_calend_screen, 0);

	// allocate the necessary memory and place data in it
	*calend_p = (struct calend_ *)pvPortMalloc(sizeof(struct calend_));
	calend = *calend_p;		//	data pointer
	
	// clear memory for data
	_memclr(calend, sizeof(struct calend_));
	
	calend->proc = param0;
	
	// remember the address of the pointer to the function you need to return to after finishing this screen
	if ( param0 && calend->proc->elf_finish ) 			//	if the return pointer is passed, then return to it
		calend->ret_f = calend->proc->elf_finish;
	else					//	if not, then on the dial
		calend->ret_f = show_watchface;
	
	struct datetime_ datetime;
	_memclr(&datetime, sizeof(struct datetime_));
	
	// get the current date
	get_current_date_time(&datetime);
	
	calend->day 	= datetime.day;
	calend->month 	= datetime.month;
	calend->year 	= datetime.year;

// we consider options from flash memory, if the value in flash memory is incorrect then we take the first scheme
// current color scheme is stored about offset 0
	ElfReadSettings(calend->proc->index_listed, &calend_opt, OPT_OFFSET_CALEND_OPT, sizeof(struct calend_opt_));
	
	if (calend_opt.color_scheme < COLOR_SCHEME_COUNT) 
			calend->color_scheme = calend_opt.color_scheme;
	else 
			calend->color_scheme = 0;

	draw_month(calend->day, calend->month, calend->year);
}

// when idle, turn off the backlight and do not exit
set_display_state_value(8, 1);
set_display_state_value(2, 1);

// timer on job for 20s where is the output.
set_update_period(1, INACTIVITY_PERIOD);

}

void draw_month(unsigned int day, unsigned int month, unsigned int year){
struct calend_** 	calend_p = get_ptr_temp_buf_2(); 		//	указатель на указатель на данные экрана 
struct calend_ *	calend = *calend_p;						//	указатель на данные экрана


/*
 0: CALEND_COLOR_BG calendar background
 1: CALEND_COLOR_MONTH current month name color
 2: CALEND_COLOR_YEAR current year color
 3: CALEND_COLOR_WORK_NAME weekday color names
 4: CALEND_COLOR_HOLY_NAME_BG weekend day names background
 5: CALEND_COLOR_HOLY_NAME_FG weekend name color
 6: CALEND_COLOR_SEPAR calendar separator color
 7: CALEND_COLOR_NOT_CUR_WORK color of numbers NOT the current weekday
 8: CALEND_COLOR_NOT_CUR_HOLY_BG the background of numbers NOT the current month weekend
 9: CALEND_COLOR_NOT_CUR_HOLY_FG the color of the numbers NOT the current month weekend
10: CALEND_COLOR_CUR_WORK the color of the numbers of the current month of the week
11: CALEND_COLOR_CUR_HOLY_BG the background of the current month
12: CALEND_COLOR_CUR_HOLY_FG the color of the numbers of the current month weekend
13: CALEND_COLOR_TODAY_BG the background of the numbers of the current day; bit 31 - fill: = 0 fill with the background color, = 1 only the frame, the background is like the date of a non-current month
14: CALEND_COLOR_TODAY_FG the color of the numbers of the current day
*/


// black theme without highlighting the weekend with today's highlight frame*/	
static unsigned char short_color_scheme[15] = 
	{COLOR_SH_BLACK, COLOR_SH_YELLOW, COLOR_SH_AQUA, COLOR_SH_WHITE, COLOR_SH_RED, COLOR_SH_WHITE, COLOR_SH_WHITE, 
	COLOR_SH_GREEN, COLOR_SH_BLACK, COLOR_SH_AQUA, COLOR_SH_YELLOW, COLOR_SH_BLACK, COLOR_SH_WHITE, COLOR_SH_AQUA|(1<<7), COLOR_SH_BLACK};
											 
int color_scheme[15];


for (unsigned char j=0;j<15;j++){
color_scheme[j]  = (((unsigned int)short_color_scheme[j]&(unsigned char)COLOR_SH_MASK)&COLOR_SH_RED)  ?COLOR_RED   :0;	//  red component
color_scheme[j] |= (((unsigned int)short_color_scheme[j]&(unsigned char)COLOR_SH_MASK)&COLOR_SH_GREEN)?COLOR_GREEN :0;	//	green component
color_scheme[j] |= (((unsigned int)short_color_scheme[j]&(unsigned char)COLOR_SH_MASK)&COLOR_SH_BLUE) ?COLOR_BLUE  :0;	//	blue component
color_scheme[j] |= (((unsigned int)short_color_scheme[j]&(unsigned char)(1<<7))) ?(1<<31) :0;				//	for the frame
}

char text_buffer[24];
char *weekday_string_ru[] = {"??", "Пн", "Вт", "Ср", "Чт", "Пт", "Сб", "Вс"};
char *weekday_string_en[] = {"??", "Mo", "Tu", "We", "Th", "Fr", "Sa", "Su"};
char *weekday_string_it[] = {"??", "Lu", "Ma", "Me", "Gi", "Ve", "Sa", "Do"};
char *weekday_string_fr[] = {"??", "Lu", "Ma", "Me", "Je", "Ve", "Sa", "Di"};
char *weekday_string_es[] = {"??", "Lu", "Ma", "Mi", "Ju", "Vi", "Sá", "Do"};

char *weekday_string_short_ru[] = {"?", "П", "В", "С", "Ч", "П", "С", "В"};
char *weekday_string_short_en[] = {"?", "M", "T", "W", "T", "F", "S", "S"};
char *weekday_string_short_it[] = {"?", "L", "M", "M", "G", "V", "S", "D"};
char *weekday_string_short_fr[] = {"?", "M", "T", "W", "T", "F", "S", "S"};
char *weekday_string_short_es[] = {"?", "L", "M", "X", "J", "V", "S", "D"};

char *monthname_ru[] = {
	     "???",		
	     "Январь", 		"Февраль", 	"Март", 	"Апрель",
		 "Май", 		"Июнь",		"Июль", 	"Август", 
		 "Сентябрь", 	"Октябрь", 	"Ноябрь",	"Декабрь"};

char *monthname_en[] = {
	     "???",		
	     "January", 	"February", 	"March", 		"April", 	
		 "May", 		"June",			"July", 		"August",
		 "September", 	"October", 		"November",		"December"};

char *monthname_it[] = {
	     "???",		
	     "Gennaio", 	"Febbraio", "Marzo", 	"Aprile", 
		 "Maggio", 		"Giugno",   "Luglio", 	"Agosto", 
		 "Settembre", 	"Ottobre", 	"Novembre", "Dicembre"};
		 
char *monthname_fr[] = {
	     "???",		
	     "Janvier",		"Février",	"Mars",		"Avril", 
		 "Mai", 		"Juin", 	"Juillet",	"Août", 
		 "Septembre", 	"Octobre", 	"Novembre", "Décembre"};
char *monthname_es[] = {
	     "???",		
	     "Enero", 		"Febrero", 	"Marzo", 		"Abril", 	
		 "Mayo", 		"Junio", 	"Julio",		"Agosto", 	
		 "Septiembre", 	"Octubre", 	"Noviembre", 	"Diciembre"};


unsigned char day_month[] = { 0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

char**	weekday_string;
char**	weekday_string_short;
char**	monthname;

switch (get_selected_locale()){
		case locale_ru_RU:{
			weekday_string = weekday_string_ru;
			weekday_string_short = weekday_string_short_ru;
			monthname = monthname_ru;
			break;
		}
		case locale_it_IT:{
			weekday_string = weekday_string_it;
			weekday_string_short = weekday_string_short_it;
			monthname = monthname_it;
			break;
		}
		case locale_fr_FR:{
			weekday_string = weekday_string_fr;
			weekday_string_short = weekday_string_short_fr;
			monthname = monthname_fr;
			break;
		}
		case locale_es_ES:{
			weekday_string = weekday_string_es;
			weekday_string_short = weekday_string_short_es;
			monthname = monthname_es;
			break;
		}
		default:{
			weekday_string = weekday_string_en;
			weekday_string_short = weekday_string_short_en;
			monthname = monthname_en;
			break;
		}
	}

_memclr(&text_buffer, 24);

set_bg_color(color_scheme[CALEND_COLOR_BG]);	//	calendar background
fill_screen_bg();

set_graph_callback_to_ram_1();
load_font(); // load fonts

_sprintf(&text_buffer[0], " %d", year);
int month_text_width = text_width(monthname[month]);
int year_text_width  = text_width(&text_buffer[0]);

set_fg_color(color_scheme[CALEND_COLOR_MONTH]);		//	color of the month
text_out(monthname[month], (176-month_text_width-year_text_width)/2 ,1); 	// 	display the name of the month

set_fg_color(color_scheme[CALEND_COLOR_YEAR]);		//	color of the year
text_out(&text_buffer[0],  (176+month_text_width-year_text_width)/2 ,1); 	// 	conclusion of the year

//text_out("←", 5		 ,2); 		// left arrow output
//text_out("→", 176-5-text_width("→"),2); 		// right arrow output

int calend_name_height = get_text_height();

set_fg_color(color_scheme[CALEND_COLOR_SEPAR]);
draw_horizontal_line(CALEND_Y_BASE, H_MARGIN, 176-H_MARGIN);	// Upper weekday separator
draw_horizontal_line(CALEND_Y_BASE+V_MARGIN+calend_name_height+V_MARGIN, H_MARGIN, 176-H_MARGIN);	// Bottom day separator
//draw_horizontal_line(175, H_MARGIN, 176-H_MARGIN);	// Bottom month separator
 
// Names of the days of the week
for (unsigned i=1; (i<=7);i++){
	if ( i>5 ){		//	weekends
		set_bg_color(color_scheme[CALEND_COLOR_HOLY_NAME_BG]);
		set_fg_color(color_scheme[CALEND_COLOR_HOLY_NAME_FG]);
	} else {		//	work days
		set_bg_color(color_scheme[CALEND_COLOR_BG]);	
		set_fg_color(color_scheme[CALEND_COLOR_WORK_NAME]);
	}
	
	
	//  drawing the background of the weekend names
	int pos_x1 = H_MARGIN +(i-1)*(WIDTH  + H_SPACE);
	int pos_y1 = CALEND_Y_BASE+V_MARGIN+1;
	int pos_x2 = pos_x1 + WIDTH;
	int pos_y2 = pos_y1 + calend_name_height;

	// background for each day of the week name
	//draw_filled_rect_bg(pos_x1, pos_y1, pos_x2, pos_y2);
	
	// display the names of the days of the week. if the width of the name is greater than the width of the field, print short names
	//if (text_width(weekday_string[1]) <= (WIDTH - 2))
		text_out_center(weekday_string[i], pos_x1 + WIDTH/2, pos_y1 + (calend_name_height-get_text_height())/2 );	
	//else 
	//	text_out_center(weekday_string_short[i], pos_x1 + WIDTH/2, pos_y1 + (calend_name_height-get_text_height())/2 );	
}


int calend_days_y_base = CALEND_Y_BASE+1+V_MARGIN+calend_name_height+V_MARGIN+1;

if (isLeapYear(year)>0) day_month[2]=29;

unsigned char d=wday(1,month, year);
unsigned char m=month;
if (d>1) {
     m=(month==1)?12:month-1;
     d=day_month[m]-d+2;
	}

// day of the month
for (unsigned i=1; (i<=7*6);i++){
     
	 unsigned char row = (i-1)/7;
     unsigned char col = (i-1)%7+1;
         
    _sprintf (&text_buffer[0], "%2.0d", d);
	
	int bg_color = 0;
	int fg_color = 0;
	int frame_color = 0; 	// flowers
	int frame    = 0; 		// 1-frame; 0 - fill
	
	// if the current day of the current month
	if ( (m==month)&&(d==day) ){
		
		if ( color_scheme[CALEND_COLOR_TODAY_BG] & (1<<31) ) {// if the fill is disabled only the frame
			
			// set the border color to CALEND_COLOR_TODAY_BG, the background inside the border and the text color are the same as before
			frame_color = (color_scheme[CALEND_COLOR_TODAY_BG &COLOR_MASK]);
			// draw a frame
			frame = 1;
			
			if ( col > 5 ){ // if weekend
				bg_color = (color_scheme[CALEND_COLOR_CUR_HOLY_BG]); 
				fg_color = (color_scheme[CALEND_COLOR_CUR_HOLY_FG]);
			} else {		//	if weekdays
				bg_color = (color_scheme[CALEND_COLOR_BG]); 
				fg_color = (color_scheme[CALEND_COLOR_CUR_WORK]);
			};
			
		} else { 	// if fill is enabled
			if ( col > 5 ){ // if weekend
				bg_color = (color_scheme[CALEND_COLOR_TODAY_BG] & COLOR_MASK); 
				fg_color = (color_scheme[CALEND_COLOR_TODAY_FG]);
			} else {		//	if weekdays
				bg_color = (color_scheme[CALEND_COLOR_TODAY_BG] &COLOR_MASK); 
				fg_color = (color_scheme[CALEND_COLOR_TODAY_FG]);
			};
		};
		

	} else {
	if ( col > 5 ){  // if weekend
		if (month == m){
			bg_color = (color_scheme[CALEND_COLOR_CUR_HOLY_BG]); 
			fg_color = (color_scheme[CALEND_COLOR_CUR_HOLY_FG]);
		} else {
			bg_color = (color_scheme[CALEND_COLOR_NOT_CUR_HOLY_BG]); 
			fg_color = (color_scheme[CALEND_COLOR_NOT_CUR_HOLY_FG]);
		}
	} else {		// if weekdays
		if (month == m){
			bg_color = (color_scheme[CALEND_COLOR_BG]); 
			fg_color = (color_scheme[CALEND_COLOR_CUR_WORK]);
		} else {
			bg_color = (color_scheme[CALEND_COLOR_BG]); 
			fg_color = (color_scheme[CALEND_COLOR_NOT_CUR_WORK]);
		}
	}
	}
	
	
	
	//  line: from 7 to 169 = 162 px in width 7 numbers by 24 px per number 7+(22+2)*6+22+3
	//  lines: from 57 to 174 = 117px in height 6 lines at 19px per line 1+(17+2)*5+18
	
	// drawing the number background
	int pos_x1 = H_MARGIN +(col-1)*(WIDTH  + H_SPACE);
	int pos_y1 = calend_days_y_base + V_MARGIN + row *(HEIGHT + V_SPACE)+1;
	int pos_x2 = pos_x1 + WIDTH;
	int pos_y2 = pos_y1 + HEIGHT;	
	
	if (frame){
		// print the number
		set_bg_color(bg_color);
		set_fg_color(fg_color);
		text_out_center(&text_buffer[0], pos_x1+WIDTH/2, pos_y1+(HEIGHT-get_text_height())/2);
		
		//	draw a frame
		set_fg_color(frame_color);
		draw_rect(pos_x1, pos_y1, pos_x2-1, pos_y2-1);	
	} else {
		// draw a fill
		set_bg_color(bg_color);
		draw_filled_rect_bg(pos_x1, pos_y1, pos_x2, pos_y2);
		
		// print the number
		set_fg_color(fg_color);
		text_out_center(&text_buffer[0], pos_x1+WIDTH/2, pos_y1+(HEIGHT-get_text_height())/2);
	};
	
	
	
		
	if ( d < day_month[m] ) {
		d++;
	} else {
		d=1; 
		m=(m==12)?1:(m+1);
	}
}



};

unsigned char wday(unsigned int day,unsigned int month,unsigned int year)
{
    signed int a = (14 - month) / 12;
    signed int y = year - a;
    signed int m = month + 12 * a - 2;
    return 1+(((day + y + y / 4 - y / 100 + y / 400 + (31 * m) / 12) % 7) +6)%7;
}

unsigned char isLeapYear(unsigned int year){
    unsigned char result = 0;
    if ( (year % 4)   == 0 ) result++;
    if ( (year % 100) == 0 ) result--;
    if ( (year % 400) == 0 ) result++;
return result;
}

void key_press_calend_screen(){
	struct calend_** 	calend_p = get_ptr_temp_buf_2(); 		//	pointer to screen data pointer
	struct calend_ *	calend = *calend_p;			//	pointer to screen data
	
	show_menu_animate(calend->ret_f, (unsigned int)show_calend_screen, ANIMATE_RIGHT);	
};


void calend_screen_job(){
	struct calend_** 	calend_p = get_ptr_temp_buf_2(); 		//	pointer to screen data pointer
	struct calend_ *	calend = *calend_p;			//	pointer to screen data

	// when the update timer is reached, exit
	show_menu_animate(calend->ret_f, (unsigned int)show_calend_screen, ANIMATE_LEFT);
}

int dispatch_calend_screen (void *param){
	struct calend_** 	calend_p = get_ptr_temp_buf_2(); 		//	pointer to screen data pointer
	struct calend_ *	calend = *calend_p;			//	pointer to screen data
	
	struct calend_opt_ calend_opt;					//	calendar options
	
	struct datetime_ datetime;
	// get the current date
	
		
	get_current_date_time(&datetime);
	unsigned int day;
	
	//	char text_buffer[32];	
	 struct gesture_ *gest = param;
	 int result = 0;
	 
	switch (gest->gesture){
		case GESTURE_CLICK: {
			
			// vibration with any touch on the screen
			vibrate (1, 40, 0);
			
			
			if ( gest->touch_pos_y < CALEND_Y_BASE ){ // clicked on the top line
				if (gest->touch_pos_x < 44){
					if ( calend->year > 1600 ) calend->year--;
				} else 
				if (gest->touch_pos_x > (176-44)){
					if ( calend->year < 3000 ) calend->year++;
				} else {
					calend->day 	= datetime.day;
					calend->month 	= datetime.month;
					calend->year 	= datetime.year;
				}	

				 if ( (calend->year == datetime.year) && (calend->month == datetime.month) ){
					day = datetime.day;
				 } else {	
					day = 0;
				 }
					draw_month(day, calend->month, calend->year);
					repaint_screen_lines(1, 176);			
				
			} else { // clicked on calendar
			
				calend->color_scheme = ((calend->color_scheme+1)%COLOR_SCHEME_COUNT);
						
				// first refresh the screen
				if ( (calend->year == datetime.year) && (calend->month == datetime.month) ){
					day = datetime.day;
				 } else {	
					day = 0;
				 }
					draw_month(day, calend->month, calend->year);
					repaint_screen_lines(1, 176);			

				// then write options to flash memory, because it's a long operation
				// TODO: 1. if there are more options than the color scheme - redo the save to save before exiting.					
				calend_opt.color_scheme = calend->color_scheme;	
				
				// write the settings to flash memory
				ElfWriteSettings(calend->proc->index_listed, &calend_opt, OPT_OFFSET_CALEND_OPT, sizeof(struct calend_opt_));
			}
			
			// extend inactivity exit timer through INACTIVITY_PERIOD with
			set_update_period(1, INACTIVITY_PERIOD);
			break;
		};
		
		case GESTURE_SWIPE_RIGHT: 	//	swap right
		case GESTURE_SWIPE_LEFT: {	// from right to left
	
			if ( get_left_side_menu_active()){
					set_update_period(0,0);
					
					void* show_f = get_ptr_show_menu_func();

					// run dispatch_left_side_menu with param parameter as a result, the corresponding side screen will start
					// this will upload the data of the current application and deactivate it.
					dispatch_left_side_menu(param);
										
					if ( get_ptr_show_menu_func() == show_f ){
					// if dispatch_left_side_menu worked unsuccessfully (there is nowhere to scroll) then in show_menu_func it will still be
					// our show_calend_screen function is contained, then just ignore this gesture
						
						// extend the inactivity exit timer through INACTIVITY_PERIOD with
						set_update_period(1, INACTIVITY_PERIOD);
						return 0;
					}

										
// if dispatch_left_side_menu worked, then we finish our application, because screen data has already been uploaded
// at this stage, a new screen is already running (the one where you swiped)
					
					Elf_proc_* proc = get_proc_by_addr(main);
					proc->ret_f = NULL;
					
					elf_finish(main);	//	unload Elf from memory
					return 0;
				} else { 			//	if the launch is not from the quick menu, we process swipes separately
					switch (gest->gesture){
						case GESTURE_SWIPE_RIGHT: {	//	swap right
							return show_menu_animate(calend->ret_f, (unsigned int)show_calend_screen, ANIMATE_RIGHT);	
							break;
						}
						case GESTURE_SWIPE_LEFT: {	// swipe left
							// action when starting from the menu and further swipe left
							
							break;
						}
					} /// switch (gest->gesture)
				}

			break;
		};	//	case GESTURE_SWIPE_LEFT:
		
		
		case GESTURE_SWIPE_UP: {	// swipe up
			if ( calend->month < 12 ) 
					calend->month++;
			else {
					calend->month = 1;
					calend->year++;
			}
			
			if ( (calend->year == datetime.year) && (calend->month == datetime.month) )
				day = datetime.day;
			else	
				day = 0;
			draw_month(day, calend->month, calend->year);
			repaint_screen_lines(1, 176);
			
			// extend inactivity exit timer through INACTIVITY_PERIOD с
			set_update_period(1, INACTIVITY_PERIOD);
			break;
		};
		case GESTURE_SWIPE_DOWN: {	// swipe down
			if ( calend->month > 1 ) 
					calend->month--;
			else {
					calend->month = 12;
					calend->year--;
			}
			
			if ( (calend->year == datetime.year) && (calend->month == datetime.month) )
				day = datetime.day;
			else	
				day = 0;
			draw_month(day, calend->month, calend->year);			
			repaint_screen_lines(1, 176);
			
			// extend inactivity exit timer through INACTIVITY_PERIOD с
			set_update_period(1, INACTIVITY_PERIOD);
			break;
		};		
		default:{	// Something went wrong...
			break;
		};		
		
	}
	
	
	return result;
};
