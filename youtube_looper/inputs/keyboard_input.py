from inputs.parse_configs import parse_config_map

from pynput import keyboard
import ctypes

actions = ctypes.CDLL("./build/actions.so")
actions.toggle_playback.argtypes = [ctypes.c_void_p]
actions.toggle_fastforward.argtypes = [ctypes.c_void_p]
actions.toggle_rewind.argtypes = [ctypes.c_void_p]

d = None

def start_keyboard_tracking(shmem):
    def on_press(key):
        try:
            if not d:
                return
            for k in d.keys():
                try:
                    if key.char == k:
                        exec(f"actions.{d[k]}(shmem)", {"actions": actions, "shmem": shmem})
                except Exception:
                    pass
        except AttributeError:
            pass

    d = parse_config_map()
    print(d)

    listener = keyboard.Listener(on_press=on_press)
    listener.start()

    return listener
