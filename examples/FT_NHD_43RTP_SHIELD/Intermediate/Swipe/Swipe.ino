/*****************************************************************************
 * Copyright (c) Future Technology Devices International 2014
 * propriety of Future Technology devices International.
 *
 * Software License Agreement
 *
 * This code is provided as an example only and is not guaranteed by FTDI. 
 * FTDI accept no responsibility for any issues resulting from its use. 
 * The developer of the final application incorporating any parts of this 
 * sample project is responsible for ensuring its safe and correct operation 
 * and for any consequences resulting from its use.
 *****************************************************************************/
/**
 * @file		NHD_43RTP_SHIELD_Swipe_Lists.ino
 * @brief		Sketch to demonstrate FT800 graphics and swipe lists
 *				To be used in conjunction with sketch NHD_43RTP_SHIELD_RS232_Slave.ino
 *				Tested platform version: Arduino 1.0.4 and later
 * @version		0.1.0
 * @date		2014/12/19
 *
 */

// Arduino standard includes ...
#include "SPI.h"
#include "Wire.h"

// Platform specific includes ...
#include "FT_NHD_43RTP_SHIELD.h" // FT800

// Uncomment to enable serial debug output ...
//#define SERIAL_ENABLE

// Global object for FT800 implementation ...
FT800IMPL_SPI FTImpl(FT_CS_PIN,FT_PDN_PIN,FT_INT_PIN);

// ================================================================================================
// VARIABLES/ENUMERATIONS [SWIPE LIST DISPLAY AND CONTROL]
// ================================================================================================
// Structure used to hold information about swipe lists (position, size etc.) ...
typedef struct
{
	uint8_t  Layout; // direction of list (SWIPE_VERTICAL, SWIPE_HORIZONTAL, SWIPE_BOTH_WAYS)
	uint8_t  Number; // total number of items
	uint8_t  Select; // currently selected item
	uint8_t  Xentry; // X spacing of items
	uint8_t  Yentry; // Y spacing of items
	int16_t  Xspeed; // X speed of list (due to a swipe)
	int16_t  Yspeed; // Y speed of list (due to a swipe)
	int16_t  Xstart; // current display offset of list
	int16_t  Ystart; // current display offset of list
	int16_t  Xcoord; // active area X coordinate (top left)
	int16_t  Ycoord; // active area Y coordinate (top left)
	int16_t  Xwidth; // active area X width
	int16_t  Ywidth; // active area Y width
	uint8_t  Xitems; // number of items to display horizontally
	uint8_t  Yitems; // number of items to display vertically
	uint8_t  Limits; // full wrap around or stop at limits
	uint8_t  Active; // Whether or not list is active
} swipeList_t;

// Swipe list display function prototypes ...
typedef void (* openptr) (void);
typedef	void (* itemptr) (int16_t, int16_t, uint16_t, uint16_t, uint8_t, uint8_t);
typedef void (* shutptr) (void);

// Swipe list display functions structure (user supplied functions used to render swipe lists)...
typedef struct
{
	openptr openFunc; // performs any initial operations before item rendering
	itemptr itemFunc; // performs rendering of an item at supplied position
	shutptr shutFunc; // performs any final operations after item rendering
} swipeDraw_t;

// list of user defined swipe lists ...
typedef enum
{
	SWIPE_LIST_V_1C_L = 0, // vertical,   1 column, limit
	SWIPE_LIST_V_1C_W,     // vertical,   1 column, wraps
	SWIPE_LIST_V_2C_L,     // vertical,   2 column, limit
	SWIPE_LIST_V_3C_W,     // vertical,   3 column, wraps
	SWIPE_LIST_H_1C_L,     // horizontal, 1 row,    limit
	SWIPE_LIST_H_1C_W,     // horizontal, 1 row,    wraps
	SWIPE_LIST_H_2C_L,     // horizontal, 2 row,    limit
	SWIPE_LIST_H_3C_W,     // horizontal, 3 row,    wraps
	SWIPE_LIST_B_4C_L,     // both ways,  4 column, limit
	SWIPE_LIST_B_4C_W,     // both ways,  4 column, wraps
	SWIPE_LIST_TOTAL
} swipeListUser_t;

// Swipe list directions and limits ...
#define SWIPE_BOTH_WAYS  0x00
#define SWIPE_VERTICAL   0x01
#define SWIPE_HORIZONTAL 0x02
#define SWIPE_LIMIT      0x00
#define SWIPE_WRAPS      0x01

// Swipe list bits ...
#define SWIPE_NULL_TAG     255
#define SWIPE_REFRESH_RATE  20
#define SWIPE_DRAG_PIXELS    3
#define SWIPE_TOUCH_DRAG     0
#define SWIPE_TOUCH_SELECT   1

// Swipe list item info (used for all lists in this code) ...
#define ITEM_TOTAL      50
#define SWIPE_ITEM_XSIZ 100
#define SWIPE_ITEM_YSIZ  50

