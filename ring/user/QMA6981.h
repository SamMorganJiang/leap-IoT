#ifndef __QMA6981_H__
#define __QMA6981_H__

/*I2C ADDRESS*/
#define QMAX981_I2C_SLAVE_ADDR		0x12	// AD0 GND 0x12, AD0 VDD 0x13

#define QMAX981_ABS(X) 				((X) < 0 ? (-1 * (X)) : (X))

void qma6981_initialize(void);
unsigned char qmaX981_get_chip_id(void);
void qma6981_read_raw_xyz(void);
unsigned short qmaX981_step_c_read_stepCounter(void);

#endif /*__QMA6981_H__*/
