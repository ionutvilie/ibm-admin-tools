#!/usr/bin/python

import pymqi

queue_manager = "MQSD.TEST"
channel = "SYSTEM.DEF.SVRCONN"
host = "10.21.218.15"
port = "14123"
conn_info = "%s(%s)" % (host, port)

prefix = "*"
queue_type = pymqi.CMQC.MQQT_ALL
# queue_type = pymqi.CMQC.MQQT_LOCAL
excluded_prefix = ['SYSTEM', 'MSB', 'AMQ' , 'MQAI']
# excluded_prefix = [ ]

args = {pymqi.CMQC.MQCA_Q_NAME: prefix,
        pymqi.CMQC.MQIA_Q_TYPE: queue_type}

qmgr = pymqi.connect(queue_manager, channel, conn_info)
pcf = pymqi.PCFExecute(qmgr)

try:
    response = pcf.MQCMD_INQUIRE_Q(args)
except pymqi.MQMIError, e:
    if e.comp == pymqi.CMQC.MQCC_FAILED and e.reason == pymqi.CMQC.MQRC_UNKNOWN_OBJECT_NAME:
        print "No queues matched given arguments."
    else:
        raise
else:
    for queue_info in response:
# Queue Name  QueueDepth MaxDepth XMITQ Type
# https://www-01.ibm.com/support/knowledgecenter/SSFKSJ_7.1.0/com.ibm.mq.javadoc.doc/WMQJavaClasses/com/ibm/mq/pcf/CMQC.html
        queue_name = queue_info[pymqi.CMQC.MQCA_Q_NAME]
        if not any(queue_name.startswith(prefix) for prefix in excluded_prefix):
            queue_type = queue_info[pymqi.CMQC.MQIA_Q_TYPE]
            if queue_type == 1: #LOCAL
                queue_type = "LOCAL"
                queue_depth = queue_info[pymqi.CMQC.MQIA_CURRENT_Q_DEPTH]
                queue_mdepth = queue_info[pymqi.CMQC.MQIA_MAX_Q_DEPTH]
                print "%s \t %s \t %s \t %s" % (queue_name, queue_depth, queue_mdepth, queue_type)
            # elif queue_type == 2: #MODEL
            elif queue_type == 3: #ALIAS
                queue_type = "ALIAS"
                queue_depth = "-"
                queue_mdepth = "------"
                print "%s \t %s \t %s \t %s" % (queue_name, queue_depth, queue_mdepth, queue_type)
            elif queue_type == 6: #REMOTE
                queue_type = "REMOTE"
                queue_depth = "-"
                queue_mdepth = "------"
                print "%s \t %s \t %s \t %s" % (queue_name, queue_depth, queue_mdepth, queue_type)
                # print "%s \t %s" % (queue_name, queue_type)
            else:
                print "%s \t %s" % (queue_name, queue_type)
        # print "%s \t %s" % (queue_name, queue_type)

qmgr.disconnect()



