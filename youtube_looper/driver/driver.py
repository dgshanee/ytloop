import ctypes
import sys

lib = ctypes.CDLL("./build/driver.so")
lib.video_start.argtypes = [ctypes.c_int, ctypes.POINTER(ctypes.c_char_p)]

sa = [s.encode('utf-8') for s in sys.argv]
arr = (ctypes.c_char_p * len(sa))()
arr[:] = sa

lib.video_start(len(sys.argv), arr)
