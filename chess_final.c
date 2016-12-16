#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define RED "\x1b[31m"
#define BLUE "\x1b[34m"
#define RESET "\x1b[0m"

typedef enum {blank=0,bpawn,bknight,bbishop,brook,bqueen,bking,wpawn,wknight,wbishop,wrook,wqueen,wking} piece;
piece board[8][8];
int enpass_a=-1,enpass_b=-1;
const piece startup[8][8]={wrook,wknight,wbishop,wqueen,wking,wbishop,wknight,wrook,wpawn,wpawn,wpawn,wpawn,wpawn,wpawn,wpawn,wpawn,blank,blank,blank,blank,blank,blank,blank,blank,blank,blank,blank,blank,blank,blank,blank, blank, blank, blank, blank, blank, blank, blank, blank, blank, blank, blank, blank, blank, blank, blank, blank,blank, bpawn, bpawn, bpawn, bpawn, bpawn, bpawn, bpawn, bpawn, brook, bknight, bbishop, bqueen, bking, bbishop, bknight, brook};

void setup(){
    int i,j;
    for(i=0;i<8;i++){
        for(j=0;j<8;j++){
            board[i][j]=startup[i][j];
        }
    }
}

void printboard(){
    int a,b;
    char piece;
    for(a=7;a>-1;a--){
        printf("\n %d| ",a);
        for(b=0;b<8;b++){
            switch(board[a][b]){
        case blank:
            piece = ' ';
            break;
        case wpawn:
            piece = 'P';
            break;
        case wknight:
            piece = 'N';
            break;
        case wbishop:
            piece = 'B';
            break;
        case wrook:
            piece = 'R';
            break;
        case wking:
            piece = 'K';
            break;
        case wqueen:
            piece = 'Q';
            break;
        case bpawn:
            piece = 'p';
            break;
        case bknight:
            piece = 'n';
            break;
        case bbishop:
            piece = 'b';
            break;
        case brook:
            piece = 'r';
            break;
        case bqueen:
            piece = 'q';
            break;
        case bking:
            piece = 'k';
            break;
            }
            if(piece==32) printf("[%c]", piece);
            if(piece<97&&piece!=32) printf(RED"[%c]"RESET, piece);
            if(piece>=97) printf(BLUE"[%c]"RESET, piece);
        }
    }
    printf("\n");
    printf("    ________________________\n");
    printf("     0  1  2  3  4  5  6  7\n");
}

void Move(int a,int b,int c,int d){
    board[c][d] = board[a][b];
    board[a][b] = blank;
}
int ValidSquare(int a,int b,int c,int d)
{
    if((a<0)||a>7) return 0;
    if((b<0)||b>7) return 0;
    if((c<0)||c>7) return 0;
    if((d<0)||d>7) return 0;

    return 1;
}
int CheckBlack(int *whitelist)
{
    int x,y; // WKing's position
    int a,b,c,d;
    int i;
    int num;
    int result=0;
    for(a=0;a<8;a++){
        for(b=0;b<8;b++){
            if(board[a][b]==bking){
                x=a;
                y=b;
                break;
            }
        }
    }
    for(a=0;a<8;a++){
  //      if(result==1) break;
        for(b=0;b<8;b++){
     //       if(result==1) break;
            for(i=0;i<218;i++){
                num = a*1000+b*100+x*10+y;
                if(num == *(whitelist+i)) {
                    result = 1;
                    break;
                }
            }
        }
    }
    return result;
}

