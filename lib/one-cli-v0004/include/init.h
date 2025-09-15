



#pragma once


#ifdef __cplusplus
extern "C"
{
#endif /* #ifdef __cplusplus */

    void initialize_console_peripheral(void);
    void initialize_console_library(const char *history_path);
    char *setup_prompt(const char *prompt_str);

#ifdef __cplusplus
}
#endif /* #ifdef __cplusplus */