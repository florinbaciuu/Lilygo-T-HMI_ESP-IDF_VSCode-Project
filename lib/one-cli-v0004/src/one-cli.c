/***
 * Code
 */

// include/command_line_interface.h
#include "one-cli.h"
#include "modules.h"

static const char* TAG = "CLI";

// -------------------------------------------------

void cli_register_all_commands(void) {
    esp_console_register_help_command(); // asta efunctia predefinita a esp -idf.. aici trebuie
                                         // lucrat in contionuare
    cli_register_hello_command();
    cli_register_restart_command();
    cli_register_tasks_command(); // new command
    //// cli_register_tasks_info_command();
    cli_register_uptime_command();
    cli_register_info_command();
    //// cli_register_nsv_command(); // TODO de implementat cum trebuie
    cli_register_WiFi_join_command();
    cli_register_set_command();
    cli_register_perfmon_command();
    return;
}

// -------------------------------------------------

static const char* florios_banner =
    " ███████████ ████                      ███     ███████     █████████ \n"
    "░░███░░░░░░█░░███                     ░░░    ███░░░░░███  ███░░░░░███\n"
    " ░███   █ ░  ░███   ██████  ████████  ████  ███     ░░███░███    ░░░ \n"
    " ░███████    ░███  ███░░███░░███░░███░░███ ░███      ░███░░█████████ \n"
    " ░███░░░█    ░███ ░███ ░███ ░███ ░░░  ░███ ░███      ░███ ░░░░░░░░███\n"
    " ░███  ░     ░███ ░███ ░███ ░███      ░███ ░░███     ███  ███    ░███\n"
    " █████       █████░░██████  █████     █████ ░░░███████░  ░░█████████ \n"
    "░░░░░       ░░░░░  ░░░░░░  ░░░░░     ░░░░░    ░░░░░░░     ░░░░░░░░░  \n";

TaskHandle_t xHandle__CLI;

void printStartupMessage() {
    // printf("\n%s\n", florios_banner);  // Afișează blazonul
    printf("\033[1;34m%s\033[0m\n", florios_banner); // albastru intens

    printf(
        "\n"
        "╔═════════════════════════════════════════════════════════════════╗\n"
        "║                    🔧 FloriOS Console Online 🔧                 ║\n"
        "╠═════════════════════════════════════════════════════════════════╣\n"
        "║  🕹️  Type 'help'           →  List all available commands        ║\n"
        "║  🔁  Use ↑ / ↓            →  Navigate command history           ║\n"
        "║  ⚡  Press [TAB]          →  Auto-complete command names        ║\n"
        "║  💣  Ctrl + C             →  Exit the console (if you dare)     ║\n"
        "╚═════════════════════════════════════════════════════════════════╝\n"
        "\n");
    printf("Made by Florin Baciu.\n");
    if (linenoiseIsDumbMode())
    {
        printf(
            "⚠️  Terminal dumb mode detected!\n"
            "Line editing and history are disabled.\n"
            "💡 Try using a better terminal (like PuTTY or Minicom).\n\n");
    } else
    {
        printf("🧠 Terminal capabilities: FULLY ENABLED\n\n");
    }
    printf("🚀 Welcome, Commander. System is ready for input.\n");
    printf("💭 Remember: even the most powerful systems wait for a single command...\n");
    return; 
}

// -------------------------------
// Variabilă globală pentru path-ul istoriei
static char s_history_path[64] = MOUNT_PATH "/history.txt";

void cli_set_history_path(const char* path) {
    if (path == NULL)
    {
        ESP_LOGW(TAG, "No history path provided, using default: %s", s_history_path);
        return;
    }
    strncpy(s_history_path, path, sizeof(s_history_path) - 1);
    s_history_path[sizeof(s_history_path) - 1] = '\0';
    ESP_LOGI(TAG, "History path set to: %s", s_history_path);
}

void rtos_init_cli() {
    /* Initialize console output periheral (UART, USB_OTG, USB_JTAG) */
    initialize_console_peripheral();

    /* Initialize linenoise library and esp_console*/
    initialize_console_library(s_history_path);

    /* Prompt to be printed before each line.
     * This can be customized, made dynamic, etc.
     */
    const char* prompt = setup_prompt(PROMPT_STR ">");
    vTaskDelay(10);

    /* Register commands */
    // esp_console_register_help_command();
    // register_system_common();
    // cli_register_custom_help_command(); // aici am modificat ultima pt tine alua sa vezi
    cli_register_all_commands(); // my command
    return;
}

/********************************************** */
/*                   TASK                       */
/********************************************** */
void console_app(void* parameter) {
    (void) parameter;
    // vTaskDelay(300);
    rtos_init_cli();
    // vTaskDelay(300);
    printStartupMessage();

    while (true)
    {
        char* line = linenoise(prompt);

#if CONFIG_CONSOLE_IGNORE_EMPTY_LINES
        if (line == NULL)
        { /* Ignore empty lines */
            continue;
            ;
        }
#else
        if (line == NULL)
        { /* Break on EOF or error */
            break;
        }
#endif // CONFIG_CONSOLE_IGNORE_EMPTY_LINES

        /* Add the command to the history if not empty*/
        if (strlen(line) > 0)
        {
            linenoiseHistoryAdd(line);
#if CONFIG_CONSOLE_STORE_HISTORY
            /* Save command history to filesystem */
            if (s_history_path[0] != '\0')
            { // avem path valid
                linenoiseHistorySave(s_history_path);
            }
#endif // CONFIG_CONSOLE_STORE_HISTORY
        }

        /* Try to run the command */
        int       ret;
        esp_err_t err = esp_console_run(line, &ret);
        if (err == ESP_ERR_NOT_FOUND)
        {
            printf("Unrecognized command\n");
        } else if (err == ESP_ERR_INVALID_ARG)
        {
            // command was empty
        } else if (err == ESP_OK && ret != ESP_OK)
        {
            printf("Command returned non-zero error code: 0x%x (%s)\n", ret, esp_err_to_name(ret));
        } else if (err != ESP_OK)
        { printf("Internal error: %s\n", esp_err_to_name(err)); }
        /* linenoise allocates line buffer on the heap, so need to free it */
        linenoiseFree(line);
    }

    ESP_LOGE(TAG, "Error or end-of-input, terminating console");
    esp_console_deinit();
    return;
}

// -------------------------------

void StartCLI() {
    if (xHandle__CLI == NULL)
    {
        xTaskCreatePinnedToCore(console_app,        // Functia care ruleaza task-ul
            (const char*) "CLI Task",               // Numele task-ului
            (uint32_t) (8192),                     // Dimensiunea stack-ului
            (NULL),                                 // Parametri
            (UBaseType_t) configMAX_PRIORITIES - 7, // Prioritatea task-ului
            &xHandle__CLI,                          // Handle-ul task-ului
            ((0))                                   // Nucleul pe care ruleaza (ESP32 e dual-core)
        );
    }
}