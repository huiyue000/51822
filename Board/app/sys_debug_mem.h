

#ifndef _SYS_DEBUG_MEM_H_
    #define _SYS_DEBUG_MEM_H_
 
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <absacc.h>

    typedef void (*debug_func_addc)();

    struct debug_mem_data
    {
        debug_func_addc addr;
        uint32_t val;
    };


    
    extern void debug_mem_init(void);     //初始化,应该在串口之后初始化
    extern bool debug_mem_get_func_addr(struct debug_mem_data *addr);
    extern void debug_mem_add_func_addr(struct debug_mem_data *addr);
    extern void debug_mem_add_func_addr_raw(debug_func_addc func_addr,uint32_t val);
    extern void DebugGet_LR_Addr(void);
    extern void debug_mem_add_func_addr_uint32_after(void);
    extern void debug_mem_print_addr(void);  //打印信息，读取以后数据会被清除
    extern uint32_t debug_mem_get_use_stack_use_size(void); //获得最大栈深

    extern void debug_mem_addr_log_start(void);//开始记录，需要外部条件触发,复位以后是关闭的。
    extern bool debug_mem_adder_log_get_status(void);
    
  
//添加要跟踪的函数,第一个为函数名字,第二个为附加值，默认为0   
//不需要包含本文件到相关的文件，已经添加到debug.h    
    
#if USE_FUNC_LOG    
#define DEBUG_MEM_TRACK_FUNC(name,val)\
do{\
    debug_mem_add_func_addr_raw((debug_func_addc )&name,val);\
}while(0)

#else
#define DEBUG_MEM_TRACK_FUNC(name,val)  
#endif

/*

//添加当前函数,自动获取函数名字
#define DEBUG_MEM_TRACK_FUNC_CURRECT()\
do{\
   DebugGet_LR_Addr();\
   debug_mem_add_func_addr_uint32_after();\
}while(0)
*/
    
#endif