int *BlackMoveList(int epa,int epb)
{
    int i;
    static int allmoves[218];
    int fromSQa[218];
    int fromSQb[218];
    int toSQc[218];
    int toSQd[218];
    int a=0;
    int b=0;
    int c=0;
    int d=0;
    int e=0;
    int f=0;
    int z=0;
    for(i=0;i<218;i++){
        allmoves[i]=0;
    }
 //   printf("\nepa: %d, epb: %d\n",epa,epb);
    for(a=0;a<8;a++){
        for(b=0;b<8;b++){
            switch(board[a][b])
            {
            case bqueen:
                    for(e = -1;e <= 1;e++){
                        for(f= -1;f <=1;f++){
                            c= a+e;
                            d= b+f;
                            if(!(f==0&&e==0)){
                                do{
                                if(ValidSquare(a,b,c,d)!=0){
                                if(board[c][d]==0){
                                    fromSQa[z]=a;
                                    fromSQb[z]=b;
                                    toSQc[z]=c;
                                    toSQd[z]=d;
                                    allmoves[z]=fromSQa[z]*1000+fromSQb[z]*100+toSQc[z]*10+toSQd[z];
                                    z++;
                                }
                                if(board[c][d]>6){
                                    fromSQa[z]=a;
                                    fromSQb[z]=b;
                                    toSQc[z]=c;
                                    toSQd[z]=d;
                                    allmoves[z]=fromSQa[z]*1000+fromSQb[z]*100+toSQc[z]*10+toSQd[z];
                                    z++;
                                    break;
                                }
                                if(board[c][d]<7&&board[c][d]!=0) break;
                                c = c+e;
                                d = d+f;
                                }
                            }
                            while(ValidSquare(a,b,c,d)!=0);
                        }
                    }

                }
                break;
            case bpawn:
                c = a-1;
                d = b-1;
                if(ValidSquare(a,b,c,d)!=0){
                    if(board[c][d]>6||(c==epa&&d==epb)){
                        fromSQa[z] = a;
                        fromSQb[z] = b;
                        toSQc[z] = c;
                        toSQd[z] = d;
                        allmoves[z]=fromSQa[z]*1000+fromSQb[z]*100+toSQc[z]*10+toSQd[z];
                        z++;
                    }
                }
                c = a-1;
                d = b+1;
                if(ValidSquare(a,b,c,d)!=0){
                    if(board[c][d]>6||(c==epa&&d==epb)){
                        fromSQa[z] = a;
                        fromSQb[z] = b;
                        toSQc[z] = c;
                        toSQd[z] = d;
                        allmoves[z]=fromSQa[z]*1000+fromSQb[z]*100+toSQc[z]*10+toSQd[z];
                        z++;
                    }
                }
                if(a==6){
                    c=5;
                    d=b;
                    if(ValidSquare(a,b,c,d)!=0){
                        if(board[c][d]==0){
                            fromSQa[z] = a;
                            fromSQb[z] = b;
                            toSQc[z] = c;
                            toSQd[z] = d;
                            allmoves[z]=fromSQa[z]*1000+fromSQb[z]*100+toSQc[z]*10+toSQd[z];
                            z++;
                            c = 4;
                            d = b;
                            if(ValidSquare(a,b,c,d)!=0){
                                if(board[c][d]==0){
                                    fromSQa[z] = a;
                                    fromSQb[z] = b;
                                    toSQc[z] = c;
                                    toSQd[z] = d;
                                    allmoves[z]=fromSQa[z]*1000+fromSQb[z]*100+toSQc[z]*10+toSQd[z];
                                    z++;
                                }
                                }
                            }
                        }
                    }
                else{
                    c = a-1;
                    d = b;
                    if(ValidSquare(a,b,c,d)!=0){
                        if(board[c][d]==0){
                            fromSQa[z] = a;
                            fromSQb[z] = b;
                            toSQc[z] = c;
                            toSQd[z] = d;
                            allmoves[z]=fromSQa[z]*1000+fromSQb[z]*100+toSQc[z]*10+toSQd[z];
                            z++;
                        }
                    }
                }
                break;
            case brook:
                for(e=-1;e<=1;e++){
                    for(f=-1;f<=1;f++){
                        c = a+e;
                        d = b+f;
                        if(abs(e)+abs(f)==1){
                            do{
                                if(ValidSquare(a,b,c,d)!=0){
                                    if(board[c][d]==0){
                                        fromSQa[z] = a;
                                        fromSQb[z] = b;
                                        toSQc[z] = c;
                                        toSQd[z] = d;
                                        allmoves[z]=fromSQa[z]*1000+fromSQb[z]*100+toSQc[z]*10+toSQd[z];
                                        z++;
                                    }
                                    if(board[c][d]>6){
                                        fromSQa[z] = a;
                                        fromSQb[z] = b;
                                        toSQc[z] = c;
                                        toSQd[z] = d;
                                        allmoves[z]=fromSQa[z]*1000+fromSQb[z]*100+toSQc[z]*10+toSQd[z];
                                        z++;
                                        break;
                                    }
                                    if(board[c][d]<7&&board[c][d]!=0) break;
                                    c = c+e;
                                    d = d+f;
                                }
                            }
                            while(ValidSquare(a,b,c,d)!=0);
                        }
                    }
                }
                break;
            case bknight:
                for(e=-2;e<=2;e++){
                    for(f=-2;f<=2;f++){
                        c = a+e;
                        d = b+f;
                        if(abs(e)+abs(f)==3){
                            if(ValidSquare(a,b,c,d)!=0){
                                if(board[c][d]>6||board[c][d]==0){
                                    fromSQa[z] = a;
                                        fromSQb[z] = b;
                                        toSQc[z] = c;
                                        toSQd[z] = d;
                                        allmoves[z]=fromSQa[z]*1000+fromSQb[z]*100+toSQc[z]*10+toSQd[z];
                                        z++;
                                }
                            }
                        }
                    }
                }
                break;
            case bbishop:
                for(e=-1;e<=1;e++){
                    for(f=-1;f<=1;f++){
                        c = a+e;
                        d = b+f;
                        if(abs(e)+abs(f)==2){
                            do{
                                if(ValidSquare(a,b,c,d)!=0){
                                    if(board[c][d]==0){
                                        fromSQa[z] = a;
                                        fromSQb[z] = b;
                                        toSQc[z] = c;
                                        toSQd[z] = d;
                                        allmoves[z]=fromSQa[z]*1000+fromSQb[z]*100+toSQc[z]*10+toSQd[z];
                                        z++;
                                    }
                                    if(board[c][d]>6){
                                        fromSQa[z] = a;
                                        fromSQb[z] = b;
                                        toSQc[z] = c;
                                        toSQd[z] = d;
                                        allmoves[z]=fromSQa[z]*1000+fromSQb[z]*100+toSQc[z]*10+toSQd[z];
                                        z++;
                                        break;
                                    }
                                    if(board[c][d]<7&&board[c][d]!=0) break;
                                }
                                c = c+e;
                                d = d+f;
                            }
                            while(ValidSquare(a,b,c,d)!=0);
                        }
                    }
                }
                break;
            case bking:
                for(e=-1;e<=1;e++){
                    for(f=-1;f<=1;f++){
                        if(!(e==0&&f==0)){
                            c = a+e;
                            d = b+f;
                            if(ValidSquare(a,b,c,d)!=0){
                                if(board[c][d]>6||board[c][d]==0){
                                    fromSQa[z] = a;
                                    fromSQb[z] = b;
                                    toSQc[z] = c;
                                    toSQd[z] = d;
                                    allmoves[z]=fromSQa[z]*1000+fromSQb[z]*100+toSQc[z]*10+toSQd[z];
                                    z++;
                                }
                            }
                        }
                    }
                }
                if(a==7 && b==4 && board[7][7]==brook && board[7][6]==0 && board[7][5]==0){
                    fromSQa[z] = 7;
                    fromSQb[z] = 7;
                    toSQc[z] = 7;
                    toSQd[z] = 7;
                    allmoves[z]=fromSQa[z]*1000+fromSQb[z]*100+toSQc[z]*10+toSQd[z];
                    z++;
                }
                if(a==7 && b==4 && board[7][0]==brook && board[7][1]==0 && board[7][2]==0 && board[7][3]==0){
                    fromSQa[z] = 6;
                    fromSQb[z] = 6;
                    toSQc[z] = 6;
                    toSQd[z] = 6;
                    allmoves[z]=fromSQa[z]*1000+fromSQb[z]*100+toSQc[z]*10+toSQd[z];
                    z++;
                }
        }
    }
}
return allmoves;
}

