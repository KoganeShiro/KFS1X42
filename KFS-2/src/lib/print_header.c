#include "kernel.h"


void print_42_header(screen *scr) 
{
    int color_index = 0;
    int default_color = scr->color;
    uint8_t rainbow[] = {
        VGA_COLOR_RED,
        VGA_COLOR_LIGHT_BROWN,
        VGA_COLOR_LIGHT_GREEN,
        VGA_COLOR_LIGHT_CYAN,
        VGA_COLOR_LIGHT_BLUE,
        VGA_COLOR_LIGHT_MAGENTA
    };
    
    printk(scr, "********************************************************************************");
    kfs_set_color(scr, vga_entry_color(rainbow[color_index++], VGA_COLOR_BLACK));
    printk(scr, "        _  _  ____                               \n");
    printk(scr, "       | || ||___ \\                              \n");
    kfs_set_color(scr, vga_entry_color(rainbow[color_index++], VGA_COLOR_BLACK));
    printk(scr, "       | || |_ __) |                             \n");
    printk(scr, "       |__   _/ __/  _                           \n");
    kfs_set_color(scr, vga_entry_color(rainbow[color_index++], VGA_COLOR_BLACK));
    printk(scr, "       __ |_||_____|| | ___ ___  _ __ ___   ___  \n");
    printk(scr, "       \\ \\ /\\ / / _ \\ |/ __/ _ \\| '_ ` _ \\ / _ \\ \n");
    kfs_set_color(scr, vga_entry_color(rainbow[color_index++], VGA_COLOR_BLACK));
    printk(scr, "        \\ V  V /  __/ | (_| (_) | | | | | |  __/ \n");
    printk(scr, "        _\\_/\\_/ \\___|_|\\___\\___/|_| |_| |_|\\___| \n");
    kfs_set_color(scr, vga_entry_color(rainbow[color_index++], VGA_COLOR_BLACK));
    printk(scr, "       | |_ ___   | | __/ _|___      / |         \n");
    printk(scr, "       | __/ _ \\  | |/ / |_/ __|_____| |         \n");
    kfs_set_color(scr, vga_entry_color(rainbow[color_index++], VGA_COLOR_BLACK));
    printk(scr, "       | || (_) | |   <|  _\\__ \\_____| |         \n");
    printk(scr, "        \\__\\___/  |_|\\_\\_| |___/     |_|         \n\n");
    kfs_set_color(scr, default_color);
    printk(scr, "********************************************************************************");
    printk(scr, "You are on the %i screen\n\n", scr->screen_id);
    
    return;
}


