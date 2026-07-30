from inputs.parse_configs import parse_config_map
from inputs.parse_configs import NON_CHAR_KEY_MAP

from pynput import keyboard
import ctypes

# grabs from actions.c
actions = ctypes.CDLL("./build/actions.so")

# type declaration
actions.toggle_playback.argtypes = [ctypes.c_void_p]
actions.toggle_fastforward.argtypes = [ctypes.c_void_p]
actions.toggle_rewind.argtypes = [ctypes.c_void_p]
actions.open_command_prompt.argtypes = [ctypes.c_void_p]
actions.cancel.argtypes = [ctypes.c_void_p]

config_map = None

def start_keyboard_tracking(shmem):
    def on_press(key):
        if key in NON_CHAR_KEY_MAP:
            key.char = NON_CHAR_KEY_MAP[key]
        try:
            if not config_map:
                return
            if key.char in config_map.keys():
                try:
                    exec(f"actions.{config_map[key.char]}(shmem)", {"actions": actions, "shmem": shmem})
                except Exception as e:
                    print("error", e)
                    pass
        except AttributeError:
            pass

    config_map = parse_config_map()

    listener = keyboard.Listener(on_press=on_press)
    listener.start()

    return listener
