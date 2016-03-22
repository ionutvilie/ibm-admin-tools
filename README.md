# ibm_mq_tools
Middleware administration tolls 

This is work in progres. 
For now it can be used as example.


```sh
[mqm@:/home/mqm/wrk/baqsh] amqsh TEST_V7
Display all queues
Queue Name                                       QueueDepth   MaxDepth  Cluster OIC OOC XMITQ                                Type
------------------------------------------------ ---------- ---------- -------- --- --- ------------------------------------ ---------
TEST.TRANSMIT                                             0     300000   (null)   0   0 -                                    LOCAL
TEST_QALIAS                                               -          -   (null)   -   - -                                    ALIAS
TEST_QLOCAL                                               0       5000   (null)   0   0 -                                    LOCAL
TEST_REMOTE                                               -          -   (null)   -   - TEST.TRANSMIT                        REMOTE
```
