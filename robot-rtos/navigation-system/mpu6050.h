extern void rgb_activate_M1();
extern void Task_SendReceive ( void * pvParameter ) ;
extern void Task_Socket ( void * pvParameter );
extern void task_mpu6050();
extern void init_mpu6050();
void wifi_init_sta( void );


typedef struct xData {
 	int sock; 
 	uint32_t x_accel; 
	uint32_t y_accel; 
	uint32_t z_accel; 	
} xSocket_t;