int *WhiteMoveList(int epa,int epb)
{
    int i;
    static int allmoves[218];
    int fromSQa[218];
    int fromSQb[218];
    int toSQc[218];
    int toSQd[218];
    int a=0;
    int b=0;
    int c=0;
    int d=0;
    int e=0;
    int f=0;
    int z=0;
    for(i=0;i<218;i++){
        allmoves[i]=0;
    }
 //  printf("\nepa: %d, epb: %d\n",epa,epb);
    for(a=0;a<8;a++){
        for(b=0;b<8;b++){
            switch(board[a][b])
            {
            case wqueen:
                    for(e = -1;e <= 1;e++){
                        for(f= -1;f <=1;f++){
                            c= a+e;
                            d= b+f;
                            if(!(f==0&&e==0)){
                                do{
                                if(ValidSquare(a,b,c,d)!=0){
                                if(board[c][d]==0){
                                    fromSQa[z]=a;
                                    fromSQb[z]=b;
                                    toSQc[z]=c;
                                    toSQd[z]=d;
                                    allmoves[z]=fromSQa[z]*1000+fromSQb[z]*100+toSQc[z]*10+toSQd[z];
                                    z++;
                                }
                                if((board[c][d]<7)&&board[c][d]!=0){
                                    fromSQa[z]=a;
                                    fromSQb[z]=b;
                                    toSQc[z]=c;
                                    toSQd[z]=d;
                                    allmoves[z]=fromSQa[z]*1000+fromSQb[z]*100+toSQc[z]*10+toSQd[z];
                                    z++;
                                    break;
                                }
                                if(board[c][d]>6) break;
                                c = c+e;
                                d = d+f;
                                }
                            }
                            while(ValidSquare(a,b,c,d)!=0);
                        }
                    }

                }
                break;
            case wpawn:
                c = a+1;
                d = b-1;
                if(ValidSquare(a,b,c,d)!=0){
                    if(((board[c][d]<7)&&board[c][d]!=0)||(c==epa&&d==epb)){
                        fromSQa[z] = a;
                        fromSQb[z] = b;
                        toSQc[z] = c;
                        toSQd[z] = d;
                        allmoves[z]=fromSQa[z]*1000+fromSQb[z]*100+toSQc[z]*10+toSQd[z];
                        z++;
                    }
                }
                c = a+1;
                d = b+1;
                if(ValidSquare(a,b,c,d)!=0){
                    if(((board[c][d]<7)&&board[c][d]!=0)||(c==epa&&d==epb)){
                        fromSQa[z] = a;
                        fromSQb[z] = b;
                        toSQc[z] = c;
                        toSQd[z] = d;
                        allmoves[z]=fromSQa[z]*1000+fromSQb[z]*100+toSQc[z]*10+toSQd[z];
                        z++;
                    }
                }
                if(a==1){
                    c=2;
                    d=b;
                    if(ValidSquare(a,b,c,d)!=0){
                        if(board[c][d]==0){
                            fromSQa[z] = a;
                        fromSQb[z] = b;
                        toSQc[z] = c;
                        toSQd[z] = d;
                        allmoves[z]=fromSQa[z]*1000+fromSQb[z]*100+toSQc[z]*10+toSQd[z];
                        z++;
                        c = 3;
                        d = b;
                        if(ValidSquare(a,b,c,d)!=0){
                            if(board[c][d]==0){
                                fromSQa[z] = a;
                        fromSQb[z] = b;
                        toSQc[z] = c;
                        toSQd[z] = d;
                        allmoves[z]=fromSQa[z]*1000+fromSQb[z]*100+toSQc[z]*10+toSQd[z];
                        z++;
                            }
                        }
                        }
                    }
                }
                else{
                    c = a+1;
                    d = b;
                    if(ValidSquare(a,b,c,d)!=0){
                        if(board[c][d]==0){
                            fromSQa[z] = a;
                            fromSQb[z] = b;
                            toSQc[z] = c;
                            toSQd[z] = d;
                            allmoves[z]=fromSQa[z]*1000+fromSQb[z]*100+toSQc[z]*10+toSQd[z];
                            z++;
                        }
                    }
                }
                break;
            case wrook:
                for(e=-1;e<=1;e++){
                    for(f=-1;f<=1;f++){
                        c = a+e;
                        d = b+f;
                        if(abs(e)+abs(f)==1){
                            do{
                                if(ValidSquare(a,b,c,d)!=0){
                                    if(board[c][d]==0){
                                        fromSQa[z] = a;
                                        fromSQb[z] = b;
                                        toSQc[z] = c;
                                        toSQd[z] = d;
                                        allmoves[z]=fromSQa[z]*1000+fromSQb[z]*100+toSQc[z]*10+toSQd[z];
                                        z++;
                                    }
                                    if(board[c][d]<7&&board[c][d]!=0){
                                        fromSQa[z] = a;
                                        fromSQb[z] = b;
                                        toSQc[z] = c;
                                        toSQd[z] = d;
                                        allmoves[z]=fromSQa[z]*1000+fromSQb[z]*100+toSQc[z]*10+toSQd[z];
                                        z++;
                                        break;
                                    }
                                    if(board[c][d]>6) break;
                                    c = c+e;
                                    d = d+f;
                                }
                            }
                            while(ValidSquare(a,b,c,d)!=0);
                        }
                    }
                }
                break;
            case wknight:
                for(e=-2;e<=2;e++){
                    for(f=-2;f<=2;f++){
                        c = a+e;
                        d = b+f;
                        if(abs(e)+abs(f)==3){
                            if(ValidSquare(a,b,c,d)!=0){
                                if(board[c][d]<7){
                                    fromSQa[z] = a;
                                        fromSQb[z] = b;
                                        toSQc[z] = c;
                                        toSQd[z] = d;
                                        allmoves[z]=fromSQa[z]*1000+fromSQb[z]*100+toSQc[z]*10+toSQd[z];
                                        z++;
                                }
                            }
                        }
                    }
                }
                break;
            case wbishop:
                for(e=-1;e<=1;e++){
                    for(f=-1;f<=1;f++){
                        c = a+e;
                        d = b+f;
                        if(abs(e)+abs(f)==2){
                            do{
                                if(ValidSquare(a,b,c,d)!=0){
                                    if(board[c][d]==0){
                                        fromSQa[z] = a;
                                        fromSQb[z] = b;
                                        toSQc[z] = c;
                                        toSQd[z] = d;
                                        allmoves[z]=fromSQa[z]*1000+fromSQb[z]*100+toSQc[z]*10+toSQd[z];
                                        z++;
                                    }
                                    if(board[c][d]<7&&board[c][d]!=0){
                                        fromSQa[z] = a;
                                        fromSQb[z] = b;
                                        toSQc[z] = c;
                                        toSQd[z] = d;
                                        allmoves[z]=fromSQa[z]*1000+fromSQb[z]*100+toSQc[z]*10+toSQd[z];
                                        z++;
                                        break;
                                    }
                                    if(board[c][d]>6) break;
                                }
                                c = c+e;
                                d = d+f;
                            }
                            while(ValidSquare(a,b,c,d)!=0);
                        }
                    }
                }
                break;
            case wking:
                for(e=-1;e<=1;e++){
                    for(f=-1;f<=1;f++){
                        if(!(e==0&&f==0)){
                            c = a+e;
                            d = b+f;
                            if(ValidSquare(a,b,c,d)!=0){
                                if(board[c][d]<7){
                                    fromSQa[z] = a;
                                    fromSQb[z] = b;
                                    toSQc[z] = c;
                                    toSQd[z] = d;
                                    allmoves[z]=fromSQa[z]*1000+fromSQb[z]*100+toSQc[z]*10+toSQd[z];
                                    z++;
                                }
                            }
                        }
                    }
                }
                if(a==0 && b==4 && board[0][7]==wrook && board[0][6]==0 && board[0][5]==0){
                    fromSQa[z] = 2;
                    fromSQb[z] = 2;
                    toSQc[z] = 2;
                    toSQd[z] = 2;
                    allmoves[z]=fromSQa[z]*1000+fromSQb[z]*100+toSQc[z]*10+toSQd[z];
                    z++;
                }
                if(a==0 && b==4 && board[0][0]==wrook && board[0][1]==0 && board[0][2]==0 && board[0][3]==0){
                    fromSQa[z] = 1;
                    fromSQb[z] = 1;
                    toSQc[z] = 1;
                    toSQd[z] = 1;
                    allmoves[z]=fromSQa[z]*1000+fromSQb[z]*100+toSQc[z]*10+toSQd[z];
                    z++;
                }
        }
    }
}
return allmoves;
}

