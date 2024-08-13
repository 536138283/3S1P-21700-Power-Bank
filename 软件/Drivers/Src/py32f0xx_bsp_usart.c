#include "py32f0xx_bsp_usart.h"

void BSP_USART_Config(uint32_t baudRate)
{
    DEBUG_USART_CLK_ENABLE();

    /* USART Init */
    LL_USART_SetBaudRate(DEBUG_USART, SystemCoreClock, LL_USART_OVERSAMPLING_16, baudRate);
    LL_USART_SetDataWidth(DEBUG_USART, LL_USART_DATAWIDTH_8B);
    LL_USART_SetStopBitsLength(DEBUG_USART, LL_USART_STOPBITS_1);
    LL_USART_SetParity(DEBUG_USART, LL_USART_PARITY_NONE);
    LL_USART_SetHWFlowCtrl(DEBUG_USART, LL_USART_HWCONTROL_NONE);
    LL_USART_SetTransferDirection(DEBUG_USART, LL_USART_DIRECTION_TX_RX);
    LL_USART_Enable(DEBUG_USART);
    LL_USART_ClearFlag_TC(DEBUG_USART);

    /**USART GPIO Configuration
    PA2     ------> USART1_TX
    PA3     ------> USART1_RX
    */
    DEBUG_USART_RX_GPIO_CLK_ENABLE();
    DEBUG_USART_TX_GPIO_CLK_ENABLE();

    LL_GPIO_SetPinMode(DEBUG_USART_TX_GPIO_PORT, DEBUG_USART_TX_PIN, LL_GPIO_MODE_ALTERNATE);
    LL_GPIO_SetPinSpeed(DEBUG_USART_TX_GPIO_PORT, DEBUG_USART_TX_PIN, LL_GPIO_SPEED_FREQ_VERY_HIGH);
    LL_GPIO_SetPinPull(DEBUG_USART_TX_GPIO_PORT, DEBUG_USART_TX_PIN, LL_GPIO_PULL_UP);
    LL_GPIO_SetAFPin_0_7(DEBUG_USART_TX_GPIO_PORT, DEBUG_USART_TX_PIN, DEBUG_USART_TX_AF);

    LL_GPIO_SetPinMode(DEBUG_USART_RX_GPIO_PORT, DEBUG_USART_RX_PIN, LL_GPIO_MODE_ALTERNATE);
    LL_GPIO_SetPinSpeed(DEBUG_USART_RX_GPIO_PORT, DEBUG_USART_RX_PIN, LL_GPIO_SPEED_FREQ_VERY_HIGH);
    LL_GPIO_SetPinPull(DEBUG_USART_RX_GPIO_PORT, DEBUG_USART_RX_PIN, LL_GPIO_PULL_UP);
    LL_GPIO_SetAFPin_0_7(DEBUG_USART_RX_GPIO_PORT, DEBUG_USART_RX_PIN, DEBUG_USART_RX_AF);
    #ifdef USART_ENABLEIT
    LL_USART_EnableIT_PE(DEBUG_USART);
    LL_USART_EnableIT_ERROR(DEBUG_USART);
    LL_USART_EnableIT_RXNE(DEBUG_USART);
    NVIC_SetPriority(DEBUG_USART_IRQ, 1);
    NVIC_EnableIRQ(DEBUG_USART_IRQ);
    #endif
}

#ifdef USART_ENABLEIT
/*ͨ��ʵ��һ��˫ָ��Ļ��λ��������������ݣ��շ�����������ͬ
/rdָ�����wr�˶�����wr��ַ-1��ͣ�£��򻺳���������ʱ��wr������ֱ������ʱ����
/wrָ�������ѭ�Ȳ������ݣ�֮�������ƶ�ָ���˳��rdָ���෴
*/
/*****************************���Ͳ���*****************************************/
#if USART_TX_BUFFER_SIZE <= 256
    static uint8_t tx_wr_index = 0,tx_rd_index = USART_TX_BUFFER_SIZE-1;
