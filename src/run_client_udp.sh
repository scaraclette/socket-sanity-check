# localhost
# rm -f client-udp.out; c++ client-udp.cpp -o client-udp.out; ./client-udp.out localhost >output.txt 2>&1;

# Test case 4 needs window size
rm -f client-udp.out; c++ client-udp.cpp -o client-udp.out; ./client-udp.out | tee client_output.txt;




