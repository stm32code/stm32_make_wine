#ifndef __RELAY_H
#define __RELAY_H

#include "stm32f10x.h"

#define BEEP_GPIO_PORT GPIOB
#define BEEP_GPIO_CLK RCC_APB2Periph_GPIOB
#define BEEP_GPIO_PIN GPIO_Pin_8

#define RELAY1_GPIO_PORT GPIOB
#define RELAY1_GPIO_CLK RCC_APB2Periph_GPIOB
#define RELAY1_GPIO_PIN GPIO_Pin_7

#define RELAY2_GPIO_PORT GPIOB
#define RELAY2_GPIO_CLK RCC_APB2Periph_GPIOB
#define RELAY2_GPIO_PIN GPIO_Pin_9

#define BEEP PBout(8) // 接口
#define BEEPin PBin(8) // 接口

#define relay1in PBin(7)
#define relay1out PBout(7)

#define relay2in PBin(9) // LED接口
#define relay2out PBout(9) // LED接口

void RELAY_GPIO_Config(void);

#endif /* __RELAY_H */
