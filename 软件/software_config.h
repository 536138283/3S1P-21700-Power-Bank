#ifndef __OS_CONFIG_H__
#define __OS_CONFIG_H__

#define BAUD_RATE           115200                                              //����ͨ�Ų�����

#define T_SYSTICK           10                                                  //Systick���ʱ�䣨��λ��ms��

#define IBUS_NOLOAD         50                                                  //BUS�����ж���������λ��mA��
#define IBAT_NOLOAD         50                                                  //BAT�����ж���������λ��mA��

#define TMAX_DOUBLECLICK    10                                                  //˫����ʱʱ�䣨��λ��10ms��
#define REFRESH_DELAY       100                                                 //ˢ��ʱ�䣨��λ��10ms��
#define SLEEP_DELAY         600                                                 //���¼�������ʱ�䣨��λ��10ms��
#define T_ULTRA_LONGPRESS   1000                                                //��������Ӧʱ�䣨��λ��10ms��

#define LED_PWM_TOP         1024                                                //LED PWM�������ֵ/����
#define LED_PWM_MIN         4                                                   //LED����ʱ����СPWMֵ����������С����
#define LED_PWM_MAX         768                                                 //LED����ʱ�����PWMֵ���������������

#endif