// Test for let block scope

var x = 0;
{
    let x = 1;
}
if( x == 0 ){
    test01 = 1;
}

{
    let y = 2;
    {
        let y = 3;
        if( y == 3){
            test02 = 1;
        }
    }
}

result = test01 == 1 && test02 == 1;
