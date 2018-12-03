// ---------------------------------------------------------------------------
// Created by Seon Rozenblum - seon@unexpectedmaker.com
// Copyright 2016 License: GNU GPL v3 http://www.gnu.org/licenses/gpl-3.0.html
//
// See "LaserXSegment.h" for purpose, syntax, version history, links, and more.
// ---------------------------------------------------------------------------

#include "LaserXSegment.h"
// This library is 100% entwined with the Adafruit NeoPixel library - please check their licensing terms in their library.
#include <Adafruit_NeoPixel.h>

#define DEBUG
#define NUM_PIXELS_PER_BOARD 15

#define ELEMENTS(x)   (sizeof(x) / sizeof(x[0]))



// Array of pixels per segment, 7 segments with 4 pixels each
// int segmentsPixels[14][5] 	{ { 0, 1, 2, 3, 4 }, 
//                               { 5, 6, 7, 8, 9 }, 
//                               { 10, 11, 12, 13, 14 }, 
//                               { 15, 16, 17, 18, 19 }, 
//                               { 20, 21, 22, 23, 24 }, 
//                               { 25, 26, 27, 28, 29 }, 
//                               { 30, 31, 32, 33, 34 },
// 							  { 35, 36, 37, -1, -1 },
// 							  { 38, 39, 40, -1, -1 },
// 							  { 41, 42, 43, -1, -1 },
// 							  { 44, 45, 46, -1, -1 },
// 							  { 47, 48, 49, 50, 51 },
// 							  { 52, 53, 54, 55, 56 },
// 							  { 57, -1, -1, -1, -1}
// 						  	};

// Array of pixel positions in X,Y format for mapping colours in X,Y space
byte pixelsXY[17][2]		{ 	{2,0}, 
								{0,1}, {1,1}, {2,1}, {3,1}, {4,1},
								{1,2}, {3,2},
								{0,3}, {1,3}, {2,3}, {3,3}, {4,3},
								{1,4}, {3,4},
								{2,5}, {5,5}
						  	};
					  
// Array of segment based rainbow colour values
uint32_t segmentRainbow[7][3] {
                              { 255,0,0 },
                              { 255,124,42 },
                              { 255,255,0 },
                              { 69,223,91 },
                              { 31,189,255 },
                              { 150,11,255 },
                              { 255,30,237 }
                            };

// Available characters a 7 Segment display can show					  
const byte ARRAY_SIZE = 58;
				
CharacterX characters_pixel[ ARRAY_SIZE ] {
	{ ' ', 0b0000000000000000 },
	{ '0', 0b0000000000111111 }, 
	{ '1', 0b0000000100000110 },
	{ '2', 0b0000000011011011 },
	{ '3', 0b0000000011001111 },
	{ '4', 0b0000000011100110 },
	{ '5', 0b0000000011101101 },
	{ '6', 0b0000000011111100 },
	{ '7', 0b0010000100000001 },
	{ '8', 0b0000000011111111 },
	{ '9', 0b0000000011100111 },
	{ 'a', 0b0000000011110111 },
	{ 'b', 0b0011000010001111 },
	{ 'c', 0b0000000000111001 },
	{ 'd', 0b0011000000001111 },
	{ 'e', 0b0000000011111001 },
	{ 'f', 0b0000000001110001 },
	{ 'g', 0b0000000010111101 },
	{ 'h', 0b0000000011110110 },
	{ 'i', 0b0011000000001001 },
	{ 'j', 0b0000000000011110 },
	{'k', 0b0000100101110000 },
	{'l', 0b0000000000111000},
	{'m', 0b0000001100110110},
	{'n', 0b0000101000110110},
	{'o', 0b0000000000111111},
	{'p', 0b0000000011110011},
	{'q', 0b0000100000111111},
	{'r', 0b0000100101110001},
	{'s', 0b0000001010001101},
	{'t', 0b0011000000000001},
	{'u', 0b0000000000111110},
	{'v', 0b0000010100110000},
	{'w', 0b0000110000110110},
	{'x', 0b0000111100000000},
	{'y', 0b0010001100000000},
	{'z', 0b0000010100001001},
	{'*', 0b0011111100000000},
	{'+', 0b0011000011000000},
	{'-', 0b0000000011000000},
	{'/', 0b0000010100000000},
	{'<', 0b0000100100000000},
	{'>', 0b0000011000000000},
	{'|', 0b0011000000000000},
	{'$', 0b0011000011101101},
	{'?', 0b0010000010000011},

	{'!', 0b0000000000011000},
	{'#', 0b0011000011001110},
	{'%', 0b0000010100100100},
	{'&', 0b0000101101011001},
	{'`', 0b0000001000000000},
	{'[', 0b0000000000111001},
	{']', 0b0000000000001111},
	{',', 0b0000010000000000},

	{'=', 0b0000000011001000},
	{'_', 0b0000000000001000},
	{'{', 0b0000011001001001},
	{'}', 0b0000100110001001},
};

													
LaserXSegment::LaserXSegment( uint8_t displayCount, uint8_t dPin )
{
	dispCount = displayCount;
	dispPin = dPin;
	pixels = Adafruit_NeoPixel ();
	pixels.updateType( NEO_GRB + NEO_KHZ800 );
  	pixels.updateLength( dispCount * NUM_PIXELS_PER_BOARD );
  	pixels.setPin(dispPin);
	isReady = false;
}

