/******************************************************************************/
/* based on amqsail IBM MQ sample                                            */
/* amqsh has 1 parameter - the queue manager name (optional)               	*/
/*                                                                            */
/******************************************************************************/

/******************************************************************************/
/* Includes                                                                   */
/******************************************************************************/
#include <stdio.h>
#include <string.h>           
#include <stdlib.h>          
#include <ctype.h>              

#include <cmqc.h>                          /* MQI                             */
#include <cmqcfc.h>                        /* PCF                             */
#include <cmqbc.h>                         /* MQAI                            */
                                                 
/******************************************************************************/
/* Function prototypes                                                        */
/******************************************************************************/
void CheckCallResult(MQCHAR *, MQLONG , MQLONG);
                                                 
/******************************************************************************/
/* Function: main                                                             */
/******************************************************************************/
int main(int argc, char *argv[])                 
{
   /***************************************************************************/
   /* MQAI variables                                                          */
   /***************************************************************************/
   MQHCONN hConn;                          /* handle to MQ connection         */
   MQCHAR qmName[MQ_Q_MGR_NAME_LENGTH+1]=""; /* default QMgr name             */
   MQLONG reason;                          /* reason code                     */
   MQLONG connReason;                      /* MQCONN reason code              */
   MQLONG compCode;                        /* completion code                 */
   MQHBAG adminBag = MQHB_UNUSABLE_HBAG;   /* admin bag for mqExecute         */
   MQHBAG responseBag = MQHB_UNUSABLE_HBAG;/* response bag for mqExecute      */ 
   MQHBAG qAttrsBag;                       /* bag containing q attributes     */
   MQHBAG errorBag;                        /* bag containing cmd server error */
   MQLONG mqExecuteCC;                     /* mqExecute completion code       */
   MQLONG mqExecuteRC;                     /* mqExecute reason code           */
   MQLONG qNameLength;                     /* Actual length of q name         */
   MQLONG qClusterLength;                     /* Actual length of q name         */
   MQLONG qDepth;                          /* depth of queue                  */
   MQLONG maxDdepth;                       /* max depth of queue              */
   MQLONG qType;                           /* type of queue                   */
   MQLONG openIC;                          /* open input count                */
   MQLONG openOC;                          /* open output count                */
   MQCHAR qCluster[MQ_CLUSTER_NAME_LENGTH+1];       /* name of queue extracted from bag*/ 
   MQCHAR xmitQ[MQ_Q_NAME_LENGTH+1];       /* name of queue extracted from bag*/ 
   MQLONG i;                               /* loop counter                    */
   MQLONG numberOfBags;                    /* number of bags in response bag  */
   MQCHAR qName[MQ_Q_NAME_LENGTH+1];       /* name of queue extracted from bag*/ 
   char *qTypePrint;    
   

   printf("Display all queues\n");
   printf("%-48s %10s %10s %8s %3s %3s %-36s %-9s\n", "Queue Name","QueueDepth","MaxDepth","Cluster","OIC","OOC","XMITQ","Type");
   printf("%-48s %10s %10s %8s %3s %3s %-36s %-9s\n","------------------------------------------------","----------","----------","--------","---","---","------------------------------------","---------");

   
   /***************************************************************************/
   /* Connect to the queue manager                                            */
   /***************************************************************************/
   if (argc > 1)
      strncpy(qmName, argv[1], (size_t)MQ_Q_MGR_NAME_LENGTH);
   MQCONN(qmName, &hConn, &compCode, &connReason);    

   /***************************************************************************/
   /* Report the reason and stop if the connection failed.                    */
   /***************************************************************************/
   if (compCode == MQCC_FAILED)
   {
      CheckCallResult("Queue Manager connection", compCode, connReason);
      exit( (int)connReason);
   }
   
   /***************************************************************************/
   /* Create an admin bag for the mqExecute call                              */
   /***************************************************************************/
   mqCreateBag(MQCBO_ADMIN_BAG, &adminBag, &compCode, &reason);
   CheckCallResult("Create admin bag", compCode, reason);

   /***************************************************************************/
   /* Create a response bag for the mqExecute call                            */
   /***************************************************************************/
   mqCreateBag(MQCBO_ADMIN_BAG, &responseBag, &compCode, &reason);
   CheckCallResult("Create response bag", compCode, reason);

   /***************************************************************************/
   /* Put the generic queue name into the admin bag                           */
   /***************************************************************************/
   mqAddString(adminBag, MQCA_Q_NAME, MQBL_NULL_TERMINATED, "*", &compCode, &reason);
   CheckCallResult("Add q name", compCode, reason);

   /***************************************************************************/
   /* Put the local queue type into the admin bag                             */
   /***************************************************************************/
   mqAddInteger(adminBag, MQIA_Q_TYPE, MQQT_ALL, &compCode, &reason);
   CheckCallResult("Add q type", compCode, reason);

   /***************************************************************************/
   /* Send the command to find all the local queue names and queue depths.    */
   /* The mqExecute call creates the PCF structure required, sends it to      */
   /* the command server, and receives the reply from the command server into */
   /* the response bag. The attributes are contained in system bags that are  */
   /* embedded in the response bag, one set of attributes per bag.            */
   /***************************************************************************/
   mqExecute(hConn,                   /* MQ connection handle                 */
             MQCMD_INQUIRE_Q,         /* Command to be executed               */
             MQHB_NONE,               /* No options bag                       */
             adminBag,                /* Handle to bag containing commands    */
             responseBag,             /* Handle to bag to receive the response*/
             MQHO_NONE,               /* Put msg on SYSTEM.ADMIN.COMMAND.QUEUE*/  
             MQHO_NONE,               /* Create a dynamic q for the response  */ 
             &compCode,               /* Completion code from the mqexecute   */ 
             &reason);                /* Reason code from mqexecute call      */
   
   
   /***************************************************************************/
   /* Check the command server is started. If not exit.                       */
   /***************************************************************************/
   if (reason == MQRC_CMD_SERVER_NOT_AVAILABLE)
   {
      printf("Please start the command server: <strmqcsv QMgrName>\n");
      MQDISC(&hConn, &compCode, &reason);
      CheckCallResult("Disconnect from Queue Manager", compCode, reason);         
      exit(98);
   }  

   /***************************************************************************/
   /* Check the result from mqExecute call. If successful find the current    */
   /* depths of all the local queues. If failed find the error.               */
   /***************************************************************************/
   if ( compCode == MQCC_OK )                      /* Successful mqExecute    */
   {
     /*************************************************************************/
     /* Count the number of system bags embedded in the response bag from the */
     /* mqExecute call. The attributes for each queue are in a separate bag.  */
     /*************************************************************************/
     mqCountItems(responseBag, MQHA_BAG_HANDLE, &numberOfBags, &compCode, &reason);
     CheckCallResult("Count number of bag handles", compCode, reason); 

     for ( i=0; i<numberOfBags; i++)
     {   
       /***********************************************************************/
       /* Get the next system bag handle out of the mqExecute response bag.   */
       /* This bag contains the queue attributes                              */ 
       /***********************************************************************/
       mqInquireBag(responseBag, MQHA_BAG_HANDLE, i, &qAttrsBag, &compCode, &reason);
       CheckCallResult("Get the result bag handle", compCode, reason);

       /***********************************************************************/
       /* Get the queue name out of the queue attributes bag                  */
       /***********************************************************************/
       mqInquireString(qAttrsBag, MQCA_Q_NAME, 0, MQ_Q_NAME_LENGTH, qName,  
                       &qNameLength, NULL, &compCode, &reason);
       CheckCallResult("Get queue name", compCode, reason);

       /***********************************************************************/
       /* Get the type out of the queue attributes bag                       */
       /***********************************************************************/
       mqInquireInteger(qAttrsBag, MQIA_Q_TYPE, MQIND_NONE, &qType, 
                        &compCode, &reason);
       CheckCallResult("Get depth", compCode, reason); 
	   
		switch(qType) {
			case 1 : 
				qTypePrint      = "LOCAL";
				/***********************************************************************/
				/* Get the depth out of the queue attributes bag                       */
				/***********************************************************************/
				mqInquireInteger(qAttrsBag, MQIA_CURRENT_Q_DEPTH, MQIND_NONE, &qDepth, 
									&compCode, &reason);
				CheckCallResult("Get depth", compCode, reason); 
			
				/***********************************************************************/
				/* Get the type out of the queue attributes bag                       */
				/***********************************************************************/
				mqInquireInteger(qAttrsBag, MQIA_MAX_Q_DEPTH, MQIND_NONE, &maxDdepth, 
									&compCode, &reason);
				CheckCallResult("Get depth", compCode, reason); 
					
							
				/***********************************************************************/
				/* Get the type out of the queue attributes bag  MQIA_OPEN_INPUT_COUNT                     */
				/***********************************************************************/
				mqInquireInteger(qAttrsBag, MQIA_OPEN_INPUT_COUNT, MQIND_NONE, &openIC, 
									&compCode, &reason);
				CheckCallResult("Get depth", compCode, reason); 
					
					
				/***********************************************************************/
				/* Get the type out of the queue attributes bag  MQIA_OPEN_INPUT_COUNT                     */
				/***********************************************************************/
				mqInquireInteger(qAttrsBag, MQIA_OPEN_OUTPUT_COUNT, MQIND_NONE, &openOC, 
									&compCode, &reason);
				CheckCallResult("Get depth", compCode, reason); 
				
				/***********************************************************************/
				/* Get the type out of the queue attributes bag  MQIA_OPEN_INPUT_COUNT                     */
				/***********************************************************************/
			
				mqInquireString(qAttrsBag, MQCA_CLUSTER_NAME, 0, MQ_CLUSTER_NAME_LENGTH, qCluster,  
								&qClusterLength, NULL, &compCode, &reason);
				CheckCallResult("Get queue name", compCode, reason);	   
					
				/***********************************************************************/
				/* Use mqTrim to prepare the queue name for printing.                  */
				/* Print the result.                                                   */
				/***********************************************************************/
				mqTrim(MQ_Q_NAME_LENGTH, qName, qName, &compCode, &reason);
				mqTrim(MQ_CLUSTER_NAME_LENGTH, qCluster, qCluster, &compCode, &reason);
				
				if (strlen(qCluster) == 0) { strcpy(qCluster,"(null)"); }
				
				printf("%-48s %10ld %10ld %8s %3ld %3ld %-36s %-9s\n", qName, qDepth, maxDdepth, qCluster, openIC, openOC, "-" ,qTypePrint);
			break;
			case 2 :
				qTypePrint     = "MODEL";
				mqTrim(MQ_Q_NAME_LENGTH, qName, qName, &compCode, &reason);
				mqTrim(MQ_CLUSTER_NAME_LENGTH, qCluster, qCluster, &compCode, &reason);

				if (strlen(qCluster) == 0) { strcpy(qCluster,"(null)"); }
				
				printf("%-48s %10s %10s %8s %3s %3s %-36s %-9s\n", qName, "-", "-", qCluster, "-", "-", "-" ,qTypePrint);
			break;	
			case 3 : 
				qTypePrint     = "ALIAS";
				/***********************************************************************/
				/* Get the type out of the queue attributes bag  MQIA_OPEN_INPUT_COUNT                     */
				/***********************************************************************/
			
				mqInquireString(qAttrsBag, MQCA_CLUSTER_NAME, 0, MQ_CLUSTER_NAME_LENGTH, qCluster,  
								&qClusterLength, NULL, &compCode, &reason);
				CheckCallResult("Get queue name", compCode, reason);
				
				mqTrim(MQ_Q_NAME_LENGTH, qName, qName, &compCode, &reason);
				mqTrim(MQ_CLUSTER_NAME_LENGTH, qCluster, qCluster, &compCode, &reason);				

				if (strlen(qCluster) == 0) { strcpy(qCluster,"(null)"); }
				
				printf("%-48s %10s %10s %8s %3s %3s %-36s %-9s\n", qName, "-", "-", qCluster, "-", "-", "-" ,qTypePrint);
			break;
			case 6 :
				qTypePrint   = "REMOTE"; 
				/***********************************************************************/
				/* Get the type out of the queue attributes bag  MQIA_OPEN_INPUT_COUNT                     */
				/***********************************************************************/
				mqInquireString(qAttrsBag, MQCA_CLUSTER_NAME, 0, MQ_CLUSTER_NAME_LENGTH, qCluster,  
								&qClusterLength, NULL, &compCode, &reason);
				CheckCallResult("Get queue name", compCode, reason);
				
				/***********************************************************************/
				/* Get the type out of the queue attributes bag  MQIA_OPEN_INPUT_COUNT                     */
				/***********************************************************************/
				mqInquireString(qAttrsBag, MQCA_XMIT_Q_NAME, 0, MQ_Q_NAME_LENGTH, xmitQ,  
								&qNameLength, NULL, &compCode, &reason);
				CheckCallResult("Get queue name", compCode, reason);
			
				mqTrim(MQ_Q_NAME_LENGTH, qName, qName, &compCode, &reason);
				mqTrim(MQ_CLUSTER_NAME_LENGTH, qCluster, qCluster, &compCode, &reason);
				mqTrim(MQ_Q_NAME_LENGTH, xmitQ, xmitQ, &compCode, &reason);
				
				if (strlen(qCluster) == 0) { strcpy(qCluster,"(null)"); }
				if (strlen(xmitQ) == 0) { strcpy(xmitQ,"???"); }
				
				printf("%-48s %10s %10s %8s %3s %3s %-36s %-9s\n", qName, "-", "-", qCluster, "-", "-", xmitQ ,qTypePrint);
							break;
			default : /* Optional */
				// qTypePrint     = "OTHER";
				mqTrim(MQ_Q_NAME_LENGTH, qName, qName, &compCode, &reason);
				mqTrim(MQ_CLUSTER_NAME_LENGTH, qCluster, qCluster, &compCode, &reason);
				printf("%-48s %10ld %10ld %8s %3ld %3ld %36s %-9s\n", qName, "qDepth", "maxDdepth", "qCluster", "openIC", "openOC", "XMITQ" ,qTypePrint);

		}
	   
	   
  

     } 
	 
	 
	 
	 
   }

   else                                               /* Failed mqExecute     */
   {
     printf("Call to get queue attributes failed: Completion Code = %d : Reason = %d\n",
                  compCode, reason);
     /*************************************************************************/
     /* If the command fails get the system bag handle out of the mqexecute   */
     /* response bag.This bag contains the reason from the command server     */
     /* why the command failed.                                               */ 
     /*************************************************************************/
     if (reason == MQRCCF_COMMAND_FAILED)
     {
       mqInquireBag(responseBag, MQHA_BAG_HANDLE, 0, &errorBag, &compCode, &reason);
       CheckCallResult("Get the result bag handle", compCode, reason);
      
       /***********************************************************************/
       /* Get the completion code and reason code, returned by the command    */
       /* server, from the embedded error bag.                                */
       /***********************************************************************/
       mqInquireInteger(errorBag, MQIASY_COMP_CODE, MQIND_NONE, &mqExecuteCC, 
                        &compCode, &reason );
       CheckCallResult("Get the completion code from the result bag", compCode, reason); 
       mqInquireInteger(errorBag, MQIASY_REASON, MQIND_NONE, &mqExecuteRC,
                         &compCode, &reason);
       CheckCallResult("Get the reason code from the result bag", compCode, reason);
       printf("Error returned by the command server: Completion Code = %d : Reason = %d\n",
                mqExecuteCC, mqExecuteRC);       
     }
   }  

   /***************************************************************************/
   /* Delete the admin bag if successfully created.                           */
   /***************************************************************************/
   if (adminBag != MQHB_UNUSABLE_HBAG)
   {
      mqDeleteBag(&adminBag, &compCode, &reason);
      CheckCallResult("Delete the admin bag", compCode, reason);
   }

   /***************************************************************************/
   /* Delete the response bag if successfully created.                        */
   /***************************************************************************/
   if (responseBag != MQHB_UNUSABLE_HBAG)
   {
      mqDeleteBag(&responseBag, &compCode, &reason);
      CheckCallResult("Delete the response bag", compCode, reason);
   }

   /***************************************************************************/
   /* Disconnect from the queue manager if not already connected              */
   /***************************************************************************/
   if (connReason != MQRC_ALREADY_CONNECTED)
   {
      MQDISC(&hConn, &compCode, &reason);
       CheckCallResult("Disconnect from Queue Manager", compCode, reason);         
   } 
   return 0;                               
}


/******************************************************************************/
/*                                                                            */
/* Function: CheckCallResult                                                  */ 
/*                                                                            */
/******************************************************************************/
/*                                                                            */
/* Input Parameters:  Description of call                                     */
/*                    Completion code                                         */
/*                    Reason code                                             */
/*                                                                            */
/* Output Parameters: None                                                    */
/*                                                                            */
/* Logic: Display the description of the call, the completion code and the    */
/*        reason code if the completion code is not successful                */
/*                                                                            */
/******************************************************************************/
void  CheckCallResult(char *callText, MQLONG cc, MQLONG rc)
{
   if (cc != MQCC_OK)
         printf("%s failed: Completion Code = %d : Reason = %d\n", callText, cc, rc);

}
