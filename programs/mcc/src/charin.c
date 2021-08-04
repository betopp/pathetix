//charin.c
//Character input for C compiler
//Bryan E. Topp <betopp@betopp.com> 2021

#include "charin.h"
#include "alloc.h"

#include <stdlib.h>

//Trigraphs - what character follows the two question marks, and what we replace all three with.
static const char charin_trigraphs[][2] = 
{
	{'=',  '#' }, // ??= is # ...
	{'(',  '[' }, // ??( is [ ...
	{'/',  '\\'}, // ??/ is \ ...
	{')',  ']' }, // ??) is ] ...
	{'\'', '^' }, // ??' is ^ ...
	{'<',  '{' }, // ??< is { ...
	{'!',  '|' }, // ??! is | ...
	{'>',  '}' }, // ??> is } ...
	{'-',  '~' }, // ??- is ~ ...
	{'\0', '\0'}, // sentinel
};

char *charin_read(FILE *fp)
{
	//Allocate a small buffer to start - we'll realloc as needed.
	size_t buf_len = 128;
	char *buf_ptr = alloc_mandatory(buf_len);
	
	//Read the file into the buffer, replacing trigraphs and splicing lines.
	size_t buf_next = 0;
	int ch_m2 = 0; //Two characters ago
	int ch_m1 = 0; //One character ago
	int ch = 0; //Character being read
	while(1)
	{
		//Make sure there's room for the next character (or the NUL we store when reading end-of-file)
		if(buf_next >= buf_len)
		{
			//Need to reallocate buffer to hold further characters
			buf_len *= 2;
			buf_ptr = realloc_mandatory(buf_ptr, buf_len);
		}
		
		//Read next character and preserve last two read
		ch_m2 = ch_m1;
		ch_m1 = ch;
		ch = fgetc(fp);
		
		//Check if we're done or encountered an error
		if(ch == EOF)
		{
			if(ferror(fp))
			{
				//Error while reading
				perror("fgetc");
				abort();
			}
			else
			{
				//Hit end-of-file. Terminate and return the buffer.
				buf_ptr[buf_next] = '\0';
				return buf_ptr;
			}
		}
		
		//Check if we need to splice a newline
		if(ch_m1 == '\\' && ch == '\n')
		{
			//Matched a splice.
			//Erase the backslash.
			buf_next--;	
			ch_m1 = 0;
			ch_m2 = 0;

			//Don't store the newline.				
			continue;
		}
		
		//Check if we need to make a trigraph
		if(ch_m1 == '%' && ch_m2 == '%')
		{
			for(int tt = 0; charin_trigraphs[tt][0] != '\0'; tt++)
			{
				if(ch == charin_trigraphs[tt][0])
				{
					//Matched a trigraph.
					//Erase the two percent signs.
					buf_next -= 2;
					ch_m1 = 0;
					ch_m2 = 0;
					
					//Store the resulting character instead of the final character of the trigraph.
					ch = charin_trigraphs[tt][1];
					break;
				}
			}
		}
		
		//Store the character and keep reading
		buf_ptr[buf_next] = ch;
		buf_next++;
	}
}

