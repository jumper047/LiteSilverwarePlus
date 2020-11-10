#include "defines.h"
#include "drv_time.h"
#include "util.h"
#include <math.h>

extern int lowbatt;
extern int rxmode;
extern int failsafe;
extern int ledcommand;
extern char aux[];
extern char led_color;

#ifdef OSD_CHANNELS_SETTINGS
extern unsigned char chan[8];
extern unsigned char leds_on_ch;
#endif

// Colors
#define RGB_VALUE_WHITE RGB( 255, 255, 255)
#define RGB_VALUE_PINK RGB( 255, 20, 147)
#define RGB_VALUE_RED RGB( 255, 0, 0)
#define RGB_VALUE_ORANGE RGB( 255, 165, 0)
#define RGB_VALUE_YELLOW RGB( 255, 255, 0)
#define RGB_VALUE_GREEN RGB( 0, 255, 0)
#define RGB_VALUE_CYAN RGB( 0, 255, 255)
#define RGB_VALUE_BLUE RGB( 0, 0, 255)
#define RGB_VALUE_MAGENTA RGB( 255, 0, 255)

// normal flight rgb colour - LED switch ON
#define RGB_VALUE_INFLIGHT_ON RGB( 255 , 255 , 255 )

// normal flight rgb colour - LED switch OFF
#define RGB_VALUE_INFLIGHT_OFF RGB( 0 , 0 , 0 )

//  colour before bind
#define RGB_VALUE_BEFORE_BIND RGB( 0 , 128 , 128 )

// fade from one color to another when changed
#define RGB_FILTER_ENABLE
#define RGB_FILTER_TIME_MICROSECONDS 50e3

// runs the update once every 16 loop times ( 16 mS )
#define DOWNSAMPLE 16

#define RGB_FILTER_TIME FILTERCALC( 1000*DOWNSAMPLE , RGB_FILTER_TIME_MICROSECONDS)
#define RGB( r , g , b ) ( ( ((int)g&0xff)<<16)|( ((int)r&0xff)<<8)|( (int)b&0xff )) 

extern	void rgb_send( int data);

#if ( RGB_LED_NUMBER > 0)

// array with individual led brightnesses
int rgb_led_value[RGB_LED_NUMBER];
// loop count for downsampling
int rgb_loopcount = 0;
//rgb low pass filter variables 
float r_filt, g_filt, b_filt;


// sets all leds to a brightness
void rgb_led_set_all( int rgb )
{
#ifdef RGB_FILTER_ENABLE	
// deconstruct the colour into components
int g = rgb>>16;
int r = (rgb&0x0000FF00)>>8;
int b = rgb & 0xff;

// filter individual colors
lpf( &r_filt, r , RGB_FILTER_TIME);
lpf( &g_filt, g , RGB_FILTER_TIME);
lpf( &b_filt, b , RGB_FILTER_TIME);

	int temp = RGB( r_filt , g_filt , b_filt );
	
for ( int i = 0 ; i < RGB_LED_NUMBER ; i++)
	rgb_led_value[i] = temp;
	
#else
for ( int i = 0 ; i < RGB_LED_NUMBER ; i++)
	rgb_led_value[i] = rgb;
#endif
}

// set an individual led brightness
void rgb_led_set_one( int led_number , int rgb )
{
	rgb_led_value[led_number] = rgb;
}

// flashes between 2 colours, duty cycle 1 - 15
void rgb_ledflash( int color1 , int color2 , uint32_t period , int duty )
{
	if ( gettime() % period > (period*duty)>>4 )
	{
		rgb_led_set_all( color1 );
	}
	else
	{
		rgb_led_set_all( color2 );
	}

}

// speed of movement
float RAINBOW_SPEED = 5.0f;
int rgb_rainbow_phase[RGB_LED_NUMBER];
int rgb_rainbow_colour[RGB_LED_NUMBER];
#define rgb_decrease(amt) ((amt>0?fmax(amt-RAINBOW_SPEED,0):0))
#define rgb_increase(amt) ((amt<255?fmin(amt+RAINBOW_SPEED,255):255))

