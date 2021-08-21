//con.c
//Local console as terminal emulator
//Bryan E. Topp <betopp@betopp.com> 2021

#include "con.h"
#include "kspace.h"
#include "kassert.h"
#include "libcstubs.h"
#include "process.h"
#include "notify.h"

#include "hal_intr.h"

#include <errno.h>
#include <px.h>

//I don't want to actually have a driver model and I don't know how to split this up yet.
//For now just implement a PS/2 and EGA console here.


//Keyboard lock and notify for threads waiting on it
hal_spl_t con_kbd_lock;
notify_src_t con_kbd_notify;
static char con_kbd_buf_array[16];
static size_t con_kbd_buf_count;

//Keyboard modifier keys pressed
typedef enum con_kbd_mods_e
{
	CON_KBD_MOD_LSHIFT = 0,
	CON_KBD_MOD_RSHIFT = 1,
	CON_KBD_MOD_LALT = 2,
	CON_KBD_MOD_RALT = 3,
	CON_KBD_MOD_LCTRL = 4,
	CON_KBD_MOD_RCTRL = 5,
	
	CON_KBD_MOD_SHIFT = 6,
	CON_KBD_MOD_ALT = 7,
	CON_KBD_MOD_CTRL = 8,
	
	CON_KBD_MOD_MAX
} con_kbd_mods_t;
static bool con_kbd_mods[CON_KBD_MOD_MAX];

