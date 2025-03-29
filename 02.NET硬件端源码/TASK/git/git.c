#include "git.h"

Data_TypeDef Data_init;						  // �豸���ݽṹ��
Threshold_Value_TypeDef threshold_value_init; // �豸��ֵ���ýṹ��
Device_Satte_Typedef device_state_init;		  // �豸״̬


// ��ȡ���ݲ���
mySta Read_Data(Data_TypeDef *Device_Data)
{
// �¶Ȳ���
	Data_init.Temp_Test = DS18B20_GetTemp_MatchRom(ucDs18b20Id); // ��ȡ�¶�
	if (Data_init.Temp_Test > 1)
	{
		Data_init.temperatuer = Data_init.Temp_Test;
	}
	Device_Data->MQ3 = Mq3_value();
	if(relay1in == 0){
		
		Device_Data->PH = PH_Trans_Concentration();
	}
	
	return MY_SUCCESSFUL;
}
// ��ʼ��
mySta Reset_Threshole_Value(Threshold_Value_TypeDef *Value, Device_Satte_Typedef *device_state)
{

	// ״̬����
	R_Test();
	
	return MY_SUCCESSFUL;
}
// ����OLED��ʾ��������
mySta Update_oled_massage()
{
#if OLED // �Ƿ��
	char str[50];
	if(relay1in == 0){
		sprintf(str, "PH: %03d    ",Data_init.PH);
		OLED_ShowCH(0, 0, (unsigned char *)str);
		sprintf(str, "Ũ��: %02d  ppm", Data_init.MQ3);
		OLED_ShowCH(0, 2, (unsigned char *)str);
		sprintf(str, "�¶�: %.2f C   ", Data_init.temperatuer);
		OLED_ShowCH(0, 4, (unsigned char *)str);
	}

	if(LEVEL1 == 1){
		sprintf(str, "�������ˮλ !! ");
	}else{
		sprintf(str, "ˮλ����        ");
	}
	
	OLED_ShowCH(0, 6, (unsigned char *)str);

#endif

	return MY_SUCCESSFUL;
}

// �����豸״̬
mySta Update_device_massage()
{
	// Һλ
	if(LEVEL1 == 1 || Data_init.MQ3 > threshold_value_init.MQ3_value ||Data_init.PH>threshold_value_init.PH_value ){
		BEEP =~BEEP;
		if(device_state_init.water_State==0){
			device_state_init.water_State = 5;
		}
		
	}else{
		BEEP = 0;

	}
	// ����ˮ��
	if(device_state_init.water_State > 2){
		relay1out = 1;
	}else{
		relay1out = 0;
	}
	
	
	// �ش�����
	if (Data_init.App)
	{
		switch (Data_init.App)
		{
		case 1:
			OneNet_SendMqtt(1); // �������ݵ�APP
			break;
		case 2:
			OneNet_SendData(); // �������ݵ�ƽ̨
			break;
		}
		Data_init.App = 0;
	}

	return MY_SUCCESSFUL;
}

// ��ʱ��
void Automation_Close(void)
{
	if(device_state_init.water_State){
		device_state_init.water_State--;
	}
	Data_init.Time++;
	// 20S �������ݵ�ƽ̨
	if (Connect_Net && Data_init.Time % 15 == 0 && Data_init.App == 0) {
			Data_init.App = 2;
	}
}
// ��ⰴ���Ƿ���
static U8 num_on = 0;
static U8 key_old = 0;
void Check_Key_ON_OFF()
{
	U8 key;
	key = KEY_Scan(1);
	// ����һ�εļ�ֵ�Ƚ� �������ȣ������м�ֵ�ı仯����ʼ��ʱ
	if (key != 0 && num_on == 0)
	{
		key_old = key;
		num_on = 1;
	}
	if (key != 0 && num_on >= 1 && num_on <= Key_Scan_Time) // 25*10ms
	{
		num_on++; // ʱ���¼��
	}
	if (key == 0 && num_on > 0 && num_on < Key_Scan_Time) // �̰�
	{
		switch (key_old)
		{
		case KEY1_PRES:
			printf("Key1_Short\n");
	
			break;
		case KEY2_PRES:
			printf("Key2_Short\n");

			break;
		case KEY3_PRES:
			printf("Key3_Short\n");

			break;

		default:
			break;
		}
		num_on = 0;
	}
	else if (key == 0 && num_on >= Key_Scan_Time) // ����
	{
		switch (key_old)
		{
		case KEY1_PRES:
			printf("Key1_Long\n");

			break;
		case KEY2_PRES:
			printf("Key2_Long\n");

			break;
		case KEY3_PRES:
			printf("Key3_Long\n");

			break;
		default:
			break;
		}
		num_on = 0;
	}
}
// ����json����
mySta massage_parse_json(char *message)
{

	cJSON *cjson_test = NULL; // ���json��ʽ

	const char *massage;
	// ������������
	u8 cjson_cmd; // ָ��,����

	/* ��������JSO���� */
	cjson_test = cJSON_Parse(message);
	if (cjson_test == NULL)
	{
		// ����ʧ��
		printf("parse fail.\n");
		return MY_FAIL;
	}

	/* ���θ���������ȡJSON���ݣ���ֵ�ԣ� */
	cjson_cmd = cJSON_GetObjectItem(cjson_test, "cmd")->valueint;

	switch (cjson_cmd)
	{
	case 0x01: // ��Ϣ��
		Data_init.Flage = cJSON_GetObjectItem(cjson_test, "flage")->valueint;
		if(Data_init.Flage == 0){
			Data_init.Flage =1;
		}else{
			Data_init.Flage =0;
		}
		Data_init.App = 1;
		break;
	case 0x02: // ��Ϣ��
		device_state_init.Fan_State = cJSON_GetObjectItem(cjson_test, "fan")->valueint;
		if(device_state_init.Fan_State == 0){
			device_state_init.Fan_State =1;
		}else{
			device_state_init.Fan_State =0;
		}
		Data_init.App = 1;
		break;
	case 0x03: // ���ݰ�
		threshold_value_init.MQ3_value = cJSON_GetObjectItem(cjson_test, "mq_3_v")->valueint;
		threshold_value_init.PH_value = cJSON_GetObjectItem(cjson_test, "ph_v")->valueint;
		W_Test();
	
		Data_init.App = 1;

		break;
	case 0x04: // ���ݰ�

		break;
	default:
		break;
	}

	/* ���JSON����(��������)���������� */
	cJSON_Delete(cjson_test);

	return MY_SUCCESSFUL;
}
