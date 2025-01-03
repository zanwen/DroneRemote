#include "Int_Si24R1.h"
#include "Com_Logger.h"

// uint8_t TX_ADDRESS[TX_ADR_WIDTH] = {0xB1, 0xB2, 0xB3, 0xB4, 0xB5}; // 定义一个静态发送地址
// uint8_t RX_ADDRESS[RX_ADR_WIDTH] = {0xB1, 0xB2, 0xB3, 0xB4, 0xB5}; // 定义一个静态发送地址
uint8_t TX_ADDRESS[TX_ADR_WIDTH] = {0x0A, 0x01, 0x07, 0x0E, 0x01}; // 定义一个静态发送地址
uint8_t RX_ADDRESS[RX_ADR_WIDTH] = {0x0A, 0x01, 0x07, 0x0E, 0x01}; // 定义一个静态发送地址

void Int_SI24R1_InitTxMode(void) {
    SI24R1_CE_LOW;
    Int_SI24R1_WriteBytes(SPI_WRITE_REG | TX_ADDR, TX_ADDRESS, TX_ADR_WIDTH); // 写入发送地址
    Int_SI24R1_WriteBytes(SPI_WRITE_REG | RX_ADDR_P0, RX_ADDRESS,
                          RX_ADR_WIDTH); // 为了应答接收设备，接收通道0地址和发送地址相同
    Int_SI24R1_WriteByte(SPI_WRITE_REG | EN_AA, 0x01);     // 使能接收通道0自动应答
    Int_SI24R1_WriteByte(SPI_WRITE_REG | EN_RXADDR, 0x01); // 使能接收通道0
    Int_SI24R1_WriteByte(SPI_WRITE_REG | SETUP_RETR,
                         0x0a); // 自动重发延时等待250us+86us，自动重发10次
    Int_SI24R1_WriteByte(SPI_WRITE_REG | RF_CH, 40);      // 选择射频通道40
    Int_SI24R1_WriteByte(SPI_WRITE_REG | RF_SETUP, 0x0f); // 数据传输率2Mbps，发射功率7dBm
    Int_SI24R1_WriteByte(SPI_WRITE_REG | CONFIG, 0x0e); // CRC使能，16位CRC校验，上电，发送模式
    Int_SI24R1_WriteByte(SPI_WRITE_REG | STATUS, 0xff);
    Int_SI24R1_WriteByte(FLUSH_TX, 0xff);

    LOG_DEBUG("Int_SI24R1_InitTxMode done")
    uint8_t buf[5]={0};
    Int_SI24R1_ReadBytes(SPI_READ_REG | TX_ADDR, buf, TX_ADR_WIDTH);
    LOG_DEBUG("TX_ADDRESS: %02X %02X %02X %02X %02X", buf[0], buf[1], buf[2], buf[3], buf[4])
}

void Int_SI24R1_InitRxMode(void) {
    SI24R1_CE_LOW;
    Int_SI24R1_WriteBytes(SPI_WRITE_REG | RX_ADDR_P0, RX_ADDRESS,
                          RX_ADR_WIDTH); // 接收设备接收通道0使用和发送设备相同的发送地址
    Int_SI24R1_WriteByte(SPI_WRITE_REG | EN_AA, 0x01);     // 使能接收通道0自动应答
    Int_SI24R1_WriteByte(SPI_WRITE_REG | EN_RXADDR, 0x01); // 使能接收通道0
    Int_SI24R1_WriteByte(SPI_WRITE_REG | RF_CH, 40);       // 选择射频通道40
    Int_SI24R1_WriteByte(SPI_WRITE_REG | RX_PW_P0,
                         TX_PLOAD_WIDTH); // 接收通道0选择和发送通道相同有效数据宽度
    Int_SI24R1_WriteByte(SPI_WRITE_REG | RF_SETUP, 0x0f); // 数据传输率2Mbps，发射功率7dBm
    Int_SI24R1_WriteByte(SPI_WRITE_REG | CONFIG, 0x0f); // CRC使能，16位CRC校验，上电，接收模式
    Int_SI24R1_WriteByte(SPI_WRITE_REG | STATUS, 0xff); // 清除所有的中断标志位
    Int_SI24R1_WriteByte(FLUSH_RX, 0xff); // 清除RX FIFO
    SI24R1_CE_HIGH;

    LOG_DEBUG("Int_SI24R1_InitRxMode done")
    uint8_t buf[5]={0};
    Int_SI24R1_ReadBytes(SPI_READ_REG | RX_ADDR_P0, buf, RX_ADR_WIDTH);
    LOG_DEBUG("RX_ADDRESS: %02X %02X %02X %02X %02X", buf[0], buf[1], buf[2], buf[3], buf[4])
}

