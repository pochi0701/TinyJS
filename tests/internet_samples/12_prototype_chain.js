function Animal(n){this.name=n;} Animal.prototype.greet=function(){return "Hi "+this.name;}; var a=new Animal("Cat"); result = (a.greet()==="Hi Cat") ? 1 : 0;
