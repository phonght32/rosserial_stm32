// MIT License

// Copyright (c) 2020 phonght32

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#ifndef ROSSERIAL_CLIENT_SRC_ROS_LIB_SERIALCLASS_H_
#define ROSSERIAL_CLIENT_SRC_ROS_LIB_SERIALCLASS_H_

#include "stm32f4xx_hal.h"

extern UART_HandleTypeDef huart4;
extern DMA_HandleTypeDef hdma_uart4_rx;
extern DMA_HandleTypeDef hdma_uart4_tx;

static constexpr uint16_t BUF_SIZE = 4096;

class SerialClass
{
private:
    static constexpr uint16_t buf_mask = BUF_SIZE - 1;
    uint8_t tx_buf[BUF_SIZE];
    uint8_t rx_buf[BUF_SIZE];
    bool tx_cplt = true;
    uint16_t rx_tail = 0;
    uint16_t tx_head = 0;
    uint16_t tx_tail = 0;
    UART_HandleTypeDef &huart;

public:
    SerialClass(UART_HandleTypeDef &huart) : huart(huart)
    {
        //this->huart = huart;
    }

    inline UART_HandleTypeDef * const get_handle(void)
    {
        return &huart;
    }

    inline void init(void)
    {
        tx_cplt = true;
        rx_tail = 0;
        HAL_UART_Receive_DMA(&huart, (uint8_t *) rx_buf, BUF_SIZE);
    }

    inline int read(void)
    {
        uint16_t rx_head = (BUF_SIZE - huart.hdmarx->Instance->NDTR)
                           & buf_mask;
        if (rx_tail == rx_head)
        {
            return -1;
        }

        int c = (int) rx_buf[rx_tail++];
        rx_tail &= buf_mask;
        return c;
    }

    inline void write(const uint8_t * const c, const int length)
    {
        if (length > BUF_SIZE || length < 1)
        {
            return;
        }

        while (!tx_cplt)
        {

        }

        for (int i = 0; i < length; i++)
        {
            tx_buf[i] = c[i];
        }

        if (tx_cplt)
        {
            tx_cplt = false;
            HAL_UART_Transmit_DMA(&huart, tx_buf, length);
        }
    }

    inline void tx_cplt_callback(void)
    {
        tx_cplt = true;
    }

    inline void reset_rbuf(void) {
        HAL_UART_Receive_DMA(&huart, (uint8_t *) rx_buf, BUF_SIZE);
    }
};

SerialClass serial(huart4);

extern "C" void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == serial.get_handle()->Instance)
    {
        serial.tx_cplt_callback();
    }
}

extern "C" void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    serial.reset_rbuf();
}

#endif /* ROSSERIAL_CLIENT_SRC_ROS_LIB_SERIALCLASS_H_ */
