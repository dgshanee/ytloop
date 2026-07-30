from pynput import keyboard

NON_CHAR_KEY_MAP = {keyboard.Key.esc : "esc"}

def parse_config_map():
    d = {}
    with open("configs/yl.conf") as f:
        for line in f:
            (val, key) = line.split()
            d[key] = val
    return d
