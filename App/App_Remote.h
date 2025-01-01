#ifndef __APP_REMOTE_H__
#define __APP_REMOTE_H__

#include "main.h"

#define ROCKER_ADC_BUFFER_SIZE     4
#define ROCKER_SLIDING_WONDOW_SIZE 10

#define KEY_LEFT_X_PRESSED()                                                                       \
    HAL_GPIO_ReadPin(KEY_LEFT_X_GPIO_Port, KEY_LEFT_X_Pin) == GPIO_PIN_RESET

#define KEY_UP_PRESSED()    HAL_GPIO_ReadPin(KEY_UP_GPIO_Port, KEY_UP_Pin) == GPIO_PIN_RESET
#define KEY_RIGHT_PRESSED() HAL_GPIO_ReadPin(KEY_RIGHT_GPIO_Port, KEY_RIGHT_Pin) == GPIO_PIN_RESET
#define KEY_LEFT_PRESSED()  HAL_GPIO_ReadPin(KEY_LEFT_GPIO_Port, KEY_LEFT_Pin) == GPIO_PIN_RESET
#define KEY_DOWN_PRESSED()  HAL_GPIO_ReadPin(KEY_DOWN_GPIO_Port, KEY_DOWN_Pin) == GPIO_PIN_RESET

typedef struct {
    int16_t THR;
    int16_t YAW;
    int16_t ROL;
    int16_t PIT;
    int16_t AUX1;
    int16_t AUX2;
    int16_t AUX3;
    int16_t AUX4;
    int16_t AUX5;
    int16_t AUX6;
} RemoteControlData_t;

typedef struct {
    uint32_t sum;
    uint16_t buffer[ROCKER_SLIDING_WONDOW_SIZE];
} SlidingWindow_t;

typedef struct {
    int16_t THR;
    int16_t YAW;
    int16_t ROL;
    int16_t PIT;
} RemoteControlDataBias_t;

typedef struct { 
    uint16_t header;
    uint8_t function;
    uint8_t dataLen;
    RemoteControlData_t data;
    uint32_t checkSum;
} RemoteControlPacket_t;

extern uint16_t stickADCBuffer[ROCKER_ADC_BUFFER_SIZE];
extern RemoteControlData_t rcdata;

void App_Remote_StickScan(void);
void App_Remote_KeyScan(void);

void App_Remote_BuildPacket(RemoteControlData_t *pRCData, RemoteControlPacket_t *pPacket);

#endif /* __APP_REMOTE_H__ */
