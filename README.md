# Mercc
Mercc is a simple C interpreter which derived from merdog. [merdog](www.github.com/Httohu/merdog)
### Basic Type 
* int (i32)
* char (i8)
* bool 
* double (r64)
**you can define pointers , arrays and structure by these simple type**
### Control Flow 
* if-else
* while/for
* swich

### Some unsupported feature
* stdlib-> malloc
* union
* most of C libs except for stdio.h string.h math.h
* bit-fields
* static keyword
* some types (unsigned, short ,long and so on... unsigned will be finished in next version).
* register keyword / violate keyword
* and some other uncommon feature.


### How to use
1. using the following instruction, mercc should be added into the environment.
```
mercc filename
```
2. check the verison
```
mercc -v
```
3. You can download the Windows 10 version at microsoft app store.[C interpreter](https://www.microsoft.com/store/productId/9P13WCS0518Q), and the app don't support console input, but you can use pre_input
```c
    #pre_input
    xxxx
    #end_pre_input
```


### About Author 
By HttoHu(胡远韬)

Contact: [Email](huyuantao@outlook.com)
