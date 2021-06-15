import os

from . import launch


def getFrameFiles(frameid):
    if launch.g_iopath is None:
        return ()
    dirpath = os.path.join(launch.g_iopath, '{:06d}'.format(frameid))
    if not os.path.exists(os.path.join(dirpath, 'done.lock')):
        return ()
    res = []
    for name in os.listdir(dirpath):
        path = os.path.join(dirpath, name)
        name, ext = os.path.splitext(os.path.basename(path))
        if os.path.exists(path):
            res.append((name, ext, path))
    return tuple(res)


def getFrameCount(max_frameid=None):
    if launch.g_iopath is None:
        return 0
    frameid = 0
    while max_frameid is None or frameid < max_frameid:
        dirpath = os.path.join(launch.g_iopath, '{:06d}'.format(frameid))
        if not os.path.exists(os.path.join(dirpath, 'done.lock')):
            return frameid
        frameid += 1
    return max_frameid
