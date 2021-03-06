//#include <controle_de_temperatura_malha_fechada.h>

#include <16F877A.h>
#device ADC=10

#FUSES NOWDT                    //No Watch Dog Timer
#FUSES NOBROWNOUT               //No brownout reset
#FUSES NOLVP                    //No low voltage prgming, B3(PIC16) or B5(PIC18) used for I/O

#use delay(crystal=20000000)

#use rs232(baud=9600,parity=N,xmit=PIN_C6,rcv=PIN_C7,bits=8,stream=Serial)

#ifndef lcd_enable                                                             
   #define lcd_enable     pin_e1      // pino enable do LCD
   #define lcd_rs         pin_e2      // pino rs do LCD
   #define lcd_d4         pin_d4      // pino de dados d4 do LCD
   #define lcd_d5         pin_d5      // pino de dados d5 do LCD
   #define lcd_d6         pin_d6      // pino de dados d6 do LCD
   #define lcd_d7         pin_d7      // pino de dados d7 do LCD
#endif

#include "C:\Users\marcel\Dropbox\My PC (DESKTOP-KKDLKML)\Documents\Projeto de Micro\Driver\mod_lcd.c"



signed int16 temperaturaLM35 = 0.0, 
   temperaturaReferencia = 0.0, 
   ultimaTemperatura = 0.0, 
   PID = 0.0, 
   erro = 0.0,
   proporcional = 0.0,
   integrativo = 0.0, 
   derivativo = 0.0,
   kp=3,
   ki=1, 
   kd=1;
   
   
int16 passos = 0; 


#INT_RTCC
void  RTCC_isr(void) 
{
   
   if(passos <= PID){
      output_high(PIN_C5);
   }else{
      output_low(PIN_C5);
   }
   
   passos++;
   
  if(passos >= 19) { // 1000/51,2 = 19
      passos = 0;
   }
}
   
  
     
   
void main()
{
      
      setup_adc_ports(AN0_AN1_AN2_AN3_AN4);
      setup_adc(ADC_CLOCK_DIV_16);
      
       lcd_ini();
       delay_us(10);
             
      
      setup_timer_0(RTCC_INTERNAL|RTCC_DIV_1|RTCC_8_bit);   // 51,2 us overflow
      setup_timer_2(T2_DIV_BY_16,155,1);                   //499 us overflow, 499 us interrupt
   
      setup_ccp1(CCP_PWM);
      setup_ccp2(CCP_PWM);
      
      set_pwm1_duty((int16)510);        
      
      
      enable_interrupts(INT_RTCC);
      enable_interrupts(GLOBAL);

   while(TRUE)
   {
     
      set_adc_channel(0); 
      delay_us(10);
   
      //Convertendo 
      // Leitura ---> 77 - 28 = 49
      //AD -> ?C =  1023/(leitura * 50)
     // 0.048
      temperaturaReferencia = (read_adc()*0.048) + 28; //  //Temperatura minima ? 28 ent?o ? de 28 a 77
       
       
       
      set_adc_channel(2);
      delay_us(10);
      
      temperaturaLM35 = (read_adc()/2);
      
      set_adc_channel(1);
      delay_us(10);
      
      set_pwm1_duty(read_adc());
      
      delay_us(10);
      
       // ----------> C?lculo do PID -----------
       erro = temperaturaReferencia - temperaturaLM35;
       
       proporcional = kp * erro;
       
       integrativo += erro * ki;
       
       derivativo = (ultimaTemperatura  - temperaturaLM35 )* kd;
       
       ultimaTemperatura = temperaturaLM35;
        
       PID =  proporcional +  integrativo + derivativo;
       
       set_pwm2_duty((int16)PID);
        
      printf(lcd_escreve, "\fRef=%Ld,Err=%Ld\n\rTemp=%Ld,pid=%Ld", temperaturaReferencia, erro, temperaturaLM35, PID);
      
      fprintf(Serial, "%Ld %Ld %Ld\n",temperaturaReferencia, temperaturaLM35, erro);
      
      delay_ms(10);
      
   }

}
