function make(x){var y=x*2; return function(){return y;};} var f=make(5); result = f();
