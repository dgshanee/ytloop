from pynput import keyboard
import ctypes

actions = ctypes.CDLL("./build/actions.so")

def start_keyboard_tracking(shmem, lib):
    actions.toggle_playback.argtypes = [ctypes.c_void_p]
    def on_press(key):
        try:
            if key.char == 'p':
                actions.toggle_playback(shmem)
        except AttributeError:
            pass

    listener = keyboard.Listener(on_press=on_press)
    listener.start()

    return listener
