#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include "mandelbrot.h"

TFT_eSPI tft = TFT_eSPI();

//#define SER 1

#define SCROLLFAC 20

#define BG_PIN 27

// Pads to select function
#define PADSIZE 30

#define PADX0 30
#define PADY0 30

#define PADX1 319-30
#define PADY1 30

#define PADX2 30
#define PADY2 239-30

#define PADX3 319-30
#define PADY3 239-30

#define PADX4 320/2
#define PADY4 30

#define PADX5 320/2
#define PADY5 239-30

#define PADX6 30
#define PADY6 240/2

#define PADX7 319-30
#define PADY7 240/2

// Functions of Pads
#define SETMID 0
#define ZOOMIN 2
#define ZOOMOUT 3
#define CALCFRAC 1

#define SCROLLUP 4
#define SCROLLDOWN 5
#define SCROLLLEFT 6
#define SCROLLRIGHT 7

uint32_t storetime;
uint32_t presstime;
uint32_t dummytime;

uint8_t longflag;
uint32_t longtime;

int16_t machma;
int16_t iter;
uint8_t touchFlag;

typedef struct xystruct
{
  uint16_t x,y;
} XY;

XY pads[8];

uint16_t colorTable[16]=
{
  tft.color565(255,0,0), tft.color565(0,255,0), tft.color565(0,0,255), tft.color565(255,255,0),
  tft.color565(255,0,255), tft.color565(0,255,255), tft.color565(255,128,0), tft.color565(128,255,0),
  tft.color565(128,0,255), tft.color565(255,0,128), tft.color565(0,128,255), tft.color565(0,255,128),
  tft.color565(64,0,255), tft.color565(255,0,64), tft.color565(255,64,0), tft.color565(255,255,255)
};

uint8_t pixelMove[16]={0,8,4,12,2,10,6,14,1,9,5,13,3,11,7,15};
uint8_t pixelCount[16]={16,8,4,4,2,2,2,2,1,1,1,1,1,1,1,1};

uint8_t drawnPixels[16][16];

int mandelbrot(double x,double y,int maxit)
{
  int16_t i;
  double xx,yy;
  double xx2,yy2;
  double l;
  xx=yy=0.0;
  i=-1;
  do
  {
    i++;
    xx2=xx*xx-yy*yy+x;
    yy2=2.0*xx*yy+y;
    l=xx2*xx2+yy2*yy2;
    xx=xx2; yy=yy2;
  }
  while ((i<maxit)&&(l<4));
  return(i);
}

// Findout if touch is inside a range
uint8_t range(uint16_t a,uint16_t min,uint16_t max,uint8_t mode)
{
  if(!mode)
    return((a>=min)&&(a<=max));
  else
    return((a<=239-min)&&(a>=239-max));
}

uint8_t padRange(uint8_t padNr,uint16_t x,uint16_t y)
{
  uint8_t flag1,flag2;
  flag1=range(x,pads[padNr].x-PADSIZE,pads[padNr].x+PADSIZE,false);
  flag2=range(y,pads[padNr].y-PADSIZE,pads[padNr].y+PADSIZE,false);
  return(flag1&&flag2);
}

uint8_t padSearch(uint16_t x,uint16_t y)
{
  uint8_t padNr,i;
  padNr=100;
  for(i=0;i<8;i++)
  {
    if(padRange(i,x,y))
      padNr=i;
  }
  return(padNr);
}

void printParameters(double midx,double midy,double scale,int16_t iter)
{
  char s[80];
  int i;
  tft.setCursor(0,0);
  tft.setTextSize(2);
  tft.fillScreen(TFT_BLACK);
  sprintf(s,"MX %lf",midx); tft.println(s);
  sprintf(s,"MY %lf",midy); tft.println(s);
  sprintf(s,"SC %l13f",scale); tft.println(s);
  sprintf(s,"IT %d",iter); tft.println(s);
  delay(2000);
}

void drawPlane(int16_t ii,int16_t jj,int16_t x,int16_t y,uint16_t col)
{
  int16_t i,j;
  //tft.drawPixel(x,y,col);
  for(i=0;i<pixelCount[ii];i++)
  {
    for(j=0;j<pixelCount[jj];j++)
    {
      if(!drawnPixels[pixelMove[ii]+i][pixelMove[jj]+j])
      {
        tft.drawPixel(x+pixelMove[ii]+i,y+pixelMove[jj]+j,col);
      }
    }
  }
}

