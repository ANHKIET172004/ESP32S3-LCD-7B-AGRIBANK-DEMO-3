
/** Put this in the src folder **/

#include "i2c-lcd.h"
#include "esp_log.h"
#include "driver/i2c.h"
//#include "driver/i2c_master.h"
#include "unistd.h"
#include "time.h"
#include "esp_rom_sys.h"  // thêm dòng này để dùng esp_rom_delay_us
#include "mac_utils/mac_utils.h"

#define SLAVE_ADDRESS_LCD 0x4E>>1 // change this according to ur setup
#define I2C_NUM I2C_NUM_0

esp_err_t err;

char service_scroll_buffer[68];
int service_scroll_pos = 0;
bool service_scroll_enable = false;


extern char current_user_id[3];

extern void read_current_user_id();
extern char display_user_id[3];

 void lcd_lock(void) {
    if (g_mutex.lcd_mutex != NULL) {
        xSemaphoreTake(g_mutex.lcd_mutex, portMAX_DELAY);
    }
}

 void lcd_unlock(void) {
    if (g_mutex.lcd_mutex != NULL) {
        xSemaphoreGive(g_mutex.lcd_mutex);
    }
}
 
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
	if (err!=0) ESP_LOGI("LCD", "Error in sending command");
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
	if (err!=0) ESP_LOGI("LCD", "Error in sending data");
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



void lcd_show_main_screen(const char *number) {
    lcd_lock();
    
    if (start_cnt > 20) {
        start_cnt = 1;
    }
    start_cnt++;
    
    lcd_clear();
    char display[17]={0};  
    if (get_mqtt_connected()){


    lcd_put_cur(0, 0);
    sprintf(display,"ID:%s  STATUS:OK",display_user_id);//
    lcd_send_string(display);

    }
    else if (!get_mqtt_connected()) {
        
        lcd_put_cur(0, 0);
       sprintf(display,"ID:%s  STATUS:NO",display_user_id);//
       lcd_send_string(display);

    }

    lcd_put_cur(1, 0);
    lcd_send_string("                ");
    lcd_put_cur(1, 0);

    

    if (start_cnt > 1) {
        if (number && strlen(number) > 0) {
            char buf[DISPLAY_LINE_MAX + 1] = {0};
            strncpy(buf, number, DISPLAY_LINE_MAX);
            buf[DISPLAY_LINE_MAX] = '\0';
            lcd_send_string(buf);
        } else {
            lcd_send_string("___");
        }
    } else {
        size_t num_len = sizeof(g_keypad.prev_number);
        size_t status_len = 12;
        char temp_status[12] = {0};
        
        if (read_current_number_from_nvs(g_keypad.prev_number, &num_len) == ESP_OK && 
            strlen(g_keypad.prev_number) > 0 &&
            read_current_number_status_from_nvs(temp_status, status_len) == ESP_OK &&
            strlen(temp_status) > 0) {
            
            char display[DISPLAY_LINE_MAX + 1] = {0};
            snprintf(display, sizeof(display), "%s:%s", temp_status, g_keypad.prev_number);
            lcd_send_string(display);
            update_temp_buff(display);
        } else {
            lcd_send_string("___");
        }
    }
    
    lcd_unlock();
}

void lcd_show_wifi_input(const char *ssid) {
    lcd_lock();
    
    lcd_clear();
    lcd_put_cur(0, 0);
    lcd_send_string("*NHAP TEN WIFI:");
    lcd_put_cur(1, 0);
    
    
    if (ssid && strlen(ssid) > 0) {
        char short_ssid[DISPLAY_LINE_MAX + 1] = {0};
        strncpy(short_ssid, ssid, DISPLAY_LINE_MAX);
        short_ssid[DISPLAY_LINE_MAX] = '\0';
        lcd_send_string(short_ssid);
    } else {
        lcd_send_string("___");
    }
    
    lcd_unlock();
}

void lcd_show_wifi_pass(const char *pass) {
    lcd_lock();
    
    lcd_clear();
    lcd_put_cur(0, 0);
    lcd_send_string("*NHAP PASSWORD:");
    lcd_put_cur(1, 0);
    if (pass && strlen(pass) > 0 && g_keypad.hide == true) {
        int pass_len = strlen(pass);
        int show_len = pass_len;
        if (show_len > DISPLAY_LINE_MAX) show_len = DISPLAY_LINE_MAX;
        for (int i = 0; i < show_len - 1; i++) {
            lcd_send_data('*');
        }
        if (show_len > 0) {
            lcd_send_data(pass[pass_len - 1]);
        }
    } else if (pass && strlen(pass) > 0 && g_keypad.hide == false) {
        int pass_len = strlen(pass);
        int show_len = pass_len;
        if (show_len > DISPLAY_LINE_MAX) show_len = DISPLAY_LINE_MAX;
        for (int i = 0; i < show_len; i++) {
            lcd_send_data(pass[i]);
        }
    } else {
        lcd_send_string("___");
    }
    
    lcd_unlock();
}