swipeList_t swipeItemList[SWIPE_LIST_TOTAL];
swipeDraw_t swipeItemDraw[SWIPE_LIST_TOTAL];

// Swipe lists - holds value of REG_TOUCH_DIRECT_XY register ...
uint32_t touchRegsValueXY;

// Swipe lists - tags being checked and tracked ...
uint8_t swipeCheckingTag = SWIPE_NULL_TAG;
uint8_t swipeTrackingTag = SWIPE_NULL_TAG;

// Swipe lists - coordinates of initial touch point ...
int16_t swipeInitialXcrd;
int16_t swipeInitialYcrd;

// Swipe lists - coordinates of current touch point ...
int16_t swipeCurrentXcrd;
int16_t swipeCurrentYcrd;

// Swipe lists - coordinates of previous touch point ...
int16_t touchEarlierXcrd;
int16_t touchEarlierYcrd;

// Swipe lists - difference between initial and current touch points ...
int16_t swipeDiffersXcrd = 0;
int16_t swipeDiffersYcrd = 0;

// Swipe lists - whether a drag or selection ...
uint8_t swipeTypeOfTouch = SWIPE_TOUCH_SELECT;

// ================================================================================================
// VARIABLES/ENUMERATIONS [VARIOUS]
// ================================================================================================
// Used to decrement 'msTickTock...' counters at a millisecond(ish) rate ...
uint32_t msTimeCountWas;
uint32_t msTimeCountNow;

// Used to update display and track touches at the millisecond rate specified ...
#define DISPLAY_REFRESH_RATE 20
uint16_t msTickTockDisplay = DISPLAY_REFRESH_RATE; 

// Used to animate item selected in a swipe list (glowing rectangle) ...
uint8_t  animateSelect = 0;
int8_t   stepperSelect = 1;
uint8_t  stepperCycleR;
uint8_t  stepperCycleG;
uint8_t  stepperCycleB;

// Various ...
uint8_t  swipeListShow = 0;
#define BUTTON_PREV 1
#define BUTTON_NEXT 2
#define OPT_CENTREX 512

char * swipeInfoText[SWIPE_LIST_TOTAL] =
{
	"Vertical, 1 column wide, stop at list boundries",
	"Vertical, 1 column wide, wraparound at list boundries",
	"Vertical, 2 columns wide, stop at list boundries",
	"Vertical, 3 columns wide, wraparound at list boundries",
	"Horizontal, 1 row high, stop at list boundries",
	"Horizontal, 1 row high, wraparound at list boundries",
	"Horizontal, 2 rows high, stop at list boundries",
	"Horizontal, 3 rows high, wraparound at list boundries",
	"Both ways, 4 columns wide, stop at list boundries",
	"Both ways, 4 columns wide, wraparound at list boundries"
};
char * swipeItemText[ITEM_TOTAL] =
{
	"Item 01",	"Item 02",	"Item 03",	"Item 04",	"Item 05",
	"Item 06",	"Item 07",	"Item 08",	"Item 09",	"Item 10",
	"Item 11",	"Item 12",	"Item 13",	"Item 14",	"Item 15",
	"Item 16",	"Item 17",	"Item 18",	"Item 19",	"Item 20",
	"Item 21",	"Item 22",	"Item 23",	"Item 24",	"Item 25",
	"Item 26",	"Item 27",	"Item 28",	"Item 29",	"Item 30",
	"Item 31",	"Item 32",	"Item 33",	"Item 34",	"Item 35",
	"Item 36",	"Item 37",	"Item 38",	"Item 39",	"Item 40",
	"Item 41",	"Item 42",	"Item 43",	"Item 44",	"Item 45",
	"Item 46",	"Item 47",	"Item 48",	"Item 49",	"Item 50"
};

// ################################################################################################
// # BEGINNING OF CODE
// ################################################################################################

// ================================================================================================
// Timer routine
// ================================================================================================
void milliTimer()
{
	msTimeCountWas = msTimeCountNow;
	msTimeCountNow = millis();

	// update count downs if a millisecond (or so) has passed ...
	if (msTimeCountNow != msTimeCountWas)
	{
		if (msTickTockDisplay)
		{
			msTickTockDisplay--;
			
			// Update display if count down reaches 0 ...
			if (msTickTockDisplay == 0)
			{
				// reset timer ...
				msTickTockDisplay = DISPLAY_REFRESH_RATE;
				// update display ...
				renderDisplay();
			}
		}
	}
}

