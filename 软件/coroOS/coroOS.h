/*
Э��ʽ����ϵͳ
*/
#ifndef __COROOS_H__
#define __COROOS_H__

#include "pt.h"                                                                 //����protothread��ʵ��
#include "pt-sem.h"                                                             //PT����ź���
//#include//��Ӳ���ϵͳ�������ݽṹ

#include "defines.h"

//����Ϊʹ�ø�ϵͳ��һЩҪ��
//����һ��Э�̺����ķ���:static int ������(struct pt &pt)
//��һ����ʱ��������uint32_t���ͱ���:millis
extern uint32_t millis;

//�ڸ��ļ���ע�����е��߳�
//�̵߳����ֱ�������thread_name()��ʽ��nameΪ�߳���
#define OS_REGISTER_N(name) n_thread_##name
#define OS_REGISTER(name) thread_##name (tpt+n_thread_##name);

//����������ע���̵߳����֣��ǵüӶ����뷴б�ܣ�
#define OS_REGISTER_BLOCK1 \
    OS_REGISTER_N(1),\
    OS_REGISTER_N(I2C_Transmit)
//�ǵ÷�б��
#define OS_REGISTER_BLOCK2 \
    OS_REGISTER(1)\
    OS_REGISTER(I2C_Transmit)
    
//�򻯱�̶����
#define OS_REGISTER_BEGIN typedef enum{
    
#define OS_REGISTER_SEG1 \
                            n_thread_num,\
                         }ThreadIndex_t;\
                         struct pt tpt[n_thread_num];\
                         void THRD_INIT(){\
                             uint16_t p_thread;\
                             for(p_thread= 0; p_thread< n_thread_num; p_thread++) PT_INIT(tpt+p_thread);\
                         }\
                         void OS_Run(){while(1){
                             
#define OS_REGISTER_END  }}

/*�÷�����������main.c�ļ��У�λ������thread�����Ķ����������£�
OS_REGISTER_BEGIN
OS_REGISTER_BLOCK1
OS_REGISTER_SEG1
OS_REGISTER_BLOCK2
OS_REGISTER_END

int main()
{
    THRD_INIT();
    OS_Run();
    while(1);
}*/
/******************************Э��ϵͳ����************************************/
//Э������
#define THRD_DECLARE(name_args)         char name_args(struct pt *pt)
    
//Э�̿�ʼ
#define THRD_BEGIN                      static uint32_t endmillis;\
                                        PT_BEGIN(pt)

//���ÿ���Ȩ
#define THRD_YIELD                      PT_YIELD(pt)

//ϵͳ��ʱ
#define THRD_DELAY(ticks)               endmillis = ticks + millis;\
                                        PT_WAIT_UNTIL(pt,endmillis < millis)
                 
//�޳�ʱ�������ȴ�
#define THRD_WHILE(cond)                PT_WAIT_WHILE(pt,cond)
#define THRD_UNTIL(cond)                PT_WAIT_UNTIL(pt,cond)

//����ʱ����������ȴ�
#define THRD_WHILE_T(cond,ticks,func)   endmillis = ticks + millis;\
                                        if(endmillis < millis) func();\
                                        else PT_WAIT_WHILE(pt,cond)
#define THRD_UNTIL_T(cond,ticks,func)   endmillis = ticks + millis;\
                                        if(endmillis < millis) func();\
                                        else PT_WAIT_UNTIL(pt,cond)
                                            
//Э�̽���                 
#define THRD_END                        PT_END(pt)
                                        
#endif