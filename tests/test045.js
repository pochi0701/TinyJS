// do...while loop with continue/break

var i = 0;
var sum = 0;
do {
    i += 1;
    if (i == 2) continue;
    if (i > 4) break;
    sum += i;
} while (i < 10);

result = (sum == 8) && (i == 5);
