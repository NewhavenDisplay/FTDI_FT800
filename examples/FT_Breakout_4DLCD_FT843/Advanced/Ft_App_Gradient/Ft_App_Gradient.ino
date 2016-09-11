//this file contains the test application for ft800 plus board
#include "SPI.h"
#include "Wire.h"
#include "FT_Breakout_4DLCD_FT843.h"

FT800IMPL_SPI FTImpl(FT_CS_PIN,FT_PDN_PIN,FT_INT_PIN);

/* Api to bootup ft800, verify FT800 hardware and configure display/audio pins */
/* Returns 0 in case of success and 1 in case of failure */
int16_t BootupConfigure()
{
	uint32_t chipid = 0;
	FTImpl.Init(FT_DISPLAY_RESOLUTION);//configure the display to the WQVGA

	delay(20);//for safer side
	chipid = FTImpl.Read32(FT_ROM_CHIPID);
	
	/* Identify the chip */
	if(FT800_CHIPID != chipid)
	{
		Serial.print("Error in chip id read ");
		Serial.println(chipid,HEX);
		return 1;
	}
	
	/* Set the Display & audio pins */
	FTImpl.SetDisplayEnablePin(FT_DISPENABLE_PIN);
    FTImpl.SetAudioEnablePin(FT_AUDIOENABLE_PIN); 
	FTImpl.DisplayOn(); 	
	FTImpl.AudioOn();  		
	return 0;
}


