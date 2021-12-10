/*********************************************************************************************
* �ļ���MP-4.c
* ���ߣ�Lixm 2017.10.17
* ˵������ȼ������������
* �޸ģ�fuyou 2018.08.08 ���Ӷ�Ӳ���汾�����ֶ���
* ע�ͣ�
*********************************************************************************************/

/*********************************************************************************************
* ͷ�ļ�
*********************************************************************************************/
#include "MP-4.h"

/*********************************************************************************************
* ���ƣ�CombustibleGas_init()
* ���ܣ���ȼ���崫������ʼ��
* ��������
* ���أ���
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void combustiblegas_init(void)
{
    ADC_InitTypeDef ADC_InitStructure;   
    GPIO_InitTypeDef GPIO_InitStructure;  
    
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB |RCC_APB2Periph_ADC1, ENABLE ); //ʹ��ADC1ͨ��ʱ��  
    
    
    RCC_ADCCLKConfig(RCC_PCLK2_Div6);                           //����ADC��Ƶ����6 72M/6=12,ADC���ʱ�䲻�ܳ���14M  
    
    //PA1 ��Ϊģ��ͨ����������  
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;  
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;               //ģ����������  
    GPIO_Init(GPIOA, &GPIO_InitStructure);    
    
    ADC_DeInit(ADC1);                                           //��λADC1,������ ADC1 ��ȫ���Ĵ�������Ϊȱʡֵ  
    
    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;          //ADC����ģʽ:ADC1��ADC2�����ڶ���ģʽ  
    ADC_InitStructure.ADC_ScanConvMode =ENABLE;                 //ģ��ת��������ɨ��ģʽ  
    ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;          //ģ��ת������������ת��ģʽ  
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None; //ת��������������ⲿ��������  
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;      //ADC�����Ҷ���  
    ADC_InitStructure.ADC_NbrOfChannel = 1;                     //˳����й���ת����ADCͨ������Ŀ  
    ADC_Init(ADC1, &ADC_InitStructure);                         //����ADC_InitStruct��ָ���Ĳ�����ʼ������ADCx�ļĴ���     
    
    
    ADC_RegularChannelConfig(ADC1, ADC_Channel_6, 3, ADC_SampleTime_239Cycles5 );    
    
    // ����ADC��DMA֧�֣�Ҫʵ��DMA���ܣ������������DMAͨ���Ȳ�����  
    ADC_DMACmd(ADC1, ENABLE);  
    
    ADC_Cmd(ADC1, ENABLE);                                      //ʹ��ָ����ADC1  
    
    ADC_ResetCalibration(ADC1);                                 //ʹ�ܸ�λУ׼    
    
    while(ADC_GetResetCalibrationStatus(ADC1));                 //�ȴ���λУ׼����  
    
    ADC_StartCalibration(ADC1);                                 //����ADУ׼  
    
    while(ADC_GetCalibrationStatus(ADC1));                      //�ȴ�У׼����  
    
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);                     //ʹ��ָ����ADC1�����ת����������  
    
}

/*********************************************************************************************
* ���ƣ�unsigned int get_infrared_status(void)
* ���ܣ���ȡ��ȼ���崫����״̬
* ��������
* ���أ���
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
unsigned int get_combustiblegas_data(void)
{ 
    unsigned int  value;
    while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC ));             //�ȴ�ת������  
    value = ADC_GetConversionValue(ADC1);  
    
    //����ָ��ADC�Ĺ�����ͨ����һ�����У�����ʱ��  
    ADC_RegularChannelConfig(ADC1, 6, 1, ADC_SampleTime_239Cycles5 ); //ADC1,ADCͨ��,����ʱ��Ϊ239.5����                      
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);                     //ʹ��ָ����ADC1�����ת����������      
    
    if(value <= GAS_THRESHOLD)
        return 0;
    else
        return 1;
}