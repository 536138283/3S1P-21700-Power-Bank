/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "sw6306.h"
#include "debounce_key.h"
/* Private define ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private user code ---------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/

uint8_t sendbuff;

uint8_t forceoff;//�ر������ȫ�ֱ���
uint8_t main_looping;
uint16_t cd_sleep;//˯�ߵ���ʱ

extern uint8_t inttrig,keytrig;
THRD_DECLARE(thread_app)
{
    THRD_BEGIN;
    THRD_UNTIL(SW6306_Init());
    while(1)
    {
        main_looping = 1;//ˢ��ѭ����ʼ����ʱ��������
        
        //millis����ʱ��λ����ϵͳ�������������������Ԥ�Ƶĺ����Ԥ�����ʱ�䣺497��
        if(millis > 4294960000U) NVIC_SystemReset();
        
        uprintf("\n\nTime before sleep:%d0ms",cd_sleep);        
        
        THRD_UNTIL(SW6306_ADCLoad());
        THRD_UNTIL(SW6306_PortStatusLoad());
        THRD_UNTIL(SW6306_StatusLoad());
        THRD_UNTIL(SW6306_PowerLoad());
        THRD_UNTIL(SW6306_CapacityLoad());
        
        if(SW6306_IsInitialized()==0)//���SW6306�Ƿ��ѳ�ʼ����
        {
            uprintf("\n\nReInitializing SW6306..."); 
            THRD_UNTIL(SW6306_Init());
        }
        
        //A�ڷǿ�״̬�ҳ���л�BUS������С������һʱ��������A�ڰγ��¼����Ա�֤�����������ʱ�Ŀ��
        if((SW6306_IsCharging()||(SW6306_ReadIBUS() <= IBUS_NOLOAD))&&(SW6306_IsPortA1ON()||SW6306_IsPortA2ON())) THRD_UNTIL(SW6306_ByteWrite(SW6306_CTRG_PORTEVT, 0x0A));
        //�ǳ��״̬�µ�BUS��BAT����������һ�㹻��ʱ�������Ͷ̰����¼����Ա�����ʾ����
        if((SW6306_ReadIBAT() > IBAT_NOLOAD || SW6306_ReadIBUS() > IBUS_NOLOAD) && !(SW6306_IsCharging())) THRD_UNTIL(SW6306_Click());
        //���״̬������״̬��BUS��BAT�����㹻��ʱˢ��˯�ߵ���ʱ
        if((SW6306_ReadIBAT() > IBAT_NOLOAD)||(SW6306_ReadIBUS() > IBUS_NOLOAD)||SW6306_IsPortC1ON()||SW6306_IsPortC2ON()) cd_sleep = SLEEP_DELAY;
        
        if(SW6306_IsCharging()) uprintf("\nCharging.");
        if(SW6306_IsDischarging()) uprintf("\nDischarging.");
        if(SW6306_IsFullCharged()) uprintf("\nFull Charged.");
        
        if(SW6306_IsPortC1ON()) uprintf("\nPort C1 Path Enabled.");
        if(SW6306_IsPortC2ON()) uprintf("\nPort C2 Path Enabled.");
        if(SW6306_IsPortA1ON()) uprintf("\nPort A1 Path Enabled.");
        if(SW6306_IsPortA2ON()) uprintf("\nPort A2 Path Enabled.");
        
        uprintf("\nCapacity:%d%%",SW6306_ReadCapacity());
        uprintf("\nPortBus Voltage:%dmV",SW6306_ReadVBUS());     
        uprintf("\nPortBus Current:%dmA",SW6306_ReadIBUS());
        uprintf("\nBattery Voltage:%dmV",SW6306_ReadVBAT());
        uprintf("\nBattery Current:%dmA",SW6306_ReadIBAT());
        uprintf("\nNTC Temprature:%d��C",SW6306_ReadTNTC());
        uprintf("\nChip Temprature:%.2f��C\n",SW6306_ReadTCHIP());
        
        if(forceoff)
        {
            forceoff = 0;
            THRD_UNTIL(SW6306_ForceOff());
        }
        if(inttrig==2)
        {
            THRD_UNTIL(SW6306_StatusLoad());
            inttrig = 0;
        }
        if(keytrig==2)
        {
            THRD_UNTIL(SW6306_Click());
            keytrig = 0;
        }
        
        THRD_DELAY(2);//�ȴ���ӡ���
        main_looping = 0;//ѭ����һ���䣬���Խ�Stop��
        THRD_DELAY(REFRESH_DELAY-3);
    }
    THRD_END;
}

THRD_DECLARE(thread_echo)
{
    char buf[32];
    uint8_t num;
    THRD_BEGIN;
    while(1)
    {
        THRD_UNTIL(USART_getvalidnum());
        num = USART_getvalidnum();
        USART_bufread(buf,num);
        USART_bufsend(buf,num);
    }
    THRD_END;
}

THRD_DECLARE(thread_key)
{
    static uint8_t step,cnt;//˫����ʱ�ñ���
    static uint8_t holding,ledsta,leddir,ledind;//������ס��LED�Ƿ�򿪡����ⷽ����LED����
    static uint16_t cd_reset;
    THRD_BEGIN;
    while(1)
    {
        Key_DebounceService_10ms();
        Key_Scand();
        
        if(cnt) cnt--;//��ʱ���Լ�ֱ��0
        else step = 0;
        
        switch(Key_EdgeDetect(KeyIndex_KEY))
        {
            case KeyEdge_Rising:
                if(step == 0)
                {
                    cnt = TMAX_DOUBLECLICK;//�״ΰ���
                    step = 1;
                }
                else if(step == 1)
                {
                    cnt = TMAX_DOUBLECLICK;//�ڶ��ΰ���
                    step = 2;
                }
                else step = 0;
                break;
            case KeyEdge_Falling:
                if(step == 1) cnt = TMAX_DOUBLECLICK;//�״��ɿ�
                else if(step == 2)//�ڶ����ɿ�������˫��
                {
                    cnt = 0;
                    if(ledsta)
                    {
                        uprintf("\n\nWLED Off!\n");
                        ledsta = 0;
                        LED_PWM_Set(0);
                        LL_GPIO_ResetOutputPin(LED_PORT,LED_PIN);
                    }
                    else
                    {
                        uprintf("\n\nWLED On!\n");
                        ledsta = 1;
                        LL_GPIO_SetOutputPin(LED_PORT,LED_PIN);
                        LED_PWM_Set(ledind+1);
                    }
                }
                else step = 0;
                holding = 0;//�ɿ�ʱ�������
                break;
            case KeyEdge_Holding://��������
                holding = 1;
                if(ledsta)//LED��ʱ�������⣬ÿ�δ�������ʱ�ı���ⷽ��
                {
                    if(leddir) leddir = 0;
                    else leddir = 1;
                }
                else forceoff = 1;//LED�ر�ʱ�����ر����
                break;
            case KeyEdge_Error:
            case KeyEdge_Null:
            default: break;
        }
        
        if(ledsta && (SW6306_IsBatteryDepleted()||SW6306_IsOverHeated()))//�͵�ѹ����¹ر�WLED
        {
            uprintf("\n\nBattery Depleted!Unable to Enable WLED\n\n");
            ledsta = 0;
            LED_PWM_Set(0);
            LL_GPIO_ResetOutputPin(LED_PORT,LED_PIN);
        }
        
        if(ledsta && holding)
        {
            if(leddir && (ledind < 254)) ledind++;//��������
            else if(!leddir && ledind) ledind--;//��С����
            LED_PWM_Set(ledind+1);
        }
        else if(holding)
        {
            cd_reset++;
            if(cd_reset > T_ULTRA_LONGPRESS)//�����˳�����
            {
                uprintf("\n\nResetting......\n\n");
                LL_mDelay(100);
                NVIC_SystemReset();
            }
        }
        else cd_reset = 0;
        
        if(cd_sleep) cd_sleep--;//˯�߼�ʱ���Լ�
        THRD_DELAY(1);
    }
    THRD_END;
}

THRD_DECLARE(thread_trig)
{
    THRD_BEGIN;
    while(1)
    {
        if(inttrig==1)
        {
            uprintf("\n\nIRQ event occured!\n");
            inttrig = 2;
        }
        if(keytrig==1)
        {
            //uprintf("\nKEY Pressed!\n");
            keytrig = 2;
        }
        THRD_YIELD;
    }
    THRD_END;
}

struct pt pt1,pt2,pt3,pt4;//ProtoThread�����б���

int main(void)
{
    SysInit();
    Key_Init();
    cd_sleep = SLEEP_DELAY;//ˢ��˯�ߵ���ʱ
    LL_mDelay(50);//�ȴ�SW6306�ϵ��ȶ�
    
    uprintf("\n\n3S1P 21700 Power Bank");
    uprintf("\nPowered by SW6306 & PY32F002A");
    uprintf("\nTKWTL 2024/08/10\n");
    
    PT_INIT(&pt1);
    PT_INIT(&pt2);
    PT_INIT(&pt3);
    PT_INIT(&pt4);
    
    while(1)//��ѭ��
    {
        thread_app(&pt1);//SW6306��ز���
        thread_echo(&pt2);//�������յ�������ֱ�ӷ���
        thread_key(&pt3);//������������ϵͳ״̬
        thread_trig(&pt4);//EXTI��Ӧ
        
        //�������˯�߱��������������
        //�����ɿ���˯�ߵ���ʱ���㡢I2Cû�����ڶ�д��������δ���д��ڴ�ӡ
        //��������ʱ�ر��������裺
        //��SW6306�͹���ģʽ���ر�Systick��ʱ���жϣ�����DeepSleep
        if((main_looping==0)&&(cd_sleep==0)&&(LL_GPIO_IsInputPinSet(KEY_PORT, KEY_PIN)))
        {
            while(SW6306_LPSet()==0);//while����THRD_DELAY
            LL_mDelay(1);//��ʱ�ȴ��������
            LL_SYSTICK_DisableIT();
            LL_LPM_EnableDeepSleep();
            __WFI();
            LL_SYSTICK_EnableIT();
            LL_LPM_EnableSleep();
            while(SW6306_Unlock()==0);
        }
        else __WFI();//����ǳ��˯��
    }
}


/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
void APP_ErrorHandler(void)
{
    /* Infinite loop */
    while (1)
    {
    }
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* User can add his own implementation to report the file name and line number,
     for example: printf("Wrong parameters value: file %s on line %d\r\n", file, line)  */
  /* Infinite loop */
  while (1)
  {
  }
}
#endif /* USE_FULL_ASSERT */
