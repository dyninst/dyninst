#include <stdio.h>

int main(){
    int x = 2;
    x = x ^2;
    switch (x){
        case 0:
            x=0;
            break;
        case 1:
            x=1;
            break;
        case 2:
            x=2;
            break;
        case 3:
            x=3;
            break;
        case 4:
            x=4;
            break;
        case 5:
            x=5;
        case 6:
            x=6;
            break;
        case 7:
            x=7;
            break;
        default:
            return 1;
    }
    return 0;
}
