#! /bin/bash

counter=1000
step=$(( counter / 10 ))
perfnew=0
perfnew2=0
perfReadSB=0
perfGetLine=0
while [ $counter -gt 0 ]; do
    perfnew=$(( $perfnew + $(echo "Over new:" `../src/bin/FeedHandler.out ../tests/test1.txt 2> /dev/null | grep Over` | cut -d " " -f8) ));
#    perfnew2=$(( $perfnew2 + $(echo "Over new2:" `../src/FeedHandler_mmap.out ../tests/test1.txt 2> /dev/null | grep Over` | cut -d " " -f8) ));
#    perfReadSB=$(( $perfReadSB + $(echo "Over readSB:" `../src/FeedHandler_readSB.out ../tests/test1.txt 2> /dev/null | grep Over` | cut -d " " -f8) ));
#    perfGetLine=$(( $perfGetLine + $(echo "Over getline:" `../src/FeedHandler_getline.out ../tests/test1.txt 2> /dev/null | grep Over` | cut -d " " -f8) ));
    counter=$(( $counter - 1 ));
    if [[ $(( counter % $step )) == 0 ]]; then
        echo $perfnew
#        echo $perfnew $perfnew2 $perfReadSB $perfGetLine
    fi
done
#echo $perfnew $perfnew2 $perfReadSB $perfGetLine
echo $perfnew

