
import atexit
import sys
import os
import subprocess
import shutil
import string
import random
import traceback

tmp_created = False
tmp_dir = "/tmp/scc/rundir_" + str(os.getpid()) + "_"
tmp_dir += ''.join(random.choice(string.ascii_lowercase) for i in range(6))
tmp_dir += "/"

# Cleanup the tmp_dir if necessary. Called automatically on process shutdown.
def cleanup_tmpdir():
    global tmp_dir
    if not tmp_created:
        return
    shutil.rmtree(tmp_dir)
    tmp_dir = None

# Returns a unique temporary directory for this process.
# The directory is destroyed alongside this process.
def get_tmp():
    global tmp_created
    global tmp_dir
    if tmp_created:
        return tmp_dir
    os.makedirs(tmp_dir)
    atexit.register(cleanup_tmpdir)
    tmp_created = True
    return tmp_dir

# Terminates the program with the given message and score.
#
# The message is displayed in the user UI. The scores serves as a rating how
# useful the program was (with a higher score indicating a more useful program).
def giveScore(msg, score):
    print("\nFUZZ:SCORE:" + str(score))
    print("\nFUZZ:MSG:" + msg)
    sys.exit(0)

# Terminates the program and marks the test case as interesting.
def markInteresting(msg):
    print("\nFUZZ:HIT")
    print("\nFUZZ:SCORE:" + str(1))
    print("\nFUZZ:MSG:" + msg)
    sys.exit(0)


class FailedToCompile(subprocess.CalledProcessError):
    def __init__(self, e):
        self.__dict__.update(e.__dict__)

class TimeOutCompiling(subprocess.TimeoutExpired):
    def __init__(self, e):
        self.__dict__.update(e.__dict__)

class FailedToRun(subprocess.CalledProcessError):
    def __init__(self, e):
        self.__dict__.update(e.__dict__)

class TimeOutRunning(subprocess.TimeoutExpired):
    def __init__(self, e):
        self.__dict__.update(e.__dict__)

def handle_exception(exc_type, exc_value, exc_traceback):
    """ Handle uncaught exceptions. """
    if issubclass(exc_type, FailedToCompile):
        giveScore("Failed to compile program", -30000005)
    if issubclass(exc_type, TimeOutCompiling):
        giveScore("Timed out compiling", -30000005)
    if issubclass(exc_type, FailedToRun):
        giveScore("Compiled program failed to run", -30000005)
    if issubclass(exc_type, TimeOutRunning):
        giveScore("Time out running", -30000005)

    traceback.print_exception(exc_value)
    sys.exit(1)

sys.excepthook = handle_exception

binary_counter = 0
# Compiles the given source file into a standalone executable.
#
# Terminates the process with a negative score on timeout or failure to compile.
def compile(compiler, source_file, extra_flags):
    global binary_counter
    binary_counter += 1
    try:
        result = get_tmp() + "/bin" + str(binary_counter)
        args = [compiler, source_file, "-o", result]
        args += extra_flags
        subprocess.run(args, timeout=3, shell=False, capture_output=True, check=True)
        return result
    except subprocess.TimeoutExpired as e:
        raise TimeOutCompiling(e)
    except subprocess.CalledProcessError as e:
        raise FailedToCompile(e)

def compileAndRun(compiler, source_file, extra_flags):
    binary = compile(compiler, source_file, extra_flags)

    try:
        res = subprocess.run([binary], check=True, timeout=3, capture_output=True)
        # For Clang's UBSan.
        if b"UndefinedBehaviorSanitizer" in res.stderr:
           raise FailedToRun(res)
        # For GCC's UBSan.
        if b"runtime error:" in res.stderr:
           raise FailedToRun(res)
    except subprocess.TimeoutExpired as e:
        raise TimeOutRunning(e)
    except subprocess.CalledProcessError as e:
        raise FailedToRun(e)