#pragma message "Compiling lv_ui.cpp"

#include "lv_ui.h"

/*  _ __     ______ _       _   _ ___  */
/* | |\ \   / / ___| |     | | | |_ _| */
/* | | \ \ / | |  _| |     | | | || |  */
/* | |__\ V /| |_| | |___  | |_| || |  */
/* |_____\_/  \____|_____|  \___/|___| */

#include "src/font/lv_font.h"
#include "esp_heap_caps.h"
#include "esp_timer.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <lvgl.h>  // put -D LV_CONF_PATH="../../src/lv_conf.h" in build_flags
#include "misc/lv_timer.h"
//----------------------------------------------------------------------------------------------------------------------

extern lv_disp_t *disp;  // declara display dar nu il initializa pentru ca e extern si se initializeaza in main.cpp
/* lvgl objects */

lv_obj_t *millis_label;
lv_obj_t *millis_label2;
lv_obj_t *cpu_temp_label;
lv_obj_t *ram_label;
lv_obj_t *psram_label;
lv_obj_t *btn;

static lv_obj_t * list1;


//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
void button_event_handler(lv_event_t *e) {
  size_t free_heap_before = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);
  
  // Creează un label nou pe ecran (consumă memorie)
  lv_obj_t *new_label = lv_label_create(lv_scr_act());
  lv_label_set_text(new_label, "Memorie consumata!");
  lv_obj_align(new_label, LV_ALIGN_CENTER, 0, 50);

  size_t free_heap_after = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);

  printf("Memorie liberă înainte: %d bytes\n", free_heap_before);
  printf("Memorie liberă după: %d bytes\n", free_heap_after);
  printf("Memorie consumată: %d bytes\n", free_heap_before - free_heap_after);
}
/*                  _        */
/*  _ __ ___   __ _(_)_ __   */
/* | '_ ` _ \ / _` | | '_ \  */
/* | | | | | | (_| | | | | | */
/* |_| |_| |_|\__,_|_|_| |_| */

//----------------------------------------------------------------------------------------------------------------------
static void back_event_handler(lv_event_t * e)
{
    lv_obj_t * obj = lv_event_get_target(e);
    lv_obj_t * menu = (lv_obj_t *)lv_event_get_user_data(e);

    if(lv_menu_back_btn_is_root(menu, obj)) {
        lv_obj_t * mbox1 = lv_msgbox_create(NULL, "Hello", "Root back btn click.", NULL, true);
        lv_obj_center(mbox1);
    }
}
//----------------------------------------------------------------------------------------------------------------------
static void event_handler(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = lv_event_get_target(e);
    if(code == LV_EVENT_CLICKED) {
        //LV_LOG_USER("Clicked: %s", lv_list_get_btn_text(list1, obj));
        printf("Clicked: %s", lv_list_get_btn_text(list1, obj));
    }
}
//----------------------------------------------------------------------------------------------------------------------

/**
 * LVGL MAIN FUNCTION
 * INFO: Aceasta functie se ocupa este functia principala care deseneaza ui dupa incarcarea datelor
 */
