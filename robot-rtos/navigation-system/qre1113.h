// CREATE Struct
typedef struct {
 	uint32_t adc0_chars; 
 	uint32_t adc3_chars;
} ad_struct;
extern void analogic_setupx();

extern ad_struct analogic_readx();
