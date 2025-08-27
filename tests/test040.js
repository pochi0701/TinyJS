// Test for for,while,break,continue

var sum1 = 0;
for( var i = 0 ; i < 20 ; i++ ){
    if( i >= 10) continue;
    sum1 += i;
}

var sum2 = 0;
for( var j = 0 ; j < 20 ; j++ ){
    if( j == 10) break;
    sum2 += j;
}

var sum3 = 0;
var k = 0;
while( k < 20 ){
    if( k == 10 ) break;
    sum3 += k;
    k += 1;
}

var sum4 = 0;
var l = 0;
while( l < 20 ){
    if( l >= 10 ) {
        l += 1;
        continue;
    }
    sum4 += l;
    l += 1;
}

result = (sum1 == 45) && (sum2 == 45) && (sum3 == 45) && (sum4 == 45);