int CheckWhite(int *blacklist)
{
    int x,y; // WKing's position
    int a,b;
    int i;
    int num;
    int result=0;
    for(a=0;a<8;a++){
        for(b=0;b<8;b++){
            if(board[a][b]==wking){
                x=a;
                y=b;
                break;
            }
        }
    }
    for(a=0;a<8;a++){
        for(b=0;b<8;b++){
            for(i=0;i<218;i++){
                num = a*1000+b*100+x*10+y;
                if(num == *(blacklist+i)) {
 //                   printf("check move is: list[%d] %d \n",i,blacklist[i]);
                    result = 1;
                    break;
                }
            }
        }
    }
    return result;
}

int WhiteMove(char *cmd,int turn,int castlingcheck)
{
    int a,b,c,d;
    int num;
    int i;
    int *list = WhiteMoveList(enpass_a,enpass_b);
    int *blacklist = BlackMoveList(enpass_a,enpass_b);
    int castling=0;
    int queensideok=1;
    int kingsideok=1;
    int x,y;
    for(x=0;x<8;x++){
        if(queensideok==0) break;
        for(y=0;y<8;y++){
            if(queensideok==0) break;
            for(i=0;i<218;i++){
                int val1=x*1000 + y*100 + 1;
                int val2=x*1000 + y*100 + 2;
                int val3=x*1000 + y*100 + 3;
                if(val1==blacklist[i]||val2==blacklist[i]||val3==blacklist[i])
                    {
                        queensideok=0;
                        break;
                    }
            }
        }
    }
    for(x=0;x<8;x++){
        if(kingsideok==0) break;
        for(y=0;y<8;y++){
            if(kingsideok==0) break;
            for(i=0;i<218;i++){
                int val1=x*1000 + y*100 + 5;
                int val2=x*1000 + y*100 + 6;
                if(val1==blacklist[i]||val2==blacklist[i])
                    {
                        kingsideok=0;
                        break;
                    }
            }
        }
    }
   // printf("queenside: %d\n",queensideok);

    int result=0;

    if(turn != 0) return 0;
    if(strlen(cmd)!=4) return 0;
    a = cmd[0]-'0';
    b = cmd[1]-'0';
    c = cmd[2]-'0';
    d = cmd[3]-'0';
    num = a*1000 + b*100 + c *10 +d;
    if(a==c&&b==d&&(a==1&&b==1)&&queensideok==1&&castlingcheck==0&&CheckWhite(blacklist)==0){castling = 1;}
    if(a==c&&b==d&&(a==2&&b==2)&&kingsideok==1&&castlingcheck==0&&CheckWhite(blacklist)==0){castling = 1;}
   // printf("castling: %d\n",castling);
    if(a==c&&b==d&&castling==0) return 0;
    if(ValidSquare(a,b,c,d) == 0) return 0;
    if(board[a][b]<7&&castling==0) return 0;
    for(i=0;i<218;i++){
        if(num==*(list+i)){
            result = 1;
            break;
        }

    }
    return result;
}