//Scancode-to-text mapping. Index is the standard HAL scancode enum.
//If an entry is \0, then no text character is generated for making that scancode.
//Each scancode has 4 translations - regular, shift, capslock, shift + capslock
const char con_us_keymap[512][4] = 
{
	//Code                          Norm  Shft  Caps  Shft+Caps
	[HAL_KBD_SCANCODE_A]           = {'a',  'A',  'A',  'a' },
	[HAL_KBD_SCANCODE_B]           = {'b',  'B',  'B',  'b' },
	[HAL_KBD_SCANCODE_C]           = {'c',  'C',  'C',  'c' },
	[HAL_KBD_SCANCODE_D]           = {'d',  'D',  'D',  'd' },
	[HAL_KBD_SCANCODE_E]           = {'e',  'E',  'E',  'e' },
	[HAL_KBD_SCANCODE_F]           = {'f',  'F',  'F',  'f' },
	[HAL_KBD_SCANCODE_G]           = {'g',  'G',  'G',  'g' },
	[HAL_KBD_SCANCODE_H]           = {'h',  'H',  'H',  'h' },
	[HAL_KBD_SCANCODE_I]           = {'i',  'I',  'I',  'i' },
	[HAL_KBD_SCANCODE_J]           = {'j',  'J',  'J',  'j' },
	[HAL_KBD_SCANCODE_K]           = {'k',  'K',  'K',  'k' },
	[HAL_KBD_SCANCODE_L]           = {'l',  'L',  'L',  'l' },
	[HAL_KBD_SCANCODE_M]           = {'m',  'M',  'M',  'm' },
	[HAL_KBD_SCANCODE_N]           = {'n',  'N',  'N',  'n' },
	[HAL_KBD_SCANCODE_O]           = {'o',  'O',  'O',  'o' },
	[HAL_KBD_SCANCODE_P]           = {'p',  'P',  'P',  'p' },
	[HAL_KBD_SCANCODE_Q]           = {'q',  'Q',  'Q',  'q' },
	[HAL_KBD_SCANCODE_R]           = {'r',  'R',  'R',  'r' },
	[HAL_KBD_SCANCODE_S]           = {'s',  'S',  'S',  's' },
	[HAL_KBD_SCANCODE_T]           = {'t',  'T',  'T',  't' },
	[HAL_KBD_SCANCODE_U]           = {'u',  'U',  'U',  'u' },
	[HAL_KBD_SCANCODE_V]           = {'v',  'V',  'V',  'v' },
	[HAL_KBD_SCANCODE_W]           = {'w',  'W',  'W',  'w' },
	[HAL_KBD_SCANCODE_X]           = {'x',  'X',  'X',  'x' },
	[HAL_KBD_SCANCODE_Y]           = {'y',  'Y',  'Y',  'y' },
	[HAL_KBD_SCANCODE_Z]           = {'z',  'Z',  'Z',  'z' },
	[HAL_KBD_SCANCODE_0]           = {'0',  ')',  '0',  ')' },
	[HAL_KBD_SCANCODE_1]           = {'1',  '!',  '1',  '!' },
	[HAL_KBD_SCANCODE_2]           = {'2',  '@',  '2',  '@' },
	[HAL_KBD_SCANCODE_3]           = {'3',  '#',  '3',  '#' },
	[HAL_KBD_SCANCODE_4]           = {'4',  '$',  '4',  '$' },
	[HAL_KBD_SCANCODE_5]           = {'5',  '%',  '5',  '%' },
	[HAL_KBD_SCANCODE_6]           = {'6',  '^',  '6',  '^' },
	[HAL_KBD_SCANCODE_7]           = {'7',  '&',  '7',  '&' },
	[HAL_KBD_SCANCODE_8]           = {'8',  '*',  '8',  '*' },
	[HAL_KBD_SCANCODE_9]           = {'9',  '(',  '9',  '(' },
	[HAL_KBD_SCANCODE_GRAVE]       = {'`',  '~',  '`',  '~' },
	[HAL_KBD_SCANCODE_MINUS]       = {'-',  '_',  '-',  '_' },
	[HAL_KBD_SCANCODE_EQUALS]      = {'=',  '+',  '=',  '+' },
	[HAL_KBD_SCANCODE_BACKSLASH]   = {'\\', '|',  '\\', '|' },
	[HAL_KBD_SCANCODE_SPACE]       = {' ',  ' ',  ' ',  ' ' },
	[HAL_KBD_SCANCODE_TAB]         = {'\t', '\t', '\t', '\t'},
	[HAL_KBD_SCANCODE_RETURN]      = {'\n', '\n', '\n', '\n'},
	[HAL_KBD_SCANCODE_LEFTBRACKET] = {'[',  '{',  '[',  '{' },
	[HAL_KBD_SCANCODE_RIGHTBRACKET]= {']',  '}',  ']',  '}' },
	
	[HAL_KBD_SCANCODE_KP_DIVIDE]   = {'/',  '/',  '/',  '/' },
	[HAL_KBD_SCANCODE_KP_MULTIPLY] = {'*',  '*',  '*',  '*' },
	[HAL_KBD_SCANCODE_KP_MINUS]    = {'-',  '-',  '-',  '-' },
	[HAL_KBD_SCANCODE_KP_PLUS]     = {'+',  '+',  '+',  '+' },
	[HAL_KBD_SCANCODE_KP_ENTER]    = {'\n', '\n', '\n', '\n'},
	[HAL_KBD_SCANCODE_KP_PERIOD]   = {'.',  '.',  '.',  '.' },
	[HAL_KBD_SCANCODE_KP_0]        = {'0',  '0',  '0',  '0' },
	[HAL_KBD_SCANCODE_KP_1]        = {'1',  '1',  '1',  '1' },
	[HAL_KBD_SCANCODE_KP_2]        = {'2',  '2',  '2',  '2' },
	[HAL_KBD_SCANCODE_KP_3]        = {'3',  '3',  '3',  '3' },
	[HAL_KBD_SCANCODE_KP_4]        = {'4',  '4',  '4',  '4' },
	[HAL_KBD_SCANCODE_KP_5]        = {'5',  '5',  '5',  '5' },
	[HAL_KBD_SCANCODE_KP_6]        = {'6',  '6',  '6',  '6' },
	[HAL_KBD_SCANCODE_KP_7]        = {'7',  '7',  '7',  '7' },
	[HAL_KBD_SCANCODE_KP_8]        = {'8',  '8',  '8',  '8' },
	[HAL_KBD_SCANCODE_KP_9]        = {'9',  '9',  '9',  '9' },
	
	[HAL_KBD_SCANCODE_BACKSPACE]   = {8,    8,    8,    8   },
	[HAL_KBD_SCANCODE_SLASH]       = {'/',  '?',  '/',  '?' },
	[HAL_KBD_SCANCODE_PERIOD]      = {'.',  '>',  '.',  '>' },
	[HAL_KBD_SCANCODE_COMMA]       = {',',  '<',  ',',  '<' },
	[HAL_KBD_SCANCODE_SEMICOLON]   = {';',  ':',  ';',  ':' },
	[HAL_KBD_SCANCODE_APOSTROPHE]  = {'\'', '"',  '\'', '"' },
};


//EGA text plane
static uint16_t *con_ega;

//Character replaced at cursor
static uint16_t con_curs_buf;

//Text output position
static int con_curs_row;
static int con_curs_col;

//Paints character
static void con_draw_char(int row, int col, int ch, int attr)
{
	if(row < 0 || row >= 25)
		return;
	if(col < 0 || col >= 80)
		return;
	
	con_ega[ (row*80) + col ] = ch | attr;
}