#else
    static uint16_t tx_wr_index = 0,tx_rd_index = USART_TX_BUFFER_SIZE-1;
#endif
static uint8_t USART_tx_buffer[USART_TX_BUFFER_SIZE];//���������鶨��

/*
/���ڷ�������亯����Ҳ�����ڷ���������ʱ������һ�����ݵķ��ͣ���������ݽ�������
/������*UT_Data:�������ݵĵ�ַ��UT_len:�������ݵĳ���
/����ֵ��1���������0�����������
*/
#if DEBUG_USART_TX_BUFFER_SIZE <= 256
uint8_t USART_bufsend(uint8_t *UT_Data,uint8_t UT_len)
{   
    uint8_t UT_i;
#else
uint8_t USART_bufsend(uint8_t *UT_Data,uint16_t UT_len)
{   
    uint16_t UT_i;
#endif
    for(UT_i = 0;UT_i < UT_len;UT_i++)//ѭ�����
    {
        if(UT_Data[UT_i] == 0x00) break;//�����ַ�����β���˳�
        if(tx_wr_index == USART_TX_BUFFER_SIZE-1)//�Ѿ������ζ���β��
        {
            if(tx_rd_index) 
            {
                USART_tx_buffer[tx_wr_index] = UT_Data[UT_i];
                tx_wr_index = 0;
            }
            else return 1;
        }
        else
        {
            if(tx_wr_index+1 == tx_rd_index) return 1;
            else
            {   
                USART_tx_buffer[tx_wr_index] = UT_Data[UT_i];
                tx_wr_index++;
            }
        }
    }    
    if(UT_len && (LL_USART_IsEnabledIT_TXE(DEBUG_USART) == 0))//�����������Ҫ������USARTδ���ڷ���״̬
    {//����״̬��TXE�ж��Ƿ�ʹ�ܱ�ʾ
        if(tx_rd_index == USART_TX_BUFFER_SIZE-1) tx_rd_index = 0;//rdָ�����
        else tx_rd_index++;
        LL_USART_TransmitData8(DEBUG_USART,USART_tx_buffer[tx_rd_index]);
        LL_USART_EnableIT_TXE(DEBUG_USART);
    }   
    return 0;
}

/*
/���ڷ�������亯�������ֽڣ���Ҳ�����ڷ���������ʱ������һ�����ݵķ��ͣ���������ݽ�������
/������uint8_t UT_Data��Ҫ���͵�һ�ֽ�����
/����ֵ��1���������0�����������
*/
uint8_t USART_send_a_byte(uint8_t UT_Data)
{    
    if(tx_wr_index == USART_TX_BUFFER_SIZE-1)
    {
        if(tx_rd_index) 
        {
            USART_tx_buffer[tx_wr_index] = UT_Data;
            tx_wr_index = 0;
        }
        else return 1;
    }
    else
    {
        if(tx_wr_index+1 == tx_rd_index) return 1;
        else
        {   
            USART_tx_buffer[tx_wr_index] = UT_Data;
            tx_wr_index++;
        }
    }
    if(LL_USART_IsEnabledIT_TXE(DEBUG_USART) == 0)//�����������Ҫ������USARTδ���ڷ���״̬
    {//����״̬��TXE�ж��Ƿ�ʹ�ܱ�ʾ
        if(tx_rd_index == USART_TX_BUFFER_SIZE-1) tx_rd_index = 0;//rdָ�����
        else tx_rd_index++;
        LL_USART_TransmitData8(DEBUG_USART,USART_tx_buffer[tx_rd_index]);
        LL_USART_EnableIT_TXE(DEBUG_USART);
    }   
    return 0;
}

/*******************************���ղ���***************************************/
#if USART_RX_BUFFER_SIZE <= 256
    static uint8_t rx_wr_index = 0,rx_rd_index = USART_RX_BUFFER_SIZE-1;
#else
    static uint16_t rx_wr_index = 0,rx_rd_index = USART_RX_BUFFER_SIZE-1;
#endif
    
uint8_t rx_buf_ovf = 0;

static uint8_t USART_rx_buffer[USART_RX_BUFFER_SIZE];//���������鶨��

/*���ջ�������պ��������ػ���������Ч���ݸ���
/ͨ��wr-rd�ó�
*/
#if USART_RX_BUFFER_SIZE <= 256
uint8_t USART_getvalidnum()
#else
uint16_t USART_getvalidnum()
#endif
{
    if(rx_wr_index > rx_rd_index) return rx_wr_index-rx_rd_index-1;
    else return USART_RX_BUFFER_SIZE-1-rx_rd_index+rx_wr_index;
}

#define UART_READ_DEFAULT 0xFF
/*���ֽڴ��ڽ��ջ�������ȡ����
/��������
/����ֵ���ӻ������ж�����һ�ֽ�����
/��ע����Ӧ�ڻ�����Ϊ��ʱִ�иú��������򷵻�һ��Ĭ��ֵ
*/
uint8_t USART_read_a_byte()
{
    //uint8_t UT1_rx;
    if(rx_rd_index == USART_RX_BUFFER_SIZE-1)//��������ĩβ
    {
        if(rx_wr_index == 0) return UART_READ_DEFAULT;//û��������
        else
        {
            rx_rd_index = 0;
            return USART_rx_buffer[0];
        }
    }
    else
    {
        if(rx_wr_index == rx_rd_index+1) return UART_READ_DEFAULT;//û��������
        else
        {
            rx_rd_index++;
            return USART_rx_buffer[rx_rd_index];
        }
    }
}

/*
/���ֽڴ��ڽ��ջ�������ȡ��������������ָ������������д���ϲ㺯���ṩ��������
/��������������Խ����
/������*pdata:Ŀ������ĵ�ַ; num:��ȡ���ݵĸ���
/����ֵ���Ƿ���������ȡ�����ʱ����1
*/
#if DEBUG_USART_RX_BUFFER_SIZE <= 256
uint8_t USART_bufread(uint8_t *UT_Data,uint8_t UT_len)
{   
    uint8_t UT_i;
#else
uint8_t USART_bufread(uint8_t *UT_Data,uint16_t UT_len)
{   
    uint16_t UT_i;
#endif
    for(UT_i = 0;UT_i < UT_len;UT_len++)
    {
        if(rx_rd_index == USART_RX_BUFFER_SIZE-1)//��������ĩβ
        {
            if(rx_wr_index == 0) return 1;//û��������
            else 
            {
                rx_rd_index = 0;
                UT_Data[UT_i] = USART_rx_buffer[rx_rd_index];
            }
        }
        else
        {
            if(rx_wr_index == rx_rd_index+1) return 1;//û��������
            else
            {
                rx_rd_index++;
                UT_Data[UT_i] = USART_rx_buffer[rx_rd_index];
            }
        }
    }
        return 0;
}

void DEBUG_USART_IRQHandler()
{
    if(LL_USART_IsActiveFlag_TXE(DEBUG_USART))
    {/*���ڷ����ж�ISR�������ڷ��ͻ�����δ��ʱ������������������䵽UART_DR��ȥ
    /���ڷ��ͼĴ������ж�ʹ��λҲ��ʶ�Ż������ڵ������Ƿ�������
    /����ʱ���㴮�ڷ��ͼĴ������ж�ʹ��λ���ᵼ���ظ������жϴ�����*/
        if(tx_wr_index - tx_rd_index == 1) LL_USART_DisableIT_TXE(DEBUG_USART);
        else if((tx_wr_index == 0) && (tx_rd_index == USART_TX_BUFFER_SIZE-1)) LL_USART_DisableIT_TXE(DEBUG_USART);//���ݷ�����
        else
        {
            if(tx_rd_index == USART_TX_BUFFER_SIZE-1) tx_rd_index = 0;
            else tx_rd_index++;
            LL_USART_TransmitData8(DEBUG_USART,USART_tx_buffer[tx_rd_index]);
        }
    }
    if(LL_USART_IsActiveFlag_RXNE(DEBUG_USART))
    {//���ڽ����жϣ�����ʵʱ���յ�������䵽����������������ݱ�����
        if(rx_wr_index == USART_RX_BUFFER_SIZE-1)//д������ĩβ��
        {
            if(rx_rd_index == 0)//������������
            {
                rx_buf_ovf = 1;
            }
            else
            {
                USART_rx_buffer[rx_wr_index] = LL_USART_ReceiveData8(DEBUG_USART);
                rx_wr_index = 0;
            }
        }
        else
        {
            if(rx_rd_index - rx_wr_index == 1)//������������
            {
                rx_buf_ovf = 1;
            }
            else
            {
                USART_rx_buffer[rx_wr_index] = LL_USART_ReceiveData8(DEBUG_USART);
                rx_wr_index++;
            }
        }        
    }        
    if (LL_USART_IsActiveFlag_PE(DEBUG_USART))// Parity Error
    {
        LL_USART_ClearFlag_PE(DEBUG_USART);
    }  
    if (LL_USART_IsActiveFlag_FE(DEBUG_USART))// Framing Error
    {
        LL_USART_ClearFlag_FE(DEBUG_USART);
    }  
    if (LL_USART_IsActiveFlag_ORE(DEBUG_USART))// OverRun Error
    {
        LL_USART_ClearFlag_ORE(DEBUG_USART);
    }  
    if (LL_USART_IsActiveFlag_NE(DEBUG_USART))// Noise error
    {
        LL_USART_ClearFlag_NE(DEBUG_USART);
    }
}
#endif


const char HEX_TABLE[16] = { '0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};

void BSP_UART_TxChar(char ch)
{
    LL_USART_TransmitData8(DEBUG_USART, ch);
    while (!LL_USART_IsActiveFlag_TC(DEBUG_USART));
    LL_USART_ClearFlag_TC(DEBUG_USART);
}

void BSP_UART_TxHex8(uint8_t hex)
{
    BSP_UART_TxChar(HEX_TABLE[(hex >> 4) & 0x0F]);
    BSP_UART_TxChar(HEX_TABLE[hex & 0x0F]);
}

void BSP_UART_TxHex16(uint16_t hex)
{
    BSP_UART_TxChar(HEX_TABLE[(hex >> 12) & 0x0F]);
    BSP_UART_TxChar(HEX_TABLE[(hex >> 8) & 0x0F]);
    BSP_UART_TxChar(HEX_TABLE[(hex >> 4) & 0x0F]);
    BSP_UART_TxChar(HEX_TABLE[hex & 0xF]);
}

void BSP_UART_TxHex32(uint32_t hex)
{
    BSP_UART_TxChar(HEX_TABLE[(hex >> 28) & 0x0F]);
    BSP_UART_TxChar(HEX_TABLE[(hex >> 24) & 0x0F]);
    BSP_UART_TxChar(HEX_TABLE[(hex >> 20) & 0x0F]);
    BSP_UART_TxChar(HEX_TABLE[(hex >> 16) & 0x0F]);
    BSP_UART_TxChar(HEX_TABLE[(hex >> 12) & 0x0F]);
    BSP_UART_TxChar(HEX_TABLE[(hex >> 8) & 0x0F]);
    BSP_UART_TxChar(HEX_TABLE[(hex >> 4) & 0x0F]);
    BSP_UART_TxChar(HEX_TABLE[hex & 0xF]);
}

#define PRINTF_BUF_LENGTH 64
void uprintf(const char* format,...)
{
    char buf[PRINTF_BUF_LENGTH];
    va_list arg;
    va_start(arg,format);
    vsprintf(buf,format,arg);
    va_end(arg);
    #ifdef USART_ENABLEIT
    USART_bufsend(buf,PRINTF_BUF_LENGTH);
    #else
    BSP_UART_TxString(buf);
    #endif
}