uint32_t coprostatus = 0;
PROGMEM prog_uchar home_star_icon[] = {0x78,0x9C,0xE5,0x94,0xBF,0x4E,0xC2,0x40,0x1C,0xC7,0x7F,0x2D,0x04,0x8B,0x20,0x45,0x76,0x14,0x67,0xA3,0xF1,0x0D,0x64,0x75,0xD2,0xD5,0x09,0x27,0x17,0x13,0xE1,0x0D,0xE4,0x0D,0x78,0x04,0x98,0x5D,0x30,0x26,0x0E,0x4A,0xA2,0x3E,0x82,0x0E,0x8E,0x82,0xC1,0x38,0x62,0x51,0x0C,0x0A,0x42,0x7F,0xDE,0xB5,0x77,0xB4,0x77,0x17,0x28,0x21,0x26,0x46,0xFD,0x26,0xCD,0xE5,0xD3,0x7C,0xFB,0xBB,0xFB,0xFD,0xB9,0x02,0xCC,0xA4,0xE8,0x99,0x80,0x61,0xC4,0x8A,0x9F,0xCB,0x6F,0x31,0x3B,0xE3,0x61,0x7A,0x98,0x84,0x7C,0x37,0xF6,0xFC,0xC8,0xDD,0x45,0x00,0xDD,0xBA,0xC4,0x77,0xE6,0xEE,0x40,0xEC,0x0E,0xE6,0x91,0xF1,0xD2,0x00,0x42,0x34,0x5E,0xCE,0xE5,0x08,0x16,0xA0,0x84,0x68,0x67,0xB4,0x86,0xC3,0xD5,0x26,0x2C,0x20,0x51,0x17,0xA2,0xB8,0x03,0xB0,0xFE,0x49,0xDD,0x54,0x15,0xD8,0xEE,0x73,0x37,0x95,0x9D,0xD4,0x1A,0xB7,0xA5,0x26,0xC4,0x91,0xA9,0x0B,0x06,0xEE,0x72,0xB7,0xFB,0xC5,0x16,0x80,0xE9,0xF1,0x07,0x8D,0x3F,0x15,0x5F,0x1C,0x0B,0xFC,0x0A,0x90,0xF0,0xF3,0x09,0xA9,0x90,0xC4,0xC6,0x37,0xB0,0x93,0xBF,0xE1,0x71,0xDB,0xA9,0xD7,0x41,0xAD,0x46,0xEA,0x19,0xA9,0xD5,0xCE,0x93,0xB3,0x35,0x73,0x0A,0x69,0x59,0x91,0xC3,0x0F,0x22,0x1B,0x1D,0x91,0x13,0x3D,0x91,0x73,0x43,0xF1,0x6C,0x55,0xDA,0x3A,0x4F,0xBA,0x25,0xCE,0x4F,0x04,0xF1,0xC5,0xCF,0x71,0xDA,0x3C,0xD7,0xB9,0xB2,0x48,0xB4,0x89,0x38,0x20,0x4B,0x2A,0x95,0x0C,0xD5,0xEF,0x5B,0xAD,0x96,0x45,0x8A,0x41,0x96,0x7A,0x1F,0x60,0x0D,0x7D,0x22,0x75,0x82,0x2B,0x0F,0xFB,0xCE,0x51,0x3D,0x2E,0x3A,0x21,0xF3,0x1C,0xD9,0x38,0x86,0x2C,0xC6,0x05,0xB6,0x7B,0x9A,0x8F,0x0F,0x97,0x1B,0x72,0x6F,0x1C,0xEB,0xAE,0xFF,0xDA,0x97,0x0D,0xBA,0x43,0x32,0xCA,0x66,0x34,0x3D,0x54,0xCB,0x24,0x9B,0x43,0xF2,0x70,0x3E,0x42,0xBB,0xA0,0x95,0x11,0x37,0x46,0xE1,0x4F,0x49,0xC5,0x1B,0xFC,0x3C,0x3A,0x3E,0xD1,0x65,0x0E,0x6F,0x58,0xF8,0x9E,0x5B,0xDB,0x55,0xB6,0x41,0x34,0xCB,0xBE,0xDB,0x87,0x5F,0xA9,0xD1,0x85,0x6B,0xB3,0x17,0x9C,0x61,0x0C,0x9B,0xA2,0x5D,0x61,0x10,0xED,0x2A,0x9B,0xA2,0x5D,0x61,0x10,0xED,0x2A,0x9B,0xA2,0x5D,0x61,0x10,0xED,0x2A,0x9B,0xED,0xC9,0xFC,0xDF,0x14,0x54,0x8F,0x80,0x7A,0x06,0xF5,0x23,0xA0,0x9F,0x41,0xF3,0x10,0x30,0x4F,0x41,0xF3,0x18,0x30,0xCF,0xCA,0xFC,0xFF,0x35,0xC9,0x79,0xC9,0x89,0xFA,0x33,0xD7,0x1D,0xF6,0x5E,0x84,0x5C,0x56,0x6E,0xA7,0xDA,0x1E,0xF9,0xFA,0xAB,0xF5,0x97,0xFF,0x2F,0xED,0x89,0x7E,0x29,0x9E,0xB4,0x9F,0x74,0x1E,0x69,0xDA,0xA4,0x9F,0x81,0x94,0xEF,0x4F,0xF6,0xF9,0x0B,0xF4,0x65,0x51,0x08};


char *info[] = {  "FT800 Gradient Application",
                          "APP to demonstrate interactive Gradient,",
                          "using String, Gradient",
                          "& Slider."
				}; 

/***********************API used to SET the ICON******************************************/
/*Refer the code flow in the flowchart availble in the Application Note */


void home_setup()
{

	
  /*Icon  file is deflated use J1 Command to inflate the file and write into the GRAM*/
  FTImpl.WriteCmd(CMD_INFLATE);
  FTImpl.WriteCmd(250*1024L);
  FTImpl.WriteCmdfromflash(home_star_icon,sizeof(home_star_icon));
  /*Set the Bitmap properties for the ICONS*/ 	
  FTImpl.DLStart( );        // start
  FTImpl.Clear(1,1,1);
  FTImpl.ColorRGB(255, 255, 255);
  FTImpl.BitmapHandle(13);    // handle for background stars
  FTImpl.BitmapSource(250*1024L);      // Starting address in gram
  FTImpl.BitmapLayout(FT_L4, 16, 32);  // format 
  FTImpl.BitmapSize(FT_NEAREST, FT_REPEAT, FT_REPEAT, 512, 512);
  FTImpl.BitmapHandle(14);    // handle for background stars
  FTImpl.BitmapSource(250*1024L);      // Starting address in gram
  FTImpl.BitmapLayout(FT_L4, 16, 32);  // format 
  FTImpl.BitmapSize(FT_NEAREST, FT_BORDER, FT_BORDER, 32, 32  );
  FTImpl.Display();
  FTImpl.Cmd_Swap();
  FTImpl.Finish();
}
/********API to return the assigned TAG value when penup,for the primitives/widgets******/

