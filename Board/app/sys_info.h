
#ifndef _SYS_INFO_H_
    #define _SYS_INFO_H_
   
    #include <stdint.h>
    
    extern uint32_t sys_info_init(void);
    extern void sys_info_err_write_str(char *str);
    
#endif
