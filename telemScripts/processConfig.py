
import sys, getopt
import json
import os
import os.path
import PyJSONSerialization
import PyConfigLib


def usage():
    print 'processConfig.py -i <input> -r run'
         

def main(argv):
    if(len(argv)<4):
        usage()
        sys.exit()

    inputfilename=''
    run=0
    
    try:
        opts, args = getopt.getopt(argv,"hi:r:",["ifile=","run="])
    except getopt.GetoptError:
        usage()
        sys.exit(2)
    
    for opt, arg in opts:
        if opt == '-h':
            usage()
            sys.exit()
        elif opt in ("-i", "--ifile"):
            inputfilename = arg
        elif opt in ("-r", "--run"):
            run = int(arg)
    print 'Input file is ', inputfilename
    print 'Run is ', run

    #The only tricky thing is sorting out the path
    awareDir=os.getenv('AWARE_OUTPUT_DIR', "./output")
    runDir=awareDir+"/ANITA4/runs"+str(run-run%10000)+"/runs"+str(run-run%100)+"/run"+str(run)
    configBase=os.path.splitext(os.path.basename(inputfilename))[0]
    outFile=runDir+"/config/"+configBase+".json"

    if(not  os.path.isdir(runDir+"/config")):
        os.makedirs(runDir+"/config")

    myConfig=PyConfigLib.ConfigFile(configBase)
    myConfig.readFile(inputfilename)
    jsonObj=PyJSONSerialization.dump(myConfig)
    
    f=open(outFile,"w")
    f.write(jsonObj)
    f.write("\n")
    f.close()




if __name__ == "__main__":
    main(sys.argv[1:])