LaserXSegment::~LaserXSegment()
{
	isReady = false;
}

bool LaserXSegment::IsReady()
{
	return isReady;
}

void LaserXSegment::Begin( uint8_t brightness )
{

	pixels.begin(); // This initializes the NeoPixel library.
	pixels.show();
	pixels.setBrightness( brightness );

	cachedString = "";
	cachedCharacters = ( CharacterX *) malloc(dispCount * sizeof(CharacterX) );
	cachedBytes = ( uint16_t *) malloc(dispCount * sizeof(uint16_t) );
	
	// Set all cached bytes to 0
	for ( int i = 0; i < dispCount; i++ )
		cachedBytes[i] = 0;

	#ifdef DEBUG
		Serial.print("Brightness: ");
		Serial.println(brightness);
		Serial.println("Ready!");
	#endif

	//Digits are initialised and ready
	isReady = true;
	//isForcedUpper = false;
}

void LaserXSegment::Clear( uint8_t display )
{
	if ( isReady )
	{
		for ( int p = 0; p < NUM_PIXELS_PER_BOARD; p++ )
			pixels.setPixelColor( p + ( display * NUM_PIXELS_PER_BOARD ), Color(0,0,0) );
	}
}

void LaserXSegment::SetBrightness( uint8_t brightness )
{
	if ( isReady )
	{
		pixels.setBrightness( brightness );
	}
}


byte LaserXSegment::GetArraySize()
{
	return ARRAY_SIZE;
}



CharacterX LaserXSegment::FindCharacterX( byte c )
{
	for ( int i = 0; i < ELEMENTS( characters_pixel ); i++ )
	{
		if ( characters_pixel[i].id == c )
			return characters_pixel[i];
	}

	return characters_pixel[0];
}

String LaserXSegment::GetCharacterAtArrayIndex( int index )
{
	return ( (String)(char)characters_pixel[ index ].id );
}

// Convert separate R,G,B into packed 32-bit RGB color.
// Packed format is always RGB, regardless of LED strand color order.
uint32_t LaserXSegment::Color(uint8_t r, uint8_t g, uint8_t b)
{
  return ((uint32_t)r << 16) | ((uint32_t)g <<  8) | b;
}


