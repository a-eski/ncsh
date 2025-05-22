#!/bin/bash

# Comments: Explain what the script does or specific sections
# This is a single-line comment

# Variables: Store data
name="World"
age=30

# Echo: Display text
echo "Hello, $name!"
echo "Age: $age"

# User Input: Get information from the user
read -p "Please enter your city: " city
echo "You live in $city."

# Conditional Statement: Make decisions
if [ $age -ge 18 ]; then
  echo "You are an adult."
else
  echo "You are a minor."
fi

# Loops: Repeat actions
# For loop
for i in 1 2 3 4 5; do
  echo "Number: $i"
done

# While loop
counter=0
while [ $counter -lt 3 ]; do
  echo "Counter: $counter"
  counter=$((counter + 1))
done

# Functions: Group code for reuse
greet() {
  echo "Greetings, $1!"
}

greet "User" # Calling the function

# Exit Status: Indicate success or failure
exit 0 # 0 means success, any other number indicates failure
