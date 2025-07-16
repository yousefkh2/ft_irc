
#!/bin/bash
printf "PASS pass\r\n"
sleep 0.1
printf "NICK testuser\r\n" 
sleep 0.1
printf "USER testuser 0 * :Real Name\r\n"
sleep 1  # Wait to see responses before nc closes

# Run it
./test.sh | nc localhost 6667