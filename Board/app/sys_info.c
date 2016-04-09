
#include "stdint.h"
#include "flash.h"
#include "debug.h"
#include "nrf_ble.h"
#include "user_clock.h"
#include "sys_uicr.h"

#include "ydy_boards_config.h"
#include "sys.h"
#include <absacc.h>
#include "libc_sprintf.h"


#if USE_VBUS
#include "vbus.h"
#include "vbus_evt.h"
#endif

#if USE_FUNC_LOG
#include "sys_debug_mem.h"
#endif


#define COUNT_DATA_TYPE_BOOT        0
#define COUNT_DATA_TYPE_REBOOT      1
#define COUNT_DATA_TYPE_OTA         2
#define COUNT_DATA_TYPE_SYS_OFF     3
#define COUNT_DATA_TYPE_DISCONNECT  4
#define COUNT_DATA_TYPE_MAX         5

#define CHECK_CODE      0x11223344

struct save_time_data
{
    uint32_t check_code;
    clock_dtime_t time;
};


struct save_count_data
{
    uint32_t check_code;
    uint16_t count_data[COUNT_DATA_TYPE_MAX];
};

static volatile uint32_t log_index = 0;
static clock_dtime_t frist_sync_time;
struct save_count_data sys_info_count_data ADDR_ALIGNED_4;

#if USE_BIG_MEM
static char err_save_buf[20] __at(0x20008000 -0x20);
#else
static char err_save_buf[20] __at(0x20004000 -0x20);
#endif

static void clean_reset_index()
{
    log_index = 0;
}

static void sys_error_handle(uint32_t error_code, uint32_t line_num, const uint8_t * p_file_name)
{
    /*
    uint8_t buf[FLASH_BLOCK_SIZE] ADDR_ALIGNED_4 = {0};
    char new_str[20] = {0};
    libc_snprintf(new_str,sizeof(new_str),"%s",p_file_name + (strlen((const char *)p_file_name) - 5));
    libc_snprintf((char *)buf,sizeof(buf),",e:%d,f=%s,%d",line_num,new_str,error_code);
    flash_write(FLASH_BLOCK12_REBOOT_LOG,buf,FLASH_BLOCK_SIZE,0);
    DEBUG_INFO("err %s",buf);
    */
    char new_str[20] = {0};
    memset(err_save_buf,0,sizeof(err_save_buf));
    libc_snprintf(new_str,sizeof(new_str),"%s",p_file_name + (strlen((const char *)p_file_name) - 5));
    libc_snprintf(err_save_buf,sizeof(err_save_buf),"e:%d,f=%s,%d",line_num,new_str,error_code);
    DEBUG_INFO("err %s",err_save_buf);
}

void sys_info_err_write_str(char *str)
{
    uint8_t str_len = strlen(str);
    memset(err_save_buf,0,sizeof(err_save_buf));
    if(str_len > sizeof(err_save_buf))
        str_len = sizeof(err_save_buf);
    memcpy(err_save_buf,str,str_len);
}

//记录解绑的时间
static void sys_record_time(uint8_t flash_index) 
{
    struct save_time_data  data ADDR_ALIGNED_4 ;
    clock_dtime_t time;
    Clock_get_dtime(&time);
    data.check_code = CHECK_CODE;
    memcpy(&data.time,&time,sizeof(time));
    flash_write(flash_index,(uint8_t *)&data,sizeof(data),0);
    DEBUG_INFO("remove bind, %d-%d-%d:%d:%d:%d",time.year,time.month,time.day,time.hour,time.minute,time.second);

}


static void sys_get_time(uint8_t flash_index,clock_dtime_t *time)
{
    struct save_time_data  data ADDR_ALIGNED_4 ;
    flash_read(flash_index,(uint8_t *)&data,sizeof(data),0);
    if(data.check_code == CHECK_CODE)
    {
        memcpy(time,&data.time,sizeof(clock_dtime_t));
    }
    return ;

}

static void add_count_data(uint32_t type)
{
    if(type >= COUNT_DATA_TYPE_MAX)
    {
        return ;
    }
    
    sys_info_count_data.count_data[type] ++;
}

static uint16_t get_count_data_type(uint32_t type)
{
    if(type >= COUNT_DATA_TYPE_MAX)
    {
        return 0;
    }

    return sys_info_count_data.count_data[type];
}

static void clean_count_data()
{
    memset(&sys_info_count_data,0,sizeof(sys_info_count_data));
}

//记录启动计数
static void set_count_data()
{
    sys_info_count_data.check_code = CHECK_CODE;
    flash_write(FLASH_BLOCK11_BOOT_COUNT,(uint8_t *)&sys_info_count_data,sizeof(sys_info_count_data),0);
    
}