enum CURRENT_PHASE {
	WHITE_PHASE=0,
	RED_PHASE=1,
	YELLOW_PHASE=2,
	GREEN_PHASE=3,
	TURQUISE_PHASE=4,
	BLUE_PHASE=5,
	INDIGO_PHASE=6
};

void rgb_led_set_rainbow(int led_number)
{
	// deconstruct the current colour into components
	int g = rgb_rainbow_colour[led_number]>>16;
	int r = (rgb_rainbow_colour[led_number]&0x0000FF00)>>8;
	int b = rgb_rainbow_colour[led_number] & 0xff;
	
	switch(rgb_rainbow_phase[led_number])
	{
		case WHITE_PHASE:
			// fade from indigo to white (red + blue + green)
			if (r>=255 && g>=255 && b>=255)
				rgb_rainbow_phase[led_number]++;
			else
			{
					r=rgb_increase(r);
					g=rgb_increase(g);
					b=rgb_increase(b);
				}
			break;
		case RED_PHASE:
			// fade from white
			if (r>=255 && g<=0 && b<=0)
				rgb_rainbow_phase[led_number]++;
			else
			{
					r=rgb_increase(r);
					g=rgb_decrease(g);
					b=rgb_decrease(b);
				}			
			break;
		case YELLOW_PHASE:
			// fade from red to yellow (Red + Green)
			if (r>=255 && g>=255 && b<=0)
				rgb_rainbow_phase[led_number]++;
			else
			{
					r=rgb_increase(r);
					g=rgb_increase(g);
					b=rgb_decrease(b);
				}			
			break;
		case GREEN_PHASE:
			// fade from yellow to green
			if (r<=0 && g>=255 && b<=0)
				rgb_rainbow_phase[led_number]++;
			else
			{				
					r=rgb_decrease(r);
					g=rgb_increase(g);
					b=rgb_decrease(b);
				}			
			break;
		case TURQUISE_PHASE:
			// fade from green to turquise (Green+Blue)
			if (r<=0 && g>=255 && b>=255)
				rgb_rainbow_phase[led_number]++;
			else
			{
					r=rgb_decrease(r);
					g=rgb_increase(g);
					b=rgb_increase(b);
			}			
			break;
		case BLUE_PHASE:
			// fade from turquise to blue
			if (r<=0 && g<=0 && b>=255)
				rgb_rainbow_phase[led_number]++;
			else
			{
					r=rgb_decrease(r);
					g=rgb_decrease(g);
					b=rgb_increase(b);
				}			
			break;
		case INDIGO_PHASE:
			// fade from blue to indigo (Red+Blue)
			if (r>=255 && g<=0 && b>=255)
				rgb_rainbow_phase[led_number]=0;
			else
			{
					r=rgb_increase(r);
					g=rgb_decrease(g);
					b=rgb_increase(b);
				}			
			break;
	}
	
	rgb_rainbow_colour[led_number] = RGB(r,g,b);
	rgb_led_set_one( led_number , rgb_rainbow_colour[led_number] );
}

int init_complete=0;
void rgb_rainbow(void)
{
		for ( int i = 0 ; i < RGB_LED_NUMBER ; i++)
		{
			// start the LEDs in the off position, and in a rainbow sequence
			if (!init_complete)
			{
				rgb_rainbow_phase[i] = i % RGB_LED_NUMBER;
				rgb_rainbow_colour[i] = 0;
			}
			rgb_led_set_rainbow(i);
		}
		init_complete=1;
}


// speed of movement
float KR_SPEED = 0.005f * DOWNSAMPLE;

float kr_position = 0;
int kr_dir = 0;

// knight rider style led movement
void rgb_knight_rider( void)
{
	if ( kr_dir )
	{
		kr_position+= KR_SPEED;
		if ( kr_position > RGB_LED_NUMBER - 1 )
			kr_dir =!kr_dir;
	}
	else
	{
		kr_position-= KR_SPEED;
		if ( kr_position < 0 )
			kr_dir =!kr_dir;
	}

// calculate led value	
for ( int i = 0 ; i < RGB_LED_NUMBER ; i++)
	{
		float led_bright = fabsf( (float) i - kr_position);	
		if ( led_bright > 1.0f) led_bright = 1.0f;	
		led_bright = 1.0f - led_bright;	
		
		// set a green background as well, 32 brightness
		rgb_led_set_one( i , RGB( (led_bright*255.0f) , (32.0f-led_bright*32.0f) , 0) );

	}
		
}


