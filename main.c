// standard C headers
#include <string.h>
#include <stdlib.h>

// device header file
#include "msp432p401r.h"

// this constant holds the number of times the timer interrupt has to occur for an appropriate delay
#define UNIT    25000

// hold the number of times the timer has run out
unsigned int count = 0;

// used to see if the appropriate delay has been reached which will enable other operations int he main function to take place
char flag = 0;

int amount = 0;

// array of pointers holding the morse representation of each letter in the alphabet
int * letters_morse_code[26];

// morse representation of each letter
// note each letter is ended with a 0 to indicate when the code has ended for that letter
int A[] = {1, 1, 3, 0};
int B[] = {3, 1, 1, 1, 1, 1, 1, 0};
int C[] = {3, 1, 1, 1, 3, 1, 1, 0};
int D[] = {3, 1, 1, 1, 1, 0};
int E[] = {1, 0};
int F[] = {1, 1, 1, 1, 3, 1, 1, 0};
int G[] = {3, 1, 3, 1, 1, 0};
int H[] = {1, 1, 1, 1, 1, 1, 1, 0};
int I[] = {1, 1, 1, 0};
int J[] = {1, 1, 3, 1, 3, 1, 3, 0};
int K[] = {3, 1, 1, 1, 3, 0};
int L[] = {1, 1, 3, 1, 1, 1, 1, 0};
int M[] = {3, 1, 3, 0};
int N[] = {3, 1, 1, 0};
int O[] = {3, 1, 3, 1, 3, 0};
int P[] = {1, 1, 3, 1, 3, 1, 1, 0};
int Q[] = {3, 1, 3, 1, 1, 1, 3, 0};
int R[] = {1, 1, 3, 1, 1, 0};
int S[] = {1, 1, 1, 1, 1, 0};
int T[] = {3, 0};
int U[] = {1, 1, 1, 1, 3, 0};
int V[] = {1, 1, 1, 1, 1, 1, 3, 0};
int W[] = {1, 1, 3, 1, 3, 0};
int X[] = {3, 1, 1, 1, 1, 1, 3, 0};
int Y[] = {3, 1, 1, 1, 3, 1, 3, 0};
int Z[] = {3, 1, 3, 1, 1, 1, 1, 0};

// assign the morse codes to the appropriate location in the array of pointers
void init_morse_codes(void)
{
    letters_morse_code[0] = A;
    letters_morse_code[1] = B;
    letters_morse_code[2] = C;
    letters_morse_code[3] = D;
    letters_morse_code[4] = E;
    letters_morse_code[5] = F;
    letters_morse_code[6] = G;
    letters_morse_code[7] = H;
    letters_morse_code[8] = I;
    letters_morse_code[9] = J;
    letters_morse_code[10] = K;
    letters_morse_code[11] = L;
    letters_morse_code[12] = M;
    letters_morse_code[13] = N;
    letters_morse_code[14] = O;
    letters_morse_code[15] = P;
    letters_morse_code[16] = Q;
    letters_morse_code[17] = R;
    letters_morse_code[18] = S;
    letters_morse_code[19] = T;
    letters_morse_code[20] = U;
    letters_morse_code[21] = V;
    letters_morse_code[22] = W;
    letters_morse_code[23] = X;
    letters_morse_code[24] = Y;
    letters_morse_code[25] = Z;
}

int * ascii_to_morse_code_words(char * words)
{
    int * data = (int *)malloc(sizeof(int) * strlen(words) * 8);

    if(data == NULL)
    {
        exit(0);
    }

    int letter = 0;

    int total = 0;

    int i = 0;

    for(i = 0; i < strlen(words); i++)
    {
        letter = words[i] - 65;

        // if the character is a space then palce a 7
        if(words[i] == ' ')
        {
            data[total] = 7;
            total++;
        }
        else
        {
            int j = 0;
            for(j = 0; letters_morse_code[letter][j] != 0; j++)
            {
                data[total] = letters_morse_code[letter][j];
                total++;
            }
            // provided the next character is not a space then it is a letter there for put a 3
            if(words[i + 1] != ' ' && words[i + 1] != '\0')
            {
                data[total] = 3;
                total++;
            }
        }
    }

    data[total] = 0;

    total++;

    data = (int *)realloc(data, sizeof(int) * total);

    if(data == NULL)
    {
        exit(0);
    }

    return data;
}


void set_clock(void)
{
    // enable clock registers to be modified
    CS->KEY = CS_KEY_VAL;
    // frequency select of DCO
    CS->CTL0 = CS_CTL0_DCORSEL_0;
    // set
    CS->CTL1 = CS_CTL1_SELM_3 | CS_CTL1_SELS_3 | CS_CTL1_DIVM_0 | CS_CTL1_DIVS_0;
    // disable clock registers to be modified
    CS->KEY = 0;
}

void set_timer_A0(void)
{
    // set count to 62500
    // need static var to create a delay up to 1s
    TIMER_A0->CCR[0] = 62500;
    // clear TAR
    // UP mode
    // /1 divider
    // SMCLK
    TIMER_A0->CTL = TIMER_A_CTL_MC_1 | TIMER_A_CTL_ID_0 | TIMER_A_CTL_TASSEL_2 | TIMER_A_CTL_IE;
    //TIMER_A0->EX0 = TIMER_A_EX0_IDEX__8;

    TIMER_A0->CCTL[0] = TIMER_A_CCTLN_CCIE; // TACCR0 interrupt enabled
}

void set_GPIO(void)
{
    // Configure Port 1
    P1->DIR |= BIT0;
    P1->OUT |= BIT0;

    // Configure Port 2
    P2->DIR = BIT0 | BIT1 | BIT2;
    P2->OUT = BIT0;
}

// main
void main(void)
{
    WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;

    set_clock();
    set_timer_A0();
    set_GPIO();
    init_morse_codes();

    char * words = "MERRY CHRISTMAS";

    int * data = ascii_to_morse_code_words(words);
    int offset = 0;
    amount = data[offset];

    // Enable global interrupt
    __enable_irq();

    NVIC->ISER[0] = 1 << ((TA0_0_IRQn) & 31);

    while (1)
    {
        // __no_operation();                   // For debugger

        if(flag == 1)
        {
            // increment offset for the next part in the morse code
            offset++;

            // if the next part is 0 then we have reached the end of the message so start again
            if(data[offset] == 0)
            {
                amount = 7;
                offset = -1;
            }
            else
            {
                // set amount
                amount = data[offset];
            }
            // reset flag
            flag = 0;
        }
    }

    free(data);
    // free(words);
    // free(actual);
}

// Timer A0 interrupt service routine
void TA0_0_IRQHandler(void)
{
    static unsigned int total = 1;
    static unsigned int val = 0;
    val++;
    count++;

    if(val == UNIT)
    {
        total++;
        if(total == 8)
        {
            total = 1;
        }
        switch(total)
        {
        case 1:
            P2->OUT = BIT0;
            break;
        case 2:
            P2->OUT = BIT1;
            break;
        case 3:
            P2->OUT = BIT2;
            break;
        case 4:
            P2->OUT = BIT0 | BIT1;
            break;
        case 5:
            P2->OUT = BIT0 | BIT2;
            break;
        case 6:
            P2->OUT = BIT1 | BIT2;
            break;
        case 7:
            P2->OUT = BIT0 | BIT1 | BIT2;
            break;
        default:
            break;
        }
        val = 0;
    }

    if(count == UNIT * amount)
    {
        TIMER_A0->CCTL[0] &= ~TIMER_A_CCTLN_CCIFG;

        P1->OUT ^= BIT0;
        flag = 1;
        count = 0;
    }
}
