#!/usr/bin/env perl
use warnings;
use strict;

if ($#ARGV != 1) {
    print qq(
        ERROR:

            This script requires 2 arguments
              - Broker Name
              - Execution Group

        eg: $0 WMB01 EG01

    );
    exit;
}

my $msgFlowCounter = 0;
my $ComIbmMQInputNode = 0;
my $ComIbmMQOutputNode = 0;


my $wmBroker = "$ARGV[0]";
my $wmbEG = "$ARGV[1]";

my $mqsireportproperties=qq(mqsireportproperties $wmBroker -e $wmbEG -o AllMessageFlows -r 2>&1);

open (my $fh, '-|', $mqsireportproperties ) or die $!;
while (my $line = <$fh>) {
    if ( $line =~ m{^MessageFlow.*} ) {
        $msgFlowCounter += 1;
        printf("%-4d %s", $msgFlowCounter, $line);
    }

    my $flowName = '^\s\slabel=+\'(.*)\'';
    if ( $line =~ m{^$flowName} ) { printf("\t%-20s %s \n", "FlowName", $1); }
    # a switch in order to know what type of queue is it <input|output>
    # ComIbmMQInputNode
    if ( $line =~ m{^\s+ComIbmMQInputNode.*} )  { $ComIbmMQInputNode = 1; }
    # ComIbmMQInputNode
    if ( $line =~ m{^\s+ComIbmMQOutputNode.*} ) { $ComIbmMQOutputNode = 1; }
    # ComIbmFileOutputNode / don't need it
    # if ( $line =~ m{^\s+ComIbmFileOutputNode.*} ) { $ComIbmFileOutputNode = 1; }


    my $queueName = '^\s+queueName=\'(.*)\'';
    if ( $line =~ m{^$queueName} ) {
        if ($ComIbmMQInputNode == 1) {
            $ComIbmMQInputNode = 0;
            if ( $1 ) { printf("\t%-20s %s \n", "inputQueueName", $1);} # print only if $1 is not empty
        }
        elsif($ComIbmMQOutputNode == 1) {
            $ComIbmMQOutputNode = 0;
            if ( $1 ) { printf("\t%-20s %s \n", "outputQueueName", $1); } } # print only if $1 is not empty
    }


    # Printing ComIbmFileOutputNode: outputDirectory outputFilename
    my $outputDirectory = '^\s+outputDirectory=\'(.*)\'';
    my $outputFilename = '^\s+outputFilename=\'(.*)\'';
    if ( $line =~ m{^$outputDirectory} ) {
        if ( $1 ) { printf("\t%-20s %s \n", "outputDirectory", $1); } # print only if $1 is not empty
    }
    if ( $line =~ m{^$outputFilename} ) {
        if ( $1 ) { printf("\t%-20s %s \n", "outputFilename", $1); } # print only if $1 is not empty
    }

}
close $fh;
