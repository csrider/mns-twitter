#!/bin/sh
#
# strttwitter.sh
#
#	Will stop and restart the twitter process
#

SILENTM_HOME=`cat /etc/silentm/home`

if [ $(ls -l /bin/sh | grep -c dash) -eq 1 ]
then
        # pure posix shells (e.g. dash on ubuntu) don't like the bash-specific source keyword
        ##### REDACTED #####
else
        # bash on centos is fine with source
        source ##### REDACTED #####
fi

host=$MACHINENAMESHORT

ps axw > /tmp/strttwitter.$host$DOTCOMPANY

# stop SNPP on host
if [ "$COMPANY" = "" ]
then
	/bin/kill -s SIGINT `cat /tmp/strttwitter.$host$DOTCOMPANY | grep -v strttwitter | grep smtwitter | awk '{print $1}'` 2> /dev/null
else
	/bin/kill -s SIGINT `cat /tmp/strttwitter.$host$DOTCOMPANY | grep -v strttwitter | grep smtwitter | grep $COMPANY | awk '{print $1}'` 2> /dev/null
fi

##### REDACTED #####

if [ "$SNPP_PROCESS_NUMBER" = "" ]
then
	# start SNPP on host
	date >> $SILENTM_HOME/log/$COMPANY/smtwitter.$host.log
	nohup $SILENTM_HOME/bin/smtwitter $COMPANY_ARG $COMPANY >> /dev/null 2> /dev/null &
else
	# start SNPP virtual pids (upto 20 max)
	virtual_pid=0
	while [ $virtual_pid -lt $SNPP_PROCESS_NUMBER -a $virtual_pid -lt 20 ]
	do
		nohup $SILENTM_HOME/bin/smtwitter $COMPANY_ARG $COMPANY -virtual_pid $virtual_pid >> /dev/null 2> /dev/null &
		virtual_pid=`expr $virtual_pid + 1`
	done	
fi

rm -f /tmp/strttwitter.$host$DOTCOMPANY
