typedef struct {
 	uint32_t adc0_chars; 
 	uint32_t adc3_chars;
} struct_cba;

extern void anlogic_setup();

extern struct_cba alalogic_read();  