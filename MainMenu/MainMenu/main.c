#include <stdint.h> 
#include <stdlib.h> 
#include <stdio.h> 
#include <stdbool.h> 
#include <string.h> 
#include <math.h> 
#include <avr/io.h> 
#include <avr/interrupt.h> 
#include <avr/eeprom.h> 
#include <avr/portpins.h> 
#include <avr/pgmspace.h> 
#include <util/delay.h>
 
//FreeRTOS include files 
#include "FreeRTOS.h" 
#include "task.h" 
#include "croutine.h" 

#include "keypad.h"
#include "nokia5110.h"

char password[] = "00000000";
char tempPassword[] = "00000000";
char verifyPassword[] = "00000000";

unsigned char change_password_flag = 0;
unsigned char reset_system_flag = 0;
unsigned char confirm_password_attempts = 0;

unsigned char lock_one_status = 0; // 0 = locked, 1 = unlocked;
unsigned char lock_two_status = 0; // 0 = locked, 1 = unlocked;
unsigned char lock_selection = 0;
unsigned char unlocked_door_counter = 0;

void getPassword(){
	unsigned char key_value;
	unsigned char key_value_temp;
	unsigned char i;
	
	char display_password[] = "_*******";
	
	for(i = 0; i < 9; ++i){
		tempPassword[i] = '\0';
	}
	
	key_value = GetKeypadKey();
	
	while((key_value = GetKeypadKey()) == '\0'){ _delay_ms(200); }
	while((key_value_temp = GetKeypadKey()) == key_value){ _delay_ms(200); }
	
	i = 0;
	
	while(key_value != '#'){
		if(key_value == '*'){
			if(i == 0){
				display_password[i] = '_';
				tempPassword[i] = '\0';
			}
			
			else{
				--i;
				display_password[i] = '_';
				display_password[i+1] = '*';
				tempPassword[i] = '\0';
				tempPassword[i+1] = '\0';
			}
		}
		
		else{
			if(i == 0){
				display_password[i] = key_value;
				tempPassword[i+1] = '_';
				tempPassword[i] = key_value;
				++i;
				
			}
			
			else if(i == 7){
				display_password[i] = key_value;
				display_password[i-1] = '*';
				tempPassword[i] = key_value;
			}
			
			else{
				display_password[i-1] = '*';
				display_password[i] = key_value;
				display_password[i+1] = '_';
				tempPassword[i] = key_value;
				++i;
			}
		}
		
		nokia_lcd_clear();
		nokia_lcd_write_string("Enter Password",1);
		nokia_lcd_set_cursor(0,10);
		nokia_lcd_write_string(display_password,1);
		nokia_lcd_render();
		
		while((key_value = GetKeypadKey()) == '\0'){ _delay_ms(200); }
		while((key_value_temp = GetKeypadKey()) == key_value){ _delay_ms(200); }
	}
	
	return;
}

void mainMenuPrompt(){
	nokia_lcd_clear();
	nokia_lcd_write_string("Select Door", 1);
	nokia_lcd_set_cursor(0,10);
	nokia_lcd_write_string("  1. Top",1);
	nokia_lcd_set_cursor(0,20);
	nokia_lcd_write_string("  2. Bottom",1);
	nokia_lcd_set_cursor(0,30);
	nokia_lcd_write_string("System",1);
	nokia_lcd_set_cursor(0,40);
	nokia_lcd_write_string("  3. Settings",1);
	nokia_lcd_render();
}


enum MenuState {init_menu, main_menu, settings_menu, change_password, reset_system,lock_menu} menu_state;

void MENUSTATE_Init(){
	menu_state = init_menu;
}