void LaserXSegment::CheckToCacheBytes( String str )
{
	if ( str != cachedString )
	{
		cachedString = str;
		
		int lengthOfLoop = min( dispCount, (uint8_t)str.length() );
		
		int index = 0;
		for ( int s = 0; s < lengthOfLoop; s++ )
		{
			#ifdef DEBUG
			Serial.print( (String)str.charAt(s) );
			Serial.print( " .. " );
			#endif

			String temp = cachedString;
			temp.toLowerCase();
			char c = temp.charAt(s);
			

			if ( (String)c != "." )
			{ 
				cachedCharacters[index] = FindCharacterX( c );
				cachedBytes[index] = cachedCharacters[index].pixels;
				#ifdef DEBUG
				Serial.println( "1" );
				#endif
				index++;
			}
			else if ( s > 0 && bitRead( cachedCharacters[index-1].pixels, 63 ) != 1 )
			{
				cachedBytes[index-1] = cachedBytes[index-1] | 0b1000000000000000;
				#ifdef DEBUG
				Serial.println( "2" );
				#endif
			}
			else
			{
				cachedCharacters[index] = FindCharacterX( c );
				cachedBytes[index] = 0b1000000000000000;
				#ifdef DEBUG
				Serial.println( "3" );
				#endif
				index++;
			}
		}
	}
}

void LaserXSegment::DisplayTextVerticalRainbow( String text, uint32_t colorA, uint32_t colorB )
{
	if ( !isReady )
		return;

	CheckToCacheBytes( text );	

	// Clamp the length, so text longer than the display count is ignored
	int lengthOfLoop = min( dispCount, (uint8_t)text.length() );

	// Grab the byte (bits) for the segmens for the character passed in
	for ( int s = 0; s < lengthOfLoop; s++ )
	{
		for ( int i = 0; i < 63; i++ )
		{
			int y = pixelsXY[ i ][1];

			uint8_t red = ((Red(colorA) * (12 - y)) + (Red(colorB) * y)) * 0.08333;
			uint8_t green = ((Green(colorA) * (12 - y)) + (Green(colorB) * y)) * 0.08333;
			uint8_t blue = ((Blue(colorA) * (12 - y)) + (Blue(colorB) * y)) * 0.08333;

			uint32_t color = Color(red, green, blue );

			bool on = ( bitRead( cachedBytes[s], i) == 1 );
			pixels.setPixelColor( i + ( s * NUM_PIXELS_PER_BOARD ), on ? color : Color(0,0,0) );

		}
	}
		
	// we have finished setting all of the colors on each segment for this LaserXSegment, so lets turn on the pixels
	pixels.show();
		
	// CheckToCacheBytes( text );

	// // Clamp the length, so text longer than the display count is ignored
	// int lengthOfLoop = min( dispCount, (uint8_t)text.length() );

	// uint32_t color;
	
	// // Grab the byte (bits) for the segmens for the character passed in
	// for ( int s = 0; s < lengthOfLoop; s++ )
	// {
	// 	byte code = cachedBytes[s];
	// 	if(code > -1)
	// 	{
	// 		int pixelIndex = 0; // current pixel index
		
	// 		for( int segment = 0; segment < 7; segment++ )
	// 		{
	// 			bool on = ( bitRead( code, segment) == 1 );
	// 			for ( int p = 0; p < 4; p++ )
	// 			{
	// 				// we want the Y position (row) so we can use that as the colour index 
	// 				int y = pixelsXY[ pixelIndex ][1];

	// 				uint8_t red = ((Red(colorA) * (10 - y)) + (Red(colorB) * y)) * 0.1;
	// 				uint8_t green = ((Green(colorA) * (10 - y)) + (Green(colorB) * y)) * 0.1;
	// 				uint8_t blue = ((Blue(colorA) * (10 - y)) + (Blue(colorB) * y)) * 0.1;

	// 				color = Color(red, green, blue );
				
	// 				pixels.setPixelColor( segmentsPixels[ segment ][ p ] + ( s * NUM_PIXELS_PER_BOARD ), on ? color : Color(0,0,0) );
	// 				pixelIndex++;
	// 			}
	// 		}

	// 		#ifdef USEDP

	// 			bool on = ( bitRead( code, 7) == 1 );
	// 			pixels.setPixelColor( segmentsPixels[ 7 ][ 0 ], on ? color : Color(0,0,0));

	// 		#endif
	// 	}
	// }
		
	// // we have finished setting all of the colors on each segment for this LaserXSegment, so lets turn on the pixels
	// pixels.show(); 
}

