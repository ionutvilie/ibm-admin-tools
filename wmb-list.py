#!/usr/bin/env python2.7
"""
this script extracts MessageFlow, ComIbmMQOutputNode, ComIbmMQInputNode
from the mqsireportproperties command output
"""
import re
import subprocess
import sys

if len(sys.argv) != 3:
    print """
    ERROR:
    
        This script requires 2 arguments
          - Broker Name
          - Execution Group
    """
    exit(1)

broker = sys.argv[1]
executionGroup = sys.argv[2]

print "Listing for Broker: '" + broker + "' and Execution group: '" + executionGroup + "'"

# exec mqsireportproperties and send output pipe to egrep
mqsireportproperties = subprocess.Popen(['mqsireportproperties', broker, '-e', executionGroup, '-o', 'AllMessageFlows', '-r' ],
                                        stdout=subprocess.PIPE, )
egrep = subprocess.Popen(['egrep', 'MessageFlow|ComIbmMQOutputNode|queueName=|label=|ComIbmMQInputNode'],
                         stdin=mqsireportproperties.stdout,
                         stdout=subprocess.PIPE, )
mqsireportproperties.stdout.close()

# implement some switches
counter0 = 0
ComIbmMQInputNode=0
ComIbmMQOutputNode=0

# process output and extract flowname input queues and output queues
for line in iter(egrep.stdout.readline, ""):
    if line.strip() == "MessageFlow":
        counter0 += 1
        print str(counter0) + " " + line.strip()
    flowName = re.match(r'(^\s\slabel=)\'(.*)\'', line)
    if flowName:
        print "\tFlowName:  " + flowName.group(2)
    if line.strip() == "ComIbmMQInputNode":
        ComIbmMQInputNode = 1
    if line.strip() == "ComIbmMQOutputNode":
        ComIbmMQOutputNode = 1
    queueName = re.match(r'(^\s+queueName=)\'(.*)\'', line)

    if queueName and queueName.group(2) != '':
        if ComIbmMQInputNode == 1:
            ComIbmMQInputNode=0
            print "\tinputQueueName:  " + queueName.group(2)
        elif ComIbmMQOutputNode == 1:
            ComIbmMQOutputNode=0
            print "\toutputQueueName:  " + queueName.group(2)


exit(0)
