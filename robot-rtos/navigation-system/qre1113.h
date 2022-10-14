typedef struct {
 	uint32_t adc0_chars; 
 	uint32_t adc3_chars;
} channel_struct;

extern void anlogic_setup();
extern channel_struct alalogic_read();