void LaserXSegment::DisplayTextHorizontalRainbow( String text, uint32_t colorA, uint32_t colorB )
{
	if ( !isReady )
		return;	
	
	CheckToCacheBytes( text );

	int numPixelsPerColumn = 3;
	int numColumns = dispCount * numPixelsPerColumn;

	uint32_t color;

	// Clamp the length, so text longer than the display count is ignored
	int lengthOfLoop = min( dispCount, (uint8_t)text.length() );
	
	// Grab the byte (bits) for the segmens for the character passed in
	for ( int s = 0; s < lengthOfLoop; s++ )
	{
		for ( int i = 0; i < NUM_PIXELS_PER_BOARD; i++ )
		{
				bool on = ( bitRead( cachedBytes[s], i) == 1 );
				// we want the Y position (row) so we can use that as the colour index 
				int x = pixelsXY[ i ][0] + ( numPixelsPerColumn * s );

				uint8_t red = ((Red(colorA) * (numColumns - x)) + (Red(colorB) * x)) / numColumns;
				uint8_t green = ((Green(colorA) * (numColumns - x)) + (Green(colorB) * x)) / numColumns;
				uint8_t blue = ((Blue(colorA) * (numColumns - x)) + (Blue(colorB) * x)) / numColumns;

				color = Color(red, green, blue );
			
				pixels.setPixelColor( i + ( s * NUM_PIXELS_PER_BOARD ), on ? color : Color(0,0,0) );
				//pixelIndex++;
	
		}

		bool on = ( bitRead( cachedBytes[s], 7) == 1 );
		pixels.setPixelColor( 7 + ( s * NUM_PIXELS_PER_BOARD ), on ? color : Color(0,0,0));
	}
		
	// we have finished setting all of the colors on each segment for this Laser7Segment, so lets turn on the pixels
	pixels.show(); 
}

void LaserXSegment::DisplayTextMarquee( String text, uint8_t index, uint32_t color )
{
	if ( !isReady )
		return;
		
	// CheckToCacheBytes( text );
	
	// // Clamp the length, so text longer than the display count is ignored
	// int lengthOfLoop = min( dispCount, (uint8_t)text.length() );

	// // Grab the byte (bits) for the segmens for the character passed in
	// for ( int s = 0; s < lengthOfLoop; s++ )
	// {
	// 	byte code = cachedBytes[s];
		
	// 	if(code > -1)
	// 	{
	// 		for( int segment = 0; segment < 7; segment++ )
	// 		{
	// 			for ( int p = 0; p < 4; p++ )
	// 			{
	// 				bool on = ( bitRead( code, segment) == 1 );
									
	// 				if ( index == 0 && ( p == 1 || p == 3 ) )
	// 					on = false;
	// 				else if ( index == 1 && ( p == 0 || p == 2 ) )
	// 					on = false;

	// 				pixels.setPixelColor( segmentsPixels[ segment ][ p ] + ( s * NUM_PIXELS_PER_BOARD ), on ? color : Color(0,0,0) );
	// 			}
	// 		}

	// 		#ifdef USEDP

	// 			bool on = ( bitRead( code, 7) == 1 );
	// 			pixels.setPixelColor( segmentsPixels[ 7 ][ 0 ] + ( s * NUM_PIXELS_PER_BOARD ), on ? color : Color(0,0,0));

	// 		#endif
	// 	}
	// }
	// // we have finished setting all of the colors on each segment for this LaserXSegment, so lets turn on the pixels
	// pixels.show();
}

void LaserXSegment::DisplayTextChaser( String text, uint8_t index, uint32_t color )
{
	if ( !isReady )
		return;
	
	// CheckToCacheBytes( text );
	
	// // Clamp the length, so text longer than the display count is ignored
	// int lengthOfLoop = min( dispCount, (uint8_t)text.length() );

	// // Grab the byte (bits) for the segmens for the character passed in
	// for ( int s = 0; s < lengthOfLoop; s++ )
	// {
	// 	byte code = cachedBytes[s];
		
	// 	if(code > -1)
	// 	{
	// 		for( int segment = 0; segment < 7; segment++ )
	// 		{
	// 			for ( int p = 0; p < 4; p++ )
	// 			{
	// 				bool on = ( bitRead( code, segment) == 1 );
									
	// 				if ( p != index )
	// 					on = false;

	// 				pixels.setPixelColor( segmentsPixels[ segment ][ p ] + ( s * NUM_PIXELS_PER_BOARD ), on ? color : Color(0,0,0) );
	// 			}
	// 		}

	// 		#ifdef USEDP

	// 			bool on = ( bitRead( code, 7) == 1 );
	// 			pixels.setPixelColor( segmentsPixels[ 7 ][ 0 ] + ( s * NUM_PIXELS_PER_BOARD ), on ? color : Color(0,0,0));

	// 		#endif
	// 	}
	// }
	// // we have finished setting all of the colors on each segment for this LaserXSegment, so lets turn on the pixels
	// pixels.show();
}

