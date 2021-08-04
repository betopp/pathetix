//ps2kbd.c
//PS/2 keyboard driver
//Bryan E. Topp <betopp@betopp.com> 2021

#include "amd64.h"
#include "pic8259.h"
#include "hal_kbd.h"


//State of PS/2 keyboard - if a break prefix was received.
bool ps2kbd_break;

//State of PS/2 keyboard - if an extend prefix was received.
bool ps2kbd_extend;

//Mapping of PS/2 scancodes to SDL scancodes.
//Table for decoding PS/2 scan code set 2 to HAL scancodes.
//First 256 entries correspond to 1-byte make-codes, second 256 entries correspond to those following a 0xE0 code.
static const hal_kbd_scancode_t ps2kbd_table[512] = 
{
	[0x00E] = HAL_KBD_SCANCODE_GRAVE,
	[0x016] = HAL_KBD_SCANCODE_1,
	[0x01E] = HAL_KBD_SCANCODE_2,
	[0x026] = HAL_KBD_SCANCODE_3,
	[0x025] = HAL_KBD_SCANCODE_4,
	[0x02E] = HAL_KBD_SCANCODE_5,
	[0x036] = HAL_KBD_SCANCODE_6,
	[0x03D] = HAL_KBD_SCANCODE_7,
	[0x03E] = HAL_KBD_SCANCODE_8,
	[0x046] = HAL_KBD_SCANCODE_9,
	[0x045] = HAL_KBD_SCANCODE_0,
	[0x04E] = HAL_KBD_SCANCODE_MINUS,
	[0x055] = HAL_KBD_SCANCODE_EQUALS,
	[0x05D] = HAL_KBD_SCANCODE_BACKSLASH,
	[0x066] = HAL_KBD_SCANCODE_BACKSPACE,
	[0x00D] = HAL_KBD_SCANCODE_TAB,
	[0x015] = HAL_KBD_SCANCODE_Q,
	[0x01D] = HAL_KBD_SCANCODE_W,
	[0x024] = HAL_KBD_SCANCODE_E,
	[0x02D] = HAL_KBD_SCANCODE_R,
	[0x02C] = HAL_KBD_SCANCODE_T,
	[0x035] = HAL_KBD_SCANCODE_Y,
	[0x03C] = HAL_KBD_SCANCODE_U,
	[0x043] = HAL_KBD_SCANCODE_I,
	[0x044] = HAL_KBD_SCANCODE_O,
	[0x04D] = HAL_KBD_SCANCODE_P,
	[0x054] = HAL_KBD_SCANCODE_LEFTBRACKET,
	[0x05B] = HAL_KBD_SCANCODE_RIGHTBRACKET,
	[0x058] = HAL_KBD_SCANCODE_CAPSLOCK,
	[0x01C] = HAL_KBD_SCANCODE_A,
	[0x01B] = HAL_KBD_SCANCODE_S,
	[0x023] = HAL_KBD_SCANCODE_D,
	[0x02B] = HAL_KBD_SCANCODE_F,
	[0x034] = HAL_KBD_SCANCODE_G,
	[0x033] = HAL_KBD_SCANCODE_H,
	[0x03B] = HAL_KBD_SCANCODE_J,
	[0x042] = HAL_KBD_SCANCODE_K,
	[0x04B] = HAL_KBD_SCANCODE_L,
	[0x04C] = HAL_KBD_SCANCODE_SEMICOLON,
	[0x052] = HAL_KBD_SCANCODE_APOSTROPHE,
	[0x05A] = HAL_KBD_SCANCODE_RETURN,
	[0x012] = HAL_KBD_SCANCODE_LSHIFT,
	[0x01A] = HAL_KBD_SCANCODE_Z,
	[0x022] = HAL_KBD_SCANCODE_X,
	[0x021] = HAL_KBD_SCANCODE_C,
	[0x02A] = HAL_KBD_SCANCODE_V,
	[0x032] = HAL_KBD_SCANCODE_B,
	[0x031] = HAL_KBD_SCANCODE_N,
	[0x03A] = HAL_KBD_SCANCODE_M,
	[0x041] = HAL_KBD_SCANCODE_COMMA,
	[0x049] = HAL_KBD_SCANCODE_PERIOD,
	[0x04A] = HAL_KBD_SCANCODE_SLASH,
	[0x059] = HAL_KBD_SCANCODE_RSHIFT,
	[0x014] = HAL_KBD_SCANCODE_LCTRL,
	[0x011] = HAL_KBD_SCANCODE_LALT,
	[0x029] = HAL_KBD_SCANCODE_SPACE,
	[0x111] = HAL_KBD_SCANCODE_RALT,
	[0x114] = HAL_KBD_SCANCODE_RCTRL,
	[0x170] = HAL_KBD_SCANCODE_INSERT,
	[0x171] = HAL_KBD_SCANCODE_DELETE,
	[0x16B] = HAL_KBD_SCANCODE_LEFT,
	[0x16C] = HAL_KBD_SCANCODE_HOME,
	[0x169] = HAL_KBD_SCANCODE_END,
	[0x175] = HAL_KBD_SCANCODE_UP,
	[0x172] = HAL_KBD_SCANCODE_DOWN,
	[0x17D] = HAL_KBD_SCANCODE_PAGEUP,
	[0x17A] = HAL_KBD_SCANCODE_PAGEDOWN,
	[0x174] = HAL_KBD_SCANCODE_RIGHT,
	[0x077] = HAL_KBD_SCANCODE_NUMLOCKCLEAR,
	[0x06C] = HAL_KBD_SCANCODE_KP_7,
	[0x06B] = HAL_KBD_SCANCODE_KP_4,
	[0x069] = HAL_KBD_SCANCODE_KP_1,
	[0x14A] = HAL_KBD_SCANCODE_KP_DIVIDE,
	[0x075] = HAL_KBD_SCANCODE_KP_8,
	[0x073] = HAL_KBD_SCANCODE_KP_5,
	[0x072] = HAL_KBD_SCANCODE_KP_2,
	[0x070] = HAL_KBD_SCANCODE_KP_0,
	[0x07C] = HAL_KBD_SCANCODE_KP_MULTIPLY,
	[0x07D] = HAL_KBD_SCANCODE_KP_9,
	[0x074] = HAL_KBD_SCANCODE_KP_6,
	[0x07A] = HAL_KBD_SCANCODE_KP_3,
	[0x071] = HAL_KBD_SCANCODE_KP_DECIMAL,
	[0x07B] = HAL_KBD_SCANCODE_KP_MINUS,
	[0x079] = HAL_KBD_SCANCODE_KP_PLUS,
	[0x15A] = HAL_KBD_SCANCODE_KP_ENTER,
	[0x076] = HAL_KBD_SCANCODE_ESCAPE,
	[0x005] = HAL_KBD_SCANCODE_F1,
	[0x006] = HAL_KBD_SCANCODE_F2,
	[0x004] = HAL_KBD_SCANCODE_F3,
	[0x00C] = HAL_KBD_SCANCODE_F4,
	[0x003] = HAL_KBD_SCANCODE_F5,
	[0x00B] = HAL_KBD_SCANCODE_F6,
	[0x083] = HAL_KBD_SCANCODE_F7,
	[0x00A] = HAL_KBD_SCANCODE_F8,
	[0x001] = HAL_KBD_SCANCODE_F9,
	[0x009] = HAL_KBD_SCANCODE_F10,
	[0x078] = HAL_KBD_SCANCODE_F11,
	[0x007] = HAL_KBD_SCANCODE_F12,
};