int WhiteFaultCheck(int a,int b,int c,int d)
{
    piece oldboard[8][8];
    int result = 1;
    piece captured = board[c][d];
  //  printf("\n%d %d %d %d\n",a,b,c,d);
    //printf("%captured piece is : %d",captured);
    Move(a,b,c,d);
    int *list = BlackMoveList(enpass_a,enpass_b);
    if(CheckWhite(list)==1) result = 0;
   // for(i=0;i<8;i++){
   //     for(j=0;j<8;j++){
   //         board[i][j]=oldboard[i][j];
   //     }
   // }
   //printboard();
    Move(c,d,a,b);

    board[c][d] = captured;
    //printboard();
    return result;
}
int BlackMove(char *cmd,int turn,int castlingcheck)
{
    int a,b,c,d;
    int num;
    int i;
    int *list = BlackMoveList(enpass_a,enpass_b);
    int *whitelist = WhiteMoveList(enpass_a,enpass_b);
    int castling=0;
    int queensideok=1;
    int kingsideok=1;
    int x,y;
    for(x=0;x<8;x++){
        if(queensideok==0) break;
        for(y=0;y<8;y++){
            if(queensideok==0) break;
            for(i=0;i<218;i++){
                int val1=x*1000 + y*100 + 71;
                int val2=x*1000 + y*100 + 72;
                int val3=x*1000 + y*100 + 73;
              //  printf("i:%d | val3: %d | %d \n",i,val3,blacklist[i]);
                if(val1==whitelist[i]||val2==whitelist[i]||val3==whitelist[i])
                    {
                        queensideok=0;
                        break;
                    }
            }
        }
    }
    for(x=0;x<8;x++){
        if(kingsideok==0) break;
        for(y=0;y<8;y++){
            if(kingsideok==0) break;
            for(i=0;i<218;i++){
                int val1=x*1000 + y*100 + 75;
                int val2=x*1000 + y*100 + 76;
              //  printf("i:%d | val3: %d | %d \n",i,val3,blacklist[i]);
                if(val1==whitelist[i]||val2==whitelist[i])
                    {
                        kingsideok=0;
                        break;
                    }
            }
        }
    }
   // printf("queenside: %d\n",queensideok);

    int result=0;

    if(turn == 0) return 0;
    if(strlen(cmd)!=4) return 0;
    a = cmd[0]-'0';
    b = cmd[1]-'0';
    c = cmd[2]-'0';
    d = cmd[3]-'0';
    num = a*1000 + b*100 + c *10 +d;
    if(a==c&&b==d&&a==6&&b==6&&queensideok==1&&castlingcheck==0&&CheckBlack(whitelist)==0){castling = 1;}
    if(a==c&&b==d&&a==7&&b==7&&kingsideok==1&&castlingcheck==0&&CheckBlack(whitelist)==0){castling = 1;}
 //   printf("castling: %d\n",castling);
    if(ValidSquare(a,b,c,d) == 0) return 0;
  //  printf("2nd: %d\n",result);
    if(board[a][b]>6&&castling==0) return 0;
    for(i=0;i<218;i++){
        if(num==*(list+i)){
            result = 1;
            break;
        }

    }
    return result;
}

