/*
 * CFile1.c
 *
 * Created: 15.3.2020 15:33:20
 *  Author: Kristjan
 */ 

#include "GPIO.h"

char keyPressedID;

void LED_Init()
{
	DDRE = 0x0F; //---- 1111
	PORTE = 0x0; //---- 0000
};

void Keys_Init()
{
	PORTD |= 0x3C; //XX11 11XX
	DDRD &= ~(0x3C); //XX00 00XX
};

void Buzz_Init()
{
	DDRB|=(1<<2);
	PORTB&=~(1<<2);
}

void Buzz_ON() {	PORTB |= (1<<2);	}
void Buzz_OFF() {	PORTB &= ~(1<<2);	}

void LED1_On(){ PORTE |= (1<<LED_1);	}
void LED2_On(){ PORTE |= (1<<LED_2);	}
void LED3_On(){ PORTE |= (1<<LED_3);	}
void LED4_On(){ PORTE |= (1<<LED_4);	}

void LED1_Off(){ PORTE &= ~(1<<LED_1);	}
void LED2_Off(){ PORTE &= ~(1<<LED_2);	}
void LED3_Off(){ PORTE &= ~(1<<LED_3);	}
void LED4_Off(){ PORTE &= ~(1<<LED_4);	}

void LED1_Tgl(){ PORTE ^= (1<<LED_1);	}
void LED2_Tgl(){ PORTE ^= (1<<LED_2);	}
void LED3_Tgl(){ PORTE ^= (1<<LED_3);	}
void LED4_Tgl(){ PORTE ^= (1<<LED_4);	}

void LED_SetLEDs(char LED_mask)
{
	PORTE |= LED_mask;
}

void LED_ClrLEDs(char LED_mask)
{
	PORTE &= ~LED_mask;
}

char is_LED1_On(){ return PINE & LED_1;	}
char is_LED2_On(){ return PINE & LED_2;	}
char is_LED3_On(){ return PINE & LED_3;	}
char is_LED4_On(){ return PINE & LED_4;	}

char is_BTN_OK_Pressed(){ return !(PIND & (1<<BTN_OK));}
char is_BTN_ESC_Pressed(){ return !(PIND & (1<<BTN_ESC));}
char is_BTN_A_Pressed(){ return !(PIND & (1<<BTN_A));}
char is_BTN_B_Pressed(){ return !(PIND & (1<<BTN_B));}

char is_BTN_Pressed(char BTN_pin_number)
{
	return !(PIND & (1<<BTN_pin_number)); //negation because PRESSED=0
	//return (lastPressed & BTN_pin_number);
}

char is_BTN_Released(char BTN_pin_number)
{
	return (PIND & (1<<BTN_pin_number)); //no negation because RELEASED=1
	//return (lastReleased & BTN_pin_number);
}

void KBD_Read()
{
	static char oldKeyPressedID;
	static uint32_t counterPressingDet;
	
	if(counterPressingDet >= 125)
	{
		oldKeyPressedID = (0xF<<BTN_ESC);
		counterPressingDet = 0;
	}
	char newKeyPressedID = PIND&(0xF<<BTN_ESC);//maska na bitih 2-5 (kjer so tipke)
	char pressed = (newKeyPressedID^oldKeyPressedID)&oldKeyPressedID;
	
	if(pressed & (1<<BTN_ESC)) keyPressedID = BTN_ESC;
	if(pressed & (1<<BTN_OK)) keyPressedID = BTN_OK;
	if(pressed & (1<<BTN_A)) keyPressedID = BTN_A;
	if(pressed & (1<<BTN_B)) keyPressedID = BTN_B;
	oldKeyPressedID = newKeyPressedID;
	counterPressingDet++;
}

char KBD_GetKey()
{
	char keyPressedID_l = keyPressedID;
	keyPressedID = BTN_none;
	return keyPressedID_l;
}