# localhost
# rm -f client-udp.out; c++ client-udp.cpp -o client-udp.out; ./client-udp.out localhost >output.txt 2>&1;

# Test case 4 needs window size
rm -f client-udp.out; c++ client-udp.cpp -o client-udp.out; ./client-udp.out | tee -a client_output-0.txt;

# Linux lab 3
# rm -f client-udp.out; c++ client-udp.cpp -o client-udp.out; ./client-udp.out 10.155.176.28;

# Linux lab 5
# rm -f client-udp.out; c++ client-udp.cpp -o client-udp.out; ./client-udp.out 10.155.176.20;