//Sends a 1-byte command to the PS/2 controller
static void ps2kbd_cmd1(uint8_t cmd)
{
	outb(0x64, cmd);
}

//Sends a 2-byte command to the PS/2 controller
static void ps2kbd_cmd2(uint8_t cmd, uint8_t parm)
{
	outb(0x64, cmd); //Send command
	while(inb(0x64) & 0x02) { } //Wait for controller's input buffer to be empty
	outb(0x60, parm); //Send data
}

//Sends data to the PS/2 port
static int ps2kbd_send(int dev, const void *buf, int size)
{
	if(size <= 0)
		return 0;
	
	//Check status register bit 1 - if it's set, we don't have room to send another byte now.
	if(inb(0x64) & 0x02)
		return 0;
	
	//Otherwise, enqueue one byte.
	//If we're talking to the second port, preface it with command 0xD4.
	if(dev)
	{
		outb(0x64, 0xD4);
		outb(0x60, *((uint8_t*)buf));
		return 1;
	}
	else
	{
		outb(0x60, *((uint8_t*)buf));
		return 1;
	}
}

void ps2kbd_init(void)
{	
	//Initialize PS/2 controller
	ps2kbd_cmd1(0xAD); //Disable first port
	ps2kbd_cmd1(0xA7); //Disable second port
	inb(0x60); //Read data to make sure there's no stray input
	ps2kbd_cmd2(0x60, 0x00); //Disable interrupts, enable clocks, disable translation
	ps2kbd_cmd1(0xAE); //Enable first port
	//ps2kbd_cmd1(0xA8); //Enable second port
	ps2kbd_cmd2(0x60, 0x03); //Enable interrupts and disable scancode translation
	
	//Reset PS/2 devices
	const uint8_t rstcmd = 0xFF;
	while(!ps2kbd_send(0, &rstcmd, 1)) { }
	//while(!ps2kbd_send(1, &rstcmd, 1)) { }
}

void ps2kbd_isr()
{
	//Get the byte of data from the PS/2 controller
	uint8_t ps2data = inb(0x60);
	
	//If the data byte is a break-code prefix, or an extended keycode prefix, process that.
	if(ps2data == 0xF0)
	{
		//This is a break-code prefix.
		//The following bytes will specify which key is breaking, rather than making.
		ps2kbd_break = true;
		return;
	}
	
	if(ps2data == 0xE0)
	{
		//This is an extended keycode prefix.
		//The following byte will be looked up in the extended scancode table.
		ps2kbd_extend = true;
		return;
	}
	
	//Otherwise, decode the scancode and see if it's a key we care about
	int table_idx = ps2data + (ps2kbd_extend ? 256 : 0);
	if(ps2kbd_table[table_idx] != 0)
	{
		extern void kentry_isr_kbd(hal_kbd_scancode_t scancode, bool state);
		kentry_isr_kbd(ps2kbd_table[table_idx], !ps2kbd_break);
	}

	//Clear prefix flags, now that we've read their following byte.
	ps2kbd_break = false;
	ps2kbd_extend = false;
}


