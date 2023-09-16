#!/usr/bin/python

import sys, getopt
import json

default_path =  "../assets/spriteSheet32x32.json"

class SpriteSheetProcessor:
    def __init__(self, ifilename, ofilename):
        assert(ifilename)
        assert(ifilename.endswith('.json'))
        self.ifilename = ifilename

        assert(ofilename)
        assert(ofilename.endswith('.json'))
        self.ofilename = ofilename

    def enumerate_frames(self):
        "adds continuous numbering to every frame of the json sprite sheet"
        with( open(self.ifilename) as f ):
            data = json.load(f)
            id = 0
            for obj in data:
                if "animations" not in obj: continue
                for anim in obj["animations"]:
                    if "frames" not in anim: continue
                    for frame in anim["frames"]:
                        frame["id"] = id
                        id += 1
            print("%d frames are identified" % id)

        with ( open(self.ofilename, "w") as f ):
            json.dump(data, f, indent=4)


def pass_argv(argv):
    try:
        print("args: " + ",".join(argv))
        opts, args = getopt.getopt(argv[1:], "hi:o:")
    except getopt.GetoptError:
        print(argv[0] + ' -i <inputfile> -o <outputfile>')
        sys.exit(2)

    ifilename = default_path
    ofilename = default_path + '.new'

    for opt, arg in opts:
        if opt == '-h':
            print(argv[0] + ' -i <inputfile> -o <outputfile>')
            sys.exit(2)
        elif opt == '-i':
            print('input file: ' + arg)
            ifilename = arg
        elif opt == '-o':
            print('output file: ' + arg)
            ofilename = arg

    return ifilename, ofilename


def main(argv):
    ifilename, ofilename = pass_argv(argv)
    processor = SpriteSheetProcessor(ifilename, ofilename)
    processor.enumerate_frames()


if __name__ == "__main__":
    main(sys.argv)