import signal
from functools import partial
import threading

import ctypes

from inputs.keyboard_input import actions

class video_state(ctypes.Structure):
    _fields_ = [
    ('command_bar_open', ctypes.c_bool),
    ('toggle_playback', ctypes.c_bool),
    ('toggle_fastforward', ctypes.c_bool),
    ('toggle_rewind', ctypes.c_bool),
    ('command_str', ctypes.c_char * 50)
    ]

def handle_command(shmem):
    print("COMMAND GRABBED")
    ptr = ctypes.cast(shmem, ctypes.POINTER(video_state))
    state = ptr.contents
    print(state.command_str)
    actions.toggle_playback(shmem) 

def wait_for_command(event, shmem):
    while(True):
        if event.is_set():
            handle_command(shmem)
            event.clear()

