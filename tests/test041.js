// Test for for,while,break,continue

var array = ['a','b','c','d','e'];

// ['a','b','c','d','e','f'];
array.push('f');
var result01 = (array[5] == 'f');

// ['a','b','c','d','e'];
array.pop();
var result02 = (array.length == 5);

// ['b','c','d','e'];
array.shift();
var result03 = (array[0] == 'b');

// ['a','b','c','d','e'];
array.unshift('a');
var result04 = (array[0] == 'a');

// ['a','b','c','d','e'];
var result05 = (array.indexOf('b') == 1);

// ['b','c'];
var array2 = array.slice(1,3);
var result06 = (array2[1] == 'c');

array.splice(0,3);
var result07 = (array.length == 2 && array[0] == 'd');
print("OK");
result = result01 && result02 && result03 && result04 && result05 && result06;
