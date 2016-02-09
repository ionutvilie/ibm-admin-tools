#!/usr/bin/ksh
#                                          
#   Script     : display qm objects
#                                         
#   Authors    : ionutvilie 
#              :
#			
#   Version  Date        Author  Desription
#   -------- ----------- ------- -----------------------------------
#   1.0.0    22.07.2014  ii      initial version
#   1.0.1    23.07.2014  ii      added qs option
#   2.0.0    27.08.2014  ii      added chl/chs/ixt/rc options
#                                basic channel operations reboot / start / stop
#
# set -x

nrofargs=$#
opt=$1
obj=$2
qm=$3
help_function () {
echo "\n\tUsage:\t\tdis object QM [or] dis option object QM"
echo "\tOptional:\tdis - display queue"
echo "\t\t\tdis q - display queue"
echo "\t\t\tdis qa - display queuealias"
echo "\t\t\tdis qc - display queuecluster"
echo "\t\t\tdis qr - display queueremote"
echo "\t\t\tdis ql - display queuelocal"
echo "\t\t\tdis qs - display queuestatus ('') type(handle)\n"
echo "\t\t\tdis chl - display channel"
echo "\t\t\tdis chs - display chstatus"
echo "\t\t\tdis xqi - display info from xmitq"
echo "\t\t\tdis resetchl - reset channel(stop/resolve(backout)/reset/start)"
echo "\t\t\tdis stopchl - stop channel"
echo "\t\t\tdis startchl - start channel"
}

extract_info_from_xmitq_queue () {
chl=`echo "dis ql('$obj')" | runmqsc $qm | awk '{print $1"\n"$2}' | grep 'TRIGDATA' | awk -F "(" '{print $2}'|awk -F ")" '{print $1}'`
conn=`echo "dis chl($chl)" | runmqsc $qm | awk '{print $1"\n"$2}' | grep 'CONNAME' | awk -F "(" '{print $2,$3}' | sed 's/))//g'`
echo "\n-----------------------------------------------"
echo "XMITQ:   \t $obj"
echo "CHANNEL: \t $chl"
echo "CONNAME: \t $conn"
echo "-----------------------------------------------\n"
}
restart_chl() {
echo "\n-----------------------------------------------"
echo "stop chl($obj)"                    | runmqsc $qm | grep 'AMQ[0-9]' && \
echo "resolve chl($obj) action(backout)" | runmqsc $qm | grep 'AMQ[0-9]' && \
echo "reset chl($obj) "                  | runmqsc $qm | grep 'AMQ[0-9]' && \
echo "start chl($obj)"                   | runmqsc $qm | grep 'AMQ[0-9]' 
echo "-----------------------------------------------\n"
}

one_arg () {
case $opt in
		h|help)
			help_function
		;;
		*)
			echo "try \"dis h|help\" for a full description"
		;; 
esac
}
 
two_args() {
		qm=$obj
		queue=$opt
        echo "dis q('$queue')" | runmqsc $qm
}

three_args(){
case $opt in
		q|qa|qc|ql|qr)
			echo "dis $opt('$obj')" | runmqsc $qm
		;;
		qs)
			echo "dis $opt('$obj') TYPE(HANDLE)" | runmqsc $qm
		;;
		xqi)
			extract_info_from_xmitq_queue
		;;
		chl)
			echo "dis $opt('$obj')" | runmqsc $qm
		;;			 
		chs)
			echo "dis $opt('$obj')" | runmqsc $qm
		;;
		resetchl)
			restart_chl
		;;
		stopchl)
			echo "stop chl('$obj')" | runmqsc $qm
		;;			
		startchl)
			echo "start chl('$obj')" | runmqsc $qm
		;;
		*)
			echo "try \"dis h|help\" for a full description"
		;;
esac
} 

if [[ "$#" == "1" ]]; then
one_arg
elif [[ "$#" == "2" ]]; then
two_args
elif [[ "$#" == "3" ]]; then
three_args
else
		help_function
		exit;
fi
exit 0
