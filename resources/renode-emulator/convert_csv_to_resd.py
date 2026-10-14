import os, sys, subprocess

PY   = r"C:\Users\aneur\AppData\Local\Python\pythoncore-3.14-64\python.exe"
TOOL = r"C:\Program Files\Renode\tools\csv2resd\csv2resd.py"
LIBS = r"C:\path\to\dir\containing\construct"   # pip install --target

def csv2res(csv_path, mappings, out_path, frequency=None, timestamp=None):
    """Run the official csv2resd under CPython.

    mappings: list of "<type>:<from>[:<to>:<channel>]" strings, e.g.
    ["acceleration:ax,ay,az:x,y,z:0"]
    """
    cmd = [PY, TOOL, "-i", csv_path]
    for m in mappings:
        cmd += ["-m", m]
    if frequency is not None:
        cmd += ["-f", str(frequency)]
    if timestamp is not None:
        cmd += ["-t", str(timestamp)]
    cmd.append(out_path)

    env = dict(os.environ)
    env["PYTHONPATH"] = LIBS
    proc = subprocess.Popen(cmd, stdout=subprocess.PIPE,
                            stderr=subprocess.STDOUT, env=env)
    out, _ = proc.communicate()
    if proc.returncode != 0:
        raise Exception("csv2resd failed (%d):\n%s" % (proc.returncode, out))
    return out_path