//Paints string
static void con_draw_str(int row, int col, const char *str, int attr)
{
	while(*str != '\0')
	{
		con_draw_char(row, col, *str, attr);
		str++;
		col++;
	}
}

//Scrolls screen
static void con_scroll(void)
{
	for(int row = 0; row < 23; row++)
	{
		memcpy(&(con_ega[row*80]), &(con_ega[(row+1)*80]), 2*80);
	}
	for(int col = 0; col < 80; col++)
	{
		con_draw_char(23, col, ' ', 0x0700);
	}
}

//Handles a newline happening on the console - from end-of-line or explicit newline character
static void con_newline(void)
{
	con_curs_col = 0;
	con_curs_row++;
	if(con_curs_row >= 24)
	{
		con_scroll();
		con_curs_row = 23;
	}
}

//Handles a single character output to the console
static void con_outp(int ch)
{
	if(ch == '\n')
	{
		con_newline();
		return;
	}
	if(ch == '\r')
	{
		con_curs_col = 0;
		return;
	}
	if(ch == '\t')
	{
		for(int ss = 0; ss < 4; ss++)
		{
			con_outp(' ');
		}
		return;
	}
	if(ch == 8)
	{
		if(con_curs_col > 0)
			con_curs_col--;
		return;
	}
	if(ch == 7)
	{
		//Bell
		return;
	}
	
	con_draw_char(con_curs_row, con_curs_col, ch, 0x0700);
	con_curs_col++;
	if(con_curs_col >= 80)
	{
		con_newline();
	}
}


void con_init(void)
{
	//Map EGA text plane
	con_ega = kspace_phys_map(0xB8000, 4096);
	KASSERT(con_ega != NULL);
	
	//Clear screen
	for(int cc = 0; cc < 25; cc++)
	{
		con_scroll();
	}
	
	//Draw status line
	for(int col = 0; col < 80; col++)
	{
		con_draw_char(24, col, ' ', 0x2000);
	}
	con_draw_str(24, 0, "Pathetix PS2/EGA", 0x2000);
	
	//Draw kernel welcome
	con_draw_str(0, 0,  "Pathetix kernel " BUILDVERSION " built " BUILDDATE " by " BUILDUSER, 0x0700);
	con_curs_row = 1;
	con_curs_col = 0;
}

void con_panic(const char *str)
{
	for(int col = 0; col < 80; col++)
	{
		con_draw_char(24, col, ' ', 0x4F00);
	}
	con_draw_str(24, 0, str, 0x4F00);
}

ssize_t con_write(int minor, const void *buf, size_t len)
{
	if(minor != 1)
		return -ENXIO;
	
	//Remove old cursor indicator
	con_ega[ (con_curs_row*80) + con_curs_col ] = con_curs_buf;
	
	//Write all bytes
	const uint8_t *bufbytes = (const uint8_t*)buf;
	for(size_t ll = 0; ll < len; ll++)
	{
		con_outp(bufbytes[ll]);
	}
	
	//Where the cursor ends up, draw cursor indicator
	con_curs_buf = con_ega[ (con_curs_row*80) + con_curs_col ];
	con_ega[ (con_curs_row*80) + con_curs_col ] &= 0xFF;
	con_ega[ (con_curs_row*80) + con_curs_col ] |= 0xA000;
	
	return len;
}

ssize_t con_read(int minor, void *buf, size_t len)
{
	if(minor != 1)
		return -ENXIO;
	
	//Disable interrupts while checking for input.
	//Todo - figure out a safe way to do this without deadlocking ourselves if we take an ISR with spinlocks held.
	bool old_ei = hal_intr_ei(false);
	
	//Check the keyboard buffer.
	//If we lock it with no input, wait until there's input.
	hal_spl_lock(&con_kbd_lock);
	while(con_kbd_buf_count == 0)
	{	
		//Nothing in input buffer.
		
		//Add ourselves to the notify for the buffer, while still holding it.
		notify_dst_t n = {0};
		notify_add(&con_kbd_notify, &n);
		
		//Release the lock and sleep. Anyone who adds to the buffer after this will notify us.
		//If we're notified any time since the last notify_wait, notify_wait will return.
		hal_spl_unlock(&con_kbd_lock);
		int wait_err = notify_wait();
		hal_spl_lock(&con_kbd_lock);
		
		notify_remove(&con_kbd_notify, &n);
		
		if(wait_err < 0)
		{
			//Probably got interrupted while waiting
			hal_spl_unlock(&con_kbd_lock);
			hal_intr_ei(old_ei);
			return wait_err;
		}
	}
		
	//Copy bytes out of input buffer
	size_t nread = 0;
	char *buf_bytes = (char*)buf;
	while( (nread < len) && (con_kbd_buf_count > 0) )
	{
		*buf_bytes = con_kbd_buf_array[0];
		nread++;
		buf_bytes++;
		
		for(size_t ii = 0; ii < (con_kbd_buf_count - 1); ii++)
		{
			con_kbd_buf_array[ii] = con_kbd_buf_array[ii + 1];
		}
		con_kbd_buf_count--;
	}
		
	//Done
	hal_spl_unlock(&con_kbd_lock);
	hal_intr_ei(old_ei);
	return nread;
}

