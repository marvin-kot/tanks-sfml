#!/usr/bin/python

import sys, re, os

audio_set = {}

audio_path = 'assets/new_sounds/'
relative_path = '../' + audio_path
cpp_path = '../src/SoundPlayer.cpp'
used_files = '../assets/audio/'
unused_files = '../assets/audio/unused/'

with( open(cpp_path) as f ):
    content = f.read()

    existent_paths = re.findall(r'\"' + audio_path + r'([\w/]+.wav)\"', content)

    audio_set = [relative_path+match for match in existent_paths]

    for root, dnames, fnames in os.walk(relative_path):

        if root[-1] != '/':
            root = root + '/'
        for f in fnames:
            fullpath = root + f
            if fullpath in audio_set:
                print(fullpath + ' is used!')
                os.rename(fullpath, used_files + f)
            else:
                pass
                os.rename(fullpath, unused_files + f)