// ================================================================================================
// Swipe list core routines ...
// ================================================================================================
// ------------------------------------------------------------------------------------------------
// Swipe list setup (offsets, speeds and item selected all default to 0) ...
// ------------------------------------------------------------------------------------------------
void swipeSetupList(uint8_t list, uint8_t type, uint8_t line, uint8_t item, uint8_t xgrd, uint8_t ygrd, int16_t xcrd, int16_t ycrd, int16_t xsiz, int16_t ysiz, uint8_t wrap)
{
	swipeItemList[list].Layout = type;
	swipeItemList[list].Number = item;
	swipeItemList[list].Select = 0;
	swipeItemList[list].Xentry = xgrd;
	swipeItemList[list].Yentry = ygrd;
	swipeItemList[list].Xspeed = 0;
	swipeItemList[list].Yspeed = 0;
	swipeItemList[list].Xstart = 0;
	swipeItemList[list].Ystart = 0;
	swipeItemList[list].Xcoord = xcrd;
	swipeItemList[list].Ycoord = ycrd;
	swipeItemList[list].Xwidth = xsiz;
	swipeItemList[list].Ywidth = ysiz;
	swipeItemList[list].Limits = wrap;
	swipeItemList[list].Active = true;

	// 'line' specifies either the number of columns or rows depending on the layout chosen...
	switch (type)
	{
		case SWIPE_BOTH_WAYS:  // 'line' = column total, rows calculated, can move any direction
		case SWIPE_VERTICAL:   // 'line' = column total, rows calculated, can only move vertically
			swipeItemList[list].Xitems = line;
			swipeItemList[list].Yitems = (item + line - 1) / line;
			break;
		case SWIPE_HORIZONTAL: // 'line' = row total, columns calculated, can only move horizontally
			swipeItemList[list].Xitems = (item + line - 1) / line;
			swipeItemList[list].Yitems = line;
			break;
	}
}

// ------------------------------------------------------------------------------------------------
// Swipe list offset limiting routines (ensures X/Y offsets stay within relevant limits)
// ------------------------------------------------------------------------------------------------
int16_t swipeLimitX(int16_t xoff, uint8_t xtag)
{
	int16_t xmax = swipeItemList[xtag].Xitems * swipeItemList[xtag].Xentry;

	// Determine limits according to limit type ...
	if (swipeItemList[xtag].Limits == SWIPE_WRAPS)
	{
		// Ensure offset wraps around ...
		xoff = (xoff + xmax) % xmax;
	}
	else
	{
		// Ensure offset is kept within limits ...
		xmax =  xmax - swipeItemList[xtag].Xwidth;
		xoff = (xoff < 0)? 0 : (xoff > xmax)? xmax : xoff;
	}

	return xoff;
}

int16_t swipeLimitY(int16_t yoff, uint8_t ytag)
{
	int16_t ymax = swipeItemList[ytag].Yitems * swipeItemList[ytag].Yentry;

	// Determine limits according to limit type ...
	if (swipeItemList[ytag].Limits == SWIPE_WRAPS)
	{
		// Ensure offset wraps around ...
		yoff = (yoff + ymax) % ymax;
	}
	else
	{
		// Ensure offset is kept within limits ...
		ymax -= swipeItemList[ytag].Ywidth;
		yoff  = (yoff < 0)? 0 : (yoff > ymax)? ymax : yoff;
	}

	return yoff;
}

