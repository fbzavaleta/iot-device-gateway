// Criar Struct
typedef struct {
 	uint32_t adc0_chars; 
 	uint32_t adc3_chars;
} adc1_struct;

extern void anlogic_setup();
extern adc1_struct alalogic_read();