void lcd_show_message(const char *line1, const char *line2) {
    lcd_lock();
    
    lcd_clear();
    lcd_put_cur(0, 0);
    if (line1) {
        char tmp[DISPLAY_LINE_MAX + 1] = {0};
        strncpy(tmp, line1, DISPLAY_LINE_MAX);
        tmp[DISPLAY_LINE_MAX] = '\0';
        lcd_send_string(tmp);
    }
    if (line2 != NULL) {
        lcd_put_cur(1, 0);
        char tmp2[DISPLAY_LINE_MAX + 1] = {0};
        strncpy(tmp2, line2, DISPLAY_LINE_MAX);
        tmp2[DISPLAY_LINE_MAX] = '\0';
        lcd_send_string(tmp2);
    }
    
    lcd_unlock();
}


void lcd_show_device_list(void) {
    
    lcd_lock();
    lcd_clear();
    lcd_put_cur(0, 0);
    char header[17] = {0};
    //snprintf(header, sizeof(header), "QUAY:%d/%d", selected_index + 1, device_count);
    snprintf(header, sizeof(header), "*CHON QUAY:");
    lcd_send_string(header);
    

    lcd_put_cur(1, 0);
    char line[17] = {0};

    xSemaphoreTake(g_mutex.device_list_mutex, portMAX_DELAY);//


    snprintf(line, sizeof(line), ">%s", g_keypad.device_list[g_keypad.selected_index].name);
   
    
    //xSemaphoreGive(device_list_mutex);//
    
    lcd_send_string(line);
    //user_id_init();//
    read_current_user_id();//
    /*
    if ((strcmp(g_keypad.device_list[g_keypad. selected_index].device_id,g_keypad.selected_device_id)==0)&&(strcmp(g_keypad.device_list[g_keypad. selected_index].user_id,current_user_id)!=0)){
    lcd_put_cur(1,8);
    lcd_put_cur(1,strlen(g_keypad.device_list[g_keypad.selected_index].name)+2);
    lcd_send_string("(V)");
    }
    else {
    if (strcmp(g_keypad.device_list[g_keypad. selected_index].user_id,current_user_id)!=0){
    //lcd_put_cur(1,8);
    lcd_put_cur(1,strlen(g_keypad.device_list[g_keypad.selected_index].name)+1);
    lcd_send_string("   "); 
    }
    
    }
      */

    xSemaphoreGive(g_mutex.device_list_mutex);//
    lcd_unlock();
}


void lcd_show_user_list(void) {
    
    lcd_lock();
    lcd_clear();
    lcd_put_cur(0, 0);
    char header[17] = {0};
    //snprintf(header, sizeof(header), "QUAY:%d/%d", selected_index + 1, device_count);
    snprintf(header, sizeof(header), "*CHON TB:");
    lcd_send_string(header);
    

    lcd_put_cur(1, 0);
    char line[17] = {0};

    xSemaphoreTake(g_mutex.device_list_mutex, portMAX_DELAY);//


    snprintf(line, sizeof(line), ">%s", g_keypad.device_list[g_keypad.selected_index].name);
   
    
    //xSemaphoreGive(device_list_mutex);//
    
    lcd_send_string(line);
    //user_id_init();//
    read_current_user_id();//
    //if ((strcmp(g_keypad.device_list[g_keypad. selected_index].device_id,g_keypad.selected_device_id)==0)&&(strcmp(g_keypad.device_list[g_keypad. selected_index].user_id,current_user_id)!=0)){
    if (strcmp(g_keypad.device_list[g_keypad. selected_index].device_id,g_keypad.selected_device_id)==0){
    //lcd_put_cur(1,8);
    lcd_put_cur(1,strlen(g_keypad.device_list[g_keypad.selected_index].name)+2);
    lcd_send_string("(V)");
    }
    else {
    if (strcmp(g_keypad.device_list[g_keypad. selected_index].user_id,current_user_id)!=0){
    //lcd_put_cur(1,8);
    //lcd_put_cur(1,strlen(g_keypad.device_list[g_keypad.selected_index].name)+1);
    lcd_put_cur(1,strlen(g_keypad.device_list[g_keypad.selected_index].name)+2);
    lcd_send_string("   "); 
    }
    }
      
    xSemaphoreGive(g_mutex.device_list_mutex);//
    lcd_unlock();
}

