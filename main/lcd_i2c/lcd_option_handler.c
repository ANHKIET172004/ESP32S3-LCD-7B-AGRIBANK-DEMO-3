#include "lcd_option_handler.h"

static char* TAG ="DEBUG";

char service_scroll_buffer[68];
char ssid_scroll_buffer[37];

uint8_t service_scroll_pos = 0;
uint8_t ssid_scroll_pos = 0;

extern char current_counter_id[3];
extern char counter_id[3];

extern bool start;

uint8_t scroll_cnt=0;

void lcd_show_main_screen(const char *number) {
    lcd_lock();
    
    lcd_clear();
    char display[17]={0};  
    if (get_mqtt_connected()){


    lcd_put_cur(0, 0);
    //sprintf(display,"ID:%s  STATUS:OK",counter_id);//
    sprintf(display,"ID:%s",counter_id);//
    lcd_send_string(display);
    lcd_put_cur(0,7);
    lcd_send_string("STATUS:OK");
    


    }
    else if (!get_mqtt_connected()) {
        
    lcd_put_cur(0, 0);
       //sprintf(display,"ID:%s  STATUS:NO",counter_id);//
       sprintf(display,"ID:%s",counter_id);//
       lcd_send_string(display);
       lcd_put_cur(0,7);
       lcd_send_string("STATUS:NO");
    }

    lcd_put_cur(1, 0);
    lcd_send_string("                ");
    lcd_put_cur(1, 0);

    //if (start_cnt > 1) {
    if (start==false) {
         
        if (number && strlen(number) > 0) {
            char buf[DISPLAY_LINE_MAX + 1] = {0};
            strncpy(buf, number, DISPLAY_LINE_MAX);
            buf[DISPLAY_LINE_MAX] = '\0';
            lcd_send_string(buf);
        } else {
            lcd_send_string("___");
        }
    } else {
        start=false;
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

void lcd_show_user_pass(const char* number){
       lcd_lock();
       lcd_clear();
       lcd_put_cur(0,0);
       lcd_send_string("*NHAP MAT KHAU:");
       lcd_put_cur(1,0);
       lcd_send_string(number);
       lcd_unlock();
}

void lcd_show_new_user_pass(const char* number){
       lcd_lock();
       lcd_clear();
       lcd_put_cur(0,0);
       lcd_send_string("*MAT KHAU MOI:");
       lcd_put_cur(1,0);
       lcd_send_string(number);
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
    lcd_send_string("*NHAP MAT KHAU:");
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
    snprintf(header, sizeof(header), "*CHON QUAY:");
    lcd_send_string(header);
    lcd_put_cur(1, 0);
    char line[17] = {0};

    xSemaphoreTake(g_mutex.device_list_mutex, portMAX_DELAY);//
    //if (strcmp(g_keypad.device_list[g_keypad.selected_index].device_id, g_keypad.default_id)!=0){//
    snprintf(line, sizeof(line), ">%s", g_keypad.device_list[g_keypad.selected_index].name);    
    //}
   
    //else {
    //snprintf(line, sizeof(line), ">%s*", g_keypad.device_list[g_keypad.selected_index].name); 
     

  //  }
    
    lcd_send_string(line);
    read_current_counter_id();//


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

    char full_name[64] = {0};



    xSemaphoreTake(g_mutex.service_list_mutex, portMAX_DELAY);
    strncpy(full_name, g_keypad.service_list[g_keypad.selected_index2].name, sizeof(full_name) - 1);
    full_name[sizeof(full_name) - 1]='\0';//
    xSemaphoreGive(g_mutex.service_list_mutex);

    //lcd_clear();
    lcd_lock();//
    lcd_put_cur(0, 0);
    lcd_send_string("*CHON DICH VU:");
    lcd_unlock();//

    int len = strlen(full_name);
    
    if (len <= 15) {
        lcd_lock();
        set_scroll_enable(false);
        lcd_put_cur(1,0);
        lcd_send_string("                ");
        lcd_put_cur(1, 0);
        lcd_send_string(">");
        lcd_send_string(full_name);
        lcd_unlock();
    } 
    else {
        
        //strcpy(service_scroll_buffer, full_name);
        sprintf(service_scroll_buffer,"%s  ",full_name);
        set_scroll_enable(true);
        //service_scroll_pos = 0;

    }
    
    
}

void lcd_show_menu(void) {
    lcd_lock();
    lcd_clear();
    lcd_put_cur(0, 0);
    lcd_send_string("*CAI DAT");
    if (g_keypad.menu_selection==1){
        lcd_put_cur(1, 0);
        lcd_send_string(">KET NOI WIFI");
    }
    else if (g_keypad.menu_selection==2){
       lcd_put_cur(1, 0);
       lcd_send_string(">GOI SO BO QUA");
    }
    else if (g_keypad.menu_selection==3){
       lcd_put_cur(1, 0);
       lcd_send_string(">CHUYEN QUAY");
    }
    else if (g_keypad.menu_selection==4){
       lcd_put_cur(1, 0);
       lcd_send_string(">CHUYEN DICH VU");
    }
    else if (g_keypad.menu_selection==5){
       lcd_put_cur(1, 0);
       lcd_send_string(">DANG KY TB");
    }
/*
    else if (g_keypad.menu_selection==5){
       lcd_put_cur(1, 0);
       lcd_send_string(">CHUYEN TB");
    }
       */
    else if (g_keypad.menu_selection==6){
       lcd_put_cur(1, 0);
       lcd_send_string(">DONG QUAY");
    }

    else if (g_keypad.menu_selection==7){
       lcd_put_cur(1, 0);
       lcd_send_string(">RESET");
    }

    /*
    else if (g_keypad.menu_selection==8){
       lcd_put_cur(1, 0);
       lcd_send_string(">MO QUAY");
    }
       */
      /*
    else if (g_keypad.menu_selection==8){
       lcd_put_cur(1, 0);
       lcd_send_string(">DOI MAT KHAU");
    }
*/
/*
    else if (g_keypad.menu_selection==8){
       lcd_put_cur(1, 0);
       lcd_send_string(">WIFI DA LUU");
    }
       */

    /*
    else if (g_keypad.menu_selection==10){
       lcd_put_cur(1, 0);
       lcd_send_string(">TU DONG KET NOI");
    }
    else if (g_keypad.menu_selection==11){
       lcd_put_cur(1, 0);
       lcd_send_string(">DANG KET NOI");
    }
*/
    lcd_unlock();
}

void lcd_show_options(void) {
    
    lcd_lock();
    lcd_clear();
    lcd_put_cur(0, 0);
    char header[17] = {0};
    snprintf(header, sizeof(header), "*TIEP TUC?");
    lcd_send_string(header);
    lcd_put_cur(1,1);
    lcd_send_string("1.YES");
    lcd_put_cur(1,11);
    lcd_send_string("2.NO");
   
    char line[2] = {0};
    
    if (g_keypad.selected_option==true){

    snprintf(line, sizeof(line), ">");
    lcd_put_cur(1, 0);
    lcd_send_string(line);
    lcd_put_cur(1, 10);
    lcd_send_string(" ");
    
    }

    else {

        snprintf(line, sizeof(line), ">");
        lcd_put_cur(1, 10);
        lcd_send_string(line);
        lcd_put_cur(1, 0);
        lcd_send_string(" ");

    }
  
    lcd_unlock();
}


void lcd_show_delete_wifi_options(void) {
    
    lcd_lock();
    lcd_clear();
    lcd_put_cur(0, 0);
    char header[17] = {0};
    snprintf(header, sizeof(header), "*XOA LUU WIFI?");
    lcd_send_string(header);
    lcd_put_cur(1,1);
    lcd_send_string("1.YES");
    lcd_put_cur(1,11);
    lcd_send_string("2.NO");
   
    char line[2] = {0};
    
    if (g_keypad.delete_wifi_option==1){

    snprintf(line, sizeof(line), ">");
    lcd_put_cur(1, 0);
    lcd_send_string(line);
    lcd_put_cur(1, 10);
    lcd_send_string(" ");
    
    }

    else {

        snprintf(line, sizeof(line), ">");
        lcd_put_cur(1, 10);
        lcd_send_string(line);
        lcd_put_cur(1, 0);
        lcd_send_string(" ");

    }
  
    lcd_unlock();
}

void lcd_show__auto_connect_options(void) {
    
    lcd_lock();
    lcd_clear();
    lcd_put_cur(0, 0);
    char header[17] = {0};
    snprintf(header, sizeof(header), "*TU DONG KET NOI");
    lcd_send_string(header);
    lcd_put_cur(1,1);
    lcd_send_string("1.YES");
    lcd_put_cur(1,11);
    lcd_send_string("2.NO");
   
    char line[2] = {0};
    
    if (g_keypad.auto_connect_option==1){

        snprintf(line, sizeof(line), ">");
        lcd_put_cur(1, 0);
        lcd_send_string(line);
        lcd_put_cur(1, 10);
        lcd_send_string(" ");
    
    }

    else {

        snprintf(line, sizeof(line), ">");
        lcd_put_cur(1, 10);
        lcd_send_string(line);
        lcd_put_cur(1, 0);
        lcd_send_string(" ");

    }
  
    lcd_unlock();
}

void lcd_show_saved_wifi(void) {
    wifi_list *list = calloc(1, sizeof(wifi_list));
    if (!list) {
        ESP_LOGE(TAG, "calloc failed");
        return;
    }

    load_wifi_list(list);

    
    if (list->count <= 0) {
        lcd_clear();
        lcd_put_cur(0,0);
        lcd_send_string("*WIFI DA LUU:");

        lcd_put_cur(1,0);
        lcd_send_string(">CHUA LUU WIFI");

        free(list);
        return;
    }

    list->aps[g_keypad.wifi_position].ssid[31] = '\0';

    //lcd_clear();
    lcd_put_cur(0,0);
    lcd_send_string("*WIFI DA LUU:");

    char display_saved_wifi[17];
    
    display_saved_wifi[16] = '\0';


    
    if (strlen(list->aps[g_keypad.wifi_position].ssid)<=15){//15
       // if (g_keypad.wifi_position!=g_keypad.best_saved_index){
            snprintf(display_saved_wifi, sizeof(display_saved_wifi),
                ">%.*s", 14, list->aps[g_keypad.wifi_position].ssid);
         //   }
         /*
        else {
            snprintf(display_saved_wifi, sizeof(display_saved_wifi),
            ">%.*s*", 14, list->aps[g_keypad.wifi_position].ssid);
        }
            */
        lcd_put_cur(1,0);
        lcd_send_string("                ");
        lcd_put_cur(1,0);
        lcd_send_string(display_saved_wifi);
    }
    else {
       // strncpy(ssid_scroll_buffer,list->aps[g_keypad.wifi_position].ssid,sizeof(ssid_scroll_buffer));
      // if (g_keypad.wifi_position!=g_keypad.best_saved_index){
            sprintf(ssid_scroll_buffer,"%s   ",list->aps[g_keypad.wifi_position].ssid);
      //  }
      /*
       else {
            sprintf(ssid_scroll_buffer,"%s*   ",list->aps[g_keypad.wifi_position].ssid);
       }
            */
        set_ssid_scroll_enable(true);//
    }

    free(list);
    
}

void lcd_show_saved_wifi_option(void) {
    lcd_lock();
    lcd_clear();
    lcd_put_cur(0, 0);
    lcd_send_string("*TUY CHON:");
    if (g_keypad.saved_wifi_option==1){
        lcd_put_cur(1, 0);
        lcd_send_string(">XOA WIFI");
    }
    else if (g_keypad.saved_wifi_option==2){
       lcd_put_cur(1, 0);
       lcd_send_string(">TU DONG KET NOI");
    }
    lcd_unlock();
}



void service_scroll_task(void *pvParameter) {
    //char lcd_str[17];

    while (1) {

        if (get_scroll_enable()) {


            //service_scroll_pos = 0;//
            xSemaphoreTake(g_mutex.display_service_mutex, portMAX_DELAY);//


            uint8_t len = strlen(service_scroll_buffer);

            g_keypad.display_service[0] = '>';

            for (int i = 0; i < 15; i++) {
                
                uint8_t idx = (service_scroll_pos + i) % len;// vd: len=20, pos<=len: 1%20=1 20%20=0, pos>len: 21%20=1
               
                g_keypad.display_service[i + 1] = service_scroll_buffer[idx];//i+1 vì bắt đầu hiển thị từ vị trí 0 trên lcd (vị trí 0 là '>')
               
                
            }

            g_keypad.display_service[16] = '\0';

            
            lcd_lock();
            //lcd_clear();//
            //lcd_put_cur(0, 0);//
            //lcd_send_string("*CHON DICH VU:");//
            lcd_put_cur(1, 0);
            lcd_send_string("                 ");//
            lcd_put_cur(1, 0);
            lcd_send_string(g_keypad.display_service);//lcd_str
            lcd_unlock();
            

            service_scroll_pos++;
            if (service_scroll_pos >= len){
                
                service_scroll_pos = 0;                  
                
            }
                
            xSemaphoreGive(g_mutex.display_service_mutex);//
            vTaskDelay(pdMS_TO_TICKS(500)); 
           
        }

        vTaskDelay(pdMS_TO_TICKS(50)); //500
    }
}

/*
void ssid_scroll_task(void *pvParameter) {
    //char lcd_str[17];

    while (1) {

        if (get_ssid_scroll_enable()) {

            //service_scroll_pos = 0;//

           // xSemaphoreTake(g_mutex.display_saved_ssid_mutex,portMAX_DELAY);

            uint8_t len = strlen(ssid_scroll_buffer);

            g_keypad.display_saved_ssid[0] = '>';

            for (int i = 0; i < 15; i++) {
                
                uint8_t idx = (ssid_scroll_pos + i) % len;// vd: len=20, pos<=len: 1%20=1 20%20=0, pos>len: 21%20=1
               
                g_keypad.display_saved_ssid[i + 1] = ssid_scroll_buffer[idx];//i+1 vì bắt đầu hiển thị từ vị trí 0 trên lcd (vị trí 0 là '>')
              
            }

            g_keypad.display_saved_ssid[16] = '\0';

            
            lcd_lock();
            lcd_clear();
            lcd_put_cur(0,0);
            lcd_send_string("*WIFI DA LUU:");
            lcd_put_cur(1, 0);
            lcd_send_string(g_keypad.display_saved_ssid);//lcd_str
            lcd_unlock();
            

            ssid_scroll_pos++;
            if (ssid_scroll_pos >= len){
                
                ssid_scroll_pos = 0;                  
                
            }
           // xSemaphoreGive(g_mutex.display_saved_ssid_mutex);
            vTaskDelay(pdMS_TO_TICKS(500)); //
        }

        vTaskDelay(pdMS_TO_TICKS(50)); //500
    }
}

*/
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