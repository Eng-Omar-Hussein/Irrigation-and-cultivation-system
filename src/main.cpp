#define F_CPU 16000000UL
#include "omar.h"
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "adc.c"
#include "uart.c"
#include <math.h>
#include <LiquidCrystal.h>
#include <Arduino.h>
#include <avr/pgmspace.h>
#include <stdio.h>
#include <string.h>
#include <util/delay.h>
#include <ctype.h>
typedef uint8_t _bool;
#define true 1
#define false 0

int _7seg[16] = {126, 12, 182, 158, 204, 218, 250, 14, 254, 206, 238, 248, 114, 188, 242, 226};
LiquidCrystal lcd(A1, A2, A3, A4, A5, A6);

const char defaultPassword[] = "1234";
char currentPassword[8];
typedef enum
{
    ENTER_PASSWORD,
    MENU,
    CHANGE_PASSWORD,
    ADD_MODE,
    stat,
    START_MODE
} State;
typedef enum
{
    Name,
    SoilMoisture,
    Temperature
} Add;
State currentState = ENTER_PASSWORD;
Add currentAdd = Name;
typedef struct
{
    char name[20]; // Adjust size as needed
    int SoilMoisture;
    int Temperature;
} mode;
int Iterate = 0;
mode modeData[5];

void handleInput(const char *input);
void enterPassword(const char *password);
void menu(const char *menuSelection);
void changePassword(const char *newpassword);
void addMode(const char *data);
void StartMode(const char *name);
_bool validName(const char *name);
_bool validSoilMoisture(const char *Moisture);
_bool validTemperature(const char *Temperature);
_bool existName(const char *name);

uint8_t t , adc_value;
int maxTemp=100, minMoisture=0, id;
int main(void)
{
    SET_BIT(MCUCR, ISC00);
    SET_BIT(MCUCR, ISC01);
    SET_BIT(MCUCR, ISC10);
    SET_BIT(MCUCR, ISC11);

    SET_BIT(GICR, INT0);
    SET_BIT(GICR, INT1);
    DDRC = 0xff;
    uart_init(UART_VAL);
    adc_init();
    sei();
    SET_BIT(ADCSRA, ADSC);
    lcd.begin(16, 2);
    lcd.print("Enter password:");
    lcd.setCursor(0, 1);
    lcd.print(">>");
    strcpy(currentPassword, defaultPassword);
    while (1)
    {
        static char inputBuffer[20];
        uint8_t bufferIndex = 0;

        while (1)
        {
            char receivedChar = uart_rx();
            uart_tx(receivedChar);
            if (receivedChar == '\n' || receivedChar == '\r')
            {
                inputBuffer[bufferIndex] = '\0';
                handleInput(inputBuffer);
                bufferIndex = 0;
                break;
            }
            else if (receivedChar == '\b' && bufferIndex > 0)
            {
                bufferIndex--;
                inputBuffer[bufferIndex] = '\0';
            }
            else if (receivedChar >= 32 && receivedChar <= 126 && bufferIndex < sizeof(inputBuffer) - 1)
            {
                inputBuffer[bufferIndex] = receivedChar;
                bufferIndex++;
                lcd.setCursor(3, 1);
                char substringBuffer[bufferIndex + 1];
                for (int i = 0; i < bufferIndex; i++)
                    substringBuffer[i] = inputBuffer[i];
                
                substringBuffer[bufferIndex] = '\0';
                lcd.print(substringBuffer);
            }
        }
        lcd.setCursor(0, 1);
        lcd.print(">>");
    }
}
ISR(INT0_vect)
{
    currentState = ENTER_PASSWORD;
    lcd.clear();
    lcd.print("Restarting....");
    _delay_ms(500);
    lcd.clear();
    lcd.print("Enter password:");
    lcd.setCursor(0, 1);
    lcd.print(">>");
}
ISR(ADC_vect)
{
    adc_value = ADCH;
    t = (adc_value*100)/255;
    //t = ceil((adc_value * 150) / 77) + (not(adc_value / 77) and adc_value);
    if (t >= maxTemp)
    {
        SET_BIT(PORTC, 0);
        SET_BIT(PORTC, 2);
        SET_BIT(PORTC, 5);
        CLR_BIT(PORTC, 1);
    }
    else
    {
        SET_BIT(PORTC, 1);
        CLR_BIT(PORTC, 0);
        CLR_BIT(PORTC, 2);
        CLR_BIT(PORTC, 5);
    }
    SET_BIT(ADCSRA, ADSC);
}

