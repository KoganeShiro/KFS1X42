#include "kernel.h"

/*
--> if the user write shell, add $> in each line and wait for the user commands
--> when the user type exit, go back
// parse command

when the user type something, record the input
TODO strcmp

clear
--> re init the cursor at the top and clean the vga buffer (call clear_screen())

header
--> print the header (call print_header())

shutdown
--> shutdown the system (call kfs_shutdown())

reboot
--> reboot the system (call kfs_reboot())

*/


void command_parser(screen *scr)
{
    // Get the command from the cmdline buffer
    if (scr->shell_mode == false) {
        if (strcmp(scr->cmdline_buffer, "shell") == 0) {
            shell_prompt(scr);
            scr->shell_mode = true;
        }
    }
    else {
        shell_prompt(scr);
        // Compare the command and execute corresponding function
        // switch (expression)
        // {
        // case constant expression:
        //     /* code */
        //     break;
        
        // default:
        //     break;
        // }
        if (strcmp(scr->cmdline_buffer, "clear") == 0) {
            clear_terminal(scr);
        } else if (strcmp(scr->cmdline_buffer, "header") == 0) {
            print_header(scr);
        } else if (strcmp(scr->cmdline_buffer, "help") == 0) {
            printk(scr, LOG_LEVEL_INFO HELP_MESSAGE);
        } else if (strcmp(scr->cmdline_buffer, "shutdown") == 0) {
            kfs_shutdown(scr);
        } else if (strcmp(scr->cmdline_buffer, "reboot") == 0) {
            kfs_reboot(scr);
        } else if (strcmp(scr->cmdline_buffer, "exit") == 0) {
            scr->shell_mode = false;
            printk(scr, LOG_LEVEL_INFO "Exiting shell mode ( ˶°ㅁ°) !! \n");
        } else {
            printk(scr, LOG_LEVEL_ERROR "Unknown command: %s\n", scr->cmdline_buffer);
        }
        shell_prompt(scr);
    }
    for (size_t i = 0; i < scr->cmdline_index; i++) {
        scr->cmdline_buffer[i] = '\0';
    }
    scr->cmdline_index = 0;
}

