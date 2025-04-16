import sys
import emu

# ANSI escape codes for colors
BLACK = "\033[30m"
RED = "\033[31m"
GREEN = "\033[32m"
YELLOW = "\033[33m"
BLUE = "\033[34m"
MAGENTA = "\033[35m"
CYAN = "\033[36m"
WHITE = "\033[37m"
RESET = "\033[0m"

e = emu.Emulator()


def print_interactive(msg):
    """Prints messages to stderr so they are not captured when piping stdout."""
    print(msg, file=sys.stderr)


# This bit of logic, and method_names is to expose functions by name alone when running
# this file in interactive mode (python3 -i emu.py). i.e. set_a() instead of e.set_a()
def bind_method(method_name):
    def wrapper(*args, **kwargs):
        attr = getattr(e, method_name)
        if callable(attr):
            return attr(*args, **kwargs)
        else:
            return attr

    wrapper.__name__ = method_name
    return wrapper


# Names of methods exposed globally to this file
method_names = [
    # CPU Getters
    "cpu_cycles",
    "a",
    "x",
    "y",
    "p",
    "sp",
    "pc",
    "carry_flag",
    "zero_flag",
    "interrupt_flag",
    "decimal_flag",
    "break_flag",
    "overflow_flag",
    "negative_flag",
    # CPU Setters
    "set_a",
    "set_x",
    "set_y",
    "set_p",
    "set_sp",
    "set_pc",
    "set_cycles"
    # PPU Getters
    "nmi",
    "vblank",
    "scanline",
    "ppu_cycles",
    "frame"
    # PPU Setters
    "set_scanline",
    "set_ppu_cycles",
    # Cartridge Getters
    "did_mapper_load",
    "does_mapper_exist",
    # Cartridge Setters
    # Methods
    "log",
    "step",
    "test",
    "enable_mesen_trace",
    "disable_mesen_trace",
    "print_mesen_trace",
    "debug_reset",
    "read",
    "ppu_read",
]
for name in method_names:
    globals()[name] = bind_method(name)


def step_until(callback_condition):
    while not callback_condition():
        e.step()


def commands():
    for name in method_names:
        print(f"{YELLOW}{name}(){RESET}")
    print(f"{YELLOW}step_until(callback_condition){RESET}")
    print(f"{YELLOW}out(filename){RESET} # Redirect stdout to a file")


def step_and_trace(n=100):
    e.enable_mesen_trace(n)
    e.step(n)
    e.print_mesen_trace()
    e.disable_mesen_trace()


def main():
    e.load("../../roms/palette.nes")
    e.log()


if __name__ == "__main__":
    main()
