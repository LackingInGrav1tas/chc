# chc
A simple interpreter. Spaghetti code.
CHC is a simple interpreted language which supports functions. It utilizes lazy evaluation and is written completely in C++

SYNTAX


EOL:
CHC uses ; as an EOL\end of statement marker. Ex:
```
print("text");#correct#
print("text") #incorrect#
```
CASE SENSITIVITY:
CHC is case sensitive. Ex:
```
variable = 10;
print(var); # 10 #
print(VAR); # Scoping error #
```
WHITESPACE:
CHC disregards whitespace. Ex:
```
if (condition) {
print("string");
};
#string#
```
```
if(condition) {
    print("string");
};
#string#
```
```
if (condition) {
    print("string");
    };
#string#
```
```
    if(condition) {
print("string");
};
#string#
```
etc.

VARIABLE DECLARATION:
CHC has type inference, as well as immutable types. Ex:
```
variable = 5; #5#
```
```
immutable variable = 5; #5#
variable = 3; #ERROR#
```
The variable types currently supported by CHC are: string, bool, num

FUNCTIONS:

Native Functions:

input:
```
string_variable = input("this will print out, then take user input.");
```
run:
```
run("this will be sent to CMD");
```
versions of print:
```
print("This will be sent to standard output.");
#This will be sent to standard output.#
fprint("This will be sent to standard output."); #flush print#
#This will be sent to standard output. + flush#
rprint("This will be sent to standard output."); #raw print#
#"This will be sent to standard output."#
rfprint("This will be sent to standard output."); #raw flush print#
#"This will be sent to standard output." + flush#
```
writeto:
```
success = writeto("filepath", "to be wrote to file", %either @write or @append%);
writeto("filepath", "to be wrote to file", %either @write or @append%);
```
assert:
```
a = "some text";
assert(a == "some other text");
2| assert(a == "some other text");
   ^
Run-time Error: Assertion failed.
```
eval:
```
variable = eval(true == true);
```
length:
```
a = length("text"); #4#
```
rand:
```
a_random_num_between_zero_and_fifty = rand(50);
```
etc.

Methods:

Methods are called with the ```<-``` arrow token on the object.

string<-at(index)
```
string = "abcdefg";
segment = string<-at(0);#"a"#
segment2 = "abcdefg"<-at(0);#"a"#
```
string<-length()
```
string = "abcdefg";
len = string<-length();#7#
len2 = "abcdefgh"<-length();#7#
```

User defined functions:

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
foo(); #error#
var = "something";
foo();#something#
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
MACROS:

Supported macros:
```
@write #used to define the mode of file handling#
@append #^#
@sec #second of the minute#
@min #minute of the hour#
@hour #hour of the day#
@mday #day of the month#
@yday #day of the year#
@mon #month, January = 1#
@year #guess#
@clipboard #clipboard#
@home #returns the home drive letter#
@desktopH #desktop height in pixels#
@desktopW #desktop width in pixels#
@environment #prints out the current variable information#
@IP #returns the IP address of the PC#
@EOL #end of line signifier for strings#
```

Preprocessors:

Any preprocessors are defined using ```$``` before the statement.
Currently, the supported options are:

dismiss (used to remove native functions):
```
$dismiss assert
fun assert() {
    ...
};
#no error#
```
or
```
$dismiss assert
assert(true == true);
#error#
```
limit (changes the loop limit, 0=no limit. Can also be changed in the command line):
```
$limit 1
while (true) {};
2| while (true) {};
Terminate after control finds repeating while loop, limit: 1
```
import (allows for the use of multiple files):

a.chc
```
fun foo() {
    print("foo");
};
a =  "a";
```
b.chc
```
$include "c:\...\a.chc"
fprint(a);
foo();
```
```interpreter c:\...\b.chc
a
foo
