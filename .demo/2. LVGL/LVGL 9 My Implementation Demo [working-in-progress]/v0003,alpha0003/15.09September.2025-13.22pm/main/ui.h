
// made with LVGL.\n"
// by Florin Baciu, 2025\n"

// Includes
#include "esp_log.h"
#include "esp_sleep.h"
#include "lvgl.h"
#include "esp_timer.h"

// --- Variabile pentru drift monitor ---
static lv_obj_t* label_drift = NULL;

static void lv_drift_timer_cb(lv_timer_t* timer) {
    static int64_t  real0_us = 0;
    static uint32_t lv0_ms   = 0;
    static char     last_buf[64];

    if (real0_us == 0) {
        real0_us = esp_timer_get_time();
        lv0_ms   = lv_tick_get();
        strcpy(last_buf, "");  // golim ultima valoare
        return;
    }

    int64_t  real_ms = (esp_timer_get_time() - real0_us) / 1000;
    uint32_t lv_ms   = lv_tick_get() - lv0_ms;

    char buf[64];
    snprintf(buf, sizeof(buf), "Real: %lld ms\nLVGL: %lu ms\nDrift: %ld ms", (long long) real_ms, (unsigned long) lv_ms, (long) (lv_ms - real_ms));

    if (strcmp(buf, last_buf) != 0) {
        strcpy(last_buf, buf);
        lv_label_set_text(label_drift, buf);
    }
}

lv_obj_t* btn1              = NULL;  // Declarație globală pentru primul buton
lv_obj_t* btn1_label        = NULL;  // Declarație globală pentru eticheta primului buton
lv_obj_t* btn3              = NULL;  // Declarație globală pentru al treilea buton
lv_obj_t* btn3_label        = NULL;  // Declarație globală pentru eticheta celui de-al treilea buton
lv_obj_t* tab3_label        = NULL;  // Declarație globală pentru etichetă
lv_obj_t* slider_tab4       = NULL;  // Declarație globală pentru slider
lv_obj_t* slider_tab4_label = NULL;  // Declarație globală pentru eticheta slider-ului

// Callback pentru primul buton
static void btn1_event_cb(lv_event_t* e) {
    if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
        printf("Hello World!\n");
    }
}

void btn2_event_cb(lv_event_t* e) {
    ESP_LOGI("UI", "Butonul Light Sleep apăsat");
    esp_light_sleep_start();
}

// Callback pentru al treilea buton
static void btn3_event_cb(lv_event_t* e) {
    if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
        printf("Hello Pople!\n");
    }
}

static void slider_event_cb(lv_event_t* e) {
    lv_obj_t* slider = lv_event_get_target_obj(e);

    /*Refresh the text*/
    lv_label_set_text_fmt(slider_tab4_label, "%" LV_PRId32, lv_slider_get_value(slider));
    lv_obj_align_to(
        slider_tab4_label, slider, LV_ALIGN_OUT_TOP_MID, 0, -15); /*Align top of the slider*/
}


void create_tabs_ui(void) {
    // Creăm containerul de taburi
    lv_obj_t* tabview = lv_tabview_create(lv_screen_active());
    lv_tabview_set_tab_bar_size(tabview, 40);            // Setăm înălțimea tab-urilor
    lv_obj_set_size(tabview, LV_PCT(100), LV_PCT(100));  // Setăm dimensiunea tabview-ului
    lv_obj_set_flex_flow(tabview, LV_FLEX_FLOW_COLUMN);  // Setăm flex flow pentru tabview
    lv_obj_set_flex_grow(tabview, 1);                    // Permitem tabview-ului să ocupe tot spațiul disponibil
    lv_dir_t dir = LV_DIR_TOP;                           // Poziționăm tab-urile în partea de sus
    lv_tabview_set_tab_bar_position(
        tabview, dir);  // Funcția nu există în LVGL, deci comentăm această linie
    /*Adăugăm două taburi*/
    lv_obj_t* tab1 = lv_tabview_add_tab(tabview, "Tab 1");
    lv_obj_t* tab2 = lv_tabview_add_tab(tabview, "Tab 2");
    lv_obj_t* tab3 = lv_tabview_add_tab(tabview, "Tab 3");
    lv_obj_t* tab4 = lv_tabview_add_tab(tabview, "Tab 4");
    // lv_obj_t *tab5 = lv_tabview_add_tab(tabview, "Tab 5");

    // TAB 1
    btn1 = lv_button_create(tab1);  // Buton în primul tab
    lv_obj_center(btn1);
    lv_obj_add_event_cb(btn1, btn1_event_cb, LV_EVENT_CLICKED, NULL);
    btn1_label = lv_label_create(btn1);
    lv_label_set_text(btn1_label, "Hello World");
    lv_obj_center(btn1_label);

    lv_obj_t* btn2 = lv_button_create(tab1);  // Al doilea buton - Light Sleep
    lv_obj_add_event_cb(btn2, btn2_event_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t* btn2_label = lv_label_create(btn2);
    lv_label_set_text(btn2_label, "Light Sleep");
    lv_obj_center(btn2_label);
    lv_obj_align_to(btn2, btn1, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);  // 10 px sub btn1

    // TAB 2
    btn3 = lv_button_create(tab2);  // Buton în al doilea tab
    lv_obj_center(btn3);
    lv_obj_add_event_cb(btn3, btn3_event_cb, LV_EVENT_CLICKED, NULL);
    btn3_label = lv_label_create(btn3);
    lv_label_set_text(btn3_label, "Hello Pople");
    lv_obj_center(btn3_label);

    // TAB 3
    tab3_label = lv_label_create(tab3);
    lv_label_set_text(tab3_label, "Drift monitor:");
    lv_obj_align(tab3_label, LV_ALIGN_TOP_LEFT, 5, 5);

    label_drift = lv_label_create(tab3);
    lv_obj_align(label_drift, LV_ALIGN_TOP_LEFT, 5, 25);
    lv_label_set_text(label_drift, "Calculating...");

    lv_timer_create(lv_drift_timer_cb, 500, NULL);  // update la 500 ms

    // TAB 4
    slider_tab4 = lv_slider_create(tab4); /*Create a slider in the center of the display*/
    lv_obj_set_width(slider_tab4, 200);   /*Set the width*/
    lv_obj_center(slider_tab4);           /*Align to the center of the parent (screen)*/
    lv_obj_add_event_cb(
        slider_tab4, slider_event_cb, LV_EVENT_VALUE_CHANGED, NULL); /*Assign an event function*/
    slider_tab4_label = lv_label_create(tab4);                       /*Create a label above the slider*/
    lv_label_set_text(slider_tab4_label, "0");
    lv_obj_align_to(
        slider_tab4_label, slider_tab4, LV_ALIGN_OUT_TOP_MID, 0, -15); /*Align top of the slider*/
}
