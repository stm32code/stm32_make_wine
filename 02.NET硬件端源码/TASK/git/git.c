#include "git.h"

Data_TypeDef Data_init;						  // 设备数据结构体
Threshold_Value_TypeDef threshold_value_init; // 设备阈值设置结构体
Device_Satte_Typedef device_state_init;		  // 设备状态


// 获取数据参数
mySta Read_Data(Data_TypeDef *Device_Data)
{
// 温度测量
	Data_init.Temp_Test = DS18B20_GetTemp_MatchRom(ucDs18b20Id); // 获取温度
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
// 初始化
mySta Reset_Threshole_Value(Threshold_Value_TypeDef *Value, Device_Satte_Typedef *device_state)
{

	// 状态重置
	R_Test();
	
	return MY_SUCCESSFUL;
}
// 更新OLED显示屏中内容
mySta Update_oled_massage()
{
#if OLED // 是否打开
	char str[50];
	if(relay1in == 0){
		sprintf(str, "PH: %03d    ",Data_init.PH);
		OLED_ShowCH(0, 0, (unsigned char *)str);
		sprintf(str, "浓度: %02d  ppm", Data_init.MQ3);
		OLED_ShowCH(0, 2, (unsigned char *)str);
		sprintf(str, "温度: %.2f C   ", Data_init.temperatuer);
		OLED_ShowCH(0, 4, (unsigned char *)str);
	}

	if(LEVEL1 == 1){
		sprintf(str, "低于最低水位 !! ");
	}else{
		sprintf(str, "水位正常        ");
	}
	
	OLED_ShowCH(0, 6, (unsigned char *)str);

#endif

	return MY_SUCCESSFUL;
}

// 更新设备状态
mySta Update_device_massage()
{
	// 液位
	if(LEVEL1 == 1 || Data_init.MQ3 > threshold_value_init.MQ3_value ||Data_init.PH>threshold_value_init.PH_value ){
		BEEP =~BEEP;
		if(device_state_init.water_State==0){
			device_state_init.water_State = 5;
		}
		
	}else{
		BEEP = 0;

	}
	// 开启水泵
	if(device_state_init.water_State > 2){
		relay1out = 1;
	}else{
		relay1out = 0;
	}
	
	
	// 回传数据
	if (Data_init.App)
	{
		switch (Data_init.App)
		{
		case 1:
			OneNet_SendMqtt(1); // 发送数据到APP
			break;
		case 2:
			OneNet_SendData(); // 发送数据到平台
			break;
		}
		Data_init.App = 0;
	}

	return MY_SUCCESSFUL;
}

// 定时器
void Automation_Close(void)
{
	if(device_state_init.water_State){
		device_state_init.water_State--;
	}
	Data_init.Time++;
	// 20S 发送数据到平台
	if (Connect_Net && Data_init.Time % 15 == 0 && Data_init.App == 0) {
			Data_init.App = 2;
	}
}
// 检测按键是否按下
static U8 num_on = 0;
static U8 key_old = 0;
void Check_Key_ON_OFF()
{
	U8 key;
	key = KEY_Scan(1);
	// 与上一次的键值比较 如果不相等，表明有键值的变化，开始计时
	if (key != 0 && num_on == 0)
	{
		key_old = key;
		num_on = 1;
	}
	if (key != 0 && num_on >= 1 && num_on <= Key_Scan_Time) // 25*10ms
	{
		num_on++; // 时间记录器
	}
	if (key == 0 && num_on > 0 && num_on < Key_Scan_Time) // 短按
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
	else if (key == 0 && num_on >= Key_Scan_Time) // 长按
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
// 解析json数据
mySta massage_parse_json(char *message)
{

	cJSON *cjson_test = NULL; // 检测json格式

	const char *massage;
	// 定义数据类型
	u8 cjson_cmd; // 指令,方向

	/* 解析整段JSO数据 */
	cjson_test = cJSON_Parse(message);
	if (cjson_test == NULL)
	{
		// 解析失败
		printf("parse fail.\n");
		return MY_FAIL;
	}

	/* 依次根据名称提取JSON数据（键值对） */
	cjson_cmd = cJSON_GetObjectItem(cjson_test, "cmd")->valueint;

	switch (cjson_cmd)
	{
	case 0x01: // 消息包
		Data_init.Flage = cJSON_GetObjectItem(cjson_test, "flage")->valueint;
		if(Data_init.Flage == 0){
			Data_init.Flage =1;
		}else{
			Data_init.Flage =0;
		}
		Data_init.App = 1;
		break;
	case 0x02: // 消息包
		device_state_init.Fan_State = cJSON_GetObjectItem(cjson_test, "fan")->valueint;
		if(device_state_init.Fan_State == 0){
			device_state_init.Fan_State =1;
		}else{
			device_state_init.Fan_State =0;
		}
		Data_init.App = 1;
		break;
	case 0x03: // 数据包
		threshold_value_init.MQ3_value = cJSON_GetObjectItem(cjson_test, "mq_3_v")->valueint;
		threshold_value_init.PH_value = cJSON_GetObjectItem(cjson_test, "ph_v")->valueint;
		W_Test();
	
		Data_init.App = 1;

		break;
	case 0x04: // 数据包

		break;
	default:
		break;
	}

	/* 清空JSON对象(整条链表)的所有数据 */
	cJSON_Delete(cjson_test);

	return MY_SUCCESSFUL;
}
