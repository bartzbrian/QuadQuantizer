#include <Wire.h>
#include <SPI.h>
#include "Adafruit_MCP23017.h"
Adafruit_MCP23017 IO;
int DAC_SS_1 = 9;
int DAC_SS_2 = 10; 
int isScale=0;

int inOne;
int inTwo;
int inThree;
int inFour;

int outputOne;
int outputTwo;
int outputThree;
int outputFour;

int currDiffOne=0;
int currDiffTwo=0;
int currDiffThree=0;
int currDiffFour=0;

int outputDiffTwo=5000;
int outputDiffOne=5000;
int outputDiffThree=5000;
int outputDiffFour=5000;

int scaleMap[12]{0,0,0,0,0,0,0,0,0,0,0,0};

int lookup[12][5];

//buttonPin,LEDPIN (last four LEDS being on the expander chip)
int button_led_bindings[12][2]{
  {0,2},{1,3},{2,4},{3,5},{4,6},{5,7},{6,8},
  {7,20},{8,12},{9,13},{10,14},{11,15} 
};

int button_vars[12][2]{
  {0,0},{0,0},{0,0},{0,0},{0,0},{0,0},
  {0,0},{0,0},{0,0},{0,0},{0,0},{0,0} 
};

void setup() {

  Serial.begin(9600);
  IO.begin();
  pinMode(DAC_SS_1, OUTPUT); 
  digitalWrite(DAC_SS_1, HIGH); 
  pinMode(DAC_SS_2, OUTPUT); 
  digitalWrite(DAC_SS_2, HIGH); 
  SPI.begin(); 

  setPinModes();
  populateLookup();
  
}

void setPinModes(){
  
  pinMode(2,OUTPUT);
  pinMode(3,OUTPUT);
  pinMode(4,OUTPUT);
  pinMode(5,OUTPUT);
  pinMode(6,OUTPUT);
  pinMode(7,OUTPUT);
  pinMode(8,OUTPUT);
  pinMode(20,OUTPUT);
  
  for (int i=0;i<16;i++){
    if (i<12){
      IO.pinMode(i,INPUT);
      IO.pullUp(i,HIGH);  
    } else{
      IO.pinMode(i,OUTPUT);  
    } 
  }
}

void populateLookup(){
  int pasteVal=0;
  for(int i=0;i<5;i++){
    for(int j=0;j<12;j++){
       lookup[j][i]=pasteVal;
       pasteVal+=68.266; 
    }  
  }
}

int updateScale(){
  int on=0;
  for(int i=0;i<12;i++){
    button_vars[i][0]=IO.digitalRead(button_led_bindings[i][0]);
    if (button_vars[i][0]&&!button_vars[i][1]){
      if (scaleMap[i]){
        scaleMap[i]=0;
      } else {
        scaleMap[i]=1;
        on=1;  
      }             
    }
    button_vars[i][1]=button_vars[i][0];
  }
  return on;
}

void DACwrite(int cs_pin, byte dac, int value) {
  byte low = value & 0xff; 
  byte high = (value >> 8) & 0x0f; 
  dac = (dac & 1) << 7; 
  digitalWrite(cs_pin, LOW);
  SPI.transfer(dac | 0x30 | high); 
  SPI.transfer(low); 
  digitalWrite(cs_pin, HIGH);
}

void loop() {

  inOne = analogRead(A0)*4;
  inTwo = analogRead(A1)*4;
  inThree = analogRead(A2)*4;
  inFour = analogRead(A3)*4;  
  
  currDiffOne=0;
  currDiffTwo=0;
  currDiffThree=0;
  currDiffFour=0;
  
  outputDiffOne=5000;
  outputDiffTwo=5000;
  outputDiffThree=5000;
  outputDiffFour=5000;

  isScale=updateScale();
   
  if(isScale){
    for(int j=0;j<12;j++){
      
      if (scaleMap[j]){
        //turn on correct LEDS
        if(j<8){
          digitalWrite(button_led_bindings[j][1],HIGH);
        }else{
          IO.digitalWrite(button_led_bindings[j][1],HIGH);
        }
        
        for(int i=0;i<5;i++){
          currDiffOne = abs(inOne-lookup[j][i]);
          currDiffTwo = abs(inTwo-lookup[j][i]);
          currDiffThree = abs(inThree-lookup[j][i]);
          currDiffFour = abs(inFour-lookup[j][i]);
          
          if (currDiffOne<outputDiffOne){
            outputDiffOne=currDiffOne;
            outputOne=lookup[j][i];  
          }
          
          if (currDiffTwo<outputDiffTwo){
            outputDiffTwo=currDiffTwo;
            outputTwo=lookup[j][i];  
          }

          if (currDiffThree<outputDiffThree){
            outputDiffThree=currDiffThree;
            outputThree=lookup[j][i];  
          }
  
          if (currDiffFour<outputDiffFour){
            outputDiffFour=currDiffFour;
            outputFour=lookup[j][i];  
          }
                                        
        }
      }
    }
  } else {
         
      outputOne=0;
      outputTwo=0;
      outputThree=0;
      outputFour=0; 
      for(int j=0;j<12;j++){     
        if(j<8){
          digitalWrite(button_led_bindings[j][1],LOW);
        }else{
          IO.digitalWrite(button_led_bindings[j][1],LOW);
        }
      }      
    }
  
  DACwrite(DAC_SS_1,0,outputOne);
  DACwrite(DAC_SS_1,1,outputTwo);
  DACwrite(DAC_SS_2,0,outputThree);
  DACwrite(DAC_SS_2,1,outputFour);
  
}