void handleInput(const char *input)
{
    switch (currentState)
    {
    case ENTER_PASSWORD:
        enterPassword(input);
        break;
    case MENU:
        menu(input);
        break;
    case CHANGE_PASSWORD:
        changePassword(input);
        break;
    case ADD_MODE:
        addMode(input);
        break;
    case START_MODE:
        StartMode(input);
        break;
    case stat:
        lcd.clear();
        lcd.print("Temperature=");
        lcd.setCursor(12, 0);
        lcd.print(t);
        currentState = MENU;
        break;
    default:
        // Handle other states
        break;
    }
}
void enterPassword(const char *password)
{
    if (atoi(password) == atoi(currentPassword))
    {
        lcd.clear();
        lcd.print("Access granted!");
        _delay_ms(500);
        currentState = MENU;
        lcd.clear();
        lcd.print("C:1 A:2 S:3 T:4");
    }
    else
    {
        currentState = ENTER_PASSWORD;
        lcd.clear();
        lcd.print("Access denied!");
        _delay_ms(500);
        lcd.clear();
        lcd.print("Enter password:");
    }
}
void menu(const char *menuSelection)
{
    switch (atoi(menuSelection))
    {
    case 1:
        currentState = CHANGE_PASSWORD;
        lcd.clear();
        lcd.print("CHANGE PASSWORD");
        _delay_ms(1000);
        lcd.clear();
        lcd.print("New password:");
        break;
    case 2:
        currentState = ADD_MODE;
        lcd.clear();
        lcd.print("ADD MODE");
        _delay_ms(1000);
        lcd.clear();
        lcd.print("Mode Name:");
        break;
    case 3:
        currentState = START_MODE;
        lcd.clear();
        lcd.print("START MODE...");
        _delay_ms(1000);
        lcd.clear();
        lcd.print("MODE Name:");
        break;
    case 4:
        currentState = stat;
        lcd.clear();
        lcd.print("Temperature=");
        lcd.setCursor(12, 0);
        lcd.print(t);
        break;
    default:
        currentState = MENU;
        lcd.clear();
        lcd.print("C:1 A:2 S:3 T:4");
        break;
    }
}
void changePassword(const char *newpassword)
{
    if (strlen(newpassword) >= 4)
    {
        strcpy(currentPassword, newpassword);
        currentState = MENU;
        lcd.clear();
        lcd.print("C:1 A:2 S:3 T:4");
    }
    else
    {
        currentState = CHANGE_PASSWORD;
        lcd.clear();
        lcd.print("Bad Password!");
        _delay_ms(500);
        lcd.clear();
        lcd.print("New password:");
    }
}
void addMode(const char *data)
{
    switch (currentAdd)
    {
    case Name:
        if (validName(data))
        {
            currentAdd = SoilMoisture;
            lcd.clear();
            lcd.print("Name Done");
            _delay_ms(500);
            lcd.clear();
            lcd.print("Soil Moisture:");
            strcpy(modeData[Iterate].name, data);
        }
        else
        {
            lcd.print("Invalid!");
            _delay_ms(500);
            lcd.clear();
            lcd.print("Mode Name:");
        }
        break;
    case SoilMoisture:
        if (validSoilMoisture(data))
        {
            currentAdd = Temperature;
            modeData[Iterate].SoilMoisture = atoi(data);

            lcd.clear();
            lcd.print("Moisture Done");
            _delay_ms(500);
            lcd.clear();
            lcd.print("Temperature:");
        }
        else
        {
            lcd.print("Invalid!");
            _delay_ms(500);
            lcd.clear();
            lcd.print("Soil Moisture:");
        }
        break;
    case Temperature:
        if (validTemperature(data))
        {
            currentState = MENU;
            currentAdd = Name;
            modeData[Iterate].Temperature = atoi(data);
            Iterate++;
            lcd.clear();
            lcd.print("Temperature Done");
            _delay_ms(1000);
            lcd.clear();
            lcd.print("C:1 A:2 S:3 T:4");
        }
        else
        {
            lcd.print("Invalid!");
            _delay_ms(500);
            lcd.clear();
            lcd.print("Temperature:");
        }
        break;
    default:
        currentState = MENU;
        lcd.clear();
        lcd.print("C:1 A:2 S:3 T:4");
        break;
    }
}
void StartMode(const char *name)
{
    if (existName(name))
    {
        currentState = MENU;
        lcd.clear();
        lcd.print("Name Done");
        _delay_ms(500);
        lcd.clear();
        lcd.print("Mode started");
        maxTemp = modeData[id].Temperature;
        minMoisture = modeData[id].SoilMoisture;
    }
    else
    {
        lcd.print("Invalid!");
        _delay_ms(500);
        lcd.clear();
        lcd.print("Mode Name:");
    }
}
_bool validName(const char *name)
{
    if (strlen(name) < 3)
        return false;
    for (int i = 0; i <= Iterate; i++)
    {
        if (strcmp(name, modeData[i].name) == 0)
            return false;
    }
    return true;
}
_bool validSoilMoisture(const char *Moisture)
{
    if (strlen(Moisture) > 4)
        return false;
    for (int i = 0; i < (int)strlen(Moisture); i++)
    {
        if (!isdigit(Moisture[i]))
            return false;
    }
    return true;
}
_bool validTemperature(const char *Temperature)
{
    if (strlen(Temperature) > 3)
        return false;
    if (atoi(Temperature) > 150)
        return false;
    if (atoi(Temperature) < 0)
        return false;
    return true;
}
_bool existName(const char *name)
{
    for (int i = 0; i <= Iterate; i++)
    {
        if (strcmp(name, modeData[i].name) == 0)
        {
            id = i;
            return true;
        }
    }
    return false;
}
// Function definitions

