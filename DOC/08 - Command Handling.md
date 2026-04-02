# 08 — Command Handling

## What is it?
The command system provides a simple shell-like interface to interact with the kernel at runtime. Characters typed by the user are accumulated in a per-screen command-line buffer. When **Enter** is pressed, the buffer is parsed and dispatched through a static dispatch table of `{ name, handler }` pairs.

## How it works

### Input flow (keystroke → command execution)

```
    Keystroke lifecycle
    ┌──────────────────────────────────────────────────────────┐
    │  handle_keyboard_keypress(kfs *k)                        │
    │      │                                                   │
    │      └─► process_scancode(scr, scancode)                 │
    │              │                                           │
    │              ├── scancode == KEY_ENTER ?                  │
    │              │       YES ──► kfs_putchar(scr, '\n')      │
    │              │              command_parser(scr)           │
    │              │                                           │
    │              └── printable character ?                    │
    │                      YES ──► add_char_to_cmdline_buffer()│
    │                              kfs_putchar(scr, c)         │
    └──────────────────────────────────────────────────────────┘
```

Each printable key is appended to `scr->cmdline_buffer` via `add_char_to_cmdline_buffer()`. The buffer is a simple `char[]` of size `VGA_BUFFER_SIZE` with a running index `scr->cmdline_index`, always null-terminated.

### Shell mode state machine

The kernel starts **outside** shell mode. Typing `shell` and pressing Enter activates it.

```
    ┌──────────────────────────────┐
    │       shell_mode = false     │
    │   (only "shell" accepted)    │
    └──────────┬───────────────────┘
               │  user types "shell" + Enter
               ▼
    ┌──────────────────────────────┐
    │       shell_mode = true      │
    │   "$> " prompt displayed     │
    │   all commands available     │
    └──────────┬───────────────────┘
               │  user types "exit" + Enter
               ▼
    ┌──────────────────────────────┐
    │       shell_mode = false     │
    │   back to initial state      │
    └──────────────────────────────┘
```

### Command dispatch table

Commands are stored as a static array of `command` structs:

```c
    typedef struct command {
        const char *name;           // string to match against
        void (*handler)(screen *scr); // function to call
    } command;

    static const command commands[] = {
        { "clear",    cmd_clear    },
        { "help",     cmd_help     },
        { "shutdown", cmd_shutdown },
        { "reboot",   cmd_reboot   },
        { "header",   cmd_header   },
        { "COLOR",    cmd_color    },
        { "RED",      cmd_red      },
        ...
    };
```

All handlers share the same signature `void (*)(screen *scr)`, using thin wrappers when needed:

```c
    static void cmd_clear(screen *scr)  { clear_terminal(scr); }
    static void cmd_reboot(screen *scr) { kfs_reboot(scr); }
```

### Dispatch logic (`command_parser`)

```
    command_parser(scr)
    ┌───────────────────────────────────────────────────────────────┐
    │  if shell_mode == false:                                      │
    │      cmdline_buffer == "shell" ? ──► activate shell mode      │
    │                                      print "$> " prompt      │
    │                                                               │
    │  if shell_mode == true:                                       │
    │      cmdline_buffer == "exit" ?  ──► deactivate shell mode    │
    │      else ──► linear search through commands[]:               │
    │               strcmp(cmdline_buffer, commands[i].name)         │
    │               ├── match found   ──► call commands[i].handler  │
    │               └── no match      ──► print error message       │
    │      if still in shell mode ──► print "$> " prompt            │
    │                                                               │
    │  Reset: cmdline_buffer[0] = '\0', cmdline_index = 0           │
    └───────────────────────────────────────────────────────────────┘
```

The lookup is a simple **linear scan** — `O(n)` — acceptable for a small command set.

### Available commands

```
    ┌────────────┬───────────────────────────────────────────────────┐
    │  Command   │  Action                                          │
    ├────────────┼───────────────────────────────────────────────────┤
    │  shell     │  Enter shell mode (only available outside shell)  │
    │  exit      │  Leave shell mode                                │
    │  clear     │  Clear the terminal screen and reset scroll       │
    │  header    │  Print the kernel ASCII art header                │
    │  help      │  Print the list of available commands             │
    │  shutdown  │  Power off via QEMU ACPI (outw 0x604, 0x2000)    │
    │  reboot    │  CPU reset via 8042 controller (outb 0x64, 0xFE) │
    │  COLOR     │  Show available color commands                    │
    │  RED, BLUE │  Change scr->color to the matching VGA color     │
    │  GREEN ... │  (15 color commands + RESET = white)             │
    └────────────┴───────────────────────────────────────────────────┘
```

### Adding a new command

1. Write the implementation function (or a wrapper) with signature `void handler(screen *scr)`.
2. Add an entry `{ "name", handler }` to the `commands[]` array in `command_parser.c`.
3. Optionally update the `HELP_MESSAGE` macro in `kernel.h`.

No other registration is needed — the dispatch table is self-contained.

### Shutdown & Reboot internals

```
    kfs_shutdown()                      kfs_reboot()
    ┌─────────────────────┐             ┌─────────────────────────────┐
    │ outw(0x604, 0x2000) │             │ wait keyboard ctrl ready    │
    │                     │             │   (poll port 0x64 bit 1)    │
    │ QEMU ACPI power off │             │ outb(0x64, 0xFE)           │
    │ (emulator-specific) │             │                             │
    └─────────────────────┘             │ 8042 pulses CPU RESET line  │
                                        └─────────────────────────────┘
```

## Files involved

| File | Role |
|------|------|
| `src/commands/command_parser.c` | Dispatch table, `command_parser()`, wrapper functions |
| `src/commands/handle_commands.c` | `add_char_to_cmdline_buffer()`, `shell_prompt()` |
| `src/commands/clear.c` | `clear_terminal()` — clears VGA buffer and resets state |
| `src/commands/shutdown.c` | `kfs_shutdown()` — QEMU ACPI power off |
| `src/commands/reboot.c` | `kfs_reboot()` — 8042 keyboard controller CPU reset |
| `src/kernel.h` | `HELP_MESSAGE` / `COLOR_MESSAGE` macros, command prototypes |
| `src/drivers/handle_keyboard.c` | Calls `command_parser()` on Enter keypress |