// 2 led flasher
void rgb_ledflash_twin( int color1 , int color2 , uint32_t period )
{
	if ( gettime() % period > (period/2) )
	{
		for ( int i = 0 ; i < RGB_LED_NUMBER ; i++)
			{
				if( (i/2)*2 == i )  rgb_led_set_one( i ,color1 );
				else rgb_led_set_one( i ,color2 );
			}
	}
	else
	{
		for ( int i = 0 ; i < RGB_LED_NUMBER ; i++)
			{
				if( (i/2)*2 == i )  rgb_led_set_one( i ,color2 );
				else rgb_led_set_one( i ,color1 );
			}
	}

}


// main function
void rgb_led_lvc( void)
{
rgb_loopcount++;
if ( rgb_loopcount > DOWNSAMPLE )
{
	rgb_loopcount = 0;
// led flash logic	
if ( lowbatt )
{
	//rgb_led_set_all( RGB( 255 , 0 , 0 ) );
	rgb_ledflash ( RGB( 255 , 0 , 0 ), RGB( 255 , 32 , 0 ) ,500000 , 8);
}
else
{
		if ( rxmode == RXMODE_BIND)
		{
		// bind mode
		//rgb_ledflash ( RGB( 0 , 0 , 255 ), RGB( 0 , 128 , 0 ), 1000000, 12);
		//	rgb_ledflash_twin( RGB( 0 , 0 , 255 ), RGB( 0 , 128 , 0 ), 1000000);
		//	rgb_knight_rider();
			rgb_led_set_all( RGB_VALUE_BEFORE_BIND );
		}
		else
		{// non bind
			if ( failsafe) 
				{
					// failsafe flash
					rgb_ledflash ( RGB( 0 , 128 , 0 ) , RGB( 0 , 0 , 128 ) ,500000, 8);	
					//rgb_led_set_all( RGB( 0 , 128 , 128 ) );					
				}
			else 
			{

#ifndef OSD_CHANNELS_SETTINGS
				if ( aux[LEDS_ON] )
#else
				if ( aux[chan[leds_on_ch]] )
#endif
				  {if(led_color == 0){
				    rgb_led_set_all( RGB_VALUE_WHITE );
                                  } else if (led_color == 1) {
                                    rgb_led_set_all(RGB_VALUE_PINK);
                                  } else if (led_color == 2) {
                                    rgb_led_set_all(RGB_VALUE_RED);
                                  } else if (led_color == 3) {
                                    rgb_led_set_all(RGB_VALUE_ORANGE);
                                  } else if (led_color == 4) {
                                    rgb_led_set_all(RGB_VALUE_YELLOW);
                                  } else if (led_color == 5) {
                                    rgb_led_set_all(RGB_VALUE_GREEN);
                                  } else if (led_color == 6) {
                                    rgb_led_set_all(RGB_VALUE_CYAN);
                                  } else if (led_color == 7) {
                                    rgb_led_set_all(RGB_VALUE_MAGENTA);
				    } else if (led_color ==8) {
				      rgb_rainbow();
				    }
				    
                                  /* } else if (led_color == 8) { */
				  /*     rgb_ledflash( RGB_VALUE_RED, RGB_VALUE_BLUE, ) */
                                  /* } */
                                  /* rgb_led_set_all(RGB_VALUE_INFLIGHT_ON); */
                                } else
                                  rgb_led_set_all(RGB_VALUE_INFLIGHT_OFF);
			}
		} 		
		
	}

#ifdef RGB_LED_DMA
// send dma start signal    
rgb_send(0);    
#else
// send data to leds
for (int i = 0 ; i < RGB_LED_NUMBER ; i++)
	{
		rgb_send( rgb_led_value[i] );
	}
#endif
}	
}

#endif