static uint32_t get_count_data(struct save_count_data *data)
{
    if(data == NULL)
    {
        return 0;
    }
    flash_read(FLASH_BLOCK11_BOOT_COUNT,(uint8_t *)data,sizeof(struct save_count_data),0);
    return 0;
}



static uint8_t send_get_log(bool have,char *str)
{
    uint8_t str_len;
    struct vbus_log_get send;
    memset(&send,0,sizeof(send));
    send.have_data = have;
    str_len = strlen(str);
    if(have == true)
        DEBUG_INFO("index = %d,%s",log_index,str);
    if(str_len > 17)
        str_len = 17;
        
    memcpy(send.data,str,str_len);
    vbus_tx_data(VBUS_EVT_LOG_GET_SEND,&send,sizeof(send));
    return 0;
}

static void make_get_log()
{
    char send_buf[18] = {0};
    uint8_t log_items_index = 0;
    clock_dtime_t time;
    memset(&time,0,sizeof(time));
    if(log_index == (log_items_index++)) //复位次数,断线次数
    {
        DEBUG_INFO("make_get_log,sp = %X,size = %d",GET_SP(),sys_get_stack_start_addr() - GET_SP());
        libc_snprintf(send_buf,sizeof(send_buf),"qd=%d,cq=%d",get_count_data_type(COUNT_DATA_TYPE_BOOT),get_count_data_type(COUNT_DATA_TYPE_REBOOT));
        send_get_log(true,send_buf); 
    }
    else if(log_index == (log_items_index++))
    {
        libc_snprintf(send_buf,sizeof(send_buf),"sj=%d,gj=%d,dx=%d",get_count_data_type(COUNT_DATA_TYPE_OTA),get_count_data_type(COUNT_DATA_TYPE_SYS_OFF),get_count_data_type(COUNT_DATA_TYPE_DISCONNECT));
        send_get_log(true,send_buf); 
    }
    else if(log_index == (log_items_index++))
    {
        libc_snprintf(send_buf,sizeof(send_buf),"dev id=%d",DEVICE_ID);
        send_get_log(true,send_buf);
    }
    else if(log_index == (log_items_index++))  //最近最近一次复位的信息
    {
//        flash_read(FLASH_BLOCK12_REBOOT_LOG,read_buf,FLASH_BLOCK_SIZE,0);
        send_get_log(true,(char *)err_save_buf);
    }
    else if(log_index == (log_items_index++))  //解绑时间
    {
        
        sys_get_time(FLASH_BLOCK13_REMOVE_BIND_TIME,&time);
        libc_snprintf(send_buf,sizeof(send_buf),"jb:%d-%d,%d:%d",time.month,time.day,time.hour,time.minute);
        send_get_log(true,send_buf);
         
    }
    else if(log_index == (log_items_index++)) //手动复位的时间
    {
        sys_get_time(FLASH_BLOCK14_REBOOT_TIME,&time);
        libc_snprintf(send_buf,sizeof(send_buf),"fw:%d-%d,%d:%d",time.month,time.day,time.hour,time.minute);
        send_get_log(true,send_buf);
    }
    else if(log_index == (log_items_index++)) //使用时间
    {
        //前缀消耗5个字节
        libc_snprintf(send_buf,sizeof(send_buf),"sy:m,%d",user_clock_get_sys_on_time() / (60));
        send_get_log(true,send_buf);
    }
    else if(log_index == (log_items_index++)) //bootloader 版本号和协议的版本号
    {
        libc_snprintf(send_buf,sizeof(send_buf),"b_v=%d,s_v=%d",sys_get_bootloader_version(),sys_get_band_sdk_version());
        send_get_log(true,send_buf);
    }
    else if(log_index == (log_items_index++)) //第一次同步时间
    {
        libc_snprintf(send_buf,sizeof(send_buf),"bd:%d-%d,%d:%d",frist_sync_time.month,frist_sync_time.day,frist_sync_time.hour,frist_sync_time.minute);
        send_get_log(true,send_buf);
    }
    else if(log_index == (log_items_index++)) //复位标志位
    {
        libc_snprintf(send_buf,sizeof(send_buf),"RES=%X",sys_reset_flag_get());
        send_get_log(true,send_buf);
    }
   
    else if(log_index == (log_items_index++))  //ota时间
    {
        sys_get_time(FLASH_BLOCK15_OTA_TIME,&time);
        libc_snprintf(send_buf,sizeof(send_buf),"ota:%d-%d,%d:%d",time.month,time.day,time.hour,time.minute);
        send_get_log(true,send_buf); 
                
    }
    else if(log_index == (log_items_index++)) //关机
    {
        sys_get_time(FLASH_BLOCK16_SYS_OFF_TIME,&time);
        libc_snprintf(send_buf,sizeof(send_buf),"off:%d-%d,%d:%d",time.month,time.day,time.hour,time.minute);
        send_get_log(true,send_buf); 
    }
    else if(log_index == (log_items_index++))  //编译时间
    {
        libc_snprintf(send_buf,sizeof(send_buf),"by:%s",sys_get_build_time());
        send_get_log(true,send_buf);
    }
    else if(log_index == (log_items_index++))  //sn 序列号
    {
        struct sys_uicr_sn sn;
        sys_uicr_read_sn(&sn);
        libc_snprintf(send_buf,sizeof(send_buf),"SN:%X",sn.sn);
        send_get_log(true,send_buf);        
    }
    #if USE_FUNC_LOG
    else if(log_index == (log_items_index++)) //如果添加日志，修改这里的序列号
    {
        libc_snprintf(send_buf,sizeof(send_buf),"max_st:%d",debug_mem_get_use_stack_use_size());
        send_get_log(true,send_buf);
    }
    else if(log_index == (log_items_index++))
    {
        libc_snprintf(send_buf,sizeof(send_buf),"func open = %d",debug_mem_adder_log_get_status());
        send_get_log(true,send_buf);
    }
    else if(log_index >= (log_items_index++)) //如果添加日志，修改这里的序列号
    {
        /*
        
        */
        
        if(debug_mem_adder_log_get_status() == true)
        {
            send_get_log(false,NULL);
            clean_reset_index();
            return ;
        }
        
        bool ret = true;
        struct debug_mem_data data1,data2;
        memset(&data1,0,sizeof(data1));
        memset(&data2,0,sizeof(data2));
        ret = debug_mem_get_func_addr(&data1); //只处理第一个数据
        debug_mem_get_func_addr(&data2);
        if(ret == true)
        {
            libc_snprintf(send_buf,sizeof(send_buf),"tr:%d,%X,%X",log_index % 10,data1.addr,data2.addr); 
            send_get_log(true,send_buf);
        }
        else
        {
            send_get_log(false,NULL);
            clean_reset_index();
            return ;
        }
        
        
    }
    #endif
    else 
    {
        send_get_log(false,NULL);
        clean_reset_index();
        return ;
    }
    
    log_index ++;
}

