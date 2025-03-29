#include "git.h"

// �����ʱ���趨
static Timer task1_id;
static Timer task2_id;
static Timer task3_id;

u32 CO2Data, TVOCData, sgp30_dat; // ����CO2Ũ�ȱ�����TVOCŨ�ȱ���


// ��ȡȫ�ֱ���
const char *topics[] = {S_TOPIC_NAME};
char str[50];

// Ӳ����ʼ��
void Hardware_Init(void)
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); // �����ж����ȼ�����Ϊ��2��2λ��ռ���ȼ���2λ��Ӧ���ȼ�
    HZ = GB16_NUM();                                // ����
    delay_init();                                   // ��ʱ������ʼ��
    GENERAL_TIM_Init(TIM_4, 0, 1);
    Usart1_Init(115200); // ����1��ʼ��Ϊ115200

    Usart3_Init(115200); // ����3������ESP8266��
 
}
// �����ʼ��
void Net_Init()
{

#if OLED // OLED�ļ�����
    OLED_Clear();
    // дOLED����
    sprintf(str, "-���WIFI�ȵ�");
    OLED_ShowCH(0, 0, (unsigned char *)str);
    sprintf(str, "-����:%s         ", SSID);
    OLED_ShowCH(0, 2, (unsigned char *)str);
    sprintf(str, "-����:%s         ", PASS);
   
#endif
}

// ����1
void task1(void)
{
 //1�������
 	Automation_Close();
	
}
// ����2
void task2(void)
{
// �豸����
#if NETWORK_CHAEK
    if (Connect_Net == 0) {

#if OLED // OLED�ļ�����
        OLED_Clear();
        // дOLED����
        sprintf(str, "-���WIFI�ȵ�");
        OLED_ShowCH(0, 0, (unsigned char *)str);
        sprintf(str, "-����:%s         ", SSID);
        OLED_ShowCH(0, 2, (unsigned char *)str);
    
#if OLED                  // OLED�ļ�����
        OLED_Clear();
#endif
    }
#endif

    Read_Data(&Data_init);   // ���´���������
    Update_oled_massage();   // ����OLED
    Update_device_massage(); // �����豸
                             // BEEP  = ~BEEP;
    State = ~State;
}
// ����3
void task3(void)
{
    // 3sһ��
    if (Connect_Net && Data_init.App == 0) {
        Data_init.App = 1;
    }
}
// �����ʼ��
void SoftWare_Init(void)
{
    // ��ʱ����ʼ��
    timer_init(&task1_id, task1, 1000, 1); // 1sִ��һ��
    timer_init(&task2_id, task2, 100, 1);    // 100msִ��һ��
    timer_init(&task3_id, task3, 3000, 1);  // 3sִ��һ��

 
// ������
int main(void)
{

  
    Hardware_Init(); // Ӳ����ʼ��
    Net_Init();            // �����ʼ
    TIM_Cmd(TIM4, ENABLE); // ʹ�ܼ�����

    while (1) {

       
        if (dataPtr != NULL) {
            OneNet_RevPro(dataPtr); // ��������
        }
    }
}