int BlackFaultCheck(int a,int b,int c,int d)
{
    piece oldboard[8][8];
    int result = 1;
    piece captured = board[c][d];
    Move(a,b,c,d);
    int *list = WhiteMoveList(enpass_a,enpass_b);
    if(CheckBlack(list)==1) result = 0;;
    Move(c,d,a,b);
    board[c][d] = captured;
    return result;
}

int PlayGame(int side)
{
    setup();
    printboard();
    int i;
    int tmp;
    int blackfault=0;
    int whitefault=0;
    int gameresult=0; //0 - is playing; 1 - white won; 2 - black won; 3 - draw
    int a,b,c,d;
    int wka,wkb,bka,bkb;
    int check; //king's position
    int blackcastled=0;
    int whitecastled=0;
    char command[4];
    int turn = 0; //0 is white,1 is black
    while(gameresult == 0){
        switch(turn){
            case 0:
                if(gameresult!=0) break;
                check = 0;
                for(a=0;a<8;a++){
                    if(check!=0) break;
                    for(b=0;b<8;b++){
                        if(board[a][b]==bking){
                            bka = a;
                            bkb = b;
                            check = 1;
                            break;
                        }
                    }
                }
//reserved code                if(turn==side){
// make move and send move to server
                printf("\nIt's White's turn!! input next move: ");
                scanf("%s",command);
                while(WhiteMove(command,turn,whitecastled)==0){
                    printf("Invalid Move!! Please make a valid move: ");
                    scanf("%s",command);
                }
                    a = command[0]-'0';
                    b = command[1]-'0';
                    c = command[2]-'0';
                    d = command[3]-'0';
                while(WhiteFaultCheck(a,b,c,d)==0){
                    if(whitefault<3){
                        printf("White has committed a technical fault!! White still has %d technical faults left \nPlease make another move :",3-whitefault);
                        whitefault++;
                        scanf("%s",command);
                        while(WhiteMove(command,turn,whitecastled)==0){
                            printf("Invalid Move!! Please make a valid move: ");
                            scanf("%s",command);
                        }
                        a = command[0]-'0';
                        b = command[1]-'0';
                        c = command[2]-'0';
                        d = command[3]-'0';
                    }
                    if(whitefault==3){
                        printf("White has committed 3 technical fault!! White has lost the game!! \n");
                        gameresult = 2;
                        break;
                        }
                    }
                    if(gameresult != 0) break;
   //reserved code             }
   //             if(turn!=side)
        //        {
        //            receive move from server and extract into a,b,c,d
        //        }
                    if(strcmp(command,"1111")==0){
                       // tmp=1;
                       // printf("asdsad:%d \n",tmp);
                        printf("White castled queen's side \n");
                        Move(0,4,0,2);
                        Move(0,0,0,3);
                        turn++;
                        printf("\e[2J\e[H");
                        printboard();
                        break;
                        }
                    if(strcmp(command,"2222")==0){
                        printf("White castled king's side \n ");
                        Move(0,4,0,6);
                        Move(0,7,0,5);
                        turn++;
                        printf("\e[2J\e[H");
                        printboard();
                        break;
                        }
                    if(board[a][b]==wrook || board[a][b]==wking) whitecastled =1;
                    if(board[a][b]==wpawn&&c==enpass_a&&d==enpass_b)
                    {
                        Move(a,b,c,d);
                        board[enpass_a-1][enpass_b]=blank;
                        turn++;
                        printf("\e[2J\e[H");
                        printboard();
                        break;
                    }
                    if(board[a][b]==wpawn&&a<3)
                    {
                        enpass_a = c-1;
                        enpass_b = d;
                    }
                    else
                    {
                        enpass_a = -1;
                        enpass_b = -1;
                    }
                    Move(a,b,c,d);

                turn++;
                printf("\e[2J\e[H");
                printboard();
                break;
            case 1:
                if(gameresult!=0) break;
                check = 0;
                for(a=0;a<8;a++){
                    if(check!=0) break;
                    for(b=0;b<8;b++){
                        if(board[a][b]==wking){
                            wka = a;
                            wkb = b;
                            check = 1;
                            break;
                        }
                    }
                }
 //reserved code               if(turn==side){
                printf("\nIt's Black's turn!! input next move: ");
                scanf("%s",command);
                while(BlackMove(command,turn,blackcastled)==0){
                    printf("Invalid Move!! Please make a valid move: ");
                    scanf("%s",command);
                }
                    a = command[0]-'0';
                    b = command[1]-'0';
                    c = command[2]-'0';
                    d = command[3]-'0';
                while(BlackFaultCheck(a,b,c,d)==0){
                    if(blackfault<3){
                        printf("Black has committed a technical fault!! Black still has %d technical faults left \n Please make another move :",3-blackfault);
                        blackfault++;
                        scanf("%s",command);
                        while(BlackMove(command,turn,blackcastled)==0){
                            printf("Invalid Move!! Please make a valid move: ");
                            scanf("%s",command);
                        }
                        a = command[0]-'0';
                        b = command[1]-'0';
                        c = command[2]-'0';
                        d = command[3]-'0';
                    }
                    if(blackfault==3){
                        printf("Black has committed 3 technical fault!! Black has lost the game!! \n");
                        gameresult = 1;
                        break;
                    }
                }
                if(gameresult != 0) break;
//   reserve code       }
//                if(turn!=side)
 //               {
 //                   receive move form server and extract into a,b,c,d
 //               }
                if(strcmp(command,"6666")==0){
                        printf("Black castled queen's side \n");
                        Move(7,4,7,2);
                        Move(7,0,7,3);
                        turn--;
                        printf("\e[2J\e[H");
                        printboard();
                        break;
                    }
                if(strcmp(command,"7777")==0){
                        printf("Black castled king's side \n ");
                        Move(7,4,7,6);
                        Move(7,0,7,5);
                        turn--;
                        printf("\e[2J\e[H");
                        printboard();
                        break;
                    }
                if(board[a][b]==brook || board[a][b]==bking) whitecastled =1;
                if(board[a][b]==bpawn&&c==enpass_a&&d==enpass_b)
                    {
                        Move(a,b,c,d);
                        board[enpass_a+1][enpass_b]=blank;
                        turn--;
                        printf("\e[2J\e[H");
                        printboard();
                        break;
                    }
                if(board[a][b]==bpawn&&a>5)
                    {
                        enpass_a = c+1;
                        enpass_b = d;
                    }
                    else
                    {
                        enpass_a = -1;
                        enpass_b = -1;
                    }
                    Move(a,b,c,d);

                turn--;
                printf("\e[2J\e[H");
                printboard();
                break;
        }
    }
    return gameresult;
}

int main(void){
    int side = 0;
    PlayGame(side);
    switch(PlayGame(side)){
        case 1: printf("White has won the game!!");
            break;
        case 2: printf("Black has won the game!!");
            break;
        case 3: printf("Draw game!!");
           break;
    }
    return 0;
}

// wrook,wknight,wbishop,wqueen,wking,wbishop,wknight,wrook,wpawn,wpawn,wpawn,wpawn,wpawn,wpawn,wpawn,wpawn,blank,blank,blank,blank,blank,blank,blank,blank,blank,blank,blank,blank,blank,blank,blank, blank, blank, blank, blank, blank, blank, blank, blank, blank, blank, blank, blank, blank, blank, blank, blank,blank, bpawn, bpawn, bpawn, bpawn, bpawn, bpawn, bpawn, bpawn, brook, bknight, bbishop, bqueen, bking, bbishop, bknight, brook
