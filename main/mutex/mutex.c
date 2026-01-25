#include "mutex.h"

static bool _mqtt_connected = false;
static bool _wifi_connected = false;
static bool _user_selected_wifi = false;
static int  _wifi_retry_count = 0;
static bool _service_scroll_enable=false;
static bool _ssid_service_scroll_enable=false;


extern state_context_t g_state;

mutex_context_t g_mutex={

. lcd_mutex = NULL,
. state_mutex = NULL,
. display_state_mutex = NULL,
. input_mutex = NULL,
. device_list_mutex = NULL,
. service_list_mutex = NULL,
. wifi_config_mutex = NULL,
. mqtt_mutex = NULL,
. selected_id_mutex = NULL,
.scroll_mutex=NULL,
.ssid_scroll_mutex=NULL,

};

static StaticQueue_t  lcdMutexBuffer;
// Buffer tĩnh cho từng mutex (khai báo static để tránh dùng stack)
static StaticQueue_t stateMutexBuffer;
static StaticQueue_t displaystateMutexBuffer;
static StaticQueue_t inputMutexBuffer;
static StaticQueue_t deviceListMutexBuffer;
static StaticQueue_t serviceListMutexBuffer;
static StaticQueue_t wifiConfigMutexBuffer;
static StaticQueue_t mqttMutexBuffer;
static StaticQueue_t selectedIdMutexBuffer;
static StaticQueue_t mqttStartMutexBuffer;
static StaticQueue_t wifiStartMutexBuffer;
static StaticQueue_t mqttConnectedMutexBuffer;
static StaticQueue_t wifiConnectedMutexBuffer;
static StaticQueue_t wifiRetryMutexBuffer;
static StaticQueue_t userSelectedWifiMutexBuffer;
static StaticQueue_t scrollMutexBuffer;
static StaticQueue_t displayServiceMutexBuffer;
static StaticQueue_t ssidScrollMutexBuffer;
static StaticQueue_t displaySavedSsidMutexBuffer;

void mutex_init() {
    //g_mutex. lcd_mutex = xSemaphoreCreateMutex();
    /*
    g_mutex. state_mutex = xSemaphoreCreateMutex();
    g_mutex.display_state_mutex = xSemaphoreCreateMutex();
    g_mutex.input_mutex = xSemaphoreCreateMutex();
    g_mutex.device_list_mutex = xSemaphoreCreateMutex();
    g_mutex.service_list_mutex = xSemaphoreCreateMutex();
    g_mutex.wifi_config_mutex = xSemaphoreCreateMutex();
    g_mutex.mqtt_mutex = xSemaphoreCreateMutex();
    g_mutex.selected_id_mutex = xSemaphoreCreateMutex();
    g_mutex. mqtt_start_mutex = xSemaphoreCreateMutex();
    g_mutex. wifi_start_mutex = xSemaphoreCreateMutex();
    g_mutex. mqtt_connected_mutex = xSemaphoreCreateMutex();
    g_mutex. wifi_connected_mutex = xSemaphoreCreateMutex();
    g_mutex. wifi_retry_mutex = xSemaphoreCreateMutex();
    g_mutex. user_selected_wifi_mutex = xSemaphoreCreateMutex();
    g_mutex.scroll_mutex=xSemaphoreCreateMutex();
    g_mutex.display_service_mutex=xSemaphoreCreateMutex();
    g_mutex.ssid_scroll_mutex=xSemaphoreCreateMutex();
    g_mutex.display_saved_ssid_mutex=xSemaphoreCreateMutex();
    */
    g_mutex.lcd_mutex=xSemaphoreCreateMutexStatic(&lcdMutexBuffer);
    g_mutex.state_mutex= xSemaphoreCreateMutexStatic(&stateMutexBuffer);
    g_mutex.display_state_mutex= xSemaphoreCreateMutexStatic(&displaystateMutexBuffer);
    g_mutex.input_mutex= xSemaphoreCreateMutexStatic(&inputMutexBuffer);
    g_mutex.device_list_mutex= xSemaphoreCreateMutexStatic(&deviceListMutexBuffer);
    g_mutex.service_list_mutex= xSemaphoreCreateMutexStatic(&serviceListMutexBuffer);
    g_mutex.wifi_config_mutex= xSemaphoreCreateMutexStatic(&wifiConfigMutexBuffer);
    g_mutex.mqtt_mutex= xSemaphoreCreateMutexStatic(&mqttMutexBuffer);
    g_mutex.selected_id_mutex= xSemaphoreCreateMutexStatic(&selectedIdMutexBuffer);
    g_mutex.mqtt_start_mutex= xSemaphoreCreateMutexStatic(&mqttStartMutexBuffer);
    g_mutex.wifi_start_mutex= xSemaphoreCreateMutexStatic(&wifiStartMutexBuffer);
    g_mutex.mqtt_connected_mutex= xSemaphoreCreateMutexStatic(&mqttConnectedMutexBuffer);
    g_mutex.wifi_connected_mutex= xSemaphoreCreateMutexStatic(&wifiConnectedMutexBuffer);
    g_mutex.wifi_retry_mutex= xSemaphoreCreateMutexStatic(&wifiRetryMutexBuffer);
    g_mutex.user_selected_wifi_mutex = xSemaphoreCreateMutexStatic(&userSelectedWifiMutexBuffer);
    g_mutex.scroll_mutex= xSemaphoreCreateMutexStatic(&scrollMutexBuffer);
    g_mutex.display_service_mutex= xSemaphoreCreateMutexStatic(&displayServiceMutexBuffer);
    g_mutex.ssid_scroll_mutex= xSemaphoreCreateMutexStatic(&ssidScrollMutexBuffer);
    g_mutex.display_saved_ssid_mutex = xSemaphoreCreateMutexStatic(&displaySavedSsidMutexBuffer);
}

