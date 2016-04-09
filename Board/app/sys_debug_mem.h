

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


    
    extern void debug_mem_init(void);     //��ʼ��,Ӧ���ڴ���֮���ʼ��
    extern bool debug_mem_get_func_addr(struct debug_mem_data *addr);
    extern void debug_mem_add_func_addr(struct debug_mem_data *addr);
    extern void debug_mem_add_func_addr_raw(debug_func_addc func_addr,uint32_t val);
    extern void DebugGet_LR_Addr(void);
    extern void debug_mem_add_func_addr_uint32_after(void);
    extern void debug_mem_print_addr(void);  //��ӡ��Ϣ����ȡ�Ժ����ݻᱻ���
    extern uint32_t debug_mem_get_use_stack_use_size(void); //������ջ��

    extern void debug_mem_addr_log_start(void);//��ʼ��¼����Ҫ�ⲿ��������,��λ�Ժ��ǹرյġ�
    extern bool debug_mem_adder_log_get_status(void);
    
  
//���Ҫ���ٵĺ���,��һ��Ϊ��������,�ڶ���Ϊ����ֵ��Ĭ��Ϊ0   
//����Ҫ�������ļ�����ص��ļ����Ѿ���ӵ�debug.h    
    
#if USE_FUNC_LOG    
#define DEBUG_MEM_TRACK_FUNC(name,val)\
do{\
    debug_mem_add_func_addr_raw((debug_func_addc )&name,val);\
}while(0)

#else
#define DEBUG_MEM_TRACK_FUNC(name,val)  
#endif

/*

//��ӵ�ǰ����,�Զ���ȡ��������
#define DEBUG_MEM_TRACK_FUNC_CURRECT()\
do{\
   DebugGet_LR_Addr();\
   debug_mem_add_func_addr_uint32_after();\
}while(0)
*/
    
#endif

