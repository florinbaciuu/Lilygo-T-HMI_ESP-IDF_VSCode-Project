#include "lvgl.h"
#include "esp_log.h"

static const char *TAG = "UI";

// Callback pentru primul buton
static void btn1_event_cb(lv_event_t *e)
{
    if (lv_event_get_code(e) == LV_EVENT_CLICKED)
        ESP_LOGI(TAG, "Hello World!");
}

// Callback pentru al doilea buton
static void btn2_event_cb(lv_event_t *e)
{
    if (lv_event_get_code(e) == LV_EVENT_CLICKED)
        ESP_LOGI(TAG, "Hello Pople!");
}

void create_tabs_ui(void)
{
    // Creăm containerul de taburi
    lv_obj_t *tabview = lv_tabview_create(lv_screen_active());

    // Adăugăm două taburi
    lv_obj_t *tab1 = lv_tabview_add_tab(tabview, "Tab 1");
    lv_obj_t *tab2 = lv_tabview_add_tab(tabview, "Tab 2");

    // Buton în primul tab
    lv_obj_t *btn1 = lv_button_create(tab1);
    lv_obj_center(btn1);
    lv_obj_add_event_cb(btn1, btn1_event_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t *label1 = lv_label_create(btn1);
    lv_label_set_text(label1, "Hello World");
    lv_obj_center(label1);

    // Buton în al doilea tab
    lv_obj_t *btn2 = lv_button_create(tab2);
    lv_obj_center(btn2);
    lv_obj_add_event_cb(btn2, btn2_event_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t *label2 = lv_label_create(btn2);
    lv_label_set_text(label2, "Hello Pople");
    lv_obj_center(label2);
}
