

#include "debug.h"
#include <string.h>
#include <stdarg.h>


#if USE_BLE_LOG
#include "ble_log.h"
#endif

#if DEBUG_INFO_ENABLE
static bool debug_uart_start = true;
#else
static bool debug_uart_start = false;
#endif

#if USE_BLE_LOG
static bool debug_ble_start = false;
#endif


uint32_t debug_printf(const char *format,...)
{
    va_list arg;
    va_start(arg,format);
    vprintf(format,arg); 
    va_end(arg);
    return 0;
}
    

void debug_info_uart_set(bool enable)
{
    debug_uart_start = enable;
}

uint32_t debug_info_to_uart(const char *format,...)
{
    
    if(debug_uart_start == true)
    {
        #if USE_BLE_LOG
        bool last = debug_ble_start;
        debug_ble_start = false;
        #endif
        va_list arg;
        va_start(arg,format);
        vprintf(format,arg); 
        va_end(arg);
        #if USE_BLE_LOG
        debug_ble_start = last;
        #endif
    }
    return 0;
}

uint32_t debug_info_to_ble(const char *format,...)
{
    #if USE_BLE_LOG
    char buf[50] = {0};
    if(debug_ble_start == true)
    {
        va_list arg;
        va_start(arg,format);
        vsprintf(buf,format,arg);
        ble_log_send_str(buf);
        va_end(arg);
    }
    
    #endif
    return 0;
}

void debug_ble_log_set(bool enable)
{
    #if USE_BLE_LOG
    debug_ble_start = enable;
    #endif
}



uint32_t  debug_mem_dump(uint32_t addr,uint8_t buf[],uint32_t length)
{
    memcpy(buf,(uint32_t *)addr,length);
    return 0;
}

uint32_t debug_mem_set(uint32_t addr,uint8_t buf[],uint32_t length)
{
    memcpy((uint32_t *)addr,buf,length);
    return 0;
}


extern void debug_uart_send_char(int ch);

#if defined (__CC_ARM )
// printf
int putch(int ch)
{
    
    
    debug_uart_send_char(ch);
    
//    #if USE_BLE_LOG
//    static char str_buf[50] = {0};
//    static uint8_t buf_count = 0;
//    str_buf[buf_count] = ch;
//    if(((char)ch == '\n') || (buf_count >= sizeof(str_buf)))
//    {
//        ble_log_send_str(str_buf);
//        buf_count = 0;
//        memset(str_buf,0,sizeof(str_buf));
//        return ch;
//    }
//    
//    buf_count ++;
//    
//    
//    #endif
	
	return ch;
}

int fputc(int ch, FILE *f)
{
	return (putch(ch));
}
#endif

