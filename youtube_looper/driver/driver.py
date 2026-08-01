import ctypes
from os import wait
from signal import SIGUSR1, SIG_IGN, signal
import sys
import threading

sys.path.insert(1, "youtube_looper")

from inputs.keyboard_input import start_keyboard_tracking
from inputs.commands import wait_for_command

# load the DLL created from driver.c
lib = ctypes.CDLL("./build/driver.so")

# load the needed functions from driver.c
lib.video_start.argtypes = [ctypes.c_int, ctypes.POINTER(ctypes.c_char_p), ctypes.c_void_p, ctypes.py_object]
lib.create_shared_memory.restype = ctypes.c_void_p

# get args
sa = [s.encode('utf-8') for s in sys.argv]
arr = (ctypes.c_char_p * len(sa))()
arr[:] = sa

shmem = lib.create_shared_memory()

listener = start_keyboard_tracking(shmem)

event = threading.Event()
threading.Thread(target=wait_for_command, args=(event, shmem,)).start()

lib.video_start(len(sys.argv), arr, shmem, event)

listener.stop()
listener.join()
