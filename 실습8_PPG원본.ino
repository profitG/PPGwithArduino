#include <MsTimer2.h>
float sampling_time=20; // 20 ms
int total_time=0;
float hz;
float data[100]; //실시간 대이터
float data_ana[100]; // 100카운트 전 데이터 (분석 데이터)
int differ[99]; // 실시간 데이터 미분값
int differ1[99]; // 100 카운트 전 미분값
int i=0;  // for 문 인자
int peakcount=0; //피크값 계수
float max0=0; 
float min0=0;
float amp=0;
float ampval=0;
int frequence_count=0;
int calculate_interval=6;
int count = 0;
int interrupt =0;
float ts=(sampling_time/1000);
float v_n=0; 
float y_n_high_1=0;
float b_n_1=0;
float y_n_low_1=0;
float y_n_low_pass=0;
float y_n_high_pass=0;

void setup() 
{
Serial.begin(9600);
MsTimer2::set(sampling_time, sampling);
MsTimer2::start();
}

void sampling()
{
  v_n=analogRead(A0);
  interrupt=0;
}

float low_filter(float a, float cutoff_Hz_low)
{ 
  float low_cut=(1/(2*3.14*cutoff_Hz_low));
  float y_n_low = y_n_low_1*(low_cut/(low_cut+ts))+ a*(ts/(low_cut+ts));
  y_n_low_1 = y_n_low;
  return y_n_low;
}

float high_filter(float b, float cutoff_Hz_high)
{
  float high_cut=(1/(2*3.14*cutoff_Hz_high)); 
  float y_n_high=(y_n_high_1+b-b_n_1)*(high_cut/(high_cut+ts));
  y_n_high_1=y_n_high;
  b_n_1=b;
  return y_n_high;
}

void peakdetect()
{
  max0=0;
  min0=0;
    
  for (i=0; i<100; i++)
  { 
    max0=max(max0,data[i]); // Max값 생성
    min0=min(min0,data[i]);
    data_ana[i]=data[i];  // 100 개 이전 데이터 전사
    if(i<99) differ1[i]=differ[i];
  }

  amp=100/(max0-min0);
 
  for (i=0; i<100; i++) // 신호크기 Normalization
  {
    ampval=data_ana[i]-min0;
    data_ana[i]=ampval*amp;
  }
   
  for (i=0; i<99; i++) // peak 찾고 0으로 세팅
  {
   if ( differ1[i]>0 && differ1[i+1]<=0 && data_ana[i+1]>=60)
    {
      data_ana[i+1]=0;
      peakcount++;
    }
  }
   
 }

void frequence()
{
  hz=peakcount/(0.1* sampling_time*calculate_interval)*100;
  peakcount=0;
}


void printt()
{
  Serial.print(v_n);
  Serial.print(",");
  Serial.println(data_ana[count-1]);
  //Serial.println(y_n_high_pass);
}

void loop() 
{
  if(interrupt==0)
  {
    y_n_low_pass = low_filter(v_n, 3.0);
    y_n_high_pass = high_filter(y_n_low_pass,0.1);
    data[count]= y_n_high_pass;
    count++; 
    printt();
    interrupt=1;
    if(count>= 2)
    {
      differ[count-2]=data[count-1]-data[count-2];
    }
    if(count==100)
     {
      frequence_count+=1;
      peakdetect();
      if(frequence_count==calculate_interval)
      {
        frequence();
        frequence_count=0;
      }
      
      count=0;  
     }
  }
}