// UART
// #define val 103
// unsigned char uart_rx();
// void uart_init(uint16_t );

// int main()
// {
// char c;
//   DDRA=0xFF;

//   uart_init(103);

//   while(1)
//   {

//     c = uart_rx();
//     lcd.print(c);

//    }

//   }

// void uart_init(uint16_t ubrr)
// {
//   UBRRH =uint8_t(ubrr>>8);
//   UBRRL =uint8_t(ubrr);
//   UCSRB = (1<<RXEN) | (1<<TXEN);
//   SET_BIT(UCSRC,UCSZ0);
//   SET_BIT(UCSRC,UCSZ1);
// }
// unsigned char uart_rx()
// {
//   while(!(UCSRA & (1<<RXC))); //check getting full byte
//   return UDR;
// }

// int main(){
//   uint8_t rx=0;
//   uart_init(UART_VAL);
//   lcd.begin(16,2);
//   lcd.setCursor(0,0);
//   lcd.write("T = ");

//   while (1)
//   {
//     rx = uart_rx();
//     lcd.setCursor(4,0);
//     lcd.print(rx);
//     _delay_ms(50);
//     lcd.clear();
//     lcd.setCursor(0,0);
//     lcd.write("T = ");
//   }
// }

// int main(){
//   uint8_t r,t;
//   uart_init(UART_VAL);
//   adc_init();
//   while (1)
//   {
//     r = adc_read(0);
//     t = ceil((r*150)/77)+(not(r/77)and r);
//     uart_tx(t);
//     _delay_ms(1000);
//   }
// }

// int main(){
//   uint8_t r = 0;
//   uart_init(UART_VAL);

//   while(1){
//     r = adc_read(0);

//     _delay_ms(1000);
//   }
// }

// sheet 3
// 5

// int main(){
//   uint8_t r;
//   int x;
//   DDRC = 0xFF;
//   adc_init();
//   while (1)
//   {
//     r = adc_read(1)-2;
//     x = r*100/242;
//     PORTC = fmod(x,10)*16;
//     x /= 10;
//     PORTC |= x;
//     _delay_ms(1000);
//   }
// }

// sheet 3
// 2

// int main(){
//   uint8_t r;
//   int x;
//   float v;
//   DDRC = 0xFF;
//   adc_init();
//   while (1)
//   {
//     r = adc_read(1);
//     v = (r*5.0)/255;
//     x = fmod((v*10),10);
//     PORTC = v;
//     PORTC |= x*16;
//     _delay_ms(1000);
//   }
// }