void print_header(screen *scr) 
{
    int default_color = scr->color;

    printk(scr, "  _   __            _            __                     \n");
    printk(scr, " | | / /           | |          / _|                    \n");
    printk(scr, " | |/ /  ___   __ _| | __ _    | |_ _ __ ___  _ __ ___  \n");
    printk(scr, " |    \\ / _ \\ / _` | |/ _` |   |  _| '__/ _ \\| '_ ` _ \\ \n");
    printk(scr, " | |\\  \\ (_) | (_| | | (_| |   | | | | | (_) | | | | | |\n");
    printk(scr, " \\_| \\_/\\___/ \\__,_|_|\\__,_|   |_| |_|  \\___/|_| |_| |_|\n");
    printk(scr, "         _____       _                     \n");
    printk(scr, "        /  ___|     | |                    \n");
    printk(scr, "        \\ `--.  __ _| |__   __ _ _ __ __ _ \n");
    printk(scr, "         `--. \\/ _` | '_ \\ / _` | '__/ _` |\n");
    printk(scr, "        /\\__/ / (_| | | | | (_| | | | (_| |\n");
    printk(scr, "        \\____/ \\__,_|_| |_|\\__,_|_|  \\__,_|\n");
    printk(scr, "\n");
    // The Koala (Standard ASCII, will display perfectly)
    printk(scr, "                  /'--.._ `'-=\"\"\"=-'` _..--'\\\n");
    printk(scr, "                  |   ~. )  _     _  ( .~   |\n");
    printk(scr, "                   \\  '~/   a  _  a   \\~'  /\n");
    printk(scr, "                    \\  `|     / \\     |`  /\n");
    printk(scr, "                     `'--\\    \\_/    /--'`\n");
    printk(scr, "                         .'._  J__.-'.\n");
    printk(scr, "                        / /  '-/_ `-  \\\n");
    printk(scr, "                       / -\"-'-.  '-.__/\n");
    printk(scr, "                       \\__,-.\\/     | `\\        Very lost, but still living\n");
    printk(scr, "                       /  ;---.  .--'   |\n");
    printk(scr, "                      |     /\\'-'      /\n");
    printk(scr, "                  jgs '.___.\\   _.--;'`)\n");
    printk(scr, "'~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
    printk(scr, "You are on the %i screen\n\n", scr->screen_id);
    // kfs_set_color(scr, default_color); // Restore color
}


/*ANSI Shadow
██╗  ██╗ ██████╗  █████╗ ██╗      █████╗     ███████╗██████╗  ██████╗ ███╗   ███╗
██║ ██╔╝██╔═══██╗██╔══██╗██║     ██╔══██╗    ██╔════╝██╔══██╗██╔═══██╗████╗ ████║
█████╔╝ ██║   ██║███████║██║     ███████║    █████╗  ██████╔╝██║   ██║██╔████╔██║
██╔═██╗ ██║   ██║██╔══██║██║     ██╔══██║    ██╔══╝  ██╔══██╗██║   ██║██║╚██╔╝██║
██║  ██╗╚██████╔╝██║  ██║███████╗██║  ██║    ██║     ██║  ██║╚██████╔╝██║ ╚═╝ ██║
╚═╝  ╚═╝ ╚═════╝ ╚═╝  ╚═╝╚══════╝╚═╝  ╚═╝    ╚═╝     ╚═╝  ╚═╝ ╚═════╝ ╚═╝     ╚═╝

                ███████╗ █████╗ ██╗  ██╗ █████╗ ██████╗  █████╗ 
                ██╔════╝██╔══██╗██║  ██║██╔══██╗██╔══██╗██╔══██╗
                ███████╗███████║███████║███████║██████╔╝███████║
                ╚════██║██╔══██║██╔══██║██╔══██║██╔══██╗██╔══██║
                ███████║██║  ██║██║  ██║██║  ██║██║  ██║██║  ██║
                ╚══════╝╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝
                                                

                        /'--.._ `'-="""=-'` _..--'\
                        |   ~. )  _     _  ( .~   |
                         \  '~/   a  _  a   \~'  /
                          \  `|     / \     |`  /
                           `'--\    \_/    /--'`
                               .'._  J__.-'.
                              / /  '-/_ `-  \
                             / -"-'-.  '-.__/
                             \__,-.\/     | `\        Very lost, but still living
                             /  ;---.  .--'   |
                            |     /\'-'      /
                        jgs '.___.\   _.--;'`)
'~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~                        


 _   __            _            __                     
| | / /           | |          / _|                    
| |/ /  ___   __ _| | __ _    | |_ _ __ ___  _ __ ___  
|    \ / _ \ / _` | |/ _` |   |  _| '__/ _ \| '_ ` _ \ 
| |\  \ (_) | (_| | | (_| |   | | | | | (_) | | | | | |
\_| \_/\___/ \__,_|_|\__,_|   |_| |_|  \___/|_| |_| |_|
             _____       _                     
            /  ___|     | |                    
            \ `--.  __ _| |__   __ _ _ __ __ _ 
             `--. \/ _` | '_ \ / _` | '__/ _` |
            /\__/ / (_| | | | | (_| | | | (_| |
            \____/ \__,_|_| |_|\__,_|_|  \__,_|
                                               
                                   

*/