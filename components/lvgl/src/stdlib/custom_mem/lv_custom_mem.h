#ifndef LV_CUSTOM_MEM_H
#define LV_CUSTOM_MEM_H

#include <stdio.h>  // Pentru debugging

// Inițializează gestionarea memoriei (nu necesită implementare specifică pentru PSRAM)
void lv_mem_init(void);
void lv_mem_deinit(void);

// Funcții pentru alocare personalizată în PSRAM
void *lv_malloc_core(size_t size);         // Alocare în PSRAM
void *lv_realloc_core(void *p, size_t new_size);  // Realocare în PSRAM
void lv_free_core(void *p);                // Eliberare memorie din PSRAM

// Setează funcțiile personalizate de memorie pentru LVGL
void lvgl_init_custom_memory(void);

#endif // LV_CUSTOM_MEM_H