int con_ioctl(int minor, uint64_t request, void *ptr, size_t len)
{
	if(minor != 1)
		return -ENXIO;
	
	if(request == PX_FD_IOCTL_ISATTY)
		return 1;
	
	if(request == PX_FD_IOCTL_TTYNAME)
		return process_strncpy_touser(ptr, "/dev/con", len);
	
	(void)ptr;
	(void)len;
	return -EINVAL;
}

static void con_pushinput(char ch)
{
	hal_spl_lock(&con_kbd_lock);
	if(con_kbd_buf_count < sizeof(con_kbd_buf_array))
	{
		con_kbd_buf_array[con_kbd_buf_count] = ch;
		con_kbd_buf_count++;
	}
	notify_send(&con_kbd_notify);
	hal_spl_unlock(&con_kbd_lock);
}

void con_kbd(hal_kbd_scancode_t scancode, bool state)
{
	//Check if a modifier key is being changed
	static const hal_kbd_scancode_t mod_scan[CON_KBD_MOD_MAX] = 
	{
		[CON_KBD_MOD_LSHIFT] = HAL_KBD_SCANCODE_LSHIFT,
		[CON_KBD_MOD_RSHIFT] = HAL_KBD_SCANCODE_RSHIFT,
		[CON_KBD_MOD_LCTRL] = HAL_KBD_SCANCODE_LCTRL,
		[CON_KBD_MOD_RCTRL] = HAL_KBD_SCANCODE_RCTRL,
		[CON_KBD_MOD_LALT] = HAL_KBD_SCANCODE_LALT,
		[CON_KBD_MOD_RALT] = HAL_KBD_SCANCODE_RALT
	};
	for(int mm = 0; mm < CON_KBD_MOD_MAX; mm++)
	{
		if(mod_scan[mm] != 0 && scancode == mod_scan[mm])
			con_kbd_mods[mm] = state;
	}
	
	//Recompute generic modifiers
	con_kbd_mods[CON_KBD_MOD_ALT] = con_kbd_mods[CON_KBD_MOD_LALT] || con_kbd_mods[CON_KBD_MOD_RALT];
	con_kbd_mods[CON_KBD_MOD_SHIFT] = con_kbd_mods[CON_KBD_MOD_LSHIFT] || con_kbd_mods[CON_KBD_MOD_RSHIFT];
	con_kbd_mods[CON_KBD_MOD_CTRL] = con_kbd_mods[CON_KBD_MOD_LCTRL] || con_kbd_mods[CON_KBD_MOD_RCTRL];
	
	//React to normal keypresses
	if(state)
	{
		if(con_kbd_mods[CON_KBD_MOD_CTRL])
		{
			if(scancode >= HAL_KBD_SCANCODE_A && scancode <= HAL_KBD_SCANCODE_Z)
			{
				con_pushinput(1 + scancode - HAL_KBD_SCANCODE_A);
			}
		}
		else
		{
			if(scancode == HAL_KBD_SCANCODE_UP)
			{
				con_pushinput(0x1b);
				con_pushinput(0x5b);
				con_pushinput(0x41);
			}
			else if(scancode == HAL_KBD_SCANCODE_DOWN)
			{
				con_pushinput(0x1b);
				con_pushinput(0x5b);
				con_pushinput(0x42);
			}
			else if(scancode == HAL_KBD_SCANCODE_LEFT)
			{
				con_pushinput(0x1b);
				con_pushinput(0x5b);
				con_pushinput(0x44);
			}
			else if(scancode == HAL_KBD_SCANCODE_RIGHT)
			{
				con_pushinput(0x1b);
				con_pushinput(0x5b);
				con_pushinput(0x43);
			}
			else
			{
				int keymap_set = 0;
				if(con_kbd_mods[CON_KBD_MOD_SHIFT])
					keymap_set += 1;
				
				int keyval = con_us_keymap[scancode][keymap_set];
				if(keyval != 0)
					con_pushinput(keyval);
			}
		}
	}
}