#if USE_VBUS

uint32_t sys_info_vbus_control(uint32_t cmd,void *data,uint32_t size)
{
    switch(cmd)
    {
        case VBUS_EVT_BIND_REMOVE :
            sys_record_time(FLASH_BLOCK13_REMOVE_BIND_TIME);
            
            break;
        case VBUS_EVT_OTA_START :
            sys_record_time(FLASH_BLOCK15_OTA_TIME);
            add_count_data(COUNT_DATA_TYPE_OTA);
            set_count_data();
            break;
        case VBUS_EVT_SYSTEM_OFF :
            sys_record_time(FLASH_BLOCK16_SYS_OFF_TIME);
            add_count_data(COUNT_DATA_TYPE_SYS_OFF);
            set_count_data();
            break;
        case VBUS_EVT_REBOOT :
            sys_record_time(FLASH_BLOCK14_REBOOT_TIME);
            add_count_data(COUNT_DATA_TYPE_REBOOT);
            set_count_data();
            break;
        case VBUS_EVT_LOG_GET :
            make_get_log();
            break;
        case VBUS_EVT_BLE_DISCONNECT :
            add_count_data(COUNT_DATA_TYPE_DISCONNECT);
            break;
        case VBUS_EVT_LOG_CLEAN :
            {
                uint8_t buf[FLASH_BLOCK_SIZE] = {0}; 
                memset(err_save_buf,0,sizeof(err_save_buf));
                flash_write(FLASH_BLOCK11_BOOT_COUNT,buf,sizeof(buf),0);
                flash_write(FLASH_BLOCK13_REMOVE_BIND_TIME,buf,sizeof(buf),0);
                flash_write(FLASH_BLOCK14_REBOOT_TIME,buf,sizeof(buf),0);
                clean_count_data();
            }
            break;
        case VBUS_EVT_FIRST_SYNC_TIME :
            Clock_get_dtime(&frist_sync_time);
            break;
    }
    return 0;
}

static void sys_info_vbus_init()
{
    uint32_t err;
    struct vbus_t vbus;
    uint32_t id;
    memset(&vbus,0,sizeof(vbus));
    vbus.control = sys_info_vbus_control;
    vbus.name = "sys info";
    err = vbus_reg(vbus,&id);
    APP_ERROR_CHECK(err);
}

#endif

uint32_t sys_info_init(void)
{
    get_count_data(&sys_info_count_data);
    if(sys_info_count_data.check_code != CHECK_CODE)
    {
        clean_count_data();
	    	set_count_data();
    }
    add_count_data(COUNT_DATA_TYPE_BOOT);
    ble_error_reg(sys_error_handle);
    #if USE_VBUS
    sys_info_vbus_init();
    #endif
    DEBUG_INFO("sys info,last err %s",err_save_buf);
    DEBUG_PRINT("\r\n");
    return 0;
}