void Int_SI24R1_WriteByte(uint8_t reg, uint8_t value) {
    SI24R1_CSN_LOW;
    Dri_SPI1_SwapByte(reg);
    Dri_SPI1_SwapByte(value);
    SI24R1_CSN_HIGH;
}

void Int_SI24R1_WriteBytes(uint8_t reg, uint8_t *buf, uint8_t size) {
    SI24R1_CSN_LOW;
    Dri_SPI1_SwapByte(reg);
    for (uint8_t i = 0; i < size; i++) {
        Dri_SPI1_SwapByte(buf[i]);
    }
    SI24R1_CSN_HIGH;
}

uint8_t Int_SI24R1_ReadByte(uint8_t reg) {
    uint8_t value;
    SI24R1_CSN_LOW;
    Dri_SPI1_SwapByte(reg);
    value = Dri_SPI1_SwapByte(0);
    SI24R1_CSN_HIGH;
    return value;
}

void Int_SI24R1_ReadBytes(uint8_t reg, uint8_t *buf, uint8_t size) {
    SI24R1_CSN_LOW;
    Dri_SPI1_SwapByte(reg);
    for (uint8_t i = 0; i < size; i++) {
        buf[i] = Dri_SPI1_SwapByte(0);
    }
    SI24R1_CSN_HIGH;
}

/********************************************************
函数功能：读取接收数据
入口参数：rxbuf:接收数据存放首地址
返回  值：0:接收到数据
          1:没有接收到数据
*********************************************************/
uint8_t Int_SI24R1_RxPacket(uint8_t *rxbuf) {
    uint8_t state;
    state = Int_SI24R1_ReadByte(SPI_READ_REG + STATUS);  // 读取状态寄存器的值
    Int_SI24R1_WriteByte(SPI_WRITE_REG + STATUS, state); // 清除RX_DR中断标志

    if (state & STATUS_RX_DR) {                                   // 接收到数据
        Int_SI24R1_ReadBytes(RD_RX_PLOAD, rxbuf, RX_PLOAD_WIDTH); // 读取数据
        Int_SI24R1_WriteByte(FLUSH_RX, 0xff);                     // 清除RX FIFO寄存器
        return 0;
    }
    return 1; // 没收到任何数据
}

/********************************************************
函数功能：发送一个数据包
入口参数：txbuf:要发送的数据
返回  值：0x10:达到最大重发次数，发送失败
          0x20:发送成功
          0xff:发送失败
*********************************************************/
uint8_t Int_SI24R1_TxPacket(uint8_t *txbuf) {
    uint8_t state;
    SI24R1_CE_LOW;                                             // CE拉低，使能SI24R1配置
    Int_SI24R1_WriteBytes(WR_TX_PLOAD, txbuf, TX_PLOAD_WIDTH); // 写数据到TX FIFO,32个字节
    SI24R1_CE_HIGH;                                            // CE置高，使能发送

    while (READ_SI24R1_IRQ == GPIO_PIN_SET)
        ;                                                // 等待发送完成
    state = Int_SI24R1_ReadByte(SPI_READ_REG + STATUS);  // 读取状态寄存器的值
    Int_SI24R1_WriteByte(SPI_WRITE_REG + STATUS, state); // 清除TX_DS或MAX_RT中断标志
    if (state & STATUS_MAX_RT) {                         // 达到最大重发次数
        Int_SI24R1_WriteByte(FLUSH_TX, 0xff);            // 清除TX FIFO寄存器
        return STATUS_MAX_RT;
    }
    if (state & STATUS_TX_DS) { // 发送完成
        return STATUS_TX_DS;
    }
    return 0XFF; // 发送失败
}

void Int_SI24R1_Check(void) {
    uint8_t buf[5] = {0};
    while (1) {
        Int_SI24R1_WriteBytes(SPI_WRITE_REG | TX_ADDR, TX_ADDRESS, TX_ADR_WIDTH);
        Int_SI24R1_ReadBytes(SPI_READ_REG | TX_ADDR, buf, TX_ADR_WIDTH);
        for (uint8_t i = 0; i < TX_ADR_WIDTH; i++) {
            if (buf[i] != TX_ADDRESS[i]) {
                LOG_DUMP("Int_SI24R1_Test Failed, TX_ADDRESS not match", buf, TX_ADR_WIDTH);
                break;
            } else if(i == TX_ADR_WIDTH - 1) {
                LOG_DEBUG("Int_SI24R1_Test passed")
                return;
            }
        }
    }
}
