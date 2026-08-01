"""
-----driver.py-----
The starting point for the program. It manages the shared memory, two input
management threads, and the main video driver.
"""
import ctypes
import sys
import threading

# fix the path for imports
sys.path.insert(1, "youtube_looper")

from inputs.keyboard_input import start_keyboard_tracking
from inputs.commands import wait_for_command

def load_driver():
    '''
    Loads the driver from a C DLL. When using PyDLL, keyboard inputs don't register,
    so GIL management has to be done in C

    Returns
    -------
    driver_c : ctypes.CDLL
        The DLL for driver.c, contains all functions in driver.c
    '''
    # load the DLL created from driver.c
    driver_c = ctypes.CDLL("./build/driver.so")

    # load the needed functions from driver.c
    driver_c.video_start.argtypes = [ctypes.c_int, ctypes.POINTER(ctypes.c_char_p), ctypes.c_void_p, ctypes.py_object]
    driver_c.create_shared_memory.restype = ctypes.c_void_p

    return driver_c

def get_args():
    '''
    Returns all arguments given to the program as an array.
    '''
    sa = [s.encode('utf-8') for s in sys.argv]
    arr = (ctypes.c_char_p * len(sa))()
    arr[:] = sa

    return arr

def main():
    '''
    The first function to be called
    - initializes shared memory
    - starts keyboard tracking
    - starts listening for C signals from executed commands
    - starts the video playback
    '''
    # ---INITIALIZATION---
    args = get_args()
    driver_c = load_driver()
    event = threading.Event()

    # ---CREATE SHARED MEMORY---
    shmem = driver_c.create_shared_memory()

    # ---SIGNAL HANDLING---
    listener = start_keyboard_tracking(shmem)
    threading.Thread(target=wait_for_command, args=(event, shmem,)).start()

    # ---MAIN DRIVER---
    driver_c.video_start(len(sys.argv), args, shmem, event)

    # ---exit gracefully---
    listener.stop()
    listener.join()

if __name__ == "__main__":
    main()

