
/** Put this in the src folder **/

#include "i2c-lcd.h"
#include "esp_log.h"
#include "driver/i2c.h"
//#include "esp_driver_i2c/i2c_master.h"
#include "unistd.h"
#include "time.h"
#include "esp_rom_sys.h"  // thêm dòng này để dùng esp_rom_delay_us





#define SLAVE_ADDRESS_LCD 0x4E>>1 // change this according to ur setup
#define TAG "DEMO"

esp_err_t err;

#define I2C_NUM I2C_NUM_0






 


void lcd_send_cmd (char cmd)
{
  char data_u, data_l;
	uint8_t data_t[4];
	data_u = (cmd&0xf0);
	data_l = ((cmd<<4)&0xf0);
	data_t[0] = data_u|0x0C;  //en=1, rs=0
	data_t[1] = data_u|0x08;  //en=0, rs=0
	data_t[2] = data_l|0x0C;  //en=1, rs=0
	data_t[3] = data_l|0x08;  //en=0, rs=0
	err = i2c_master_write_to_device(I2C_NUM, SLAVE_ADDRESS_LCD, data_t, 4, 1000);
	if (err!=0) ESP_LOGI(TAG, "Error in sending command");
}

void lcd_send_data (const char data)
{
	char data_u, data_l;
	uint8_t data_t[4];
	data_u = (data&0xf0);
	data_l = ((data<<4)&0xf0);
	data_t[0] = data_u|0x0D;  //en=1, rs=0
	data_t[1] = data_u|0x09;  //en=0, rs=0
	data_t[2] = data_l|0x0D;  //en=1, rs=0
	data_t[3] = data_l|0x09;  //en=0, rs=0
	err = i2c_master_write_to_device(I2C_NUM, SLAVE_ADDRESS_LCD, data_t, 4, 1000);
	if (err!=0) ESP_LOGI(TAG, "Error in sending data");
}

void lcd_clear (void)
{
	lcd_send_cmd (0x01);
	usleep(5000);
}

void lcd_put_cur(int row, int col)
{
    switch (row)
    {
        case 0:
            col |= 0x80;
            break;
        case 1:
            col |= 0xC0;
            break;
    }

    lcd_send_cmd (col);
}


void lcd_init(void)
{
    // 4 bit initialisation
    esp_rom_delay_us(50000);  // wait for >40ms
    lcd_send_cmd(0x30);
    esp_rom_delay_us(5000);   // wait for >4.1ms
    lcd_send_cmd(0x30);
    esp_rom_delay_us(200);    // wait for >100us
    lcd_send_cmd(0x30);
    esp_rom_delay_us(10000);
    lcd_send_cmd(0x20);       // 4bit mode
    esp_rom_delay_us(10000);

    // display initialisation
    lcd_send_cmd(0x28); // Function set --> DL=0 (4 bit), N=1 (2 line), F=0 (5x8)
    esp_rom_delay_us(1000);
    lcd_send_cmd(0x08); // Display off
    esp_rom_delay_us(1000);
    lcd_send_cmd(0x01); // Clear display
    esp_rom_delay_us(2000);   // tăng delay vì lệnh clear cần >1.52ms
    lcd_send_cmd(0x06); // Entry mode: increment cursor, no shift
    esp_rom_delay_us(1000);
    lcd_send_cmd(0x0C); // Display ON, cursor OFF, blink OFF
    esp_rom_delay_us(1000);
}

void lcd_send_string (char *str)
{
	while (*str) lcd_send_data (*str++);
}
