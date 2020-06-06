# chc
A simple interpreter. My first real attmept, so keep that in mind.
CHC is a simple interpreted language which supports class and functions systems. It utilizes strict evaluation and is written completely in C++

**SYNTAX**
EOL:
CHC uses ; as an EOL\end of statement marker. Ex:
```
print("text");#correct#
print("text") #incorrect#
```
**CASE SENSITIVITY:**
CHC is case sensitive. Ex:
```
variable = 10
print(var); # 10 #
print(VAR); # Scoping error #
```
**WHITESPACE:**
CHC disregards whitespace. Ex:
```
if (condition) {
print("string");
}
#string#
```
```
if(condition) {
    print("string");
}
#string#
```
```
if (condition) {
    print("string");
    }
#string#
```
```
    if(condition) {
print("string");
}
#string#
```
etc.

**VARIABLE DECLARATION:**
CHC has type inference, as well as immutable types. Ex:
```
variable = 5; #5#
```
```
immutable variable = 5; #5#
variable = 3; #ERROR#
```
The variable types currently supported by CHC are: string and int

**FUNCTIONS:**
input:
```
string_variable = input("this will print out, then take user input.");
```
run:
```
run("this will be sent to CMD");
```
**MACROS:**
Supported macros:
```
@sec #second of the minute#
@min #minute of the day#
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