void LaserXSegment::DisplayTextColorCycle( String text, uint8_t index )
{
	if ( !isReady )
		return;
		
	CheckToCacheBytes( text );	

	// Clamp the length, so text longer than the display count is ignored
	int lengthOfLoop = min( dispCount, (uint8_t)text.length() );

	// Grab the byte (bits) for the segmens for the character passed in
	for ( int s = 0; s < lengthOfLoop; s++ )
	{
		int colorStart = index;
		for ( int i = 0; i < NUM_PIXELS_PER_BOARD; i++ )
		{
			// twirly the colours
			uint32_t color = Wheel( colorStart & 255 );
			colorStart+=(255/28);

			bool on = ( bitRead( cachedBytes[s], i) == 1 );
			pixels.setPixelColor( i + ( s * NUM_PIXELS_PER_BOARD ), on ? color : Color(0,0,0) );

		}
	}
		
	// we have finished setting all of the colors on each segment for this LaserXSegment, so lets turn on the pixels
	pixels.show();
}


void LaserXSegment::DisplayTextColor( String text, uint32_t color )
{
	if ( !isReady )
		return;
		
	CheckToCacheBytes( text );
	
	// Clamp the length, so text longer than the display count is ignored
	int lengthOfLoop = min( dispCount, (uint8_t)text.length() );

	// Grab the byte (bits) for the segmens for the character passed in
	for ( int s = 0; s < lengthOfLoop; s++ )
	{
		for ( int i = 0; i < NUM_PIXELS_PER_BOARD; i++ )
		{
			bool on = ( bitRead( cachedBytes[s], i) == 1 );
			pixels.setPixelColor( i + ( s * NUM_PIXELS_PER_BOARD ), on ? color : Color(0,0,0) );
		}
	}
	// we have finished setting all of the colors on each segment for this LaserXSegment, so lets turn on the pixels
	pixels.show();
}


void LaserXSegment::DisplayTime( uint8_t hours, uint8_t mins, uint8_t secs, uint32_t colorH, uint32_t colorM )
{
	if ( !isReady )
		return;
		
	// String text = PadTimeData( hours ) + PadTimeData( mins );

	// if ( text.length() < dispCount )
	// {
	// 	for ( int i = 0; i < (dispCount - text.length()); i++ )
	// 		text = " " + text;
	// }

	// CheckToCacheBytes( text );

	// uint8_t r = Red( colorM ) * 0.5;
	// uint8_t g = Green( colorM ) * 0.5;
	// uint8_t b = Blue( colorM ) * 0.5;
	// uint32_t colorM2 = Color( r, g, b );
	
	// // Clamp the length, so text longer than the display count is ignored
	// int lengthOfLoop = min( dispCount, (uint8_t)text.length() );

	// // Grab the byte (bits) for the segmens for the character passed in
	// for ( int s = 0; s < lengthOfLoop; s++ )
	// {
	// 	byte code = cachedBytes[s];
		
	// 	if(code > -1)
	// 	{
	// 		uint32_t cachedColor = colorH;

	// 		// displaying mins, so work out new color
	// 		if ( s >= dispCount - 2 )
	// 		{
	// 			cachedColor = ( secs % 2 == 0 ) ? colorM2 : colorM;
	// 		}

	// 		for( int segment = 0; segment < 7; segment++ )
	// 		{
	// 			bool on = ( bitRead( code, segment) == 1 );
	// 			for ( int p = 0; p < 4; p++ )
	// 				pixels.setPixelColor( segmentsPixels[ segment ][ p ] + ( s * NUM_PIXELS_PER_BOARD ), on ? cachedColor : Color(0,0,0) );
	// 		}

	// 		#ifdef USEDP

	// 			bool on = ( bitRead( code, 7) == 1 );
	// 			pixels.setPixelColor( segmentsPixels[ 7 ][ 0 ] + ( s * NUM_PIXELS_PER_BOARD ), on ? cachedColor : Color(0,0,0));

	// 		#endif
	// 	}
	// }
	// // we have finished setting all of the colors on each segment for this LaserXSegment, so lets turn on the pixels
	// pixels.show();
}