bool get_mqtt_connected(void) {
    bool val;
    xSemaphoreTake(g_mutex.mqtt_connected_mutex, portMAX_DELAY);
    val = _mqtt_connected;
    xSemaphoreGive(g_mutex.mqtt_connected_mutex);
    return val;
}

void set_mqtt_connected(bool val) {
    xSemaphoreTake(g_mutex.mqtt_connected_mutex, portMAX_DELAY);
    _mqtt_connected = val;
    xSemaphoreGive(g_mutex.mqtt_connected_mutex);
}


bool get_wifi_connected(void) {
    bool val;
    xSemaphoreTake(g_mutex.wifi_connected_mutex, portMAX_DELAY);
    val = _wifi_connected;
    xSemaphoreGive(g_mutex.wifi_connected_mutex);
    return val;
}

void set_wifi_connected(bool val) {
    xSemaphoreTake(g_mutex.wifi_connected_mutex, portMAX_DELAY);
    _wifi_connected = val;
    xSemaphoreGive(g_mutex.wifi_connected_mutex);
}

bool get_user_selected_wifi(void) {
    bool val;
    xSemaphoreTake(g_mutex.user_selected_wifi_mutex, portMAX_DELAY);
    val = _user_selected_wifi;
    xSemaphoreGive(g_mutex.user_selected_wifi_mutex);
    return val;
}

void set_user_selected_wifi(bool val) {
    xSemaphoreTake(g_mutex.user_selected_wifi_mutex, portMAX_DELAY);
    _user_selected_wifi = val;
    xSemaphoreGive(g_mutex.user_selected_wifi_mutex);
}


int get_wifi_retry_count(void) {
    int val;
    xSemaphoreTake(g_mutex.wifi_retry_mutex, portMAX_DELAY);
    val = _wifi_retry_count;
    xSemaphoreGive(g_mutex.wifi_retry_mutex);
    return val;
}

void set_wifi_retry_count(int val) {
    xSemaphoreTake(g_mutex.wifi_retry_mutex, portMAX_DELAY);
    _wifi_retry_count = val;
    xSemaphoreGive(g_mutex.wifi_retry_mutex);
}

void increment_wifi_retry_count(void) {
    xSemaphoreTake(g_mutex.wifi_retry_mutex, portMAX_DELAY);
    _wifi_retry_count++;
    xSemaphoreGive(g_mutex.wifi_retry_mutex);
}



SystemState get_sys_state(void) { 
    SystemState s;
    xSemaphoreTake(g_mutex.state_mutex, portMAX_DELAY);
    s =g_state.sys_state;
    xSemaphoreGive(g_mutex.state_mutex);
    return s;

     }

void set_sys_state(SystemState s) {
    xSemaphoreTake(g_mutex.state_mutex, portMAX_DELAY);
    g_state.sys_state = s;
    xSemaphoreGive(g_mutex.state_mutex);
}


 DisplayState get_display_state(void) {
    DisplayState d;
    xSemaphoreTake(g_mutex.display_state_mutex, portMAX_DELAY);
    d=g_state.display_state;
    xSemaphoreGive(g_mutex.display_state_mutex);
    return d;
}

 void set_display_state(DisplayState d) {
    xSemaphoreTake(g_mutex.display_state_mutex, portMAX_DELAY);
    g_state.display_state=d;
    xSemaphoreGive(g_mutex.display_state_mutex);
}

void set_scroll_enable(bool val){
    xSemaphoreTake(g_mutex.scroll_mutex,portMAX_DELAY);
    _service_scroll_enable=val;
    xSemaphoreGive(g_mutex.scroll_mutex);
}

bool get_scroll_enable(void){
    bool val;
    xSemaphoreTake(g_mutex.scroll_mutex,portMAX_DELAY);
    val=_service_scroll_enable;
    xSemaphoreGive(g_mutex.scroll_mutex);
    return val;
}

void set_ssid_scroll_enable(bool val){
    xSemaphoreTake(g_mutex.scroll_mutex,portMAX_DELAY);
    _ssid_service_scroll_enable=val;
    xSemaphoreGive(g_mutex.scroll_mutex);
}

bool get_ssid_scroll_enable(void){
    bool val;
    xSemaphoreTake(g_mutex.scroll_mutex,portMAX_DELAY);
    val=_ssid_service_scroll_enable;
    xSemaphoreGive(g_mutex.scroll_mutex);
    return val;
}



