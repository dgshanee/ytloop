from pynput import keyboard
import ctypes


def start_keyboard_tracking(shmem, lib):
    lib.toggle_playback.argtypes = [ctypes.c_void_p]
    def on_press(key):
        if key.char == 'p':
            lib.toggle_playback(shmem)

    listener = keyboard.Listener(on_press=on_press)
    listener.start()

    return listener
