
#ifndef _DEBUG_H_
	#define _DEBUG_H_

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdarg.h>
//#include "ydy_boards_config.h"
//#include "sys_debug_mem.h"
/*
    DEBUG_INFO(...),带文件名和行的printf
    DEBUG_PRINT(...)，直接和printf 相同
    DEBUG_ASSERT_STR(code,str) 如果判断失败打印
    DEBUG_FUN(func{}) 里面可以写函数块   
    
    DEBUG_TO_UART(...) 只向串口发送
    DEBUG_TO_BLE(...) 只向蓝牙发送
    
*/

/*
调试日志使用说明
DEBUG_GLOBAL_ENABLE  1 为打开,0 为关闭
1.如何打印指定文件的日志
  a->DEBUG_GLOBAL_ENABLE = 0,关闭全局
  b->在你要打印的文件,在#include "debug.h"的前面,自定义开关,<DEBUG_INFO_ENABLE 包含文件名和行数的打印,DEBUG_PRINT_ENABLE 和printf一样,DEBUG_ERROR_ENABLE 错误打印,DEBUG_FUN_ENABLE 函数块>
  

*/


/*
新增功能
1.是否使用蓝牙日志
2.新增定向日志(只向蓝牙打印,只向串口打印,DEBUG_INFO和DEBUG_PRINT是双向打印)
3.去掉了led的调试

*/

//全局使能
#define DEBUG_GLOBAL_ENABLE			0
//定向 串口,DEBUG_GLOBAL_ENABLE = 1 方可配置
#define DEBUG_TO_UART_ENABLE        1
//定向 蓝牙, 不受DEBUG_GLOBAL_ENABLE控制
#define DEBUG_TO_BLE_ENABLE         0

#if DEBUG_GLOBAL_ENABLE
#define DEBUG_INFO_ENABLE			1
#define DEBUG_ERROR_ENABLE			1
#define DEBUG_PRINT_ENABLE			1
#define DEBUG_FUN_ENABLE			1
#else
//单个使能
#ifndef DEBUG_INFO_ENABLE
	#define DEBUG_INFO_ENABLE			0
#endif
#ifndef DEBUG_PRINT_ENABLE
	#define DEBUG_PRINT_ENABLE			0
#endif
#ifndef DEBUG_ERROR_ENABLE
	#define DEBUG_ERROR_ENABLE			0
#endif
#ifndef DEBUG_FUN_ENABLE
	#define DEBUG_FUN_ENABLE			 0
#endif

#endif


#ifndef DEBUG_STRING
	#define DEBUG_STRING __FILE__##":" 
#endif

//导出函数
extern uint32_t debug_mem_dump(uint32_t addr,uint8_t buf[],uint32_t length);
extern uint32_t debug_mem_set(uint32_t addr,uint8_t buf[],uint32_t length);
    
extern uint32_t debug_info_to_uart(const char *format,...);
extern void debug_info_uart_set(bool enable);   //DEBUG_GLOBAL_ENABLE = 1，默认是打开的.反之是关闭的

extern uint32_t debug_info_to_ble(const char *format,...);
extern void debug_ble_log_set(bool enable); //默认关闭

/*-------------------*/
#if DEBUG_INFO_ENABLE	
#define DEBUG_INFO(msg...) \
do	{ \
		printf(DEBUG_STRING);\
		printf("line = %d:",__LINE__);\
		printf(msg);\
		printf("\r\n");\
        debug_info_to_ble(msg);\
        debug_info_to_ble("\r\n");\
}while(0)

#else	
#define DEBUG_INFO(msg...) 
#endif

/*------------------*/
#if DEBUG_ERROR_ENABLE
#define DEBUG_ERROR(msg...) \
do	{ \
		printf(DEBUG_STRING msg);\
		printf("\r\n");\
}while(0)
#endif

/*-----------------*/
#if DEBUG_PRINT_ENABLE
#define DEBUG_PRINT(msg...) \
do { \
	printf(msg);\
}while(0)

#else
	#define DEBUG_PRINT(msg...) 
#endif
/*-----------------------*/

#define DEBUG_ASSERT_STR(err,str) \
do {\
	if(err != NRF_SUCCESS)\
		DEBUG_INFO(str);\
}while(0)
/*-----------------*/	

#if DEBUG_FUN_ENABLE

#define DEBUG_FUN(fun)\
    do { \
        fun\
    }while(0)
#else
    #define DEBUG_FUN(fun) 
#endif


#define DEBUG_ASSERT(err) \
    do {\
        if(err != NRF_SUCCESS)\
            DEBUG_INFO("err = 0x%X",err);\
    }while(0)


        


    
    
#if DEBUG_TO_UART_ENABLE  
        #define DEBUG_TO_UART(msg...) \
        do {\
        debug_info_to_uart(DEBUG_STRING);\
        debug_info_to_uart("line = %d:",__LINE__);\
        debug_info_to_uart(msg);\
        debug_info_to_uart("\r\n");\
        }while(0)
#else
    #define DEBUG_TO_UART(msg...)
#endif
    
#if USE_BLE_LOG

#if DEBUG_TO_BLE_ENABLE
    #define DEBUG_TO_BLE(msg...) \
        debug_info_to_ble(msg)
#else
  #define DEBUG_TO_BLE(msg...)  
#endif // end DEBUG_TO_BLE_ENABLE

#endif // end USE_BLE_LOG


#endif		/*end */
