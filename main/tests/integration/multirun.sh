#! /bin/bash

counter=1000
step=$(( counter / 10 ))
perfnew=0
perf1=0
perfmmap=0
perfReadSB=0
perfGetLine=0
while [ $counter -gt 0 ]; do
    perfnew=$(( $perfnew + $(echo "Over new:" `../src/bin/FeedHandler.out ../tests/test1.txt 2> /dev/null | grep Over` | cut -d " " -f8) ));
    perf1=$(( $perf1 + $(echo "Over 1.0:" `../src/FeedHandler_1.0.out ../tests/test1.txt 2> /dev/null | grep Over` | cut -d " " -f8) ));
    perfmmap=$(( $perfmmap + $(echo "Over mmap:" `../src/FeedHandler_mmap.out ../tests/test1.txt 2> /dev/null | grep Over` | cut -d " " -f9) ));
#    perfReadSB=$(( $perfReadSB + $(echo "Over readSB:" `../src/FeedHandler_readSB.out ../tests/test1.txt 2> /dev/null | grep Over` | cut -d " " -f8) ));
#    perfGetLine=$(( $perfGetLine + $(echo "Over getline:" `../src/FeedHandler_getline.out ../tests/test1.txt 2> /dev/null | grep Over` | cut -d " " -f8) ));
    counter=$(( $counter - 1 ));
    if [[ $(( counter % $step )) == 0 ]]; then
        echo $perfnew $perf1 $perfmmap
    fi
done
echo $perfnew $perf1 $perfmmap

