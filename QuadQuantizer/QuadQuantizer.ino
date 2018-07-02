#include <Wire.h>
#include <SPI.h>
#include "Adafruit_MCP23017.h"
#include <MCPDAC.h>

Adafruit_MCP23017 IO;
MCPDACClass dacOne;
MCPDACClass dacTwo;

int DAC_SS_1 = 10;
int DAC_SS_2 = 9; 
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

//buttonPin,LEDPIN (last four LEDS being on the expander chip )
int button_led_bindings[12]{
  13,14,12,15,14,15,8,2,3,6,4,5  
};

//current,previous
int button_vars[12][2]{
  {1,1},{1,1},{1,1},{1,1},{1,1},{1,1},
  {1,1},{1,1},{1,1},{1,1},{1,1},{1,1} 
};

void setup() {
  
  Serial.begin(9600);
  
  pinMode(DAC_SS_1, OUTPUT); 
  digitalWrite(DAC_SS_1, HIGH); 
  pinMode(DAC_SS_2, OUTPUT); 
  digitalWrite(DAC_SS_2, HIGH);
   
  dacOne.begin(DAC_SS_1);
  dacTwo.begin(DAC_SS_2);
  dacOne.setGain(CHANNEL_A,GAIN_HIGH);
  dacOne.setGain(CHANNEL_B,GAIN_HIGH);
  dacTwo.setGain(CHANNEL_A,GAIN_HIGH);
  dacTwo.setGain(CHANNEL_B,GAIN_HIGH);
  
  IO.begin();

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
  pinMode(14,OUTPUT);
  pinMode(15,OUTPUT);

  for (int i=0;i<16;i++){
    if (i<12){
      IO.pinMode(i,INPUT);
      IO.pullUp(i,HIGH);  
    } else{
      IO.pinMode(i,OUTPUT);
      IO.digitalWrite(i,LOW);  
    } 
  }
}

void populateLookup(){
  int pasteVal=0;
  for(int i=0;i<5;i++){
    for(int j=0;j<12;j++){
       lookup[j][i]=pasteVal;
       pasteVal+=83.33;
    }  
  }
}

//update scale by toggling the momentary buttons
int updateScale(){
  int on=0;
  for(int i=0;i<12;i++){
    button_vars[i][0]=IO.digitalRead(i);
    if (!button_vars[i][0]&&button_vars[i][1]){
      if (scaleMap[i]){
        scaleMap[i]=0;
      } else {
        scaleMap[i]=1;  
      }
      if(button_led_bindings[i]<9||i==1||i==3){
        digitalWrite(button_led_bindings[i],scaleMap[i]);
      }else{
        IO.digitalWrite(button_led_bindings[i],scaleMap[i]);
      }             
    }
    if (scaleMap[i]){on=1;}
    button_vars[i][1]=button_vars[i][0];
  }
  return on;
}

void loop() {
  
  inOne = map(analogRead(A7),0,1023,0,4096);
  inTwo = map(analogRead(A6),0,1023,0,4096);
  inThree = map(analogRead(A2),0,1023,0,4096);
  inFour = map(analogRead(A3),0,1023,0,4096); 

  currDiffOne=0;
  currDiffTwo=0;
  currDiffThree=0;
  currDiffFour=0;
  
  outputDiffOne=5000;
  outputDiffTwo=5000;
  outputDiffThree=5000;
  outputDiffFour=5000;

  isScale=updateScale();

  //main quantization algorithm
  if(isScale){
    
    //iterate through scale 
    for(int j=0;j<12;j++){

      // if the note is on (i.e. is part of the selected scale)
      if (scaleMap[j]){
        
        //iterate through possible quantization values 
        //from the lookup table for each of the four channels
        
        for(int i=0;i<5;i++){
          
          currDiffOne = abs(inOne-lookup[j][i]);
          currDiffTwo = abs(inTwo-lookup[j][i]);
          currDiffThree = abs(inThree-lookup[j][i]);
          currDiffFour = abs(inFour-lookup[j][i]);

          //update each channel's output with the best target quantization 
          //value so far (i.e. the output which has the minimum difference between
          //the input to any of the possible outputs)
          
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
    
      //if no scale output 0V on DACs   
      outputOne=0;
      outputTwo=0;
      outputThree=0;
      outputFour=0; 
  
    }

  dacOne.setVoltage(CHANNEL_A,outputOne&0x0fff);
  dacOne.setVoltage(CHANNEL_B,outputTwo&0x0fff);
  dacTwo.setVoltage(CHANNEL_A,outputThree&0x0fff);
  dacTwo.setVoltage(CHANNEL_B,outputFour&0x0fff);

}