static uint8_t keypressed=0;
uint8_t Read_Keys()
{
  static uint8_t Read_tag=0,temp_tag=0,ret_tag=0;	
  Read_tag = FTImpl.Read(REG_TOUCH_TAG);
  ret_tag = NULL;
  if(Read_tag!=NULL)								// Allow if the Key is released
  {
    if(temp_tag!=Read_tag)
    {
      temp_tag = Read_tag;	
      keypressed = Read_tag;										// Load the Read tag to temp variable	
    }  
  }
  else
  {
    if(temp_tag!=0)
    {
      ret_tag = temp_tag;
	  temp_tag = 0;
    }  
    keypressed = 0;
  }
  return ret_tag;
}

// Info Screen//
void Info()
{
  uint16_t dloffset = 0,z;

  FTImpl.DLStart( ); 
  FTImpl.Clear(1,1,1);
  FTImpl.ColorRGB(255,255,255);
  FTImpl.Cmd_Text(FT_DISPLAYWIDTH/2,FT_DISPLAYHEIGHT/2,26,FT_OPT_CENTERX|FT_OPT_CENTERY,"Please tap on a dot");
  FTImpl.Cmd_Calibrate(0);
  FTImpl.Display();
  FTImpl.Cmd_Swap();
  FTImpl.Finish();
// Ftdi Logo animation 
  FTImpl.Cmd_Logo( );
  do
  {  
    coprostatus = FTImpl.CheckLogo();
  }while(coprostatus!=2);
  dloffset = FTImpl.Read16(REG_CMD_DL);
  dloffset -=4;
  //Copy the Displaylist from DL RAM to GRAM
  FTImpl.WriteCmd(CMD_MEMCPY);
  FTImpl.WriteCmd(100000L);
  FTImpl.WriteCmd(FT_RAM_DL);
  FTImpl.WriteCmd(dloffset);
  //Enter into Info Screen
  do
  {
    FTImpl.DLStart( );   
    FTImpl.Cmd_Append(100000L,dloffset);
    //Reset the BITMAP properties used during Logo animation
    FTImpl.BitmapTransformA(256);
    FTImpl.BitmapTransformA(256);
    FTImpl.BitmapTransformB(0);
    FTImpl.BitmapTransformC(0);
    FTImpl.BitmapTransformD(0);
    FTImpl.BitmapTransformE(256);
    FTImpl.BitmapTransformF(0);  
    FTImpl.SaveContext();	
    //Display the information with transparent Logo using Edge Strip  
    FTImpl.ColorRGB(219,180,150);
    FTImpl.ColorA(220);
    FTImpl.Begin(FT_EDGE_STRIP_A);
    FTImpl.Vertex2f(0,FT_DISPLAYHEIGHT*16);
    FTImpl.Vertex2f(FT_DISPLAYWIDTH*16,FT_DISPLAYHEIGHT*16);
    FTImpl.ColorA(255);
    FTImpl.RestoreContext();	
    FTImpl.ColorRGB(0,0,0);
   // INFORMATION 
    FTImpl.Cmd_Text(FT_DISPLAYWIDTH/2,20,28,FT_OPT_CENTERX|FT_OPT_CENTERY,(char*)info[0]);
    FTImpl.Cmd_Text(FT_DISPLAYWIDTH/2,60,26,FT_OPT_CENTERX|FT_OPT_CENTERY,(char*)info[1]);
    FTImpl.Cmd_Text(FT_DISPLAYWIDTH/2,90,26,FT_OPT_CENTERX|FT_OPT_CENTERY,(char*)info[2]);  
    FTImpl.Cmd_Text(FT_DISPLAYWIDTH/2,120,26,FT_OPT_CENTERX|FT_OPT_CENTERY,(char*)info[3]);  
    FTImpl.Cmd_Text(FT_DISPLAYWIDTH/2,FT_DISPLAYHEIGHT-30,26,FT_OPT_CENTERX|FT_OPT_CENTERY,"Click to play");
    //Check if the Play key enter then change the color
    if(keypressed!='P')
    FTImpl.ColorRGB(255,255,255);
    else
    FTImpl.ColorRGB(100,100,100);
    FTImpl.Begin(FT_POINTS);   
    FTImpl.PointSize(20*16);
    FTImpl.Tag('P');
    FTImpl.Vertex2f((FT_DISPLAYWIDTH/2)*16,(FT_DISPLAYHEIGHT-60)*16);
    FTImpl.ColorRGB(180,35,35);
    FTImpl.Begin(FT_BITMAPS);
    FTImpl.Vertex2ii((FT_DISPLAYWIDTH/2)-14,(FT_DISPLAYHEIGHT-75),14,4);
    FTImpl.Display();
    FTImpl.Cmd_Swap();
	FTImpl.Finish();
  }while(Read_Keys()!='P');
  
  /* wait until Play key is not pressed*/ 
} 


