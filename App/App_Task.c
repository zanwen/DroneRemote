#include "App_Task.h"
#include "App_Remote.h"
#include "FreeRTOS.h"
#include "Int_IP5305T.h"
#include "task.h"
#include "Int_Si24R1.h"

#define START_STACK_DEPTH   256
#define START_TASK_PRIORITY 1
TaskHandle_t start_task_handle;
void Start_Task(void *pvParameters);

/* 4ms周期任务 */
#define TASK_4MS_STACK_DEPTH 256
#define TASK_4MS_PRIORITY    3
TaskHandle_t task_4ms_handle;
void App_Task_4MS(void *pvParameters);

/* 10000ms周期任务 */
#define TASK_10000MS_STACK_DEPTH 128
#define TASK_10000MS_PRIORITY    2
TaskHandle_t task_10000ms_handle;
void App_Task_10000MS(void *pvParameters);

/**
 * @description: 入口函数：创建启动任务、启动调度器
 * @return {*}
 */
void FreeRTOS_Start(void) {
    /* 1. 创建启动任务 */
    xTaskCreate((TaskFunction_t)Start_Task, (char *)"Start_Task",
                (configSTACK_DEPTH_TYPE)START_STACK_DEPTH, (void *)NULL,
                (UBaseType_t)START_TASK_PRIORITY, (TaskHandle_t *)&start_task_handle);

    /* 2. 启动调度器 */
    vTaskStartScheduler();
}

/**
 * @description: 启动任务，创建其他任务
 * @param {void *} pvParameters
 * @return {*}
 */
void Start_Task(void *pvParameters) {
    taskENTER_CRITICAL();
    xTaskCreate((TaskFunction_t)App_Task_4MS, (char *)"App_Task_4MS",
                (configSTACK_DEPTH_TYPE)TASK_4MS_STACK_DEPTH, (void *)NULL,
                (UBaseType_t)TASK_4MS_PRIORITY, (TaskHandle_t *)&task_4ms_handle);

    xTaskCreate((TaskFunction_t)App_Task_10000MS, (char *)"App_Task_10000MS",
                (configSTACK_DEPTH_TYPE)TASK_10000MS_STACK_DEPTH, (void *)NULL,
                (UBaseType_t)TASK_10000MS_PRIORITY, (TaskHandle_t *)&task_10000ms_handle);
    vTaskDelete(NULL);
    taskEXIT_CRITICAL();
}

/**
 * @description: 4ms周期任务
 * @param {void *} pvParameters
 * @return {*}
 */
void App_Task_4MS(void *pvParameters) {

    TickType_t pxPreviousWakeTime;
    pxPreviousWakeTime = xTaskGetTickCount();
    RemoteControlPacket_t packet;

    while (1) {
        App_Remote_StickScan();
        App_Remote_KeyScan();

        App_Remote_BuildPacket(&rcdata, &packet);
        Int_SI24R1_TxPacket((uint8_t *)&packet);
        vTaskDelayUntil(&pxPreviousWakeTime, 4);
    }
}

void App_Task_10000MS(void *pvParameters) {
    TickType_t pxPreviousWakeTime;
    vTaskDelay(10000);
    pxPreviousWakeTime = xTaskGetTickCount();
    while (1) {
        Int_IP5305T_Wakeup();
        vTaskDelayUntil(&pxPreviousWakeTime, 10000);
    }
}
