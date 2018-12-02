#!/bin/bash

for file in /home/ubuntu/test_files/*
do
        fileN=${file##*/}
        cmd="time curl -H Transfer-Encoding:chunked -F FileName="$fileN" -F FileData=@"$file" http://172.28.1.180:6666/resources/"
        echo $cmd
        output=`($cmd 2>&1)`
        echo $output
        ret=`echo $output | sed -n 's/.*system//;s/elapsed.*//p'`
        echo "\n======\nRETURN : " + $ret + "\n=======\n"
        echo $fileN,$ret >> sendFiles_2CN.csv
done

for file in /home/ubuntu/test_files/*
do
        fileN=${file##*/}
        cmd="time curl http://172.28.1.180:6666/resources/"$fileN" -o first_rep.txt"
        echo $cmd
        output=`($cmd 2>&1)`
        ret=`echo $output | sed -n 's/.*system//;s/elapsed.*//p'`
        echo "\n======\nRETURN : " + $ret + "\n=======\n"
        echo ${fileN##*/},$ret >> getFiles_2CN.csv
done

