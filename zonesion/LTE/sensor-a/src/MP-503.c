/*********************************************************************************************
* �ļ���MP-503.c
* ���ߣ�Lixm 2017.10.17
* ˵��������������������
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/

/*********************************************************************************************
* ͷ�ļ�
*********************************************************************************************/
#include "MP-503.h"

/*********************************************************************************************
* ���ƣ�airgas_init()
* ���ܣ�����������������ʼ��
* ��������
* ���أ���
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void airgas_init(void)
{
  ADC_InitTypeDef ADC_InitStructure; 
  GPIO_InitTypeDef GPIO_InitStructure;  
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA |RCC_APB2Periph_ADC1, ENABLE );//ʹ��ADC1ͨ��ʱ��   
  RCC_ADCCLKConfig(RCC_PCLK2_Div6);                             //����ADC��Ƶ����6 72M/6=12,ADC���ʱ�䲻�ܳ���14M 
  
  //PA1 ��Ϊģ��ͨ����������                         
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;		        //ģ����������
  GPIO_Init(GPIOA, &GPIO_InitStructure);	 
  ADC_DeInit(ADC1);                                             //��λADC1   
  ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;	        //ADC����ģʽ:ADC1��ADC2�����ڶ���ģʽ
  ADC_InitStructure.ADC_ScanConvMode = DISABLE;	                //ģ��ת�������ڵ�ͨ��ģʽ
  ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;	        //ģ��ת�������ڵ���ת��ģʽ
  ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;//ת��������������ⲿ��������
  ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;	//ADC�����Ҷ���
  ADC_InitStructure.ADC_NbrOfChannel = 1;	                //˳����й���ת����ADCͨ������Ŀ
  ADC_Init(ADC1, &ADC_InitStructure);	                        //����ADC_InitStruct��ָ���Ĳ�����ʼ������ADCx�ļĴ���   
    
  ADC_Cmd(ADC1, ENABLE);	                                //ʹ��ָ����ADC1  
  ADC_ResetCalibration(ADC1);	                                //ʹ�ܸ�λУ׼    
  while(ADC_GetResetCalibrationStatus(ADC1));	                //�ȴ���λУ׼����  
  ADC_StartCalibration(ADC1);	                                //����ADУ׼
  while(ADC_GetCalibrationStatus(ADC1));	                //�ȴ�У׼����
}

/*********************************************************************************************
* ���ƣ�unsigned int get_airgas_data(void)
* ���ܣ���ȡ��������������״̬
* ��������
* ���أ���
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
unsigned int get_airgas_data(void)
{ 
  //����ָ��ADC�Ĺ�����ͨ����һ�����У�����ʱ��
  ADC_RegularChannelConfig(ADC1, 6, 1, ADC_SampleTime_239Cycles5 );//ADC1,ADCͨ��,����ʱ��Ϊ239.5����	  			    
  ADC_SoftwareStartConvCmd(ADC1, ENABLE);		        //ʹ��ָ����ADC1�����ת����������	
  while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC ));               //�ȴ�ת������
  return ADC_GetConversionValue(ADC1);	                        //�������һ��ADC1�������ת�����
}