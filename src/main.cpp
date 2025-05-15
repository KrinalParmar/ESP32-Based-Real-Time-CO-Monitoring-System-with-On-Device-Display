#include <Arduino.h>
#include <Wire.h>
#include <SensirionI2cScd4x.h>
#include <LovyanGFX.hpp>
#include <lvgl.h>
#include "ui.h"

#define POWER_BUTTON_PIN 40  
#define SYS_EN_PIN 41        
#define NO_ERROR 0

extern void update_tvoc_screen(float co2_value, float temperature, float humidity);

// Display configuration
class LGFX : public lgfx::LGFX_Device {
  lgfx::Panel_ST7789 _panel_instance;
  lgfx::Bus_SPI _bus_instance;

public:
  LGFX(void) {
    {
      auto cfg = _bus_instance.config();
      cfg.spi_host = SPI2_HOST;
      cfg.pin_sclk = 6;
      cfg.pin_mosi = 7;
      cfg.pin_miso = -1;
      cfg.pin_dc   = 4;
      cfg.freq_write = 40000000;
      _bus_instance.config(cfg);
      _panel_instance.setBus(&_bus_instance);
    }
    {
      auto cfg = _panel_instance.config();
      cfg.pin_cs   = 5;
      cfg.pin_rst  = 8;
      cfg.pin_busy = -1;
      cfg.panel_width  = 240;
      cfg.panel_height = 280;
      cfg.offset_x = 0;
      cfg.offset_y = 20;
      cfg.readable = true;
      cfg.invert = true;
      cfg.rgb_order = true;
      cfg.dlen_16bit = false;
      cfg.bus_shared = true;
      _panel_instance.config(cfg);
    }
    setPanel(&_panel_instance);
  }
};

static LGFX lcd;
static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf1[240 * 40];
static lv_color_t buf2[240 * 40];
SensirionI2cScd4x sensor;

static char errorMessage[64];
static int16_t error;
bool ui_initialized = false;
bool system_on = false;

void PrintUint64(uint64_t& value) {
  Serial.print("0x");
  Serial.print((uint32_t)(value >> 32), HEX);
  Serial.print((uint32_t)(value & 0xFFFFFFFF), HEX);
}

void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p) {
  lcd.startWrite();
  lcd.setAddrWindow(area->x1, area->y1,
                    area->x2 - area->x1 + 1,
                    area->y2 - area->y1 + 1);
  lcd.writePixels((lgfx::rgb565_t *)color_p,
                  (area->x2 - area->x1 + 1) * (area->y2 - area->y1 + 1));
  lcd.endWrite();
  lv_disp_flush_ready(disp);
}

void my_touchpad_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data) {
  uint16_t touchX, touchY;
  bool touched = lcd.getTouch(&touchX, &touchY);
  if (touched) {
    data->state = LV_INDEV_STATE_PR;
    data->point.x = touchX;
    data->point.y = touchY;
  } else {
    data->state = LV_INDEV_STATE_REL;
  }
}

void read_scd4x_data(lv_timer_t *timer) {
  uint16_t co2 = 0;
  float temp = 0.0f, rh = 0.0f;

  error = sensor.readMeasurement(co2, temp, rh);
  if (error != NO_ERROR) {
    errorToString(error, errorMessage, sizeof errorMessage);
    Serial.print("Read error: "); Serial.println(errorMessage);
    return;
  }

  update_tvoc_screen((float)co2, temp, rh);
}

void shutdown_system() {

  lv_disp_t* disp = lv_disp_get_default();
  if (disp) {
    lv_disp_clean_dcache(disp);
    lv_disp_remove(disp);
  }

  lcd.setBrightness(0);
  lcd.fillScreen(TFT_BLACK);
  digitalWrite(SYS_EN_PIN, LOW);  // Power off external systems
  digitalWrite(15, LOW);          // Optionally turn off sensor power

  ui_initialized = false;
  system_on = false;
}

void init_ui_and_sensor() {
  if (ui_initialized) return;
  ui_initialized = true;

  digitalWrite(SYS_EN_PIN, HIGH); 

  lcd.init();
  lcd.initDMA();
  lcd.setBrightness(255);
  lcd.setRotation(0);
  lcd.fillScreen(TFT_BLACK);

  lv_init();
  lv_disp_draw_buf_init(&draw_buf, buf1, buf2, sizeof(buf1) / sizeof(lv_color_t));

  static lv_disp_drv_t disp_drv;
  lv_disp_drv_init(&disp_drv);
  disp_drv.hor_res = 240;
  disp_drv.ver_res = 280;
  disp_drv.flush_cb = my_disp_flush;
  disp_drv.draw_buf = &draw_buf;
  lv_disp_drv_register(&disp_drv);

  static lv_indev_drv_t indev_drv;
  lv_indev_drv_init(&indev_drv);
  indev_drv.type = LV_INDEV_TYPE_POINTER;
  indev_drv.read_cb = my_touchpad_read;
  lv_indev_drv_register(&indev_drv);

  uint16_t calib[5];
  lcd.calibrateTouch(calib, TFT_WHITE, TFT_BLACK, 10);

  Wire.begin(11, 10);
  sensor.begin(Wire, SCD41_I2C_ADDR_62);
  delay(30);
  sensor.wakeUp();
  sensor.stopPeriodicMeasurement();
  sensor.reinit();
  sensor.startPeriodicMeasurement();

  uint64_t serialNumber = 0;
  error = sensor.getSerialNumber(serialNumber);
  if (error == NO_ERROR) {
    PrintUint64(serialNumber);
    Serial.println();
  } else {
    errorToString(error, errorMessage, sizeof errorMessage);
  }

  ui_init();
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  pinMode(POWER_BUTTON_PIN, INPUT_PULLUP);  // SYS_OUT
  pinMode(SYS_EN_PIN, OUTPUT);              // SYS_EN
  pinMode(15, OUTPUT);                      // Sensor power (if used)
  digitalWrite(15, HIGH);
  digitalWrite(SYS_EN_PIN, LOW);            // Do not power yet
}

unsigned long last_tick = 0;
unsigned long last_sensor_read = 0;

void loop() {
  unsigned long now = millis();

  // UI activation
  static bool button_was_pressed = false;
  bool button_pressed = (digitalRead(POWER_BUTTON_PIN) == LOW);

  if (button_pressed && !button_was_pressed) {
    system_on = !system_on;
    if (system_on) {
      init_ui_and_sensor();
    } else {
      shutdown_system();
    }
  }
  button_was_pressed = button_pressed;


  // LVGL and sensor logic only if UI initialized
  if (ui_initialized) {
    if (now - last_tick >= 1) {
      lv_tick_inc(now - last_tick);
      last_tick = now;
    }

    if (now - last_sensor_read >= 5000) {
      last_sensor_read = now;
      read_scd4x_data(NULL);
    }

    lv_task_handler();
  }
}