void Gradient()
{
  uint32_t Read_xy = 0,
              tracker;
  uint16_t x1,x2,y1,y2,
              Read_x = 0,Read_y = 0,
  	      val1=48568,  val2=32765,
	      tx = 48,ty = 20,tval;
					
  uint8_t		
         Read_Tag  = 0,
	     drag,buff[512];


  x1 = 50;y1 = 50;
  sTagXY sTagxy;
  x2 = FT_DISPLAYWIDTH-60;
  y2 = FT_DISPLAYHEIGHT-40;
  drag = 0;
  // compute the background gradient effect
  for(tval=0;tval<(FT_DISPLAYHEIGHT/2);tval++)	
  {
    buff[FT_DISPLAYHEIGHT-1-tval] = buff[tval] = (tval*0.9);		
  }
  FTImpl.Write(4096L,buff,FT_DISPLAYHEIGHT);
  
  // Set the bitmap properties for the background
  FTImpl.DLStart();
  FTImpl.Clear(1,1,1); 
  FTImpl.BitmapHandle(2);	
  FTImpl.BitmapSource(4096L);	
  FTImpl.BitmapLayout(FT_L8,1,FT_DISPLAYHEIGHT);	
  FTImpl.BitmapSize(FT_NEAREST, FT_REPEAT, FT_BORDER, FT_DISPLAYWIDTH, FT_DISPLAYHEIGHT);
  FTImpl.Cmd_Track((FT_DISPLAYWIDTH-38),40,8,FT_DISPLAYHEIGHT-65,3);	
  FTImpl.Cmd_Track((FT_DISPLAYWIDTH-15),40,8,FT_DISPLAYHEIGHT-65,4);	
  FTImpl.Display();
  FTImpl.Cmd_Swap();
  FTImpl.Finish(); 
  do
  {
	/* Read the touch screen xy and tag from GetTagXY API */
	FTImpl.GetTagXY(sTagxy);
		
    Read_Tag = sTagxy.tag;
    // pick the x& y cordinates of the points  
    Read_y = sTagxy.y;
    Read_x = sTagxy.x;
    if(FTImpl.IsPendown() == 0)  drag = 0;
    else if(Read_Tag==1 || Read_Tag==2)drag = Read_Tag; 
    /* compute the coordintes of two points x&y*/
    if(drag == 1)
    {
	if(Read_x>=(FT_DISPLAYWIDTH-60))0; else	x1 = Read_x; 
	if(Read_y>=(FT_DISPLAYHEIGHT-20))0; else	y1 = Read_y;
    }
    if(drag == 2)
    {
	if(Read_x>=(FT_DISPLAYWIDTH-60))0; else	x2 = Read_x; 
	if(Read_y>=(FT_DISPLAYHEIGHT-20))0; else	y2 = Read_y;
    }
    /* read the tracker*/
    tracker = FTImpl.Read32(REG_TRACKER);	
    /* compute the color for the gradient */
    if((tracker&0xff) > 2)
    {
      if((tracker&0xff)==3)
      {
		val1 = (tracker>>16);		
      }
      else if((tracker&0xff)==4)
      {
	val2 = (tracker>>16);		
      }
    }
// start the new displaylist
    FTImpl.DLStart();
    FTImpl.ClearColorRGB((55,55,55));
    FTImpl.Clear(1,1,1);
    FTImpl.Begin(FT_BITMAPS);
    FTImpl.Vertex2ii(0,0,2,0);
    FTImpl.Cmd_FGColor(0xffffff); 
    FTImpl.TagMask(1);	 
    FTImpl.Cmd_BGColor(val2*255);	
    FTImpl.Tag(4);
    FTImpl.Cmd_Slider((FT_DISPLAYWIDTH-15),40,8,(FT_DISPLAYHEIGHT-65),0,val2,65535);	 
    FTImpl.Cmd_BGColor(val1*255);
    FTImpl.Tag(3);
    FTImpl.Cmd_Slider((FT_DISPLAYWIDTH-38),40,8,(FT_DISPLAYHEIGHT-65),0,val1,65535);	
    FTImpl.TagMask(0);	 
  
  
    FTImpl.ColorRGB(255,255,255);
    FTImpl.Cmd_Text(FT_DISPLAYWIDTH-45,10,26,0,"C1");
    FTImpl.Cmd_Text(FT_DISPLAYWIDTH-20,10,26,0,"C2");
    FTImpl.ScissorXY(0,10);
    FTImpl.ScissorSize(FT_DISPLAYWIDTH-50,FT_DISPLAYHEIGHT-30);
    FTImpl.Clear(1,1,1);
    FTImpl.Cmd_Gradient(x1,y1,val1*255,x2,y2,val2*255);
  		
    FTImpl.ColorRGB(0,0,0);
  // Place a dot for drag the color
    FTImpl.Begin(FT_POINTS);
    FTImpl.ColorMask(0,0,0,0);
    FTImpl.PointSize(10*16);
    FTImpl.TagMask(1);
    FTImpl.Tag(1);
    FTImpl.Vertex2f(x1*16,y1*16);
    FTImpl.Tag(2);
    FTImpl.Vertex2f(x2*16,y2*16);
    FTImpl.TagMask(0);
  
    FTImpl.ColorMask(1,1,1,1);
    FTImpl.PointSize(5*16);
    FTImpl.Begin(FT_POINTS);
    FTImpl.Vertex2f(x1*16,y1*16);
    FTImpl.Vertex2f(x2*16,y2*16);
    FTImpl.ColorRGB(255,255,255);
    FTImpl.Begin(FT_LINES);
    FTImpl.Vertex2f((x1-5)*16,(y1)*16);
    FTImpl.Vertex2f((x1+5)*16,(y1)*16);
    FTImpl.Vertex2f((x1)*16,(y1-5)*16);
    FTImpl.Vertex2f((x1)*16,(y1+5)*16);
    FTImpl.Vertex2f((x2-5)*16,(y2)*16);
    FTImpl.Vertex2f((x2+5)*16,(y2)*16);
    FTImpl.Vertex2f((x2)*16,(y2-5)*16);
    FTImpl.Vertex2f((x2)*16,(y2+5)*16);
    FTImpl.End();
    FTImpl.Display();
    FTImpl.Cmd_Swap();
    FTImpl.Finish();
  }while(1);
}


void setup()
{
	/* Initialize serial print related functionality */
	Serial.begin(9600);
	
	/* Set the Display Enable pin*/   
	Serial.println("--Start Application--");
	if(BootupConfigure())
	{
		//error case - do not do any thing
	}
  	else
	{
		home_setup();
		Info();
		Gradient();   
		FTImpl.Exit();
	}
	Serial.println("--End Application--");

}

void loop()
{
}
/* Nothing beyond this */