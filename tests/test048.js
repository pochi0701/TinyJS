// test048: try/catch/finally/throw
var caughtVal = "";
var finallyRan = 0;

// 1) throw string → catch
try {
    throw "hello";
} catch(e) {
    caughtVal = e;
}

// 2) finally always runs
try {
    throw 42;
} catch(e) {
    // caught
} finally {
    finallyRan = 1;
}

// 3) finally runs even when no exception
var noExFinally = 0;
try {
    noExFinally = 10;
} finally {
    noExFinally = noExFinally + 1;
}

result = (caughtVal == "hello") && (finallyRan == 1) && (noExFinally == 11);
