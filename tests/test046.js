// typeof + NaN/Infinity + isNaN/isFinite

var u;
var o = {};
function f(x) { return x; }

result =
    (typeof u == "undefined") &&
    (typeof 1 == "number") &&
    (typeof "abc" == "string") &&
    (typeof o == "object") &&
    (typeof f == "function") &&
    isNaN(NaN) &&
    !isNaN(1) &&
    isFinite(123.5) &&
    !isFinite(Infinity);
