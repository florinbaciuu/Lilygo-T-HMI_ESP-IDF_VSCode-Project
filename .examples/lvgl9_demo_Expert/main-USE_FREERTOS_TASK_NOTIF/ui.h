#include "lvgl.h"
#include "esp_log.h"


lv_obj_t *btn1 = NULL;          // Declarație globală pentru primul buton
lv_obj_t *btn1_label = NULL;    // Declarație globală pentru eticheta primului buton
lv_obj_t *btn2 = NULL;          // Declarație globală pentru al doilea buton
lv_obj_t *btn2_label = NULL;    // Declarație globală pentru eticheta celui de-al doilea buton
lv_obj_t *tab3_label = NULL;    // Declarație globală pentru etichetă
lv_obj_t *slider_tab4 = NULL;        // Declarație globală pentru slider
lv_obj_t *slider_tab4_label = NULL;  // Declarație globală pentru eticheta slider-ului

// Callback pentru primul buton
static void btn1_event_cb(lv_event_t *e) {
  if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
    printf("Hello World!\n");
  }
}

// Callback pentru al doilea buton
static void btn2_event_cb(lv_event_t *e) {
  if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
    printf("Hello Pople!\n");
  }
}

static void slider_event_cb(lv_event_t *e) {
  lv_obj_t *slider = lv_event_get_target_obj(e);

  /*Refresh the text*/
  lv_label_set_text_fmt(slider_tab4_label, "%" LV_PRId32, lv_slider_get_value(slider));
  lv_obj_align_to(slider_tab4_label, slider, LV_ALIGN_OUT_TOP_MID, 0, -15); /*Align top of the slider*/
}

void create_tabs_ui(void) {
  // Creăm containerul de taburi
  lv_obj_t *tabview = lv_tabview_create(lv_screen_active());
  lv_tabview_set_tab_bar_size(tabview, 40);            // Setăm înălțimea tab-urilor
  lv_obj_set_size(tabview, LV_PCT(100), LV_PCT(100));  // Setăm dimensiunea tabview-ului
  lv_obj_set_flex_flow(tabview, LV_FLEX_FLOW_COLUMN);  // Setăm flex flow pentru tabview
  lv_obj_set_flex_grow(tabview, 1);                    // Permitem tabview-ului să ocupe tot spațiul disponibil
  lv_dir_t dir = LV_DIR_TOP;                           // Poziționăm tab-urile în partea de sus
  lv_tabview_set_tab_bar_position(tabview, dir);       // Funcția nu există în LVGL, deci comentăm această linie
  /*Adăugăm două taburi*/
  lv_obj_t *tab1 = lv_tabview_add_tab(tabview, "Tab 1");
  lv_obj_t *tab2 = lv_tabview_add_tab(tabview, "Tab 2");
  lv_obj_t *tab3 = lv_tabview_add_tab(tabview, "Tab 3");
  lv_obj_t *tab4 = lv_tabview_add_tab(tabview, "Tab 4");
  lv_obj_t *tab5 = lv_tabview_add_tab(tabview, "Tab 5");

  btn1 = lv_button_create(tab1);  // Buton în primul tab
  lv_obj_center(btn1);
  lv_obj_add_event_cb(btn1, btn1_event_cb, LV_EVENT_CLICKED, NULL);
  btn1_label = lv_label_create(btn1);
  lv_label_set_text(btn1_label, "Hello World");
  lv_obj_center(btn1_label);

  btn2 = lv_button_create(tab2);  // Buton în al doilea tab
  lv_obj_center(btn2);
  lv_obj_add_event_cb(btn2, btn2_event_cb, LV_EVENT_CLICKED, NULL);
  btn2_label = lv_label_create(btn2);
  lv_label_set_text(btn2_label, "Hello Pople");
  lv_obj_center(btn2_label);

  tab3_label = lv_label_create(tab3); /*Create a white label, set its text and align it to the center*/
  lv_label_set_text(tab3_label, "Hello world");
  lv_obj_set_style_text_color(tab3_label, lv_color_hex(0xffffff), LV_PART_MAIN);
  lv_obj_align(tab3_label, LV_ALIGN_CENTER, 0, 0);

  slider_tab4 = lv_slider_create(tab4);                                            /*Create a slider in the center of the display*/
  lv_obj_set_width(slider_tab4, 200);                                              /*Set the width*/
  lv_obj_center(slider_tab4);                                                      /*Align to the center of the parent (screen)*/
  lv_obj_add_event_cb(slider_tab4, slider_event_cb, LV_EVENT_VALUE_CHANGED, NULL); /*Assign an event function*/
  slider_tab4_label = lv_label_create(tab4);                                       /*Create a label above the slider*/
  lv_label_set_text(slider_tab4_label, "0");
  lv_obj_align_to(slider_tab4_label, slider_tab4, LV_ALIGN_OUT_TOP_MID, 0, -15); /*Align top of the slider*/
}
