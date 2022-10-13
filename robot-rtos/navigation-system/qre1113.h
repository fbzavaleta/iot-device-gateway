
extern void anlogic_setup();
extern adc_channel_struct alalogic_read();

typedef struct 
{
    uint32_t voltage_ADC1C0;
    uint32_t voltage_ADC1C3;

} adc_channel_struct;