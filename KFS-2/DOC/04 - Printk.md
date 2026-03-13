# 04 — Printk

## What is it?
`printk` is the kernel's formatted print function, similar to `printf` in userspace C but tailored for kernel use. It supports **variadic arguments**, **format specifiers** (`%d`, `%s`, `%c`, `%x`, `%p`), **log levels** (INFO, DEBUG, WARNING, ERROR) with color coding, and writes directly to the VGA buffer.

## How it works

### Custom `va_list` (no libc)

Since we have no standard library, we implement variadic arguments manually by walking the stack:

```
    Stack after printk("Hello %s, num=%d", "world", 42):

    High address
    ┌──────────────┐
    │     42       │  ◄── 2nd variadic arg (int)
    ├──────────────┤
    │  "world" ptr │  ◄── 1st variadic arg (char*)
    ├──────────────┤
    │  "Hello..." ptr │  ◄── 'data' parameter (last named param)
    ├──────────────┤
    │  scr ptr     │  ◄── 'scr' parameter
    ├──────────────┤
    │  return addr │
    └──────────────┘
    Low address

    va_start(args, data):
        args.ptr = &data + 1  ──► points to "world" ptr

    va_arg(args, const char*):
        reads "world" ptr, advances ptr by sizeof(char*)

    va_arg(args, int):
        reads 42, advances ptr by sizeof(int)
```

The macros:
- `va_start(argptr, last)` → sets `ptr` to the address right after the last named parameter
- `va_arg(argptr, type)` → reads the value at `ptr` as `type`, advances `ptr` by `sizeof(type)`
- `va_end(argptr)` → sets `ptr` to NULL (cleanup)

### Format string parsing

```
    printk(scr, LOG_LEVEL_INFO "val=%d, name=%s\n", 42, "kfs")

    ┌─────────────────────────────────────────────────────────┐
    │  Step 1: parse_log_level()                              │
    │     "[INFO] " detected                                  │
    │     → set color to LIGHT_CYAN                           │
    │     → print "[INFO] "                                   │
    │     → return length (7)                                 │
    ├─────────────────────────────────────────────────────────┤
    │  Step 2: Walk remaining string starting at index 7      │
    │                                                         │
    │     'v' → kfs_putchar                                   │
    │     'a' → kfs_putchar                                   │
    │     'l' → kfs_putchar                                   │
    │     '=' → kfs_putchar                                   │
    │     '%' → next char is 'd'                              │
    │          → format_specifier_handler('d', args)           │
    │          → va_arg(args, int) = 42                        │
    │          → itoa(42) → "42"                              │
    │          → kfs_putstr("42")                              │
    │     ',' → kfs_putchar                                   │
    │     ' ' → kfs_putchar                                   │
    │     ... etc                                              │
    │     '%' → next char is 's'                              │
    │          → va_arg(args, char*) = "kfs"                   │
    │          → kfs_putstr("kfs")                             │
    │     '\n' → kfs_putchar                                  │
    ├─────────────────────────────────────────────────────────┤
    │  Step 3: Restore saved_color, update cursor              │
    └─────────────────────────────────────────────────────────┘
```

### Log levels with colors

```
    ┌──────────────────┬───────────────────┬──────────────┐
    │  Log Level       │  Color            │  Prefix      │
    ├──────────────────┼───────────────────┼──────────────┤
    │  LOG_LEVEL_INFO  │  Light Cyan       │  "[INFO] "   │
    │  LOG_LEVEL_DEBUG │  Light Grey       │  "[DEBUG] "  │
    │ LOG_LEVEL_WARNING│  Light Brown      │  "[WARNING] "│
    │  LOG_LEVEL_ERROR │  Light Red        │  "[ERROR] "  │
    └──────────────────┴───────────────────┴──────────────┘

    On screen:
    ┌──────────────────────────────────────────────────┐
    │ [INFO] Welcome to KFS-1!          ← light cyan   │
    │ [DEBUG] ptr: 0x2F4C00             ← light grey   │
    │ [WARNING] This is a warning       ← light brown  │
    │ [ERROR] This is an error          ← light red    │
    └──────────────────────────────────────────────────┘

    After printing the log level tag, the color is changed.
    After the full printk call, the color is RESTORED to
    the original (saved_color).
```

### Format specifiers

```
    ┌─────────┬────────────────────────────────────────────┐
    │  Spec   │  What it does                              │
    ├─────────┼────────────────────────────────────────────┤
    │  %d, %i │  Signed integer → itoa() → decimal string  │
    │  %s     │  String pointer → kfs_putstr()              │
    │  %c     │  Character (passed as int) → kfs_putchar()  │
    │  %x     │  Unsigned int → itoh() → lowercase hex      │
    │  %X     │  Unsigned int → itoh() → uppercase hex      │
    │  %p     │  Pointer → "0x" + itoh() lowercase           │
    │  %%     │  Literal '%' (fallback for unknown spec)     │
    └─────────┴────────────────────────────────────────────┘
```

### Integer to string conversion (`itoa`)

```
    itoa(42, buffer):

    Step 1: Extract digits (reverse order)
    42 % 10 = 2  → buffer[0] = '2'    42 / 10 = 4
     4 % 10 = 4  → buffer[1] = '4'     4 / 10 = 0  → stop
    buffer = "24\0"

    Step 2: reverse_str()
    buffer = "42\0"  ✓
```

### Integer to hex conversion (`itoh`)

```
    itoh(0xDEAD, buffer, "0123456789abcdef"):

    Step 1: Extract hex digits (4 bits at a time, reverse)
    0xDEAD & 0xF = 0xD → 'd'    >> 4 → 0xDEA
    0xDEA  & 0xF = 0xA → 'a'    >> 4 → 0xDE
    0xDE   & 0xF = 0xE → 'e'    >> 4 → 0xD
    0xD    & 0xF = 0xD → 'd'    >> 4 → 0x0  → stop
    buffer = "daed\0"

    Step 2: reverse_str()
    buffer = "dead\0"  ✓
```

## Files involved

| File | Role |
|------|------|
| `src/printk/printk.c` | Main `printk()`, log level parsing, format specifier dispatch |
| `src/printk/convert.c` | `itoa()`, `itoh()`, `reverse_str()` — number-to-string conversion |
| `src/kernel.h` | `va_list`, `va_start`, `va_arg`, `va_end` macros, log level defines |
| `src/lib/string.c` | `strlen()` — used by printk to measure strings |
