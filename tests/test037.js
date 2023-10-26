// simple for loop containing initialisation, using +=
var a = 0;
for (var i=1;i<10;i++)
{
   if(i==10){
      break;
   }
   a += i;
}
result = a==45;
