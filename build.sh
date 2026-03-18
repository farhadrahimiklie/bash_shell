#!/bin/bash

# Colors (no bold)
RED="\033[31m"
GREEN="\033[32m"
RESET="\033[0m"

print_time() {
        local title="$1"
        local DATE=$(date "+%A, %d %B %Y")
        local TIME=$(date "+%I:%M:%S %p")

        echo -e "$title $DATE | $TIME"
}

# Start line
print_time "Compilation ${GREEN}Started${RESET} at"

# Compile
gcc -Wall -Wextra -Wpedantic shell.c -o main

# Run program
./main

# Finish line
print_time "Compilation ${RED}Finished${RESET} at"
