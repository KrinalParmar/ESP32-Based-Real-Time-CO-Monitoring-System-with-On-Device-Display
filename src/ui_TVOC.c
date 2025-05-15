#include "ui.h"
#include <stdio.h>

void update_tvoc_screen(float co2_value, float temperature, float humidity) {
    
    if (!ui_Value || !ui_Arc || !ui_TempValue || !ui_HumidValue) {
        printf("Some LVGL objects are NULL!\n");
    }
    // Limit CO2 value to max 5000
    if (co2_value > 5000) co2_value = 5000;

    // Update the arc based on CO2 value
    lv_arc_set_value(ui_Arc, co2_value);

    // Update the label showing the CO2 value
    char co2_text[16];
    snprintf(co2_text, sizeof(co2_text), "%.0f", co2_value);
    lv_label_set_text(ui_Value, co2_text);

    // Update temperature label
    char temp_text[16];
    snprintf(temp_text, sizeof(temp_text), "%.1f °C", temperature);
    lv_label_set_text(ui_TempValue, temp_text);

    // Update humidity label
    char humid_text[16];
    snprintf(humid_text, sizeof(humid_text), "%.1f %%", humidity);
    lv_label_set_text(ui_HumidValue, humid_text);
}

void ui_TVOC_screen_init(void) {
    ui_TVOC = lv_obj_create(NULL);
    lv_obj_clear_flag(ui_TVOC, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(ui_TVOC, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_TVOC, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_Container1 = lv_obj_create(ui_TVOC);
    lv_obj_remove_style_all(ui_Container1);
    lv_obj_set_size(ui_Container1, 240, 280);
    lv_obj_set_align(ui_Container1, LV_ALIGN_CENTER);
    lv_obj_clear_flag(ui_Container1, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);

    ui_Arc = lv_arc_create(ui_TVOC);
    lv_obj_set_size(ui_Arc, 200, 200);
    lv_obj_set_pos(ui_Arc, 0, 20);
    lv_obj_set_align(ui_Arc, LV_ALIGN_CENTER);
    lv_arc_set_value(ui_Arc, 0);
    lv_arc_set_range(ui_Arc, 0, 5000);  // <-- IMPORTANT: set arc max to 5000
    lv_arc_set_bg_angles(ui_Arc, 90, 10);

    lv_obj_set_style_arc_color(ui_Arc, lv_color_hex(0xA0FCAC), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_arc_opa(ui_Arc, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_arc_color(ui_Arc, lv_color_hex(0x30F836), LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_arc_opa(ui_Arc, 255, LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_Arc, lv_color_hex(0x00DE16), LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_Arc, 255, LV_PART_KNOB | LV_STATE_DEFAULT);

    ui_Value = lv_label_create(ui_TVOC);
    lv_obj_set_pos(ui_Value, 0, 20);
    lv_obj_set_align(ui_Value, LV_ALIGN_CENTER);
    lv_label_set_text(ui_Value, "0");
    lv_obj_set_style_text_color(ui_Value, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_Value, &lv_font_montserrat_24, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_CO2 = lv_label_create(ui_TVOC);
    lv_obj_set_pos(ui_CO2, 63, 69);
    lv_obj_set_align(ui_CO2, LV_ALIGN_CENTER);
    lv_label_set_text(ui_CO2, "CO2");
    lv_obj_set_style_text_color(ui_CO2, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_CO2, &lv_font_montserrat_20, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_CO2Unit = lv_label_create(ui_TVOC);
    lv_obj_set_pos(ui_CO2Unit, 63, 92);
    lv_obj_set_align(ui_CO2Unit, LV_ALIGN_CENTER);
    lv_label_set_text(ui_CO2Unit, "PPM");
    lv_obj_set_style_text_color(ui_CO2Unit, lv_color_hex(0x43FD54), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_CO2Unit, &lv_font_montserrat_16, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_Temperature = lv_label_create(ui_TVOC);
    lv_obj_set_pos(ui_Temperature, -68, -100);
    lv_obj_set_align(ui_Temperature, LV_ALIGN_CENTER);
    lv_label_set_text(ui_Temperature, "Temperature");
    lv_obj_set_style_text_color(ui_Temperature, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_Temperature, &lv_font_montserrat_12, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_Humidity = lv_label_create(ui_TVOC);
    lv_obj_set_pos(ui_Humidity, 68, -100);
    lv_obj_set_align(ui_Humidity, LV_ALIGN_CENTER);
    lv_label_set_text(ui_Humidity, "Humidity");
    lv_obj_set_style_text_color(ui_Humidity, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_Humidity, &lv_font_montserrat_12, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_TempValue = lv_label_create(ui_TVOC);
    lv_obj_set_pos(ui_TempValue, -70, -123);
    lv_obj_set_align(ui_TempValue, LV_ALIGN_CENTER);
    lv_label_set_text(ui_TempValue, "0.0 °C");
    lv_obj_set_style_text_color(ui_TempValue, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_TempValue, &lv_font_montserrat_12, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_HumidValue = lv_label_create(ui_TVOC);
    lv_obj_set_pos(ui_HumidValue, 70, -123);
    lv_obj_set_align(ui_HumidValue, LV_ALIGN_CENTER);
    lv_label_set_text(ui_HumidValue, "0.0 %");
    lv_obj_set_style_text_color(ui_HumidValue, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_HumidValue, &lv_font_montserrat_12, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Events from SquareLine (optional)
    lv_obj_add_event_cb(ui_Arc, ui_event_Arc, LV_EVENT_ALL, NULL);
    lv_obj_add_event_cb(ui_Value, ui_event_Value, LV_EVENT_ALL, NULL);
    lv_obj_add_event_cb(ui_TVOC, ui_event_TVOC, LV_EVENT_ALL, NULL);
}
