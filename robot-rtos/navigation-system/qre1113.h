typedef struct {
 	uint32_t adc0_chars; 
 	uint32_t adc3_chars;
} adc_struct;

extern void anlogic_setup();
extern adc_struct alalogic_read();  