#include "Int_IP5305T.h"

void Int_IP5305T_ShortPress(void);

void Int_IP5305T_Wakeup(void) {
    Int_IP5305T_ShortPress();
}

void Int_IP5305T_PowerDown(void) {
    Int_IP5305T_ShortPress();
    HAL_Delay(200);
    Int_IP5305T_ShortPress();
}

void Int_IP5305T_ShortPress(void) {
    HAL_GPIO_WritePin(POWER_KEY_GPIO_Port, POWER_KEY_Pin, GPIO_PIN_RESET);
    HAL_Delay(100);
    HAL_GPIO_WritePin(POWER_KEY_GPIO_Port, POWER_KEY_Pin, GPIO_PIN_SET);
}
