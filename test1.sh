gcc -g mergeSort.c -o mergeSort
gcc -g psort.c -o psort
mergeSort ./inputfiles/input1.bin ./testoutput/output1.bin
psort ./inputfiles/input1.bin ./testoutput/output2.bin
diff ./testoutput/output1.bin ./testoutput/output2.bin
ret=$?
if [[ $ret -eq 0 ]]; then
    echo "test 1 passed."
else
    echo "test 1 failed."
fi
rm -r ./testoutput/output1.bin ./testoutput/output2.bin

mergeSort ./inputfiles/input2.bin ./testoutput/output1.bin
psort ./inputfiles/input2.bin ./testoutput/output2.bin
diff ./testoutput/output1.bin ./testoutput/output2.bin
ret=$?
if [[ $ret -eq 0 ]]; then
    echo "test 2 passed."
else
    echo "test 2 failed."
fi
rm -r ./testoutput/output1.bin ./testoutput/output2.bin

mergeSort ./inputfiles/input3.bin ./testoutput/output1.bin
psort ./inputfiles/input3.bin ./testoutput/output2.bin
diff ./testoutput/output1.bin ./testoutput/output2.bin
ret=$?
if [[ $ret -eq 0 ]]; then
    echo "test 3 passed."
else
    echo "test 3 failed."
fi
rm -r ./testoutput/output1.bin ./testoutput/output2.bin

mergeSort ./inputfiles/input4.bin ./testoutput/output1.bin
psort ./inputfiles/input4.bin ./testoutput/output2.bin
diff ./testoutput/output1.bin ./testoutput/output2.bin
ret=$?
if [[ $ret -eq 0 ]]; then
    echo "test 4 passed."
else
    echo "test 4 failed."
fi
rm -r ./testoutput/output1.bin ./testoutput/output2.bin

mergeSort ./inputfiles/input5.bin ./testoutput/output1.bin
psort ./inputfiles/input5.bin ./testoutput/output2.bin
diff ./testoutput/output1.bin ./testoutput/output2.bin
ret=$?
if [[ $ret -eq 0 ]]; then
    echo "test 5 passed."
else
    echo "test 5 failed."
fi
rm -r ./testoutput/output1.bin ./testoutput/output2.bin

mergeSort ./inputfiles/input6.bin ./testoutput/output1.bin
psort ./inputfiles/input6.bin ./testoutput/output2.bin
diff ./testoutput/output1.bin ./testoutput/output2.bin
ret=$?
if [[ $ret -eq 0 ]]; then
    echo "test 6 passed."
else
    echo "test 6 failed."
fi
rm -r ./testoutput/output1.bin ./testoutput/output2.bin

mergeSort ./inputfiles/input7.bin ./testoutput/output1.bin
psort ./inputfiles/input7.bin ./testoutput/output2.bin
diff ./testoutput/output1.bin ./testoutput/output2.bin
ret=$?
if [[ $ret -eq 0 ]]; then
    echo "test 7 passed."
else
    echo "test 7 failed."
fi
rm -r ./testoutput/output1.bin ./testoutput/output2.bin

mergeSort ./inputfiles/input8.bin ./testoutput/output1.bin
psort ./inputfiles/input8.bin ./testoutput/output2.bin
diff ./testoutput/output1.bin ./testoutput/output2.bin
ret=$?
if [[ $ret -eq 0 ]]; then
    echo "test 8 passed."
else
    echo "test 8 failed."
fi
rm -r ./testoutput/output1.bin ./testoutput/output2.bin