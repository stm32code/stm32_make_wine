#include "git.h"

// 软件定时器设定
static Timer task1_id;
static Timer task2_id;
static Timer task3_id;

u32 CO2Data, TVOCData, sgp30_dat; // 定义CO2浓度变量与TVOC浓度变量


// 获取全局变量
const char *topics[] = {S_TOPIC_NAME};
char str[50];

// 硬件初始化
void Hardware_Init(void)
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); // 设置中断优先级分组为组2：2位抢占优先级，2位响应优先级
    HZ = GB16_NUM();                                // 字数
    delay_init();                                   // 延时函数初始化
    GENERAL_TIM_Init(TIM_4, 0, 1);
    Usart1_Init(115200); // 串口1初始化为115200

    Usart3_Init(115200); // 串口3，驱动ESP8266用
 
}
// 网络初始化
void Net_Init()
{

#if OLED // OLED文件存在
    OLED_Clear();
    // 写OLED内容
    sprintf(str, "-请打开WIFI热点");
    OLED_ShowCH(0, 0, (unsigned char *)str);
    sprintf(str, "-名称:%s         ", SSID);
    OLED_ShowCH(0, 2, (unsigned char *)str);
    sprintf(str, "-密码:%s         ", PASS);
   
#endif
}

// 任务1
void task1(void)
{
 //1秒计算器
 	Automation_Close();
	
}
// 任务2
void task2(void)
{
// 设备重连
#if NETWORK_CHAEK
    if (Connect_Net == 0) {

#if OLED // OLED文件存在
        OLED_Clear();
        // 写OLED内容
        sprintf(str, "-请打开WIFI热点");
        OLED_ShowCH(0, 0, (unsigned char *)str);
        sprintf(str, "-名称:%s         ", SSID);
        OLED_ShowCH(0, 2, (unsigned char *)str);
    
#if OLED                  // OLED文件存在
        OLED_Clear();
#endif
    }
#endif

    Read_Data(&Data_init);   // 更新传感器数据
    Update_oled_massage();   // 更新OLED
    Update_device_massage(); // 更新设备
                             // BEEP  = ~BEEP;
    State = ~State;
}
// 任务3
void task3(void)
{
    // 3s一次
    if (Connect_Net && Data_init.App == 0) {
        Data_init.App = 1;
    }
}
// 软件初始化
void SoftWare_Init(void)
{
    // 定时器初始化
    timer_init(&task1_id, task1, 1000, 1); // 1s执行一次
    timer_init(&task2_id, task2, 100, 1);    // 100ms执行一次
    timer_init(&task3_id, task3, 3000, 1);  // 3s执行一次

 
// 主函数
int main(void)
{

  
    Hardware_Init(); // 硬件初始化
    Net_Init();            // 网络初始
    TIM_Cmd(TIM4, ENABLE); // 使能计数器

    while (1) {

       
        if (dataPtr != NULL) {
            OneNet_RevPro(dataPtr); // 接收命令
        }
    }
}