// ------------------------------------------------------------------------------------------------
// Swipe list touch handling routine (handles user selection and dragging actions)
// ------------------------------------------------------------------------------------------------
void swipeTouchHandler()
{
	static int16_t swipePrecedeXcrd;
	static int16_t swipePrecedeYcrd;
	int16_t xmin, ymin;
	int16_t xmax, ymax;
	int16_t xsiz, ysiz;
	int16_t xoff, yoff;

	uint8_t tag;
	uint8_t item;
	uint8_t xitm, yitm;

	// Get current touch coordinates and time
	touchRegsValueXY = FTImpl.Read32(REG_TOUCH_DIRECT_XY);
	swipeCheckingTag = SWIPE_NULL_TAG;

	// Determine swipe area being touched (if any) and set tag appropriately ...
	if ( (touchRegsValueXY & 0x80000000) == 0)
	{
		// Record preceding touch coordinates (used to calculate direction and speed of swipe) ...
		swipePrecedeXcrd = swipeCurrentXcrd;
		swipePrecedeYcrd = swipeCurrentYcrd;

		// Extract current touch coordinates (need to invert touch Y to match screen Y) ...
		swipeCurrentXcrd =      (((touchRegsValueXY >> 16) & 0x3FF) * 0.468750); // Scale to the range 0-479
		swipeCurrentYcrd = 271 -(((touchRegsValueXY      ) & 0x3FF) * 0.265625); // Scale to the range 0-271

		// Check to see if touch has occurred in any of the swipe areas ...
		for (tag = 0; tag < SWIPE_LIST_TOTAL; tag++)
		{
			if (swipeItemList[tag].Active == true)
			{
				xmin = swipeItemList[tag].Xcoord;
				ymin = swipeItemList[tag].Ycoord;
				xmax = swipeItemList[tag].Xwidth + xmin;
				ymax = swipeItemList[tag].Ywidth + ymin;
				
				// Once a swipe area is being tracked it will continue to be tracked until the touch point
				// moves outside of the touch area or the touch finishes, even if swipe areas overlap and
				// are listed earlier in the swipe areas to be checked.
				if ( (swipeCurrentXcrd >= xmin) && (swipeCurrentXcrd <= xmax) && (swipeCurrentYcrd >= ymin) && (swipeCurrentYcrd <= ymax) )
				{
					// Variable will be set to track the first swipe area which the touch position is bounded by, but will
					// be ignored below if we are already tracking another swipe area.
					swipeCheckingTag = tag;
					break;
				}
			}
		}
	}

	// Determine whether to start, continue or finish tracking ...
	if ((swipeCheckingTag == SWIPE_NULL_TAG) && (swipeTrackingTag != SWIPE_NULL_TAG))
	{
		// FINISH TRACKING ...
		xsiz = swipeItemList[swipeTrackingTag].Xentry;
		ysiz = swipeItemList[swipeTrackingTag].Yentry;
		xmax = swipeItemList[swipeTrackingTag].Xitems * xsiz;
		ymax = swipeItemList[swipeTrackingTag].Yitems * ysiz;

		// Perform appropriate action depending on whether a select or drag action ..
		if (swipeTypeOfTouch == SWIPE_TOUCH_SELECT)
		{
			// Determine coordinate of point touched with respect to current offsets ...
			xoff = swipeCurrentXcrd - swipeItemList[swipeTrackingTag].Xcoord;
			yoff = swipeCurrentYcrd - swipeItemList[swipeTrackingTag].Ycoord;

			// Determine item being selected ...
			xitm = ((swipeItemList[swipeTrackingTag].Xstart + xoff) % xmax) / xsiz;
			yitm = ((swipeItemList[swipeTrackingTag].Ystart + yoff) % ymax) / ysiz;
			item = (swipeItemList[swipeTrackingTag].Xitems * yitm) + xitm;

			// Only update selected item if valid (multi column/row lists may not be fully populated) ...
			if (item < swipeItemList[swipeTrackingTag].Number)
			{
				swipeItemList[swipeTrackingTag].Select = item;
			}
		}
		else
		{
			// Update start positions of list display and keep within range ...
			swipeItemList[swipeTrackingTag].Xstart = swipeLimitX(swipeItemList[swipeTrackingTag].Xstart + swipeDiffersXcrd, swipeTrackingTag);
			swipeItemList[swipeTrackingTag].Ystart = swipeLimitY(swipeItemList[swipeTrackingTag].Ystart + swipeDiffersYcrd, swipeTrackingTag);
			// Determine speed/direction of movement ...
			swipeItemList[swipeTrackingTag].Xspeed = (swipeItemList[swipeTrackingTag].Layout == SWIPE_VERTICAL)?   0 : swipePrecedeXcrd - swipeCurrentXcrd;
			swipeItemList[swipeTrackingTag].Yspeed = (swipeItemList[swipeTrackingTag].Layout == SWIPE_HORIZONTAL)? 0 : swipePrecedeYcrd - swipeCurrentYcrd;
		}

		// Reset to initial values ...
		swipeTrackingTag = SWIPE_NULL_TAG;
		swipeTypeOfTouch = SWIPE_TOUCH_SELECT;
		swipeDiffersXcrd = 0;
		swipeDiffersYcrd = 0;
	}
	else if ((swipeCheckingTag != SWIPE_NULL_TAG) && (swipeTrackingTag == SWIPE_NULL_TAG))
	{
		// START TRACKING ...
		// Make note of tag to track and coordinates ...
		swipeTrackingTag = swipeCheckingTag;
		swipeInitialXcrd = swipeCurrentXcrd;
		swipeInitialYcrd = swipeCurrentYcrd;

		// Initialise speed/direction for tag area touched...
		swipeItemList[swipeTrackingTag].Xspeed = 0;
		swipeItemList[swipeTrackingTag].Yspeed = 0;

		// Initialise last two touch coordinates (used to calculate speed/direction of swipe) ...
		swipePrecedeXcrd = swipeCurrentXcrd;
		swipePrecedeYcrd = swipeCurrentYcrd;
	}
	else if ((swipeCheckingTag != SWIPE_NULL_TAG) && (swipeTrackingTag != SWIPE_NULL_TAG))
	{
		// CONTINUE TRACKING ...
		// Determine difference between initial and current touch positions ...
		swipeDiffersXcrd = (swipeItemList[swipeTrackingTag].Layout == SWIPE_VERTICAL)?   0 : swipeInitialXcrd - swipeCurrentXcrd;
		swipeDiffersYcrd = (swipeItemList[swipeTrackingTag].Layout == SWIPE_HORIZONTAL)? 0 : swipeInitialYcrd - swipeCurrentYcrd;

		// Only becomes a drag touch if difference between initial and current touch
		// positions exceeds the specified number of pixels ...
		if ((abs(swipeDiffersXcrd) > SWIPE_DRAG_PIXELS) || (abs(swipeDiffersYcrd) > SWIPE_DRAG_PIXELS))
		{
			// Change to a drag touch and track movement ...
			swipeTypeOfTouch = SWIPE_TOUCH_DRAG;
		}
	}
}

