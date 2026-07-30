import signal

def handle_command(signum, frame):
    print("handling command!!")

def wait_for_command():
    signal.signal(signal.SIGUSR1, handle_command)
