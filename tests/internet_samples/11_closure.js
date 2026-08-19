function make(x){return function(){return x*2;};} var f=make(5); result = (f()===10) ? 1 : 0;
