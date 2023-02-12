#include "driver/adc.h"
#include "esp_adc_cal.h"
#include "qre1113.h"

//A39  GPIO39 ADC1_CHANNEL_3
//A36  GPIO36 ADC1_CHANNEL_0

esp_adc_cal_characteristics_t adc0_chars;
esp_adc_cal_characteristics_t adc3_chars;

void anlogic_setup()
{
    //Atenuação 11 dB attenuation ->full-scale voltage 3.9 V (see note below)
    adc1_config_channel_atten(ADC1_CHANNEL_0, ADC_ATTEN_DB_11);
    adc1_config_channel_atten(ADC1_CHANNEL_3, ADC_ATTEN_DB_11);

    //calibração y = coeff_a * x + coeff_b
    esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_DEFAULT, 0, &adc0_chars );
    esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_DEFAULT, 0, &adc3_chars );

    //verifcar erros

    ESP_ERROR_CHECK(adc1_config_width(ADC_WIDTH_BIT_DEFAULT));
    ESP_ERROR_CHECK(adc1_config_channel_atten(ADC1_CHANNEL_0, ADC_ATTEN_DB_11));
    ESP_ERROR_CHECK(adc1_config_channel_atten(ADC1_CHANNEL_3, ADC_ATTEN_DB_11));
    
}

//checkpoint2C - Adicionar suporte para o outro channel numa struct
uint32_t alalogic_read()
{
    adc1_struct adcStruct;

    adcStruct.adc0_chars = esp_adc_cal_raw_to_voltage(adc1_get_raw(ADC1_CHANNEL_0), &adc0_chars);
    adcStruct.adc3_chars = esp_adc_cal_raw_to_voltage(adc1_get_raw(ADC1_CHANNEL_3), &adc3_chars);

    return adcStruct;
}