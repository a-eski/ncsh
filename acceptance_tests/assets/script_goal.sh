# /bin/ncsh

STR=hello
echo $STR

STR2=there
echo $STR $STR2

ls | wc -c

# ignored

ls | wc -c && ls | wc -c

# Variables: Store and manipulate data
name="World"
age=30

# Echo command: Print output to the console
echo "Hello, $name!"
echo "Age: $age"

# User Input: Read input from the user
read -p "Please enter your city: " city
echo "You live in: $city"

# Conditional statement: Execute code based on a condition
if [ $age -ge 18 ]; then
  echo "You are an adult."
else
  echo "You are a minor."
fi

# Loop: Repeat a block of code multiple times
for i in 1 2 3 4 5; do
  echo "Number: $i"
done

# Function: Define reusable blocks of code
greet() {
  echo "Greetings, $1!"
}

greet "User"

# File operations
touch my_file.txt
echo "This is some text" > my_file.txt
rm my_file.txt

echo 'finished!'
