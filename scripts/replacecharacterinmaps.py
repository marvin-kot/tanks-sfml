#!/usr/bin/python

import sys, getopt, os


def parse_args(argv):
    try:
        opts, args = getopt.getopt(argv[1:], 'hd:i:o:')
    except getopt.GetoptError:
        print(argv[0] + ' -d <folder> -i <which char to replace> -o <replace to THIS character>')
        sys.exit(2)
    pass

    folder = '../assets/maps/segments'
    ichar = ''
    ochar = ''

    for opt, arg in opts:
        print(opt + ' : ' + arg)
        if opt == '-h':
            print(argv[0] + ' -d <folder> -i <which char to replace> -o <replace to THIS character>')
            sys.exit(0)
        elif opt == '-d':
            folder = arg
        elif opt == '-i':
            ichar = arg
        elif opt == '-o':
            ochar = arg

    assert(len(ichar) == 1)
    assert(len(ochar) == 1)
    assert(os.path.exists(folder) == True)

    return folder, ichar, ochar

def main(argv):
    folder, ichar, ochar = parse_args(argv)
    print('folder: %s replace %c with %c' % (folder, ichar, ochar))

    for root, dnames, fnames in os.walk(folder):
        if root[-1] != '/':
            root = root + '/'
        for fname in fnames:
            if fname[-4:] != '.txt':
                continue
            path = root + fname
            if not os.path.exists(path):
                continue

            new_content = ''
            with ( open(path, 'r') as f):
                content = f.read()
                new_content = content.replace(ichar, ochar)
                print(new_content)

            with ( open(path, 'w') as f):
                f.write(new_content)



if __name__ == '__main__':
    main(sys.argv)