// ------------------------------------------------------------------------------------------------
// Swipe list display generation routine (uses user supplied functions to do actual drawing)
// ------------------------------------------------------------------------------------------------
void swipeDisplayHandler()
{
	int16_t xcrd, ycrd; // Coordinates to display item
	int16_t xsiz, ysiz; // Size of items to be displayed
	int16_t xoff, yoff; // Coordinates of display offset
	int16_t xtmp;
	uint8_t tag;        // List being displayed
	uint8_t xitm, yitm; // Used to determine item at specific coordinates
	uint8_t item, itmp;

	// Update item selection animation ...
	animateSelect += stepperSelect;
	if ((animateSelect == 0) || (animateSelect == 15))
	{
		stepperSelect = -stepperSelect;
	}

	// Render all active swipe lists ...
	for (tag = 0; tag < SWIPE_LIST_TOTAL; tag++)
	{
		if (swipeItemList[tag].Active == true)
		{
			xoff = swipeItemList[tag].Xstart;
			yoff = swipeItemList[tag].Ystart;
			xsiz = swipeItemList[tag].Xentry;
			ysiz = swipeItemList[tag].Yentry;

			// Alter display position if touch is being tracked for this swipe list and list is being dragged ... 
			if ((swipeTrackingTag == tag) && (swipeTypeOfTouch == SWIPE_TOUCH_DRAG))
			{
				xoff = swipeLimitX(xoff + swipeDiffersXcrd, tag);
				yoff = swipeLimitY(yoff + swipeDiffersYcrd, tag);
			}

			// Determine position of first item to be rendered ...
			xcrd = swipeItemList[tag].Xcoord - (xoff % xsiz);
			ycrd = swipeItemList[tag].Ycoord - (yoff % ysiz);
			xtmp = xcrd;

			// Determine first item to be rendered ...
			xitm = (xoff / xsiz);
			yitm = (yoff / ysiz);
			itmp = xitm;
			
			// Save graphics context ...
			FTImpl.SaveContext();
			
			// Set clip rectangle to swipe list display area ...
			FTImpl.ScissorXY(  swipeItemList[tag].Xcoord, swipeItemList[tag].Ycoord);
			FTImpl.ScissorSize(swipeItemList[tag].Xwidth, swipeItemList[tag].Ywidth);
			
			// Perform any item specific operation required before rendering items (user supplied routine) ...
			swipeItemDraw[tag].openFunc();

			// Render items, automatically determining how many are required ...
			while (ycrd < (swipeItemList[tag].Ycoord + swipeItemList[tag].Ywidth))
			{
				while (xcrd < (swipeItemList[tag].Xcoord + swipeItemList[tag].Xwidth))
				{
					// Determine next item to be displayed ...
					item = (yitm * swipeItemList[tag].Xitems) + xitm;

					// Only display if a valid item ...
					if (item < swipeItemList[tag].Number)
					{
						// Display item at coordinates specified (user supplied routine) ...
						swipeItemDraw[tag].itemFunc(xcrd, ycrd, xsiz, ysiz, item, swipeItemList[tag].Select);
					}

					// Update position and item to be displayed (X direction) ...
					xcrd += xsiz;
					xitm  = (xitm + 1) % swipeItemList[tag].Xitems;
				}

				// Update position and item to be displayed (Y direction) ...
				ycrd += ysiz;
				yitm  = (yitm + 1) % swipeItemList[tag].Yitems;
				xcrd  = xtmp;
				xitm  = itmp;			
			}

			// Perform any item specific operation required after rendering items (user supplied routine) ...
			swipeItemDraw[tag].shutFunc();

			// Restore graphics context ...
			FTImpl.RestoreContext();

			// Reduce X/Y speed ...
			swipeItemList[tag].Xspeed += (swipeItemList[tag].Xspeed < 0)? 1 : (swipeItemList[tag].Xspeed > 0)? -1 : 0;
			swipeItemList[tag].Yspeed += (swipeItemList[tag].Yspeed < 0)? 1 : (swipeItemList[tag].Yspeed > 0)? -1 : 0;

			// Update X/Y offsets (keeping within limits) ...
			swipeItemList[tag].Xstart  = swipeLimitX(swipeItemList[tag].Xstart + swipeItemList[tag].Xspeed, tag);
			swipeItemList[tag].Ystart  = swipeLimitY(swipeItemList[tag].Ystart + swipeItemList[tag].Yspeed, tag);
		}
	}
}

// ================================================================================================
// Swipe list display rendering routines for the IMAGE list
// ================================================================================================
void swipeRenderOpen_Demo()
{
	// nothing to do!
}