void lcd_show_position_list(void) {
    
    lcd_lock();
    lcd_clear();
    lcd_put_cur(0, 0);
    char header[17] = {0};
    snprintf(header, sizeof(header), "*CHON VI TRI:");
    lcd_send_string(header);
    lcd_put_cur(1,1);
    lcd_send_string("1.DAU");
    lcd_put_cur(1,10);
    lcd_send_string("2.CUOI");

    

    
    char line[2] = {0};
    
    if (g_keypad.selected_positon==false){

    snprintf(line, sizeof(line), ">");
    lcd_put_cur(1, 0);
    lcd_send_string(line);
    lcd_put_cur(1, 9);
    lcd_send_string(" ");
    
    }

    else {

        snprintf(line, sizeof(line), ">");
        lcd_put_cur(1, 9);
        lcd_send_string(line);
        lcd_put_cur(1, 0);
        lcd_send_string(" ");

    }

    
    
    lcd_unlock();
}



void lcd_show_service_list(void) {

    lcd_lock();
    lcd_clear();
    lcd_put_cur(0, 0);
    lcd_send_string("*CHON DICH VU:");

    char full_name[64] = {0};

    xSemaphoreTake(g_mutex.service_list_mutex, portMAX_DELAY);
    strncpy(full_name, g_keypad.service_list[g_keypad.selected_index2].name, sizeof(full_name) - 1);
    xSemaphoreGive(g_mutex.service_list_mutex);

    int len = strlen(full_name);

    if (len <= 15) {
        service_scroll_enable = false;
        lcd_put_cur(1, 0);
        lcd_send_string(">");
        lcd_send_string(full_name);
    } 
    else {
        service_scroll_enable = true;
        //strcpy(service_scroll_buffer, full_name);
        sprintf(service_scroll_buffer,"%s   ",full_name);
        service_scroll_pos = 0;
    }

    lcd_unlock();
}

void lcd_show_menu(void) {
    lcd_lock();
    lcd_clear();
    lcd_put_cur(0, 0);
    lcd_send_string("*SETTING");
    if (g_keypad.menu_selection==1){
    lcd_put_cur(1, 0);
    lcd_send_string(">CAU HINH WIFI");
    }
    else if (g_keypad.menu_selection==2){
       lcd_put_cur(1, 0);
       lcd_send_string(">CHUYEN QUAY");
    }
    else if (g_keypad.menu_selection==3){
       lcd_put_cur(1, 0);
       lcd_send_string(">CHUYEN DICH VU");
    }
    else if (g_keypad.menu_selection==4){
       lcd_put_cur(1, 0);
       lcd_send_string(">DANG KY TB");
    }

    else if (g_keypad.menu_selection==5){
       lcd_put_cur(1, 0);
       lcd_send_string(">CHUYEN TB");
    }
    else if (g_keypad.menu_selection==6){
       lcd_put_cur(1, 0);
       lcd_send_string(">RESET");
    }
    else if (g_keypad.menu_selection==7){
       lcd_put_cur(1, 0);
       lcd_send_string(">DONG QUAY");
    }
    else if (g_keypad.menu_selection==8){
       lcd_put_cur(1, 0);
       lcd_send_string(">MO QUAY");
    }


    lcd_unlock();
}

void service_scroll_task(void *pvParameter) {
    char view[17];
    

    while (1) {

        if (service_scroll_enable) {

            int len = strlen(service_scroll_buffer);

            view[0] = '>';

            for (int i = 0; i < 15; i++) {
                
                int idx = (service_scroll_pos + i) % len;
               
                view[i + 1] = service_scroll_buffer[idx];
                
                

                
            }

            view[16] = '\0';

            lcd_lock();
            lcd_put_cur(1, 0);
            lcd_send_string(view);
            lcd_unlock();

            service_scroll_pos++;
            if (service_scroll_pos >= len){
                
                service_scroll_pos = 0;
               
                    
                
            }
        }

        vTaskDelay(pdMS_TO_TICKS(500)); 
    }
}


void lcd_mainscreen_init(){


    lcd_init();
    lcd_lock();
    lcd_clear();
    lcd_put_cur(0, 0);
    lcd_send_string("    XIN CHAO");
    lcd_put_cur(1, 0);
    lcd_send_string("DANG KHOI DONG..");
    lcd_unlock();

    vTaskDelay(pdMS_TO_TICKS(800));



}

esp_err_t i2c_master_init(void) {
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master = {.clk_speed = I2C_MASTER_FREQ_HZ}
    };

    esp_err_t err = i2c_param_config(I2C_MASTER_NUM, &conf);
    if (err != ESP_OK) return err;
    return i2c_driver_install(I2C_MASTER_NUM, conf.mode,
                              I2C_MASTER_RX_BUF_DISABLE,
                              I2C_MASTER_TX_BUF_DISABLE, 0);
}



