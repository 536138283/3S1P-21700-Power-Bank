/*PY32 ����LL��ʵ�ֵ�I2C�첽������
Ŀǰ��֧��7λ�ӻ���ַ������ģʽ,��֧�������ٲ�*/
#ifndef __PY32F0XX_BSP_I2C_H__
#define __PY32F0XX_BSP_I2C_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "defines.h"
#include "coroOS.h"
/*****************************�û���������ʼ***********************************/
//I2C�Ƿ����ó�ʱ�����ʵ�ַ�ʽ
//#define I2C_WDT_HWTIMER                                                       //ʹ�ö�ʱ��
#define I2C_WDT_THREAD                                                      //ʹ���߳�
    
#define I2C_TIMEOUT_TIME 20
//I2C�ı�־λ��ⷽ����while(condition)Ϊ��������
#define THRD_I2C_WHILE(condition)     THRD_WHILE_T(condition,10,THRD_I2C_Diagnosis)
    
//I2C������ʹ�÷�ʽ�������ȴ���־λ���̹߳����жϣ�DMA
//��������ʽĬ��֧�֣�����������Ҫ�����Ӧ�ֻ꣬����ѡһ������Ҫ����ע��
//#define I2C_USE_THREAD
#define I2C_USE_IT
//#define I2C_USE_DMA//δ���

/*******************************�û�����������*********************************/
#ifndef SDA1_PIN
#define SDA1_PIN LL_GPIO_PIN_0                  
#endif
#ifndef SDA1_PORT
#define SDA1_PORT GPIOF                                                         //SDAĬ��PF0                  
#endif
#ifndef SCL1_PIN
#define SCL1_PIN LL_GPIO_PIN_1                  
#endif
#ifndef SCL1_PORT
#define SCL1_PORT GPIOF                                                         //SCLĬ��PF1                  
#endif
#ifndef I2C_ADDRESS
#define I2C_ADDRESS        0xAA                                                 //Ĭ�ϱ�����ַ����
#endif

//I2Cϵͳ״̬ö��
typedef enum
{
    I2C_IDLE = 0x00,
    
    I2C_TX_ADDR,
    I2C_TX_ACKED,    
    I2C_TX_ING,
    
    I2C_RX_POINTER_ADDR,
    I2C_RX_POINTER_ACKED,
    I2C_RX_POINTER_SENT,
    I2C_RX
    
}I2C_tranceiver_status;

//I2Cϵͳ״̬�ṹ�嶨��
struct I2C_StatusTypedef
{
    uint8_t async_dev_addr;
    uint8_t async_reg_addr;
    uint8_t *async_data;
    uint8_t *async_flag;
    uint16_t async_len;
    I2C_tranceiver_status i2c1_status;
};
/*async_flag��ָ���û������ṩ�ı�־λ������������ʾ�û��������ݶ�ȡ���/�豸����Ӧ
 *0��������
 *1���������
 *128�����豸����Ӧ
 */
#define I2C_FLAG_PROCESSING         0x00
#define I2C_FLAG_DONE               0x01
#define I2C_FLAG_NORESPONSE         0x02
#define I2C_FLAG_BUSERROR           0x04

THRD_DECLARE(thread_i2c_wdt);

void BSP_I2C_Config(void);                                                      //��������
ErrorStatus APP_I2C_TestAddress(uint8_t dev_addr);                              //����������ض���ַ���豸
    
void APP_I2C_Transmit(uint8_t devAddress, uint8_t memAddress, uint8_t *pData, uint16_t len);//����ʽ���ͳ������쳣������
void APP_I2C_Receive(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint16_t len);//����ʽ���ճ������쳣������

uint8_t ASYNC_I2C_Transmit(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint16_t len, uint8_t *flag);
uint8_t ASYNC_I2C_Receive(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint16_t len, uint8_t *flag);//
    
    
#ifdef __cplusplus
}
#endif

#endif