void swipeRenderItem_Demo(int16_t xcrd, int16_t ycrd, uint16_t xsiz, uint16_t ysiz, uint8_t item, uint8_t pick)
{
	// Highlight item if currently selected ...
	if (item == pick)
	{
		// Open section ...
		FTImpl.Begin(FT_RECTS);
		// Draw rectangle to indicate item is selected ...
		FTImpl.LineWidth(8 * 16);
		FTImpl.ColorRGB(stepperCycleR, stepperCycleG, stepperCycleB);
		FTImpl.Vertex2f((xcrd + 9       ) * 16, (ycrd + 9       ) * 16); // Top left
		FTImpl.Vertex2f((xcrd + xsiz - 9) * 16, (ycrd + ysiz - 9) * 16); // Bottom right
		FTImpl.ColorRGB(0xFF, 0xFF, 0xFF);
		// Shut section ...
		FTImpl.End();
	}

	// Draw button showing item text ...
	FTImpl.Cmd_Button(xcrd + 6, ycrd + 6, xsiz - 13, ysiz - 13, 27, 0, swipeItemText[item]);
}

void swipeRenderShut_Demo()
{
	// nothing to do!
}

// ================================================================================================
// Routine to generate display list
// ================================================================================================
void renderDisplay()
{
	int16_t x1 = swipeItemList[swipeListShow].Xcoord;
	int16_t y1 = swipeItemList[swipeListShow].Ycoord;
	int16_t x2 = swipeItemList[swipeListShow].Xwidth + x1;
	int16_t y2 = swipeItemList[swipeListShow].Ywidth + y1;

	// ------------------------------------------------------------------------
	// Track all touches on swipe lists ...
	// ------------------------------------------------------------------------
	swipeTouchHandler();
	
	// ------------------------------------------------------------------------
	// Backdrop for image transfer display (max image size of 200 x 150) ...
	// ------------------------------------------------------------------------
	FTImpl.DLStart(); // Note DLStart and DLEnd are helper apis, Cmd_DLStart() and Display() can also be utilized.

	// Draw entire background using a couple of gradient fills ...
	FTImpl.ScissorXY(     0,   0);
	FTImpl.ScissorSize( 480, 136);
	FTImpl.Cmd_Gradient(  0,   0, 0x404040,   0, 136, 0xFFFFFF);
	FTImpl.ScissorXY(     0, 136);
	FTImpl.ScissorSize( 480, 136);
	FTImpl.Cmd_Gradient(  0, 136, 0xFFFFFF,   0, 272, 0x404040);

	// Reset scissor rectangle to full screen ...
	FTImpl.ScissorXY(     0,   0);
	FTImpl.ScissorSize( 480, 272);

	// Open section ...
	FTImpl.Begin(FT_RECTS);
	FTImpl.ColorA(128);
	FTImpl.LineWidth(4 * 16);

	// Set colour of buttons and item selected colour according to swipe list type ...
	switch (swipeListShow)
	{
		case SWIPE_LIST_V_1C_L: // yellow (R+G)
		case SWIPE_LIST_V_1C_W:
		case SWIPE_LIST_V_2C_L:
		case SWIPE_LIST_V_3C_W:
			FTImpl.Cmd_FGColor(0x404000);
			FTImpl.Cmd_GradColor(0xFFFF00);
			FTImpl.ColorRGB(0x40, 0x40, 0x00);
			stepperCycleR = 0xFF - (animateSelect * 8);
			stepperCycleG = 0xD7 - (animateSelect * 8);
			stepperCycleB = 0x00;
			break;

		case SWIPE_LIST_H_1C_L: // magenta (R+B)
		case SWIPE_LIST_H_1C_W:
		case SWIPE_LIST_H_2C_L:
		case SWIPE_LIST_H_3C_W:
			FTImpl.Cmd_FGColor(0x400040);
			FTImpl.Cmd_GradColor(0xFF00FF);
			FTImpl.ColorRGB(0x40, 0x00, 0x40);
			stepperCycleR = 0xFF - (animateSelect * 8);
			stepperCycleG = 0x00;
			stepperCycleB = 0xD7 - (animateSelect * 8);
			break;

		case SWIPE_LIST_B_4C_L: // cyan (G+B)
		case SWIPE_LIST_B_4C_W:
			FTImpl.Cmd_FGColor(0x004040);
			FTImpl.Cmd_GradColor(0x00FFFF);
			FTImpl.ColorRGB(0x00, 0x40, 0x40);
			stepperCycleR = 0x00;
			stepperCycleG = 0xFF - (animateSelect * 8);
			stepperCycleB = 0xD7 - (animateSelect * 8);
			break;
	}

	// Semi-transparent rectangle for background of swipe list (set to display area of list) ...
	FTImpl.Vertex2f(x1 * 16, y1 * 16);
	FTImpl.Vertex2f(x2 * 16, y2 * 16);

	// Semi-transparent rectangle for background of text display (bottom of display) ...
	FTImpl.ColorRGB(0x00, 0x00, 0x30);
	FTImpl.Vertex2f( 58 * 16, 242 * 16);
	FTImpl.Vertex2f(421 * 16, 262 * 16);

	// Shut section ...
	FTImpl.ColorRGB(0xFF, 0xFF, 0xFF);
	FTImpl.End();

	FTImpl.Cmd_Text(240, 244, 26, OPT_CENTREX, swipeInfoText[swipeListShow]);
	FTImpl.ColorA(255);

	// ------------------------------------------------------------------------
	// Render all active swipe lists ...
	// ------------------------------------------------------------------------
	swipeDisplayHandler();

	// ------------------------------------------------------------------------
	// Add a couple of buttons to step through the available swipe lists
	// ------------------------------------------------------------------------
	FTImpl.ColorA(128);
	FTImpl.Cmd_FGColor(0x000030);
	FTImpl.Cmd_GradColor(0x202020);
	FTImpl.Tag(BUTTON_PREV);
	FTImpl.Cmd_Button(  4,235,44,31,30, 0, "<");
	FTImpl.Tag(BUTTON_NEXT);
	FTImpl.Cmd_Button(430,235,44,31,30, 0, ">");
	FTImpl.ColorA(255);

	// ------------------------------------------------------------------------
	// End of display list
	// ------------------------------------------------------------------------
	FTImpl.DLEnd();  // End the display list
	FTImpl.Finish(); // Render the display list and wait for the completion of the DL
}

