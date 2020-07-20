# chc #

CHC is a simple interpreted language. It utilizes lazy evaluation and is written completely in C++.

### SYNTAX ###

**EOL:**
CHC uses ; as an EOL\end of statement marker. Ex:
```
print("text");#correct
print("text") #incorrect
```
**CASE SENSITIVITY:**
CHC is case sensitive. Ex:
```
variable = 10;
print(var); # 10
print(VAR); # Scoping error
```
**WHITESPACE:**
CHC disregards whitespace. Ex:
```
if (condition) {
print("string");
};
#string
```
```
if(condition) {
    print("string");
};
#string
```
```
if (condition) {
    print("string");
    };
#string
```
```
    if(condition) {
print("string");
};
#string
```
etc.

**VARIABLE DECLARATION:**
CHC has type inference, as well as immutable types. Ex:
```
variable = 5; #5
```
```
immutable variable = 5; #5
variable = 3; #ERROR
```
The variable types currently supported by CHC are: string, bool, num

### FUNCTIONS: ###

**Native Functions:**

*input (returns: string):*
```
string_variable = input("this will print out, then take user input.");
```
 
*run (returns: void):*
```
run("this will be sent to CMD");
```

*versions of print (returns: void):*
```
print("This will be sent to standard output.");
#This will be sent to standard output.
fprint("This will be sent to standard output."); #flush print
#This will be sent to standard output. + flush
rprint("This will be sent to standard output."); #raw print
#"This will be sent to standard output."
rfprint("This will be sent to standard output."); #raw flush print
#"This will be sent to standard output." + flush
```
 
*writeto (returns: void):*
```
success = writeto("filepath", "to be wrote to file", %either @write or @append%);
writeto("filepath", "to be wrote to file", %either @write or @append%);
```
 
*assert (returns: void):*
```
a = "some text";
assert(a == "some other text");
2| assert(a == "some other text");
   ^
Run-time Error: Assertion failed.
```

*eval (returns: bool):*
```
variable = eval(true == true);
``` 
*rand (returns: number):*
```
a_random_num_between_zero_and_fifty = rand(50);
```
*str (returns: string):*
```
string = str(45);
rprint(string);
# "45"
```
*length (returns: num):*
```
sizeofin = length(input(""));# gets length of input
```
*save_scope, set_scope (returns: void):*
```
save_scope(%literal%);
set_scope(%literal%);
#example
save_scope(1);# saves environment
a = "a";
set_scope(1);# sets environment
fprint(a);# scope error
```
*is_string, is_bool, is_number (returns: bool):*
```
a = "string";
variable = is_bool(a);//false
``` 
*to_str (returns: string):*
```
variable = to_str(47); #"47"
``` 
*to_num (returns: number)*
```
variable = to_num("47"); #47
``` 


**User defined functions:**

Declare functions by using the ```fun``` keyword. Ex.
```
fun foo() {
    return("something");
};
```
Scoped functions can be made using the ```aware``` keyword, which gives the function the full environment of it's call location. Ex.
```
aware fun foo() {
    print(var);
};
foo(); #error
var = "something";
foo();#something
```
Additionally, ```aware``` can be used to create recursive functions.
```
aware fun recursive(n) {
    n = n * n;
    print(n + @EOL);
    if (n > 50) {
        return(n);
    };
    to_return = recursive(n);
    return(to_return);
};

r = recursive(2);
```

### TOOLS ###

if:
```
if (condition) {
    #execute
};
```
while:
```
while (condition) {
    #execute
};
do {
    #execute
} while (condition);
```
use/disable:
```
use #@strict, @errors, @output, @warnings, @assume#;
disable #^#;
```
paste (adds code to existing functions):
```
fun foo() {
    fprint("line 1" + @EOL);
};
foo();
paste(foo) {
    fprint("line 2");
};
foo();
```

```
line 1

line 1
line 2
```
### MACROS: ###

Supported macros:
```
@write #used to define the mode of file handling
@append #^

@errors #used for use and disable, turns off/on error printout
@output #^, turns off/on print output
@strict #^, turns off/on strict mode
@warnings #^, turns off/on warnings
@assume #^, turns off/on assume mode. When chc detects a variable out of scope, instead of throwing an error, it finds the closest variable to it in scope.

@sec #second of the minute
@min #minute of the hour
@hour #hour of the day
@mday #day of the month
@yday #day of the year
@mon #month, January = 1
@year #guess
@clipboard #clipboard
@home #returns the home drive letter
@environment #prints out the current variable information
@IP #returns the IP address of the PC
@EOL #end of line signifier for strings
```

### Preprocessors: ###

Any preprocessors are defined using ```$``` before the statement.
Currently, the supported options are:

*dismiss (used to remove native functions):*
```
$dismiss assert
fun assert() {
    ...
};
#no error
```
or
```
$dismiss assert
assert(true == true);
#error
```
*limit (changes the loop limit, 0=no limit. Can also be changed in the command line):*
```
$limit 1
while (true) {};
2| while (true) {};
Terminate after control finds repeating while loop, limit: 1
```
*import (allows for the use of multiple files):*

a.chc
```
fun foo() {
    print("foo");
};
a =  "a's contents";
```
b.chc
```
$import "c:\...\a.chc"
fprint(a);
foo();
```
Output of b:
```
a's contents
foo
```
Using files in lib:

test.chc
```
$import lib:strings
print(back("abc"));
```
output:
```
c
```
*precision (changes number precision, auto is 6, inf for infinity)*
```
$precision 1
num = 3.75/2;
print(num);
```
```
1.9
```

### Command Line Interface ###

Just type ```drive:/.../chc``` into cmd/command line to open interface. 
Type anything that crashes the lexer, or "quit" or "exit" to exit the cli.
Preprocessors don't work, nor do multiline things such as functions because I dont care enough to implement them.
