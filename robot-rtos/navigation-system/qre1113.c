#include "driver/adc.h"
#include "esp_adc_cal.h"

// Inclui .h
#include "qre1113.h"

//A39  GPIO39 ADC1_CHANNEL_3
//A36  GPIO36 ADC1_CHANNEL_0

esp_adc_cal_characteristics_t adc0_chars;
esp_adc_cal_characteristics_t adc3_chars;

void anlogic_setup()
{
    //Atenuação 11 dB attenuation ->full-scale voltage 3.9 V (see note below)
    adc1_config_channel_atten(MY_CHANNEL_0, MY1_ATTEN_DB_11);
    adc1_config_channel_atten(MY_CHANNEL_3, MY1_ATTEN_DB_11);

    //calibração y = coeff_a * x + coeff_b
    esp_adc_cal_characterize(MY1_UNIT_1, MY1_ATTEN_DB_11, MY1_WIDTH_BIT_DEFAULT, 0, &my0_chars );
    esp_adc_cal_characterize(MY1_UNIT_1, MY1_ATTEN_DB_11, MY1_WIDTH_BIT_DEFAULT, 0, &my3_chars );

    //verifcar erros

    ESP_ERROR_CHECK(my_config_width(MY1_WIDTH_BIT_DEFAULT));
    ESP_ERROR_CHECK(my_config_channel_atten(MY_CHANNEL_0, MY1_ATTEN_DB_11));
    ESP_ERROR_CHECK(my_config_channel_atten(MY_CHANNEL_3, MY1_ATTEN_DB_11));
    
}

//checkpoint2C - Adicionar suporte para o outro channel numa struct
my_struct alalogic_read()
{
    adc1_struct adcStruct;

    adcStruct.my0_chars = esp_adc_cal_raw_to_voltage(my_get_raw(MY_CHANNEL_0), &my0_chars);
    adcStruct.my3_chars = esp_adc_cal_raw_to_voltage(my_get_raw(MY_CHANNEL_3), &my3_chars);

    return myStruct;
}