void setup()
{
  int16_t ii,jj,xi,yi;
  int16_t i,j;
  uint16_t x,y;
  double xx,yy;
  double x1,y1,x2,y2;
  double sx,sy;
  double midx,midy;
  double midx2,midy2;
  double scal;
  int16_t val,val2;
  char s[80];
  uint8_t selPad,selPad2;
  uint8_t pressflag;

#ifdef SER
  Serial.begin(115200);
#endif

  pads[0].x=PADX0;
  pads[0].y=PADY0;
  pads[1].x=PADX1;
  pads[1].y=PADY1;
  pads[2].x=PADX2;
  pads[2].y=PADY2;
  pads[3].x=PADX3;
  pads[3].y=PADY3;
  pads[4].x=PADX4;
  pads[4].y=PADY4;
  pads[5].x=PADX5;
  pads[5].y=PADY5;
  pads[6].x=PADX7;
  pads[6].y=PADY7;
  pads[7].x=PADX6;
  pads[7].y=PADY6;
//  Serial.begin(115200);
  pinMode(BG_PIN,OUTPUT);
  digitalWrite(BG_PIN,HIGH);
  iter=40;
  scal=1.4;
  midx=-0.666; midy=0;
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  tft.setSwapBytes(true);
  uint16_t calData[5]={200,3600,200,3600,4};
  tft.setTouch(calData);
  tft.pushImage(0,0,320,240,image);
  delay(2000);
  do
  {
    x1=midx-scal; x2=midx+scal;
    y1=midy-scal*11/16; y2=midy+scal*11/16;
    sx=(x2-x1)/320.0;
    sy=(y2-y1)/240.0;
    tft.fillScreen(TFT_BLACK);
    printParameters(midx,midy,scal,iter);
    for(i=0;i<16;i++)
    {
      for(j=0;j<16;j++)
      {
        drawnPixels[i][j]=false;
      }
    }
    for(ii=0;ii<16;ii++)
    {
      for(jj=0;jj<16;jj++)
      {
      //xi=screenWush[ii].x-1;
      //yi=screenWush[ii].y-1;
        xi=pixelMove[ii];
        yi=pixelMove[jj];
        for(y=0;y<240;y+=16)
        {
          yy=y1+sy*(float)(y+yi);
          for(x=0;x<320;x+=16)
          {
            xx=x1+sx*(float)(x+xi);
            val=mandelbrot(xx,yy,iter);
            val2=val%16;
            if (val<iter)
              drawPlane(ii,jj,x,y,colorTable[val2]);
            else
              drawPlane(ii,jj,x,y,0);
          }
        }
        drawnPixels[pixelMove[ii]][pixelMove[jj]]=true;
      }
    }
    midx=(x1+x2)/2; midy=(y1+y2)/2;    
    touchFlag=false;
    selPad=100;
    pressflag=false;
    do
    {
      touchFlag=tft.getTouch(&x,&y);
      y=239-y;
      if(touchFlag)
      {
        if(!pressflag)
        {
          presstime=millis();
          dummytime=presstime;
          pressflag=true;
        }
        if(millis()-dummytime>1000)
        {
          dummytime=millis();
          tft.setCursor(0,0);
          //tft.fillRect(0, 0, 100, 8, TFT_BLACK);
          sprintf(s,"%d Sec.",((int)(millis()-presstime)/1000));
          tft.print(s);
        }
        selPad2=padSearch(x,y);
      }
      else
      {
        if(pressflag&&(millis()-presstime>1000))
        {
          selPad=selPad2;
          longtime=millis()-presstime;
          longflag=true;
        }
        else
        {
          if(pressflag)
            selPad=selPad2;
          longflag=false;
        }
      }
      #ifdef SER
      sprintf(s,"SELPAD %d",selPad);
      Serial.println(s);
      #endif
    }
    while(selPad==100);
    switch(selPad)
    {
      case SETMID:
        if(!longflag)
        {
          do
          {
            touchFlag=tft.getTouch(&x,&y);
            y=239-y;
          } while(touchFlag);
          do
          {
            touchFlag=tft.getTouch(&x,&y);
            y=239-y;
          } while(!touchFlag);
          midx2=midx+2.0*scal*(((double)x)-160.0)/320.0;
          midy2=midy+(11.0/6.0)*scal*(((double)y)-120)/240.0;
          midx=midx2;
          midy=midy2;
          scal/=2;
        #ifdef SER
        sprintf(s,"mid %lf %lf",midx,midy);
        Serial.println(s);
        #endif
          do
          {
            touchFlag=tft.getTouch(&x,&y);
          } while(touchFlag);
        }
        else
        {
          scal=1.4;
          midx=-0.666; midy=0;
        }
      break;
      case ZOOMIN:
        if(!longflag)
        {
          scal/=3;
        }
        else
        {
          iter+=10*(uint16_t)longtime/1000;
        }
      break;
      case ZOOMOUT:
        if(!longflag)
        {
          scal*=3;
        }
        else
        {
          iter-=10*(uint16_t)longtime/1000;
          if(iter<10) iter=10;
        }
      break;
      case CALCFRAC:
      break; 
      case SCROLLUP:
        if(!longflag)
        {
          midy+=scal/SCROLLFAC;
        }
        else
        {
          midy+=scal*((float)(longtime/1000)/SCROLLFAC);
        }
      break;
      case SCROLLDOWN:
        if(!longflag)
        {
          midy-=scal/SCROLLFAC;
        }
        else
        {
          midy-=scal*((float)(longtime/1000)/SCROLLFAC);
        }
      break;
      case SCROLLLEFT:
        if(!longflag)
        {
          midx-=scal/SCROLLFAC;
        }
        else
        {
          midx-=scal*((float)(longtime/1000)/SCROLLFAC);
        }
      break;
      case SCROLLRIGHT:
        if(!longflag)
        {
          midx+=scal/SCROLLFAC;
        }
        else
        {
          midx+=scal*((float)(longtime/1000)/SCROLLFAC);
        }
      break;
    }
  } while (1==1);
}

void loop()
{
  // put your main code here, to run repeatedly:
}
