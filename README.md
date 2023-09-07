# find-files
Linux based project in C that can find files, directories, or text in directory/subdirectories according to the specific input the user enters. Utilizes up to 10 children processes and pipe to communicate, allowing parent to accept input any time. 

# to compile
gcc *c

# to run
./a.out

# to find file in current directory
find example.txt

# to find file in current directory and all it's subdirectories
find example.txt -s

# to find text in current directory
find "hello" 

# to find text in current directory and all it's subdirectories
find "hello" -s

# to find text in a certain file type in current directory and all it's subdirectories (ex: in all .c files)
find "hello" -f:c -s
