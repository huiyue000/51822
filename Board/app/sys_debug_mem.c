

#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <absacc.h>
#include "sys_debug_mem.h"
#include "debug.h"
#include "nrf.h"
#include "sys.h"
#include "ydy_boards_config.h"


#ifndef USE_FUNC_LOG
#error "USE_FUNC_LOG not define "  //USE_FUNC_LOG 函数地址记录
#endif

#if USE_FUNC_LOG == 0
#error "USE_FUNC_LOG define 0,if set 0,don't include file" //如果不用这个功能,就不要包含次文件
#endif

#define CHECK_CODE                          0x12345678

//最大记录数量
#define SYS_DEBUG_MEM_SAVE_MAX_NUM          40


struct debug_mem_index
{
    uint16_t head;
    uint16_t tail;
    uint16_t count;
    uint16_t size;
    uint16_t items_count;
    uint8_t *mem_addr;
    uint32_t check_code;
};
    
//只能在32K的芯片下工作
/*
0x20002000 0x5BD0
0x20007BD0 0x400 noinit
*/
static struct debug_mem_index mem_index __at(0x20008000 - 100);
static bool debug_mem_is_start = false;
static uint8_t debug_mem_buf[sizeof(struct debug_mem_data) * (SYS_DEBUG_MEM_SAVE_MAX_NUM + 1)] __at(0x20008000 - 1000) ;
//static uint32_t tmp_addr;
static uint32_t debug_mem_max_stack_size = 0;

static bool is_full()
{
    if((mem_index.tail + 1) % mem_index.items_count == mem_index.head)
    {
        return true;
    }
    return false;
}

static bool is_empty()
{
    if(mem_index.tail == mem_index.head)
        return true;
    return false;
}

void debug_mem_add_func_addr(struct debug_mem_data *addr)
{
    if(is_full())
    {
        mem_index.count --;
        mem_index.head = (mem_index.head + 1) % mem_index.items_count;
    }
    
    memcpy(mem_index.mem_addr + mem_index.tail * mem_index.size,addr,mem_index.size);
    mem_index.tail = (mem_index.tail + 1) % mem_index.items_count;
    mem_index.count ++;
    mem_index.check_code = CHECK_CODE;
    DEBUG_PRINT("func addr = %X,val = %d\r\n",(uint32_t *)addr->addr,addr->val);    
}


uint32_t debug_mem_get_use_stack_use_size()
{
    return debug_mem_max_stack_size;
}

void debug_mem_add_func_addr_raw(debug_func_addc func_addr,uint32_t val)
{
   
    struct debug_mem_data data;
    
    uint32_t stack_use_size = sys_get_stack_cur_use_size();
    if(stack_use_size > debug_mem_max_stack_size)
    {
        debug_mem_max_stack_size = stack_use_size;
    }
    
    if(debug_mem_is_start == false)
    {
        return ;
    }
    
    data.addr = func_addr;
    data.val = val;
    debug_mem_add_func_addr(&data);
}
//
//static __ASM uint32_t __INLINE get_lr()
//{
//    MOV R0,LR
//    BX LR
//}

//void debug_mem_add_func_addr_auto()
//{
//    struct debug_mem_data data;
//    data.addr = (debug_func_addc)get_lr();
//    data.val = 0;
//    debug_mem_add_func_addr(&data);
//}

/*
void debug_mem_add_func_addr_uint32(uint32_t addr)
{
    tmp_addr = addr;
}

void debug_mem_add_func_addr_uint32_after()
{
    DEBUG_INFO("tmp addr = %X",tmp_addr - 6);
}
*/
bool debug_mem_get_func_addr(struct debug_mem_data *addr)
{
    if(is_empty() == true)
    {
        return false;
    }
    
    if(mem_index.check_code != CHECK_CODE)
    {
        return false;
    }
    
    if((mem_index.head > SYS_DEBUG_MEM_SAVE_MAX_NUM) || (mem_index.tail > SYS_DEBUG_MEM_SAVE_MAX_NUM))
    {
        return false;
    }
    
    
    memcpy(addr,mem_index.mem_addr + mem_index.head * mem_index.size,mem_index.size);
    mem_index.head = (mem_index.head + 1) % mem_index.items_count;
    mem_index.count --;
    
    return true;
}

void debug_mem_print_addr()
{
    struct debug_mem_data data;
    int i = 0;
    for(;;)
    {
        if(debug_mem_get_func_addr(&data) == true)
        {
            DEBUG_PRINT("%d = %X,\r\n",i ++,data.addr);
        }
        else
        {
            break;
        }
    }
    DEBUG_PRINT("\r\n");
}

void debug_mem_addr_log_start()
{
    mem_index.count = 0;
    mem_index.head = 0;
    mem_index.tail = 0;
    debug_mem_is_start = true;
    DEBUG_INFO("debug_mem_addr_log_start");
}


bool debug_mem_adder_log_get_status()
{
    return debug_mem_is_start;
}

void debug_mem_init()
{
    mem_index.items_count = SYS_DEBUG_MEM_SAVE_MAX_NUM;
    mem_index.size = sizeof(struct debug_mem_data);
    mem_index.mem_addr = debug_mem_buf;
    debug_mem_is_start = false;
    DEBUG_INFO("debug_mem_init items count = %d,size = %d,addr = %X",mem_index.items_count,mem_index.size,mem_index.mem_addr);
}
