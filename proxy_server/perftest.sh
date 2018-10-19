#!/bin/bash

for file in /home/anukriti/test/PDP_Master_Thesis/proxy_server/sample/*
do
        fileN=${file##*/}
	cmd="time curl -H Transfer-Encoding:chunked -F FileName="$fileN" -F Secured="true" -F FileData=@"$file" http://192.168.56.101:6666/resources/"  $'\r' >> challenge_req.txt
	echo $cmd
	output=`($cmd 2>&1)`
        echo $output
	ret=`echo $output | sed -n 's/.*system//;s/elapsed.*//p'`
	echo "\n======\nRETURN : " + $ret + "\n=======\n"
	echo $fileN,$ret >> sendFiles.csv
done

for file in /home/anukriti/test/PDP_Master_Thesis/proxy_server/sample/*
do
        fileN=${file##*/}
	cmd="time curl http://192.168.56.101:6666/resources/"$fileN" -o first_rep.txt"
        echo $cmd
        output=`($cmd 2>&1)`
        ret=`echo $output | sed -n 's/.*system//;s/elapsed.*//p'`
        echo "\n======\nRETURN : " + $ret + "\n=======\n"
        echo ${fileN##*/},$ret >> getFiles.csv
done	

for file in /home/anukriti/test/PDP_Master_Thesis/proxy_server/sample/*
do
        fileN=${file##*/}
	cmd="time curl -XCHALLENGE http://192.168.56.101:6666/challenge/"$fileN"" $'\r' >> challenge_rep.txt
        echo $cmd
        output=`($cmd 2>&1)`
        ret=`echo $output | sed -n 's/.*system//;s/elapsed.*//p'`
        echo "\n======\nRETURN : " + $ret + "\n=======\n"
        echo ${fileN##*/},$ret >> getFiles.csv
done	


