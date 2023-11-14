tinyjs
========

fork of https://code.google.com/p/tiny-js/

TinyJS aims to be an extremely simple (~2000 line) JavaScript interpreter, meant for inclusion in applications that require a simple, familiar script language that can be included with no dependencies other than normal C++ libraries. It currently consists of two source files - one containing the interpreter, another containing built-in functions such as String.substring.

TinyJS is not designed to be fast or full-featured. However it is great for scripting simple behaviour, or loading & saving settings.

Currently TinyJS supports:

- Variables, Arrays, Structures
- JSON parsing and output
- Functions
- Calling C/C++ code from JavaScript
- Objects with Inheritance (not fully implemented)
- Please see examples that work at bottom of readme...

For a list of known issues, please see the comments at the top of the TinyJS.cpp file.

TinyJS is released under an MIT license.

Internal Structure
TinyJS uses a Recursive Descent Parser, so there is no 'Parser Generator' required. It does not compile to an intermediate code, and instead executes directly from source code. This makes it quite fast for code that is executed infrequently, and slow for loops.

Variables, Arrays and Objects are stored in a simple linked list tree structure (42tiny-js uses a C++ Map). This is simple, but relatively slow for large structures or arrays.

Simple for loop

    var a = 0;
    for (var i=1;i<10;i++) a = a + i;
    result = a==45;

Simple function

    function add(x,y) { return x+y; }
    result = add(3,6)==9;

Functions in variables using JSON-style initialisation

    var bob = { add : function(x,y) { return x+y; } };

    result = bob.add(3,6)==9;
    a = 345;    // an "integer", although there is only one numeric type in JavaScript
    b = 34.5;   // a floating-point number
    c = 3.45e2; // another floating-point, equivalent to 345
    d = 0377;   // an octal integer equal to 255
    e = 0xFF;   // a hexadecimal integer equal to 255, digits represented by the letters A-F may be upper or lowercase

    result = a==345 && b*10==345 && c==345 && d==255 && e==255;

References for arrays

    var a;
    a[0] = 10;
    a[1] = 22;

    b = a;

    b[0] = 5;

    result = a[0]==5 && a[1]==22 && b[1]==22;

References with functions
    
    var a = 42;
    var b;
    b[0] = 43;

    function foo(myarray) {
      myarray[0]++;
    }

    function bar(myvalue) {
      myvalue++;
    }

    foo(b);
    bar(a);

    result = a==42 && b[0]==44;

Built-in functions

    foo = "foo bar stuff";
    r = Math.rand();

    parsed = Integer.parseInt("42");

    aStr = "ABCD";
    aChar = aStr.charAt(0);

    obj1 = new Object();
    obj1.food = "cake";
    obj1.desert = "pie";

    obj2 = obj1.clone();
    obj2.food = "kittens";

    result = foo.length()==13 && foo.indexOf("bar")==4 && foo.substring(8,13)=="stuff" && parsed==42 && 
    Integer.valueOf(aChar)==65 && obj1.food=="cake" && obj2.desert=="pie";

New features

break,continue

    var a = 0;
    for (var i=1;i<10;i++) {
        if ( i == 2 ) continue;
        if ( i == 3 ) break;
        a = a + i;
    }
    
regurar expression


break in for,while support
regurar expression support
   preg_replace like PHP