// ================================================================================================
// Routine to boot up FT800 module, verify hardware and configure display/audio pins
//(returns 0 in case of success and 1 in case of failure)
// ================================================================================================
int16_t BootupConfigure()
{
	uint32_t chipid = 0;
	// Configure the display to the WQVGA ...
	FTImpl.Init(FT_DISPLAY_RESOLUTION);
	
	delay(20);
	chipid = FTImpl.Read32(FT_ROM_CHIPID);

	// Identify the chip ...
	if(FT800_CHIPID != chipid)
	{
		#ifdef SERIAL_ENABLE
		Serial.print("Error in chip id read ");
		Serial.println(chipid,HEX);
		#endif
		return 1;
	}

	// Set the Display & audio pins
	FTImpl.SetDisplayEnablePin(FT_DISPENABLE_PIN);
	FTImpl.SetAudioEnablePin(FT_AUDIOENABLE_PIN); 
	FTImpl.DisplayOn(); 
	FTImpl.AudioOn();  			
	return 0;
}

// ================================================================================================
// Routine to calibrate touch screen
// ================================================================================================
void Calibrate()
{  
	// ------------------------------------------------------------------------
	// Below code demonstrates the usage of calibrate function.
	// - The 'Calibrate' function waits until the user presses all three dots.
	// - Only way to exit this api is to reset the coprocessor bit.
	// ------------------------------------------------------------------------

	// Construct the display list with grey as background color, display informative
	// string "Please Tap on the dot" followed by inbuilt calibration command ...
	FTImpl.DLStart();
	FTImpl.ClearColorRGB(64,64,64);
	FTImpl.Clear(1,1,1);    
	FTImpl.ColorRGB(0xff, 0xff, 0xff);
	FTImpl.Cmd_Text((FT_DISPLAYWIDTH/2), (FT_DISPLAYHEIGHT/2), 27, FT_OPT_CENTER, "Please Tap on the dot");
	FTImpl.Cmd_Calibrate(0);
	
	// Wait for the completion of calibration(either finish can be used for flush and check can be used)
	FTImpl.Finish();
}

