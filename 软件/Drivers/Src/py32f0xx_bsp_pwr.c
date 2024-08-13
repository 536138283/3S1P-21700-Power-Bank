#include "py32f0xx_bsp_pwr.h"

void BSP_PWR_Config()
{
    LL_LPM_DisableEventOnPend();//����ʹ���¼����жϲ��ܻ��Ѵ�����
    LL_PWR_SetRegulVoltageScaling(LL_PWR_REGU_VOLTAGE_SCALE2);
    LL_PWR_EnableLowPowerRunMode();//ʹ��LPRΪStopģʽ�µ�оƬ����
    LL_LPM_DisableSleepOnExit();//��ֹ�жϷ�������˳�������
    LL_LPM_EnableSleep();//WFI���CPUͣ��
}
    