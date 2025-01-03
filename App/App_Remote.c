#include "App_Remote.h"

uint16_t stickADCBuffer[ROCKER_ADC_BUFFER_SIZE];
RemoteControlData_t rcdata;
SlidingWindow_t swTHR = {0}, swYAW = {0}, swPIT = {0}, swROL = {0};
RemoteControlDataBias_t rcdataBias = {0};

static void App_Remote_SlidingWindowFilter(RemoteControlData_t *pRCData);
static void App_Remote_Calibrate(RemoteControlData_t *pRCData);
static void App_Remote_PeakLimit(RemoteControlData_t *pRCData);
static void App_Remote_MidAdjust(RemoteControlData_t *pRCData);

/**
 * @brief 摇杆扫描
 *
 */
void App_Remote_StickScan(void) {
    // LOG_DEBUG("l1: %u, l2: %u, r1: %u, r2: %u", stickADCBuffer[0], stickADCBuffer[1],
    //           stickADCBuffer[2], stickADCBuffer[3]);

    // 将ADC范围[0,4095]缩放为[0,1000]，并使用2000-adc来调整极性 => 摇杆控制区间[1000,2000]
    rcdata.THR = 2000 - (stickADCBuffer[0] / 4) - rcdataBias.THR;
    rcdata.YAW = 2000 - (stickADCBuffer[1] / 4) - rcdataBias.YAW;
    rcdata.PIT = 2000 - (stickADCBuffer[2] / 4) - rcdataBias.PIT;
    rcdata.ROL = 2000 - (stickADCBuffer[3] / 4) - rcdataBias.ROL;

    // 滑动窗口滤波，避免控制数据突变，考虑运动惯性，平滑控制
    App_Remote_SlidingWindowFilter(&rcdata);
    // 摇杆初始状态校准：油门最低点校准、其他中点校准
    App_Remote_Calibrate(&rcdata);
    // 最值限幅
    App_Remote_PeakLimit(&rcdata);
    // 摇杆回弹自适应调整
    App_Remote_MidAdjust(&rcdata);

    LOG_DEBUG("thr: %u, yaw: %u, pit: %u, rol: %u", rcdata.THR, rcdata.YAW, rcdata.PIT, rcdata.ROL)
}

void App_Remote_SlidingWindowFilter(RemoteControlData_t *pRCData) {
    static uint8_t index = 0;

    swTHR.sum -= swTHR.buffer[index];
    swTHR.sum += pRCData->THR;
    swTHR.buffer[index] = pRCData->THR;
    pRCData->THR = swTHR.sum / ROCKER_SLIDING_WONDOW_SIZE;

    swYAW.sum -= swYAW.buffer[index];
    swYAW.sum += pRCData->YAW;
    swYAW.buffer[index] = pRCData->YAW;
    pRCData->YAW = swYAW.sum / ROCKER_SLIDING_WONDOW_SIZE;

    swPIT.sum -= swPIT.buffer[index];
    swPIT.sum += pRCData->PIT;
    swPIT.buffer[index] = pRCData->PIT;
    pRCData->PIT = swPIT.sum / ROCKER_SLIDING_WONDOW_SIZE;

    swROL.sum -= swROL.buffer[index];
    swROL.sum += pRCData->ROL;
    swROL.buffer[index] = pRCData->ROL;
    pRCData->ROL = swROL.sum / ROCKER_SLIDING_WONDOW_SIZE;

    index++;
    if (index >= ROCKER_SLIDING_WONDOW_SIZE) {
        index = 0;
    }
}

void App_Remote_Calibrate(RemoteControlData_t *pRCData) {
    static uint8_t keyPressedTimes = 0;
    static int32_t sumTHR = 0, sumYAW = 0, sumPIT = 0, sumROL = 0;
    static uint16_t sumTimes = 0;
    if (!KEY_LEFT_X_PRESSED()) {
        return;
    }
    keyPressedTimes++;
    if (keyPressedTimes >= 20) {
        if (sumTimes == 0) {
            LOG_DEBUG("start calibrate...");
            sumTHR = sumYAW = sumPIT = sumROL = 0;
        }
        sumTHR += pRCData->THR;
        sumYAW += pRCData->YAW;
        sumPIT += pRCData->PIT;
        sumROL += pRCData->ROL;
        sumTimes++;
        if (sumTimes >= 50) {
            // 油门校准为最低值，其他校准为中值
            rcdataBias.THR = sumTHR / sumTimes - 1000;
            rcdataBias.YAW = sumYAW / sumTimes - 1500;
            rcdataBias.PIT = sumPIT / sumTimes - 1500;
            rcdataBias.ROL = sumROL / sumTimes - 1500;
            LOG_DEBUG("calibrate done, thr: %d, yaw: %d, pit: %d, rol: %d", rcdataBias.THR,
                      rcdataBias.YAW, rcdataBias.PIT, rcdataBias.ROL);
            keyPressedTimes = 0;
            sumTimes = 0;
        }
    }
}

void App_Remote_PeakLimit(RemoteControlData_t *pRCData) {
    if (pRCData->THR <= 1000)
        pRCData->THR = 1000;
    if (pRCData->THR >= 2000)
        pRCData->THR = 2000;

    if (pRCData->YAW <= 1000)
        pRCData->YAW = 1000;
    if (pRCData->YAW >= 2000)
        pRCData->YAW = 2000;

    if (pRCData->PIT <= 1000)
        pRCData->PIT = 1000;
    if (pRCData->PIT >= 2000)
        pRCData->PIT = 2000;

    if (pRCData->ROL <= 1000)
        pRCData->ROL = 1000;
    if (pRCData->ROL >= 2000)
        pRCData->ROL = 2000;
}

void App_Remote_MidAdjust(RemoteControlData_t *pRCData) {
    // 除油门外其他的回自动回弹，因此油门中值自适应范围给大一点
    if (pRCData->THR >= 1470 && pRCData->THR <= 1530)
        pRCData->THR = 1500;
    if (pRCData->YAW >= 1490 && pRCData->YAW <= 1510)
        pRCData->YAW = 1500;
    if (pRCData->PIT >= 1490 && pRCData->PIT <= 1510)
        pRCData->PIT = 1500;
    if (pRCData->ROL >= 1490 && pRCData->ROL <= 1510)
        pRCData->ROL = 1500;
}

void App_Remote_KeyScan(void) {
    // 通过改变静态偏差来实现按键微调
    if (KEY_UP_PRESSED()) {
        rcdataBias.PIT -= 10;
        LOG_DEBUG("KEY UP")
    }
    if (KEY_DOWN_PRESSED()) {
        rcdataBias.PIT += 10;
        LOG_DEBUG("KEY DOWN")
    }
    if (KEY_LEFT_PRESSED()) {
        rcdataBias.ROL += 10;
        LOG_DEBUG("KEY LEFT")
    }
    if (KEY_RIGHT_PRESSED()) {
        rcdataBias.ROL -= 10;
        LOG_DEBUG("KEY RIGHT")
    }
}

void App_Remote_BuildPacket(RemoteControlData_t *pRCData, RemoteControlPacket_t *pPacket) {
    pPacket->header = 0xAAAF;
    pPacket->function = 0x02;
    pPacket->dataLen = sizeof(RemoteControlData_t);
    memcpy(&pPacket->data, pRCData, sizeof(RemoteControlData_t));
    // 计算校验和
    uint8_t *p = (uint8_t *)pPacket;
    uint8_t cnt = sizeof(RemoteControlPacket_t) - sizeof(pPacket->checkSum);
    for (size_t i = 0; i < cnt; i++) {
        pPacket->checkSum += p[i];
    }
}
