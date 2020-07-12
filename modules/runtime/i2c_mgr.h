
#ifndef I2C_MGR_H
#define I2C_MGR_H

#include <stdint.h>

int i2c_init(uint32_t port, uint32_t freq, uint32_t sda, uint32_t scl);

int i2c_deinit(uint32_t port);

int i2c_write(uint32_t port, uint32_t address, uint8_t *data_out, uint32_t length_out);

int i2c_read(uint32_t port, uint32_t address, uint8_t *data_in, uint32_t length_in);

int i2c_write_read(int port, int address,
                    uint8_t *data_out, int length_out,
                    uint8_t *data_in, int length_in);

#endif
