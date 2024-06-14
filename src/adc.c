#include "adc.h"


void adc_init(){
    SET_BIT(ADMUX,REFS0);
    SET_BIT(ADMUX,ADLAR);
    SET_BIT(ADCSRA,ADPS0);
    SET_BIT(ADCSRA,ADPS1);
    SET_BIT(ADCSRA,ADPS2);
    SET_BIT(ADCSRA,ADEN);
    
    SET_BIT(ADCSRA,ADIE);
}

uint8_t adc_read(uint8_t _ch){
    ADMUX &= 0xE0;
    ADMUX |= _ch;
    SET_BIT(ADCSRA,ADSC);
    while (ADCSRA & (1<<ADSC));
    return ADCH;
}


// void adc_init(){
//     SET_BIT(ADMUX,REFS0);
//     SET_BIT(ADCSRA,ADPS0);
//     SET_BIT(ADCSRA,ADPS1);
//     SET_BIT(ADCSRA,ADPS2);
//     SET_BIT(ADCSRA,ADEN);
//     SET_BIT(ADMUX,ADLAR);
// }

// uint8_t adc_read(uint8_t _ch){
//     ADMUX &= 0xE0;
//     ADMUX |= _ch;
//     SET_BIT(ADCSRA,ADSC);
//     while (ADCSRA & (1<<ADSC));
//     return ADCH;
// }