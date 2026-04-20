// test042 - variadic arguments and arguments object

// 1. arguments object is available in user-defined functions
function testArgs(a, b) {
  return arguments.length;
}
var result01 = (testArgs(1, 2) == 2);
var result02 = (testArgs(1, 2, 3) == 3);
var result03 = (testArgs(10) == 1);

// 2. arguments array contains all passed values
function sumAll() {
  var total = 0;
  var i = 0;
  while (i < arguments.length) {
    total = total + arguments[i];
    i = i + 1;
  }
  return total;
}
var result04 = (sumAll(1, 2, 3) == 6);
var result05 = (sumAll(10, 20, 30, 40) == 100);

// 3. named params still work alongside arguments
function namedAndExtra(a, b) {
  return a + b + arguments.length;
}
var result06 = (namedAndExtra(10, 20) == 32);
var result07 = (namedAndExtra(10, 20, 99) == 33);

// 4. String.substring with omitted second argument
var s = "Hello World";
var result08 = (s.substring(6) == "World");
var result09 = (s.substring(0, 5) == "Hello");
var result10 = (s.substring(0) == "Hello World");

result = result01 && result02 && result03 && result04 && result05 && result06 && result07 && result08 && result09 && result10;
