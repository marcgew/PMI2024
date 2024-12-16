#ifndef QMC5883L_H
#define QMC5883L_H

#include <stdint.h>



#define QMC5883L_ADDR (0x0D<<1)
#define QMC5883L_REG_CONTROL1 0x09  // Control Register 1
#define QMC5883L_REG_CONTROL2 0x0A  // Control Register 2
#define QMC5883L_REG_SET_RESET 0x0B  // Set/Reset Period Register
#define X_LSB 0x00  // X-axis LSB Register
#define X_MSB 0x01  // X-axis MSB Register
#define Y_LSB 0x02  // Y-axis LSB Register
#define Y_MSB 0x03  // Y-axis MSB Register
#define Z_LSB 0x04  // Z-axis LSB Register
#define Z_MSB 0x05  // Z-axis MSB Register



int32_t qmc5883l_init ( void );
int32_t qmc5883l_mag_get ( float *x , float *y , float * z );


#endif /*QMC5883L.H*/