void LaserXSegment::DisplayKnightRider( uint8_t index, uint32_t color )
{
	if ( !isReady )
		return;

	// bool isForward = true;

	// uint8_t r = Red( color ) * 0.5;
	// uint8_t g = Green( color ) * 0.5;
	// uint8_t b = Blue( color ) * 0.5;

	// uint32_t colorFade = Color( r, g, b );

	// int pixelIndex = ( index % (dispCount * 8 ) );
	// if ( pixelIndex > (dispCount * 4) - 1 )
	// {
	// 	pixelIndex = ( dispCount * 8 ) - (pixelIndex + 1 );
	// 	isForward = false;
	// }
	
	// // Grab the byte (bits) for the segmens for the character passed in
	// for ( int s = 0; s < dispCount; s++ )
	// {
	// 	for( int segment = 0; segment < 7; segment++ )
	// 	{
	// 		for ( int p = 0; p < 4; p++ )
	// 		{
	// 			if ( segment != 6 )
	// 			{
	// 				pixels.setPixelColor( segmentsPixels[ segment ][ p ] + ( s * NUM_PIXELS_PER_BOARD ), Color(0,0,0) );
	// 			}
	// 			else
	// 			{
	// 				uint8_t currentIndex = ( s * 4 + p );
	// 				if ( currentIndex == pixelIndex )
	// 					pixels.setPixelColor( segmentsPixels[ segment ][ p ] + ( s * NUM_PIXELS_PER_BOARD ), color );
	// 				else if ( isForward && currentIndex == pixelIndex - 1 )
	// 					pixels.setPixelColor( segmentsPixels[ segment ][ p ] + ( s * NUM_PIXELS_PER_BOARD ), colorFade );
	// 				else if ( !isForward && currentIndex == pixelIndex + 1 )
	// 					pixels.setPixelColor( segmentsPixels[ segment ][ p ] + ( s * NUM_PIXELS_PER_BOARD ), colorFade );
	// 				else
	// 					pixels.setPixelColor( segmentsPixels[ segment ][ p ] + ( s * NUM_PIXELS_PER_BOARD ), Color(0,0,0) );
	// 			}
	// 		}		
	// 	}

	// 	#ifdef USEDP

	// 		pixels.setPixelColor( segmentsPixels[ 7 ][ 0 ] + ( s * NUM_PIXELS_PER_BOARD ), Color(0,0,0) );

	// 	#endif
	// }
	// // we have finished setting all of the colors on each segment for this LaserXSegment, so lets turn on the pixels
	// pixels.show();
}

String LaserXSegment::PadTimeData( int8_t data )
{
  if( data < 10 )
	return String("0") + String(data);

  return String(data);
}



// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t LaserXSegment::Wheel(byte WheelPos )
{
	WheelPos = 255 - WheelPos;
	if(WheelPos < 85)
		return Color(255 - WheelPos * 3, 0, WheelPos * 3);

	if(WheelPos < 170)
	{
		WheelPos -= 85;
		return Color(0, WheelPos * 3, 255 - WheelPos * 3);
	}
	
	WheelPos -= 170;
	return Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}

uint8_t LaserXSegment::Red( uint32_t col )
{
	return col >> 16;
}

uint8_t LaserXSegment::Green( uint32_t col )
{
	return col >> 8;
}


uint8_t LaserXSegment::Blue( uint32_t col )
{
	return col;
}
