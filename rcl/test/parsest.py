import re
import os
import sys

rx = re.compile(r'^[0-9]+: \[\s+RUN\s+\] QOSGroup/TestMemoryFixture__rmw_(\w+)_cpp\.test_memory_(\w+)/(\d+)$\n((^.*$\n)*?)^[0-9]+: \[\s+(FAILED|OK)\s+\]', re.MULTILINE)
ry = re.compile(r'(^.*Stack trace.*$\n)(^.*$\n)*?(^.*\(not expected\).*$\n)', re.MULTILINE)

with open(sys.argv[1]) as f:
    raw_data = f.read()
    for match in rx.finditer(raw_data):
        dir = match.group(1) + '_' + match.group(2) + '_' + match.group(3)
        os.mkdir(dir)
        i = 0
        print dir
        for match2 in ry.finditer(match.group(4)):
            print './' + dir + '/ST_' + str(i) + '.txt'
            with open('./' + dir + '/ST_' + str(i) + '.txt', 'w') as o:
                o.write(match2.group(0))
                i += 1
