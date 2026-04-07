import ctypes
import sys

lib = ctypes.CDLL("./build/driver.so")
lib.video_start.argtypes = [ctypes.c_int, ctypes.POINTER(ctypes.c_char_p), ctypes.c_void_p]
lib.create_shared_memory.restype = ctypes.c_void_p

sa = [s.encode('utf-8') for s in sys.argv]
arr = (ctypes.c_char_p * len(sa))()
arr[:] = sa

shmem = lib.create_shared_memory()
lib.video_start(len(sys.argv), arr, shmem)
