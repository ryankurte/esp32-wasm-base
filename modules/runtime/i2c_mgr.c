
//#include "i2c_mgr.h"

#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "esp_console.h"

#include "driver/i2c.h"

#define TAG "I2C_MGR"

#define WRITE_BIT I2C_MASTER_WRITE  /*!< I2C master write */
#define READ_BIT  I2C_MASTER_READ    /*!< I2C master read */
#define ACK_CHECK_EN 0x1            /*!< I2C master will check ack from slave*/
#define ACK_CHECK_DIS 0x0           /*!< I2C master will not check ack from slave */
#define ACK_VAL 0x0                 /*!< I2C ack value */
#define NACK_VAL 0x1                /*!< I2C nack value */


static esp_err_t i2c_get_port(uint32_t port, i2c_port_t *i2c_port) {
    switch (port) {
        case 0:
            *i2c_port = I2C_NUM_0;
            break;
         case 1:
            *i2c_port = I2C_NUM_1;
            break;
        default:
            ESP_LOGE(TAG, "Invalid port number: %d", port);
            return ESP_FAIL;
    }

    return ESP_OK;
}

// TODO: track which I2C is used / unused, return errors on attempted duplication (but allow reconfiguration?)
static bool driver_installed[2] = {false, false};

int i2c_init(uint32_t port, uint32_t freq, uint32_t sda, uint32_t scl) {
    int res = 0;
    
    i2c_config_t conf;
    
    i2c_port_t i2c_port;
    if( i2c_get_port(port, &i2c_port) != ESP_OK ) {
        return ESP_FAIL;
    }
    
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = sda;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_io_num = scl;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = freq;
    
    // Configure port
    i2c_param_config(port, &conf);

    // Install driver
    // TODO: do not attempt to reinstall driver if already installed..?
    if (driver_installed[port] == false) {
        res = i2c_driver_install(i2c_port, conf.mode, 0, 0, 0);

        driver_installed[port] = true;
    }


    return res;
}

int i2c_deinit(uint32_t port) {
    i2c_port_t i2c_port;

    if( i2c_get_port(port, &i2c_port) != ESP_OK ) {
        return ESP_FAIL;
    }

    i2c_driver_delete(i2c_port);

    return ESP_OK;
}

int i2c_write(uint32_t port, uint32_t address, uint8_t *data_out, uint32_t length_out) {

    i2c_port_t i2c_port;
    if( i2c_get_port(port, &i2c_port) != ESP_OK ) {
        return ESP_FAIL;
    }

    // Create a link handle
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();

    // Build command
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, address | I2C_MASTER_WRITE, ACK_CHECK_DIS);
    i2c_master_write(cmd, data_out, length_out, ACK_CHECK_EN);
    i2c_master_stop(cmd);
    
    // Execute command
    esp_err_t ret = i2c_master_cmd_begin(i2c_port, cmd, 1000 / portTICK_RATE_MS);

    // Remove link
    i2c_cmd_link_delete(cmd);

    return ret;
}

int i2c_read(uint32_t port, uint32_t address, uint8_t *data_in, uint32_t length_in) {
    if (length_in == 0) {
        return ESP_OK;
    }

    i2c_port_t i2c_port;
    if( i2c_get_port(port, &i2c_port) != ESP_OK ) {
        return ESP_FAIL;
    }

    // Create link handle
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();

    // Build command
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, address | I2C_MASTER_READ, ACK_CHECK_DIS);
    if (length_in > 1) {
        i2c_master_read(cmd, data_in, length_in - 1, ACK_VAL);
    }
    i2c_master_read_byte(cmd, data_in + length_in - 1, NACK_VAL);
    i2c_master_stop(cmd);

    // Execute command
    esp_err_t ret = i2c_master_cmd_begin(i2c_port, cmd, 1000 / portTICK_RATE_MS);

    // Remove link and return
    i2c_cmd_link_delete(cmd);
    return ret;
}

int i2c_write_read(uint32_t port, uint32_t address,
        uint8_t *data_out, uint32_t length_out,
        uint8_t *data_in, uint32_t length_in) {
        

    i2c_port_t i2c_port;
    if( i2c_get_port(port, &i2c_port) != ESP_OK ) {
        return ESP_FAIL;
    }

    // Create link handle
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();

    // Build command
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, address | I2C_MASTER_READ, ACK_CHECK_DIS);
    i2c_master_write(cmd, data_out, length_out, ACK_CHECK_EN);
    if (length_in > 1) {
        i2c_master_read(cmd, data_in, length_in - 1, ACK_VAL);
    }
    i2c_master_read_byte(cmd, data_in + length_in - 1, NACK_VAL);
    i2c_master_stop(cmd);

    // Execute command
    esp_err_t ret = i2c_master_cmd_begin(i2c_port, cmd, 1000 / portTICK_RATE_MS);

    // Remove link and return
    i2c_cmd_link_delete(cmd);
    return ret;
}