// sheet 2
//  6
//  volatile int i=15;
//  int main(void){
//    // led green & red
//    SET_BIT(DDRC,0);
//    SET_BIT(DDRC,1);
//    SET_BIT(PORTC,1);
//    // 7seg
//    DDRA = 0xFE;
//    SET_BIT(DDRC,7);
//    PORTA=_7seg[i];

//   SET_BIT(MCUCR,ISC00);
//   SET_BIT(MCUCR,ISC01);
//   SET_BIT(MCUCR,ISC10);
//   SET_BIT(MCUCR,ISC11);

//   SET_BIT(GICR,INT0);
//   SET_BIT(GICR,INT1);
//   sei();

//   while (1){}
// }
// ISR(INT0_vect){
//   if(i)
//     PORTA=_7seg[--i];
//   if(!i){
//     SET_BIT(PORTC,0);
//     CLR_BIT(PORTC,1);
//   }
// }
// ISR(INT1_vect){
//   if(i!=15){
//     PORTA=_7seg[++i];
//     SET_BIT(PORTC,1);
//     CLR_BIT(PORTC,0);
//   }
// }

// sheet 2
//  3
//  int main(void){
//    SET_BIT(DDRC,0);

//   SET_BIT(MCUCR,ISC01);
//   SET_BIT(MCUCR,ISC00);

//   SET_BIT(GICR,INT0);

//   sei();
//   while (1){
//   }
// }
// ISR(INT0_vect){
//   TOG_BIT(PORTC,0);
// }

// sheet 1
//  6
//  int main(void){
//    DDRA = 0xFF;
//    DDRB = 0x80;
//    DDRC = 0xFF;
//    DDRD = 0x00;
//    PORTD = 0x0C;
//    int counter;
//    while (1){
//      if (!((PIND>>2)&1)){
//        counter=0;
//          PORTC=0b00000010;
//          PORTB=0x80;
//        while (counter<12){
//          if(((PINB>>6)&1)){
//            counter++;
//            PORTA = counter;
//            _delay_ms(2000);
//          }
//        }
//        PORTC=0b00100001;
//        PORTB=0x00;

//     }
//   }
// }

// sheet 1
//  5
//  int main(void){
//    DDRC = 0xC0;
//    PORTC = 0x80;
//    DDRA = 0xFE;
//    PORTD = 0b00001100;
//    int i=0;
//    while (1){
//      PORTA = _7seg[i];
//      if ((!((PIND>>2)&1))&&i<9){i++;_delay_ms(2000);}
//      else if((!((PIND>>3)&1))&&i!=0){i--;_delay_ms(2000);}
//    }
//  }

// sheet 1
//  4
//  int main(void){
//    DDRA = 0xFE;
//    DDRC = 0xC0;
//    PORTC = 0x80;
//    while (1){
//      for (int i = 0; i < 16; i++){
//        PORTA = _7seg[i];
//        _delay_ms(9000);
//      }
//    }
//  }

// sheet 1
//  3
//  int main(void){
//    DDRC = 0xFF;
//    PORTD = 0b00001100;
//    int i=0;
//    while (1){
//      PORTC = i;
//      if (!((PIND>>2)&1)){i++;_delay_ms(2000);}
//      else if((!((PIND>>3)&1))&&i!=0){i--;_delay_ms(2000);}
//    }
//  }

// sheet 1
//  2
//  int main(void){
//    PORTD = 0b00001100;
//    DDRC = 0x03;
//    while (1){
//      if (!((PIND>>2)&1)) PORTC = 0x01;
//      else if(!((PIND>>3)&1)) PORTC = 0x00;

//   }
// }

// sheet 1
//  1
//  int main(void){
//    DDRC = 0x01;
//    while (1){
//      PORTC ^= 0x01;
//      _delay_ms(1000);
//    }
//  }

// int main(void){
//   DDRC = 0x01;
//   uint8_t counter;
//   while (1){
//     for(counter=0;counter<=7;counter++){
//       PORTD=(1<<counter);
//       _delay_ms(100);
//     }
//   }
// }