// ================================================================================================
// Routine to boot up the module and render display
// ================================================================================================
void setup()
{
	uint8_t swipeTag;
	uint8_t touchPrev = 0;
	uint8_t touchCurr = 0;
	
	// Initialize serial print related functionality
	#ifdef SERIAL_ENABLE
	Serial.begin(9600);
	Serial.println("-- Start Application --");	
	#endif

	if (BootupConfigure())
	{
		// Error case - do not do any thing
		#ifdef SERIAL_ENABLE
		Serial.println("Error in: BootupConfigure()");
		#endif
	}
	else
	{
		// Calibrate touch screen ...
		Calibrate();

		// set up swipe lists (movement = SWIPE_BOTH_WAYS, SWIPE_VERTICAL or SWIPE_HORIZONTAL) ...
		swipeSetupList(SWIPE_LIST_V_1C_L, SWIPE_VERTICAL,   1, ITEM_TOTAL, SWIPE_ITEM_XSIZ, SWIPE_ITEM_YSIZ, 190,   8, SWIPE_ITEM_XSIZ * 1, 220, SWIPE_LIMIT);
		swipeSetupList(SWIPE_LIST_V_1C_W, SWIPE_VERTICAL,   1, ITEM_TOTAL, SWIPE_ITEM_XSIZ, SWIPE_ITEM_YSIZ, 190,   8, SWIPE_ITEM_XSIZ * 1, 220, SWIPE_WRAPS);
		swipeSetupList(SWIPE_LIST_V_2C_L, SWIPE_VERTICAL,   2, ITEM_TOTAL, SWIPE_ITEM_XSIZ, SWIPE_ITEM_YSIZ, 140,   8, SWIPE_ITEM_XSIZ * 2, 220, SWIPE_LIMIT);
		swipeSetupList(SWIPE_LIST_V_3C_W, SWIPE_VERTICAL,   3, ITEM_TOTAL, SWIPE_ITEM_XSIZ, SWIPE_ITEM_YSIZ,  90,   8, SWIPE_ITEM_XSIZ * 3, 220, SWIPE_WRAPS);
		swipeSetupList(SWIPE_LIST_H_1C_L, SWIPE_HORIZONTAL, 1, ITEM_TOTAL, SWIPE_ITEM_XSIZ, SWIPE_ITEM_YSIZ,   8,  85, 464, SWIPE_ITEM_YSIZ * 1, SWIPE_LIMIT);
		swipeSetupList(SWIPE_LIST_H_1C_W, SWIPE_HORIZONTAL, 1, ITEM_TOTAL, SWIPE_ITEM_XSIZ, SWIPE_ITEM_YSIZ,   8,  85, 464, SWIPE_ITEM_YSIZ * 1, SWIPE_WRAPS);
		swipeSetupList(SWIPE_LIST_H_2C_L, SWIPE_HORIZONTAL, 2, ITEM_TOTAL, SWIPE_ITEM_XSIZ, SWIPE_ITEM_YSIZ,   8,  60, 464, SWIPE_ITEM_YSIZ * 2, SWIPE_LIMIT);
		swipeSetupList(SWIPE_LIST_H_3C_W, SWIPE_HORIZONTAL, 3, ITEM_TOTAL, SWIPE_ITEM_XSIZ, SWIPE_ITEM_YSIZ,   8,  35, 464, SWIPE_ITEM_YSIZ * 3, SWIPE_WRAPS);
		swipeSetupList(SWIPE_LIST_B_4C_L, SWIPE_BOTH_WAYS,  4, ITEM_TOTAL, SWIPE_ITEM_XSIZ, SWIPE_ITEM_YSIZ,  90,   8, SWIPE_ITEM_XSIZ * 3, 220, SWIPE_LIMIT);
		swipeSetupList(SWIPE_LIST_B_4C_W, SWIPE_BOTH_WAYS,  4, ITEM_TOTAL, SWIPE_ITEM_XSIZ, SWIPE_ITEM_YSIZ,  90,   8, SWIPE_ITEM_XSIZ * 3, 220, SWIPE_WRAPS);

		// set up swipe list rendering functions and set to inactive ...
		for (swipeTag = 0; swipeTag < SWIPE_LIST_TOTAL; swipeTag++)
		{
			swipeItemDraw[swipeTag].openFunc = &swipeRenderOpen_Demo;
			swipeItemDraw[swipeTag].itemFunc = &swipeRenderItem_Demo;
			swipeItemDraw[swipeTag].shutFunc = &swipeRenderShut_Demo;
			swipeItemList[swipeTag].Active   = false;
		}
		
		swipeListShow = SWIPE_LIST_V_1C_L;
		swipeItemList[swipeListShow].Active = true;

		while (1)
		{
			// Check countdown timers and perform defined operations if required ...
			milliTimer();

			touchPrev = touchCurr;
			touchCurr = FTImpl.Read(REG_TOUCH_TAG);

			// Ensures button must be released before being pressed again ...
			if (touchPrev != touchCurr)
			{
				if (touchCurr == BUTTON_PREV)
				{
					// Deactivate current swipe list ...
					swipeItemList[swipeListShow].Active = false;
					// Select the next swipe list ...
					swipeListShow = (swipeListShow + SWIPE_LIST_TOTAL - 1) % SWIPE_LIST_TOTAL;
					// Activate next swipe list ...
					swipeItemList[swipeListShow].Active = true;
				}
				if (touchCurr == BUTTON_NEXT)
				{
					// Deactivate current swipe list ...
					swipeItemList[swipeListShow].Active = false;
					// Select the next swipe list ...
					swipeListShow = (swipeListShow + SWIPE_LIST_TOTAL + 1) % SWIPE_LIST_TOTAL;
					// Activate next swipe list ...
					swipeItemList[swipeListShow].Active = true;
				}
			}
		}
	}

	FTImpl.Exit();
	#ifdef SERIAL_ENABLE
	Serial.println("-- End Application --");
	#endif
}


void loop()
{
	// nothing in loop
}

// ################################################################################################
// END OF CODE
// ################################################################################################