void MENUSTATE_Tick(){
	unsigned char keypad_key_selection = '\0';
	unsigned char keypad_key_comparison = '\0';
	//Actions
	switch(menu_state){
		case init_menu:
			mainMenuPrompt();
			lock_one_status = 0;
			lock_two_status = 0;
			lock_selection = 0;
			
			break;
			
		case main_menu:
			while((keypad_key_selection = GetKeypadKey()) == '\0'){ _delay_ms(200); }
			while((keypad_key_comparison = GetKeypadKey()) == keypad_key_selection){ _delay_ms(200); }
			break;
			
		case settings_menu:
			while((keypad_key_selection = GetKeypadKey()) == '\0'){ _delay_ms(200); }
			while((keypad_key_comparison = GetKeypadKey()) == keypad_key_selection){ _delay_ms(200); }
			break;
			
		case change_password:
			break;
			
		case reset_system:
			break;
			
		case lock_menu:
			break;
			
		default:
			break;
	}
	
	//Transitions
	switch(menu_state){
		case init_menu:
			menu_state = main_menu;
			break;
		
		case main_menu:
			if(keypad_key_selection == '3'){
				nokia_lcd_clear();
				nokia_lcd_write_string("1. Change Password",1);
				nokia_lcd_set_cursor(0,10);
				nokia_lcd_write_string("2. Reset System",1);
				nokia_lcd_set_cursor(0,20);
				nokia_lcd_write_string("3. Main Menu",1);
				nokia_lcd_render();
				menu_state = settings_menu;
			}
			
			else if(keypad_key_selection == '1'){
				lock_selection = 1;
				nokia_lcd_clear();
				nokia_lcd_write_string("Enter Password",1);
				nokia_lcd_set_cursor(0,10);
				nokia_lcd_write_string("_*******",1);
				nokia_lcd_render();
				menu_state = lock_menu;
			}
			
			else if(keypad_key_selection == '2'){
				lock_selection = 2;
				nokia_lcd_clear();
				nokia_lcd_write_string("Enter Password",1);
				nokia_lcd_set_cursor(0,10);
				nokia_lcd_write_string("_*******",1);
				nokia_lcd_render();
				menu_state = lock_menu;
			}
			
			else{
				lock_selection = 0;
				menu_state = main_menu;
			}
			
			break;
		
		case settings_menu:
			if(keypad_key_selection == '3'){
				mainMenuPrompt();
				menu_state = main_menu;
			}
			
			else if(keypad_key_selection == '1'){
				change_password_flag = 1;
				menu_state = change_password;
			}
			
			else if(keypad_key_selection == '2'){
				reset_system_flag = 1;
				menu_state = reset_system;
			}
			
			else
			
			break;
			
		case change_password:
			if(change_password_flag){
				menu_state = change_password;
			}
			
			else{
				menu_state = main_menu;
			}
			
			break;
		
		case reset_system:
			if(reset_system_flag){
				menu_state = reset_system;
			}
		
			else{
				menu_state = main_menu;
			}
		
			break;
			
		case lock_menu:

			getPassword();
			
			if(!strcmp(password,tempPassword)){
				
				nokia_lcd_clear();
				nokia_lcd_write_string("Door Unlocked",1);
				nokia_lcd_render();
				
				if(lock_selection == 1){
					lock_one_status = 1;
					PORTB = SetBit(PORTB,0,lock_one_status);
					
					_delay_ms(10000);
					_delay_ms(10000);
					
					lock_one_status = 0;
					PORTB = SetBit(PORTB,0,lock_one_status);
				}
				
				else{
					lock_two_status = 1;
					PORTB = SetBit(PORTB,1,lock_two_status);
					
					_delay_ms(10000);
					_delay_ms(10000);
					
					lock_two_status = 0;
					PORTB = SetBit(PORTB,1,lock_two_status);
				}
				
				lock_selection = 0;
			}
			
			else{
				nokia_lcd_clear();
				nokia_lcd_write_string("Incorrect Password",1);
				nokia_lcd_render();
				
				_delay_ms(2000);
				_delay_ms(2000);
			}
			
			mainMenuPrompt();
			
			menu_state = main_menu;
			
			break;
		
		default:
			menu_state = init_menu;
			break;
	}
}

void MENUSTATE_Task(){
	MENUSTATE_Init();
	for(;;){
		MENUSTATE_Tick();
		vTaskDelay(100);
	}
}

void MENUSTATEPulse(unsigned portBASE_TYPE Priority)
{
	xTaskCreate(MENUSTATE_Task, (signed portCHAR *)"MENUSTATE_Task", configMINIMAL_STACK_SIZE, NULL, Priority, NULL );
}	

//	State Machine will allow the user to change the password of the system

enum ChangePassword {cp_wait, cp_original_password, cp_new_password, cp_verify_password} change_password_state;
	
void ChangePassword_Init(){
	change_password_state = cp_wait;
}
	
