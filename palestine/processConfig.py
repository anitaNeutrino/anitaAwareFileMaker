#!/usr/bin/python

import sys, getopt
import json
import os
import convertConfigToJson


def usage():
    print 'processConfig.py -i <input> -r run'
         

def main(argv):
    if(len(argv)<4):
        usage()
        sys.exit()

    inputfilename=''
    run=0

    try:
               opts, args = getopt.getopt(argv,"hi:o:",["ifile=","ofile="])
               except getopt.GetoptError:
                           usage()
                                   sys.exit(2)

                                   for opt, arg in opts:
                                       if opt == '-h':
                                                       usage()
                                                                   sys.exit()
                                                                   elif opt in ("-i", "--ifile"):
                                                                                   inputfilename = arg
                                                                                   elif opt in ("-o", "--ofile"):
                                                                                                   outputfilename = arg
                                                                                                   


if __name__ == "__main__":
    main(sys.argv[1:])
