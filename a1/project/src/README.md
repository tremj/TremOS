This submission used the starter code provided on MyCourses

I made a modification to the starter code to handle how differently the 'print' and 'echo' commands handle non-existant variables.

When a variable doesn't exist, calling 'print' prints "Variable does not exist" and calling 'echo' prints an empty line. To handle this I created a 'memory_return' struct that has a 'res' field for a result string, and a 'status' integer that signals success (0) or failure (-1). This allows us to not have to check if the result string does not equal "Variable does not exist" and allows for a user to set their variable value to "Variable does not exist" if they so wished to. The definition for the 'memory_return' struct is in the shell_memory.h file.
