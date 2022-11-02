gcc -g mergeSort.c -o mergeSort
gcc -g psort.c -o psort
mergeSort ./inputfiles/input1.bin 
psort ./inputfiles/input1.bin
diff output1.bin output2.bin
ret=$?
if [[ $ret -eq 0 ]]; then
    echo "passed."
else
    echo "failed."
fi
rm -r output1.bin output2.bin