void lv_MainUI_Init() {
  lv_theme_t *theme = lv_theme_default_init(disp,                                                               /*Use the DPI, size, etc from this display*/
                                              lv_palette_main(LV_PALETTE_TEAL), lv_palette_main(LV_PALETTE_BLUE), /*Primary and secondary palette*/
                                              true,                                                               /*Light or dark mode*/
                                              &lv_font_montserrat_10);                                            /*Small, normal, large fonts*/
    lv_disp_set_theme(disp, theme);                                                                               /*Assign the theme to the display*/

    lv_obj_t *tabview;
    tabview = lv_tabview_create(lv_scr_act(), LV_DIR_TOP, 35);

    /*Add tabs (the tabs are page (lv_page) and can be scrolled*/
    lv_obj_t *tab1 = lv_tabview_add_tab(tabview, "About Dev"); // TAB 1

    lv_obj_set_style_bg_color(tab1, lv_palette_main(LV_PALETTE_BLUE), LV_PART_SCROLLBAR);
    lv_obj_set_style_bg_color(tab1, lv_palette_main(LV_PALETTE_RED), LV_PART_MAIN);

    lv_obj_t *tab2 = lv_tabview_add_tab(tabview, "WIFI");     // TAB 2
    lv_obj_t *tab3 = lv_tabview_add_tab(tabview, "System");   // TAB 3
    lv_obj_t *tab4 = lv_tabview_add_tab(tabview, "Settings"); // TAB 4
    lv_tabview_set_act(tabview, 1, LV_ANIM_ON);

    // tab1
    //
    //
    //  Crearea unui label pentru a afișa "millis: "
    lv_obj_t *static_label;
    
    static_label = lv_label_create(tab1); // tab1 ca părinte
    lv_label_set_text(static_label, "millis: ");
    lv_obj_align(static_label, LV_ALIGN_TOP_LEFT, 0, 0);                                               // Poziționare în colțul din stânga sus
    lv_obj_set_style_text_font(static_label, &lv_font_montserrat_10, LV_PART_MAIN | LV_STATE_DEFAULT); // Setează font

    // Crearea unui al doilea label pentru a afișa valoarea `millis()`
    millis_label = lv_label_create(tab1);                                                         // tab1 ca părinte
    lv_obj_align(millis_label, LV_ALIGN_TOP_LEFT, 65, 0);                                         // Poziționare lângă textul "millis: "
    lv_obj_set_style_text_font(millis_label, &lv_font_montserrat_10, LV_PART_MAIN | LV_STATE_DEFAULT); // Setează font

    // Crearea unui label pentru a afișa temperatura CPU
    lv_obj_t *temp_static_label;
    temp_static_label = lv_label_create(tab1); // tab1 ca părinte
    lv_label_set_text(temp_static_label, "CPU Temp: ");
    lv_obj_align(temp_static_label, LV_ALIGN_TOP_LEFT, 0, 10);                                              // Poziționare sub "millis: "
    lv_obj_set_style_text_font(temp_static_label, &lv_font_montserrat_10, LV_PART_MAIN | LV_STATE_DEFAULT); // Setează font

    // Crearea unui label pentru valoarea temperaturii CPU
    cpu_temp_label = lv_label_create(tab1);                                                              // tab1 ca părinte
    lv_obj_align(cpu_temp_label, LV_ALIGN_TOP_LEFT, 65, 10);                                             // Poziționare lângă "CPU Temp: "
    lv_obj_set_style_text_font(cpu_temp_label, &lv_font_montserrat_10, LV_PART_MAIN | LV_STATE_DEFAULT); // Setează font

    // Crearea label-urilor pentru afișarea informațiilor despre RAM internă
    lv_obj_t *ram_static_label;
    ram_static_label = lv_label_create(tab1); // tab1 ca părinte
    lv_label_set_text(ram_static_label, "RAM: ");
    lv_obj_align(ram_static_label, LV_ALIGN_TOP_LEFT, 0, 20);                                              // Poziționare sub "CPU Temp: "
    lv_obj_set_style_text_font(ram_static_label, &lv_font_montserrat_10, LV_PART_MAIN | LV_STATE_DEFAULT); // Setează font

    ram_label = lv_label_create(tab1);                                                              // tab1 ca părinte
    lv_obj_align(ram_label, LV_ALIGN_TOP_LEFT, 65, 20);                                             // Poziționare lângă "Free Internal RAM: "
    lv_obj_set_style_text_font(ram_label, &lv_font_montserrat_10, LV_PART_MAIN | LV_STATE_DEFAULT); // Setează font

    // Crearea label-urilor pentru afișarea informațiilor despre PSRAM
    lv_obj_t *psram_static_label;
    psram_static_label = lv_label_create(tab1); // tab1 ca părinte
    lv_label_set_text(psram_static_label, "PSRAM: ");
    lv_obj_align(psram_static_label, LV_ALIGN_TOP_LEFT, 0, 30);                                              // Poziționare sub RAM
    lv_obj_set_style_text_font(psram_static_label, &lv_font_montserrat_10, LV_PART_MAIN | LV_STATE_DEFAULT); // Setează font

    psram_label = lv_label_create(tab1);                                                              // tab1 ca părinte
    lv_obj_align(psram_label, LV_ALIGN_TOP_LEFT, 65, 30);                                             // Poziționare lângă "Free PSRAM: "
    lv_obj_set_style_text_font(psram_label, &lv_font_montserrat_10, LV_PART_MAIN | LV_STATE_DEFAULT); // Setează font

    // tab2
    //
    //
    //  Crearea unui label pentru a afișa "millis: "
    lv_obj_t *static_label2;
    static_label2 = lv_label_create(tab2); // tab1 ca părinte
    lv_label_set_text(static_label2, "millis: ");
    lv_obj_align(static_label2, LV_ALIGN_TOP_LEFT, 0, 0);                                               // Poziționare în colțul din stânga sus
    lv_obj_set_style_text_font(static_label2, &lv_font_montserrat_10, LV_PART_MAIN | LV_STATE_DEFAULT); // Setează font

    // Crearea unui al doilea label pentru a afișa valoarea `millis()`

    millis_label2 = lv_label_create(tab2);                                                         // tab1 ca părinte
    lv_obj_align(millis_label2, LV_ALIGN_TOP_LEFT, 65, 0);                                         // Poziționare lângă textul "millis: "
    lv_obj_set_style_text_font(millis_label2, &lv_font_montserrat_10, LV_PART_MAIN | LV_STATE_DEFAULT); // Setează font

    // tab3
    //
    //
    /*Create a list*/
    list1 = lv_list_create(tab3);
    lv_obj_set_size(list1, 180, 220);
    lv_obj_center(list1);

    /*Add buttons to the list*/
    lv_obj_t * btn;

    lv_list_add_text(list1, "File");
    btn = lv_list_add_btn(list1, LV_SYMBOL_FILE, "New");
    lv_obj_add_event_cb(btn, event_handler, LV_EVENT_CLICKED, NULL);
    btn = lv_list_add_btn(list1, LV_SYMBOL_DIRECTORY, "Open");
    lv_obj_add_event_cb(btn, event_handler, LV_EVENT_CLICKED, NULL);
    btn = lv_list_add_btn(list1, LV_SYMBOL_SAVE, "Save");
    lv_obj_add_event_cb(btn, event_handler, LV_EVENT_CLICKED, NULL);
    btn = lv_list_add_btn(list1, LV_SYMBOL_CLOSE, "Delete");
    lv_obj_add_event_cb(btn, event_handler, LV_EVENT_CLICKED, NULL);
    btn = lv_list_add_btn(list1, LV_SYMBOL_EDIT, "Edit");
    lv_obj_add_event_cb(btn, event_handler, LV_EVENT_CLICKED, NULL);

    lv_list_add_text(list1, "Connectivity");
    btn = lv_list_add_btn(list1, LV_SYMBOL_BLUETOOTH, "Bluetooth");
    lv_obj_add_event_cb(btn, event_handler, LV_EVENT_CLICKED, NULL);
    btn = lv_list_add_btn(list1, LV_SYMBOL_GPS, "Navigation");
    lv_obj_add_event_cb(btn, event_handler, LV_EVENT_CLICKED, NULL);
    btn = lv_list_add_btn(list1, LV_SYMBOL_USB, "USB");
    lv_obj_add_event_cb(btn, event_handler, LV_EVENT_CLICKED, NULL);
    btn = lv_list_add_btn(list1, LV_SYMBOL_BATTERY_FULL, "Battery");
    lv_obj_add_event_cb(btn, event_handler, LV_EVENT_CLICKED, NULL);

    lv_list_add_text(list1, "Exit");
    btn = lv_list_add_btn(list1, LV_SYMBOL_OK, "Apply");
    lv_obj_add_event_cb(btn, event_handler, LV_EVENT_CLICKED, NULL);
    btn = lv_list_add_btn(list1, LV_SYMBOL_CLOSE, "Close");
    lv_obj_add_event_cb(btn, event_handler, LV_EVENT_CLICKED, NULL);

    // tab4
    //
    //
    lv_obj_t *container = lv_obj_create(tab4); // create a container for the menu
    lv_obj_set_size(container, 300, 170); // Ajustează dimensiunea
    lv_obj_set_pos(container, 0, 0);     // Setează poziția pentru a evita suprapunerea
    //
    lv_obj_t *menu = lv_menu_create(container); /*Create a menu object*/
    lv_obj_set_flex_flow(menu, LV_FLEX_FLOW_ROW_WRAP);

    lv_menu_set_mode_root_back_btn(menu, LV_MENU_ROOT_BACK_BTN_ENABLED);
    lv_obj_add_event_cb(menu, back_event_handler, LV_EVENT_CLICKED, menu);
    /*Modify the header*/
    lv_obj_t * back_btn = lv_menu_get_main_header_back_btn(menu);
    lv_obj_t * back_btn_label = lv_label_create(back_btn);
    lv_label_set_text(back_btn_label, "Back");

    lv_obj_t *cont;
    lv_obj_t *label_label;
    /*Create a sub page*/
    /*Create sub pages*/
    lv_obj_t * sub_1_page = lv_menu_page_create(menu, "Page 1");

    cont = lv_menu_cont_create(sub_1_page);
    label_label = lv_label_create(cont);
    lv_label_set_text(label_label, "Hello, I am hiding here");

    lv_obj_t * sub_2_page = lv_menu_page_create(menu, "Page 2");

    cont = lv_menu_cont_create(sub_2_page);
    label_label = lv_label_create(cont);
    lv_label_set_text(label_label, "Hello, I am hiding here");

    lv_obj_t * sub_3_page = lv_menu_page_create(menu, "Page 3");

    cont = lv_menu_cont_create(sub_3_page);
    label_label = lv_label_create(cont);
    lv_label_set_text(label_label, "Hello, I am hiding here");

    /*Create a main page*/
    lv_obj_t * main_page = lv_menu_page_create(menu, NULL);

    cont = lv_menu_cont_create(main_page);
    label_label = lv_label_create(cont);
    lv_label_set_text(label_label, "Item 1 (Click me!)");
    lv_menu_set_load_page_event(menu, cont, sub_1_page);

    cont = lv_menu_cont_create(main_page);
    label_label = lv_label_create(cont);
    lv_label_set_text(label_label, "Item 2 (Click me!)");
    lv_menu_set_load_page_event(menu, cont, sub_2_page);

    cont = lv_menu_cont_create(main_page);
    label_label = lv_label_create(cont);
    lv_label_set_text(label_label, "Item 3 (Click me!)");
    lv_menu_set_load_page_event(menu, cont, sub_3_page);

    lv_menu_set_page(menu, main_page);
}
//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
