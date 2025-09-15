
#include "argtable3/argtable3.h"
#include "esp_console.h"
#include "esp_log.h"
#include "set_log.h"
#include <string.h>

static const char *TAG = "CLI";

static struct {
    struct arg_str* subcommand;
    struct arg_lit* list; // <-- opțiunea nouă
    struct arg_lit* help; // ⬅️ NOU
    struct arg_end* end;
} set_args;

// old
// typedef void (*info_func_t)(void);
// typedef struct {
//     const char* name;
//     void (*function)(void);
//     const char* description;
// } info_command_entry_t;

// new
typedef void (*info_func_t)(int argc, char** argv);
typedef struct {
    const char* name;
    info_func_t function;
    const char* description;
} info_command_entry_t;

void set_log_cmd(int argc, char** argv) {
    if (argc <= 2 || argc < 4) {
        printf("Usage: set log <tag> <level>\n");
        return;
    }
    // argv[0] = "set"
    // argv[1] = "log"
    // argv[2] = <tag>
    // argv[3] = <level>
    log_level(argc - 2, &argv[2]);
}

static const info_command_entry_t set_cmds[] = {
    {"log", set_log_cmd, "💥 Set log level for all tags or a specific tag."},
};

void printSetCommandList() {
    printf("╔═══════════════════════════ AVAILABLE  SET  COMMANDS ═══════════════════════════╗\n");
    printf("║ %-10s │ %-65s ║\n", "Command", "Description");
    printf("╟────────────┼──────────────────────────────────────────────────────────────────╢\n");
    for (size_t i = 0; i < sizeof(set_cmds) / sizeof(set_cmds[0]); ++i) {
        printf("║ %-10s │ %-60s ║\n", set_cmds[i].name,
            set_cmds[i].description ? set_cmds[i].description : "No description");
    }
    printf("╚══════════════════════════════════════════════════════════════════════════════╝\n");
}

#define INFO_SET_COUNT (sizeof(set_cmds) / sizeof(set_cmds[0]))

static char set_cmds_help[128] = {0};

static void generate_set_cmds_help_text(void) {
    strcpy(set_cmds_help, ":   ");
    for (size_t i = 0; i < INFO_SET_COUNT; i++) {
        strcat(set_cmds_help, set_cmds[i].name);
        if (i < INFO_SET_COUNT - 1)
            strcat(set_cmds_help, "; ");
    }
}

static int set_command(int argc, char** argv) {
    int nerrors = arg_parse(argc, argv, (void**)&set_args);
    // Dacă nu are niciun argument sau a cerut help global
    if (argc == 1 || set_args.help->count > 0) {
        printf(
            "╔══════════════════════════════ SET  COMMAND HELP ═════════════════════════════╗\n");
        printf(
            "║ Usage: set <subcommand> [--help]                                            ║\n");
        printf(
            "║                                                                              ║\n");
        printf("║ Available subcommands:                                                      ║\n");
        for (size_t i = 0; i < INFO_SET_COUNT; i++) {
            printf("║   %-10s - %s\n", set_cmds[i].name, set_cmds[i].description);
        }
        printf(
            "║                                                                              ║\n");
        printf("║ Use 'set <subcommand> --help' for more information.                        ║\n");
        printf(
            "╚══════════════════════════════════════════════════════════════════════════════╝\n");
        return 0;
    }
    // Afișare listă dacă s-a cerut explicit
    if (set_args.list->count > 0) {
        printSetCommandList();
        return 0;
    }
    if (nerrors != 0) {
        arg_print_errors(stderr, set_args.end, argv[0]);
        return 1;
    }
    // Verificare subcommand valid
    if (!set_args.subcommand || set_args.subcommand->count == 0 || !set_args.subcommand->sval[0]) {
        printf("No subcommand provided. Use `set --help`.\n");
        return 1;
    }
    const char* subcommand = set_args.subcommand->sval[0];
    size_t num_cmds = sizeof(set_cmds) / sizeof(set_cmds[0]);
    for (size_t i = 0; i < num_cmds; ++i) {
        if (strcmp(subcommand, set_cmds[i].name) == 0) {
            if (argc > 2 && (strcmp(argv[2], "--help") == 0 || strcmp(argv[2], "-h") == 0)) {
                printf("Help for '%s': %s\n", set_cmds[i].name, set_cmds[i].description);
                return 0;
            }
            set_cmds[i].function(argc, argv);
            return 0;
        }
    }
    printf("Unknown subcommand: %s\n", subcommand);
    printf("Type `set --list` to see available subcommands.\n");
    return 1;
}

void cli_register_set_command(void) {
    generate_set_cmds_help_text();
    set_args.subcommand = arg_str1(NULL, // nu are flag scurt, gen `-s
        NULL,                            // nu are flag lung, gen `--subcmd`
        "<subcommand>",                  // numele argumentului (pentru help/usage)
        set_cmds_help);                  // descrierea lui
    set_args.list = arg_lit0("l", "list", "List all available subcommands");
    set_args.help = arg_lit0("h", "help", "Show help for 'info' command");
    set_args.end = arg_end(1);

    const esp_console_cmd_t cmd = {
        .command = "set",
        .help = "Set",
        .hint = NULL,
        .func = &set_command,
        .argtable = &set_args,
    };

    ESP_ERROR_CHECK(esp_console_cmd_register(&cmd));
    ESP_LOGI(TAG, "'%s' command registered.", cmd.command);
    return;
}

