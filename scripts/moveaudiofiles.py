#!/usr/bin/python

import sys, re, os

audio_set = {}

audio_path = 'unused_assets/audio/'
relative_path = '../' + audio_path
cpp_path = '../src/SoundPlayer.cpp'
used_files = '../assets/audio/'
unused_files = '../assets/audio/unused/'

with( open(cpp_path) as f ):
    content = f.read()

    existent_paths = re.findall(r'\"assets/audio/([\w/ ]+.wav)\"', content)

    audio_set = [x for x in existent_paths]
    print(audio_set)

    for root, dnames, fnames in os.walk(relative_path):

        if root[-1] != '/':
            root = root + '/'
        for f in fnames:
            fullpath = root + f
            if not os.path.exists(fullpath):
                continue
            if f in audio_set:
                print(f + ' is used!')
                os.rename(fullpath, used_files + f)
            else:
                pass
                #os.rename(fullpath, unused_files + f)


