
#include <stdarg.h>
#include "libc_sprintf.h"


static unsigned int output_convert( unsigned int num, const int base ,char *str_buf,unsigned int buf_length)
{
	const char *digit = "0123456789ABCDEF";
	unsigned int buf[32];	
	int i = 0;	
	char ch;
    unsigned int str_index  = 0;
	if (base == 10) 
	{	
		do
		{
			buf[i] = num % 10;
			num = num / 10;
			i++;
		} while ( num > 0 );
	}

	if (base == 16)
	{
		do
		{
			buf[i] = num % 16;
			num = num / 16;
			i++;
		} while ( num > 0 );
	}
	
	while ( --i >= 0 )	
	{
		ch = digit[buf[i]];
        
        if(str_index >= (buf_length - 1))
        {
            return str_index;
        }
        
        str_buf[str_index] = ch;
        str_index ++;
	}
    return str_index;
}


int libc_snprintf(char * str_buf,unsigned int str_buf_size,const char *format, ...)
{
	va_list unnamed_p;   

	char *p, *sval;
	unsigned int value_i;
    unsigned int str_index = 0;
    unsigned int tmp;
	va_start( unnamed_p, format);  

	for ( p=(char *)format; *p; p++ )
	{
        if(str_index >= (str_buf_size - 1))
        {
            goto libc_snprintf_end;
        }
        
		if ( *p != '%' )
		{
			//simple_uart_put( *p );
            str_buf[str_index ++] = *p;
			continue;
		}

		switch ( *++p )
		{
		case 'd':
			value_i = va_arg( unnamed_p, unsigned int );		
			tmp = output_convert( value_i, 10,str_buf + str_index,str_buf_size - str_index);
            str_index += tmp;
			break;

		case 's':
			for (sval = va_arg(unnamed_p, char*); *sval; sval++)
            {
                str_buf[str_index ++] = *sval;
				//simple_uart_put( *sval );
            }
			break;

		case 'X':
			value_i = va_arg( unnamed_p, unsigned int );		
			tmp = output_convert( value_i, 16 ,str_buf + str_index,str_buf_size - str_index);
            str_index += tmp;
			break;

		default:
            str_buf[str_index ++] = *p;
			//simple_uart_put( *p );
			break;
		}
	}
	va_end( unnamed_p );

libc_snprintf_end:
	str_buf[str_index] = '\0';
	return 0;
}

