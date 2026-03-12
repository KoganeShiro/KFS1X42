#include "kernel.h"


typedef struct command {
    const char *name;
    void (*handler)(screen *scr);
} command;

/* Thin wrappers so every handler has the same signature */
static void cmd_clear(screen *scr)    { clear_terminal(scr); }
static void cmd_header(screen *scr)   { print_header(scr); }
static void cmd_help(screen *scr)     { printk(scr, LOG_LEVEL_INFO HELP_MESSAGE); }
static void cmd_shutdown(screen *scr) { kfs_shutdown(scr); }
static void cmd_reboot(screen *scr)   { kfs_reboot(scr); }
static void cmd_color(screen *scr)    { printk(scr, LOG_LEVEL_INFO COLOR_MESSAGE); }
static void cmd_red(screen *scr)      { scr->color = VGA_COLOR_RED; }
static void cmd_blue(screen *scr)     { scr->color = VGA_COLOR_BLUE; }
static void cmd_green(screen *scr)    { scr->color = VGA_COLOR_GREEN; }
static void cmd_cyan(screen *scr)     { scr->color = VGA_COLOR_CYAN; }
static void cmd_magenta(screen *scr)  { scr->color = VGA_COLOR_MAGENTA; }
static void cmd_brown(screen *scr)    { scr->color = VGA_COLOR_BROWN; }
static void cmd_light_grey(screen *scr)   { scr->color = VGA_COLOR_LIGHT_GREY; }
static void cmd_dark_grey(screen *scr)    { scr->color = VGA_COLOR_DARK_GREY; }
static void cmd_light_blue(screen *scr)   { scr->color = VGA_COLOR_LIGHT_BLUE; }
static void cmd_light_green(screen *scr)  { scr->color = VGA_COLOR_LIGHT_GREEN; }
static void cmd_light_cyan(screen *scr)     { scr->color = VGA_COLOR_LIGHT_CYAN; }
static void cmd_light_red(screen *scr)    { scr->color = VGA_COLOR_LIGHT_RED; }
static void cmd_light_magenta(screen *scr)  { scr->color = VGA_COLOR_LIGHT_MAGENTA; }
static void cmd_light_brown(screen *scr)  { scr->color = VGA_COLOR_LIGHT_BROWN; }
static void cmd_white(screen *scr)    { scr->color = VGA_COLOR_WHITE; }

static const command commands[] = {
    { "clear",    cmd_clear    },
    { "header",   cmd_header   },
    { "help",     cmd_help     },
    { "shutdown", cmd_shutdown },
    { "reboot",   cmd_reboot   },
    { "COLOR",    cmd_color    },
    { "RED",      cmd_red      },
    { "BLUE",     cmd_blue     },
    { "GREEN",    cmd_green    },
    { "CYAN",     cmd_cyan     },
    { "MAGENTA",  cmd_magenta  },
    { "BROWN",    cmd_brown    },
    { "LIGHT_GREY",   cmd_light_grey },
    { "DARK_GREY",    cmd_dark_grey },
    { "LIGHT_BLUE",   cmd_light_blue },
    { "LIGHT_GREEN",  cmd_light_green },
    { "LIGHT_CYAN",   cmd_light_cyan },
    { "LIGHT_RED",    cmd_light_red },
    { "LIGHT_MAGENTA",  cmd_light_magenta },
    { "LIGHT_BROWN",  cmd_light_brown },
    { "WHITE",     cmd_white },
    { "RESET",     cmd_white },
};

#define NUM_COMMANDS (sizeof(commands) / sizeof(commands[0]))


void command_parser(screen *scr)
{
    if (scr->shell_mode == false) {
        if (strcmp(scr->cmdline_buffer, "shell") == 0) {
            shell_prompt(scr);
            scr->shell_mode = true;
        }
    }
    else {
        /* Handle "exit" specially (changes shell state) */
        if (strcmp(scr->cmdline_buffer, "exit") == 0) {
            scr->shell_mode = false;
            printk(scr, LOG_LEVEL_INFO "Exiting shell mode ~ !! \n");
        }
        else {
            /* Dispatch table lookup */
            bool found = false;
            for (size_t i = 0; i < NUM_COMMANDS; i++) {
                if (strcmp(scr->cmdline_buffer, commands[i].name) == 0) {
                    commands[i].handler(scr);
                    found = true;
                    break;
                }
            }
            if (!found)
                printk(scr, LOG_LEVEL_ERROR "Unknown command: %s\n", scr->cmdline_buffer);
        }
        if (scr->shell_mode)
            shell_prompt(scr);
    }

    /* Reset command buffer — just null-terminate and reset index */
    scr->cmdline_buffer[0] = '\0';
    scr->cmdline_index = 0;
}