void ChangePassword_Tick(){
	//actions
	switch(change_password_state){
		case cp_wait:
			break;
			
		case cp_original_password:
			getPassword();
			break;
		
		case cp_new_password:
			getPassword();
			break;
			
		case cp_verify_password:
			getPassword();
			break;
			
		default:
			break;
	}
	
	//transitions
	switch(change_password_state){
		case cp_wait:
			if(change_password_flag){
				nokia_lcd_clear();
				nokia_lcd_write_string("Current Password",1);
				nokia_lcd_set_cursor(0,10);
				nokia_lcd_write_string("_*******",1);
				nokia_lcd_render();
				change_password_state = cp_original_password;
			}
			break;
		
		case cp_original_password:
			if(!strcmp(password,tempPassword)){
				nokia_lcd_clear();
				nokia_lcd_write_string("New Password",1);
				nokia_lcd_set_cursor(0,10);
				nokia_lcd_write_string("_*******",1);
				nokia_lcd_render();
				change_password_state = cp_new_password;
			}
			
			else{
				nokia_lcd_clear();
				nokia_lcd_write_string("Incorrect Password",1);
				nokia_lcd_render();
				
				_delay_ms(2000);
				_delay_ms(2000);
				
				change_password_flag = 0;
				mainMenuPrompt();
				change_password_state = cp_wait;
			}
			
			break;
			
		case cp_new_password:
			nokia_lcd_clear();
			nokia_lcd_write_string("Verify Password",1);
			nokia_lcd_set_cursor(0,10);
			nokia_lcd_write_string("_*******",1);
			nokia_lcd_render();
			strcpy(verifyPassword,tempPassword);
			confirm_password_attempts = 0;
			change_password_state = cp_verify_password;
			break;
		
		case cp_verify_password:
			if(!strcmp(verifyPassword,tempPassword)){
				strcpy(password,verifyPassword);
				nokia_lcd_clear();
				nokia_lcd_write_string("New Password Set",1);
				nokia_lcd_render();
				
				_delay_ms(1000);
				_delay_ms(1000);
				_delay_ms(1000);
				
				mainMenuPrompt();
				
				change_password_flag = 0;
				change_password_state = cp_wait;
			}
			
			else{
				if(confirm_password_attempts < 2){
					nokia_lcd_clear();
					nokia_lcd_write_string("Incorrect Password, try again!",1);
					nokia_lcd_render();
					
					_delay_ms(1000);
					_delay_ms(1000);
					
					nokia_lcd_clear();
					nokia_lcd_write_string("Verify Password",1);
					nokia_lcd_set_cursor(0,10);
					nokia_lcd_write_string("_*******",1);
					nokia_lcd_render();
					++confirm_password_attempts;
					change_password_state = cp_verify_password;
					
				}
				
				else{
					nokia_lcd_clear();
					nokia_lcd_write_string("New Password not set. Try again later.",1);
					nokia_lcd_render();
					
					_delay_ms(1000);
					_delay_ms(1000);
					
					mainMenuPrompt();
					change_password_state = cp_wait;
				}
			}
			
			break;
			
		default:
			change_password_state = cp_wait;
			break;
	}
}
	
void ChangePassword_Task(){
	ChangePassword_Init();
	for(;;){
		ChangePassword_Tick();
		vTaskDelay(100);
	}
}
	
void ChangePasswordPulse(unsigned portBASE_TYPE Priority)
{
	xTaskCreate(ChangePassword_Task, (signed portCHAR *)"ChangePassword_Task", configMINIMAL_STACK_SIZE, NULL, Priority, NULL );
}

//	State Machine will allow the user to reset the system

enum ResetSystem {rs_wait, rs_password, rs_verify_password, rs_reset} reset_system_state;

void ResetSystem_Init(){
	reset_system_state = cp_wait;
}

void ResetSystem_Tick(){
	//actions
	switch(reset_system_state){
		case rs_wait:
			break;
		
		case rs_password:
			break;
		
		case rs_verify_password:
			break;
		
		case rs_reset:
			break;
		
		default:
			break;
	}
	
	//transitions
	switch(reset_system_state){
		case rs_wait:
			break;
		
		case rs_password:
			break;
		
		case rs_verify_password:
			break;
		
		case rs_reset:
			break;
		
		default:
			reset_system_state = rs_wait;
			break;
	}
}

void ResetSystem_Task(){
	ResetSystem_Init();
	for(;;){
		ResetSystem_Tick();
		vTaskDelay(100);
	}
}

void ResetSystemPulse(unsigned portBASE_TYPE Priority)
{
	xTaskCreate(ResetSystem_Task, (signed portCHAR *)"ResetSystem_Task", configMINIMAL_STACK_SIZE, NULL, Priority, NULL );
}
 
int main(void) 
{
	DDRB = 0xFF;	PORTB = 0x00;
	DDRC = 0x0F;	PORTC = 0xF0;
	DDRD = 0xFF;	PORTD = 0x00;
	
	nokia_lcd_init();
	
	//Start Tasks  
	MENUSTATEPulse(1);
	ChangePasswordPulse(1);
	ResetSystemPulse(1);
	
    //RunSchedular 
	vTaskStartScheduler(); 
 
	return 0; 
}