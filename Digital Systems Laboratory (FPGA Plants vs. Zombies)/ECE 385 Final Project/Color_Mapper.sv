//-------------------------------------------------------------------------
//    Color_Mapper.sv                                                    --
//    Stephen Kempf                                                      --
//    3-1-06                                                             --
//                                                                       --
//    Modified by David Kesler  07-16-2008                               --
//    Translated by Joe Meng    07-07-2013                               --
//                                                                       --
//    Fall 2014 Distribution                                             --
//                                                                       --
//    For use with ECE 385 Lab 7                                         --
//    University of Illinois ECE Department                              --
//-------------------------------------------------------------------------

module  color_mapper ( input Reset, blank, frame_clk, vga_clk, select, spacebar, delete, enter, keyWasPressed,
							  input [1:0] inMenu,
							  input [9:0] CursorX, CursorY, DrawX, DrawY,
                       output logic [7:0]  Red, Green, Blue);
							  
  	logic [7:0] BG_Red, BG_Green, BG_Blue;
	logic [7:0] FG_Red, FG_Green, FG_Blue;
	logic [7:0] cursor_Red, cursor_Green, cursor_Blue;
	
	logic [10:0] Sun_Count;
	int sunCounter;
	logic [6:0] zombieMoveCounter;
	
	logic [3:0] hundredsPlace;
	logic [3:0] tensPlace;
	logic [3:0] onesPlace;
	
	logic [3:0] number;
	logic [3:0] offset;
	
	logic [2:0] curPlantType;
	logic [2:0] curPlantSprite;
	logic curPlantHasSun;
	logic peaIsHere;
	logic zombieIsHere;
	logic [3:0] curZombieXPos;
	logic [3:0] curZombieYPos;
	
	logic [1:0] plantToPurchase;
	logic purchasing;
	
	logic [10:0] PS_cooldown;
	logic [10:0] SF_cooldown;
	logic [10:0] WN_cooldown;
	
	parameter [10:0] PS_cooldownMax = 700; // CHANGE BACK TO 700
	parameter [10:0] SF_cooldownMax = 700; // CHANGE BACK TO 700
	parameter [10:0] WN_cooldownMax = 1400;
	
	parameter [9:0] aboveLawn = 127;//32*4-1;
	parameter [9:0] leftOfLawn = 31;//8*4-1;
	parameter [9:0] rightOfLawn = 152*4;
	parameter [9:0] belowLawn = 112*4;
	parameter [9:0] aboveFence = 23*4-1;
	
	parameter [9:0] DrawXMax = 639;
	parameter [9:0] DrawYMax = 479;
	
	parameter [9:0] textXmin = 3*4;
	parameter [9:0] textXmax = 20*4-1;
	parameter [9:0] textYmin = 3*4;
	parameter [9:0] textYmax = 16*4-1;
	
	parameter [9:0] numbersXmin = 5*4;
	parameter [9:0] numbersXmax = 18*4-1;
	parameter [9:0] numbersYmin = 10*4;
	parameter [9:0] numbersYmax = 15*4-1;
	
	parameter [9:0] sillouetteYmin = 4*4;
	parameter [9:0] sillouetteYmax = 20*4-1;
	parameter [9:0] PSXmin = 40*4;
	parameter [9:0] PSXmax = 56*4-1;
	parameter [9:0] SFXmin = 56*4;
	parameter [9:0] SFXmax = 72*4-1;
	parameter [9:0] WNXmin = 72*4;
	parameter [9:0] WNXmax = 88*4-1;
							  
	parameter [0:63][0:15] CURSOR_SPRITE = {
		// 0
		16'b1111000000001111, // 0
		16'b1110000000000111, // 1
		16'b1100000000000011, // 2
		16'b1000000000000001, // 3
		16'b0000000000000000, // 4
		16'b0000000000000000, // 5
		16'b0000000000000000, // 6
		16'b0000000000000000, // 7
		16'b0000000000000000, // 8
		16'b0000000000000000, // 9
		16'b0000000000000000, // a
		16'b0000000000000000, // b
		16'b1000000000000001, // c
		16'b1100000000000011, // d
		16'b1110000000000111, // e
		16'b1111000000001111, // f
		// 1
		16'b1111000000001111,
		16'b1110000000000111,
		16'b1100000000000011,
		16'b1000101111000001,
		16'b0000011111101000,
		16'b0000011111111000,
		16'b0000011111101000,
		16'b0000001111000000,
		16'b0000000100000000,
		16'b0000000100000000,
		16'b0001110101110000,
		16'b0000111111000000,
		16'b1000000100000001,
		16'b1100000000000011,
		16'b1110000000000111,
		16'b1111000000001111,
		// 2
		16'b1111000000001111,
		16'b1110000000000111,
		16'b1100010110100011,
		16'b1000111111110001,
		16'b0001111111111000,
		16'b0000111111110000,
		16'b0001111111111000,
		16'b0000111111110000,
		16'b0000010110100000,
		16'b0000000110000000,
		16'b0001110110111000,
		16'b0000111111110000,
		16'b1000000110000001,
		16'b1100000000000011,
		16'b1110000000000111,
		16'b1111000000001111,
		// 3
		16'b1111000000001111,
		16'b1110000000000111,
		16'b1100011111100011,
		16'b1000111111110001,
		16'b0001111111111000,
		16'b0001111111111000,
		16'b0001111111111000,
		16'b0001111111111000,
		16'b0001111111111000,
		16'b0001111111111000,
		16'b0001111111111000,
		16'b0000111111110000,
		16'b1000011111100001,
		16'b1100000000000011,
		16'b1110000000000111,
		16'b1111000000001111
		
	};
	
	parameter [0:8][0:6] FENCE_SPRITE = {
		7'b0010000, // 1
		7'b0010000, // 2
		7'b0111000, // 3
		7'b0111000, // 4
		7'b1111100, // 5
		7'b1111100, // 6
		7'b1111111, // 7
		7'b1111100, // 8
		7'b1111100  // 9
	};
	
	parameter [0:12][0:16] SUN_TEXT = {
		17'b01111010010100100, // 1
		17'b01000010010110101, // 2
		17'b01111010010111100, // 3
		17'b00001010010101100, // 4
		17'b01111011110100101, // 5
		17'b00000000000000000, // 6
		17'b11111111111111111, // 7
		17'b11111111111111111, // 8
		17'b11111111111111111, // 9
		17'b11111111111111111, // 10
		17'b11111111111111111, // 11
		17'b11111111111111111, // 12
		17'b11111111111111111  // 13
	};
	
	parameter [0:54][0:2] NUMBERS = {
		// 0
		3'b111,
		3'b101,
		3'b101,
		3'b101,
		3'b111,
		// 1
		3'b001,
		3'b001,
		3'b001,
		3'b001,
		3'b001,
		// 2
		3'b111,
		3'b001,
		3'b111,
		3'b100,
		3'b111,
		// 3
		3'b111,
		3'b001,
		3'b111,
		3'b001,
		3'b111,
		// 4
		3'b101,
		3'b101,
		3'b111,
		3'b001,
		3'b001,
		// 5
		3'b111,
		3'b100,
		3'b111,
		3'b001,
		3'b111,
		// 6
		3'b111,
		3'b100,
		3'b111,
		3'b101,
		3'b111,
		// 7
		3'b111,
		3'b001,
		3'b001,
		3'b001,
		3'b001,
		// 8
		3'b111,
		3'b101,
		3'b111,
		3'b101,
		3'b111,
		// 9
		3'b111,
		3'b101,
		3'b111,
		3'b001,
		3'b111,
		// [BLANK]
		3'b000,
		3'b000,
		3'b000,
		3'b000,
		3'b000
	};
	
	parameter [0:15][0:15][1:0] PEASHOOTER = '{
		'{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		'{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		'{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		'{0,0,0,0,2,0,1,1,1,1,0,0,0,0,0,0},
		'{0,0,0,0,0,1,1,1,1,3,1,0,1,0,0,0},
		'{0,0,0,0,0,1,1,3,1,1,1,1,1,0,0,0},
		'{0,0,0,0,0,1,1,1,1,1,1,0,1,0,0,0},
		'{0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0},
		'{0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0},
		'{0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0},
		'{0,0,0,2,2,2,0,2,0,2,2,2,0,0,0,0},
		'{0,0,0,0,2,2,2,2,2,2,0,0,0,0,0,0},
		'{0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0},
		'{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		'{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		'{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
	};
	
	parameter [0:15][0:15][2:0] SUNFLOWER = '{
		'{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		'{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		'{0,0,0,0,0,4,0,4,4,0,4,0,0,0,0,0},
		'{0,0,0,0,4,1,1,1,1,1,1,4,0,0,0,0},
		'{0,0,0,4,1,1,1,1,1,1,1,1,4,0,0,0},
		'{0,0,0,0,1,3,1,1,1,1,3,1,0,0,0,0},
		'{0,0,0,4,1,1,3,3,3,3,1,1,4,0,0,0},
		'{0,0,0,0,4,1,1,1,1,1,1,4,0,0,0,0},
		'{0,0,0,0,0,4,0,4,4,0,4,0,0,0,0,0},
		'{0,0,0,0,0,0,0,2,2,0,0,0,0,0,0,0},
		'{0,0,0,2,2,2,0,2,2,0,2,2,2,0,0,0},
		'{0,0,0,0,2,2,2,2,2,2,2,2,0,0,0,0},
		'{0,0,0,0,0,0,0,2,2,0,0,0,0,0,0,0},
		'{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		'{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		'{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
	};
	
//	parameter [0:63][0:15][2:0] WALNUT = '{
//		// 0
//		'{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
//		'{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
//		'{0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0},
//		'{0,0,0,0,1,1,1,1,1,1,1,1,0,0,0,0},
//		'{0,0,0,1,1,1,1,1,1,1,1,1,1,0,0,0},
//		'{0,0,0,1,1,1,1,1,1,2,2,2,1,0,0,0},
//		'{0,0,0,1,2,2,2,1,1,2,2,3,1,0,0,0},
//		'{0,0,0,1,2,3,2,1,1,2,2,3,1,0,0,0},
//		'{0,0,0,1,2,3,2,1,1,1,1,1,1,0,0,0},
//		'{0,0,0,1,1,1,1,3,3,3,3,1,1,0,0,0},
//		'{0,0,0,1,1,1,1,1,1,1,1,1,1,0,0,0},
//		'{0,0,0,0,1,1,1,1,1,1,1,1,0,0,0,0},
//		'{0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0},
//		'{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
//		'{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
//		'{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
//		// 1
//		'{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
//		'{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
//		'{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
//		'{0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0},
//		'{0,0,0,1,1,1,1,1,0,0,0,1,1,0,0,0},
//		'{0,0,0,1,1,1,1,1,1,2,2,2,1,0,0,0},
//		'{0,0,0,1,2,2,2,1,1,2,2,2,1,0,0,0},
//		'{0,0,0,1,2,2,2,1,1,2,2,3,1,0,0,0},
//		'{0,0,0,1,2,3,2,1,1,1,1,1,1,0,0,0},
//		'{0,0,0,1,1,1,1,3,3,3,3,1,1,0,0,0},
//		'{0,0,0,1,1,1,1,1,1,1,1,1,1,0,0,0},
//		'{0,0,0,0,1,1,1,1,1,1,1,1,0,0,0,0},
//		'{0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0},
//		'{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
//		'{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
//		'{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
//		// 2
//		'{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
//		'{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
//		'{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
//		'{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
//		'{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
//		'{0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0},
//		'{0,0,0,1,2,3,3,1,0,0,0,0,1,0,0,0},
//		'{0,0,0,1,2,3,3,1,1,2,3,3,1,0,0,0},
//		'{0,0,0,1,2,2,2,1,1,2,2,2,1,0,0,0},
//		'{0,0,0,1,1,1,4,1,1,1,1,1,1,0,0,0},
//		'{0,0,0,1,1,1,1,1,3,3,3,1,1,0,0,0},
//		'{0,0,0,0,1,1,1,1,3,3,3,1,0,0,0,0},
//		'{0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0},
//		'{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
//		'{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
//		'{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
//		// 3
//		'{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
//		'{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
//		'{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
//		'{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
//		'{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
//		'{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
//		'{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
//		'{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
//		'{0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0},
//		'{0,0,0,1,1,0,0,0,0,0,0,0,1,0,0,0},
//		'{0,0,0,1,1,1,1,0,0,0,1,1,1,0,0,0},
//		'{0,0,0,0,1,1,1,1,1,1,1,1,0,0,0,0},
//		'{0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0},
//		'{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
//		'{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
//		'{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
//	};																	o7 o7 o7 o7

	parameter [0:15][0:15][1:0] WALNUT = '{
		// 0
		'{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		'{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		'{0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0},
		'{0,0,0,0,1,1,1,1,1,1,1,1,0,0,0,0},
		'{0,0,0,1,1,1,1,1,1,1,1,1,1,0,0,0},
		'{0,0,0,1,1,1,1,1,1,2,2,2,1,0,0,0},
		'{0,0,0,1,2,2,2,1,1,2,2,3,1,0,0,0},
		'{0,0,0,1,2,3,2,1,1,2,2,3,1,0,0,0},
		'{0,0,0,1,2,3,2,1,1,1,1,1,1,0,0,0},
		'{0,0,0,1,1,1,1,3,3,3,3,1,1,0,0,0},
		'{0,0,0,1,1,1,1,1,1,1,1,1,1,0,0,0},
		'{0,0,0,0,1,1,1,1,1,1,1,1,0,0,0,0},
		'{0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0},
		'{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		'{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		'{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
	};
	
	parameter [0:15][0:15][2:0] SUN = '{
		'{0,0,0,0,0,0,0,2,1,0,0,0,0,0,0,0},
		'{0,0,0,1,3,0,0,0,0,0,0,2,1,0,0,0},
		'{0,0,0,2,0,0,3,4,3,2,0,0,2,0,0,0},
		'{0,0,0,0,0,3,4,3,2,1,1,0,0,0,0,0},
		'{0,0,0,0,3,4,3,2,1,1,1,1,0,0,0,0},
		'{0,0,3,0,3,4,2,2,1,1,1,1,0,1,0,0},
		'{0,0,2,0,3,4,3,2,2,1,1,2,0,2,0,0},
		'{0,0,0,0,3,4,4,3,2,2,2,2,0,0,0,0},
		'{0,0,0,0,0,3,4,4,3,3,3,0,0,0,0,0},
		'{0,0,0,3,0,0,3,4,4,4,0,0,2,0,0,0},
		'{0,0,0,1,2,0,0,0,0,0,0,2,1,0,0,0},
		'{0,0,0,0,0,0,0,3,2,0,0,0,0,0,0,0},
		'{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		'{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		'{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		'{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
	};		

	parameter [0:15][0:10][2:0] ZOMBIE = '{
		'{1,1,1,1,2,0,0,0,0,0,0},
		'{3,2,3,3,1,2,0,0,0,0,0},
		'{3,2,3,3,1,2,0,0,0,0,0},
		'{1,1,1,1,2,1,0,0,0,0,0},
		'{0,0,4,4,1,2,0,0,0,1,0},
		'{0,1,1,1,0,0,0,0,1,0,1},
		'{0,0,0,0,0,4,4,5,5,0,2},
		'{0,0,0,0,2,5,4,4,4,0,1},
		'{0,1,0,0,1,5,4,4,4,0,1},
		'{1,0,1,2,0,0,4,4,0,0,0},
		'{0,0,0,0,0,4,4,4,4,0,0},
		'{0,0,0,0,0,6,6,6,6,0,0},
		'{0,0,1,1,6,0,6,0,0,0,0},
		'{0,6,0,0,0,0,0,6,6,1,0},
		'{0,1,0,0,0,0,0,0,0,1,0},
		'{0,0,0,0,0,0,0,0,0,0,0}
	};
	
	// STRUCT PARAMETERS
	parameter [11:0] SFcounterMax = 1700;
	parameter [11:0] PS_health = 500;
	parameter [11:0] SF_health = 300;
	parameter [11:0] WN_health = 1700;
	
	parameter [6:0] PS_mouth = 83;
	
	//logic zombieInLane [5];
	logic zombieInFront;
	
	logic gameOver;
	logic flipGameOver;
	
	logic [2:0] peaI;
	logic [3:0]	peaJ;
	logic [2:0] zombI;
	logic [3:0]	zombJ;
	
	// LAWN STRUCT
	typedef struct {
		logic [1:0] plant_type;
		int health;
		logic [2:0] sprite;
		int SFcounter;
	} square;
	
	typedef square grid [0:4][0:8];
	grid myLawn;
	
	// PEA STRUCT
	typedef struct {
		logic isVisible;
		int peaX;
		int peaY;
		int peashooterX;
	} pea;
	
	typedef pea pea_grid [0:4][0:8];
	pea_grid myPeas; 
	
	
	// ZOMBIE STRUCT
	typedef struct {
		logic exists;
		int health;
		int xPos;
		int yPos;
	} zomb;
	typedef zomb ies [0:4][0:4];
	ies zombies;
	
	// ZOMBIE COLLISION
	logic zombieGrid [0:4][0:8];
	logic peaCanShoot [0:4][0:8];
	
	// random generation
	int randomCounter;
	int flipper;
	logic makeZombie;
	int randomCounterMax;
	
	
	
	// LOGIC FOR ZOMBIE STRUCT
	always_ff @ (posedge Reset or posedge (vga_clk)) begin
		if (Reset) begin
			// hardcode CHANGE LATER
			for (int i = 0; i < 5; i++) begin
				for (int j = 0; j < 5; j++) begin
					zombies[i][j].health <= 10;
					zombies[i][j].xPos <= 640;
					zombies[i][j].yPos <= 30*4 + 64*i;
					zombies[i][j].exists <= 1'b0;
				end
			end
			
			for (int i = 0; i < 5; i++) begin
				for (int j = 0; j < 9; j++) begin
					zombieGrid[i][j] <= 0;
				end
			end
			
			gameOver <= 1'b0;
			flipGameOver <= 1'b0;
			zombieMoveCounter <= 7'b0000000;
			randomCounterMax <= 2000;
		// always looping through zombies
		end else if (peaIsHere && zombieIsHere) begin
			if (myPeas[peaI][peaJ].isVisible) begin
				if (zombies[zombI][zombJ].health != 1) begin	// >
					zombies[zombI][zombJ].health <= zombies[zombI][zombJ].health - 1;
				end else begin
					zombies[zombI][zombJ].exists <= 1'b0;
					zombies[zombI][zombJ].health <= 10;
					zombies[zombI][zombJ].xPos <= 764; // CHANGE
					zombieGrid[zombI][(zombies[zombI][zombJ].xPos - leftOfLawn + 8)/64] <= 1'b0;
				end
			end
		end else if (DrawY == 520 && DrawX == 0) begin
			for (int i = 0; i < 5; i++) begin
				for (int j = 0; j < 5; j++) begin
				
					if (zombies[i][j].exists == 1'b1) begin
						if (zombies[i][j].xPos <= 5*4) begin
							gameOver <= 1'b1;
						end
						
						if (zombies[i][j].xPos > leftOfLawn - 8 && zombies[i][j].xPos < rightOfLawn - 1 - 8) begin
							if (zombieGrid[i][(zombies[i][j].xPos - leftOfLawn + 8)/64] == 1'b0) begin
								if ((zombies[i][j].xPos - leftOfLawn + 8)/64 == 8) begin
									zombieGrid[i][(zombies[i][j].xPos - leftOfLawn + 8)/64] <= 1'b1;
								end else begin
									zombieGrid[i][(zombies[i][j].xPos - leftOfLawn + 8)/64] <= 1'b1;
									zombieGrid[i][((zombies[i][j].xPos - leftOfLawn + 8)/64) + 1] <= 1'b0;
								end
							end
							
							if (myLawn[i][(zombies[i][j].xPos - leftOfLawn + 8)/64].plant_type == 0) begin
								if (zombieMoveCounter == 7'b1110000) begin
									zombies[i][j].xPos <= zombies[i][j].xPos - 4;
								end else if (zombieMoveCounter == 7'b1111110) begin
									zombies[i][j].xPos <= zombies[i][j].xPos - 4;
								end
							end
						end
					end
					
					// TEST
					if (enter)
						zombies[i][j].xPos <= zombies[i][j].xPos - 4;
					// TEST
					
					if (zombies[i][j].xPos >= rightOfLawn - 1 - 8) begin
					
						// ADD: if it gets on screen, make it exist
						
						if (zombieMoveCounter == 7'b1110000) begin
							zombies[i][j].xPos <= zombies[i][j].xPos - 4;
						end else if (zombieMoveCounter == 7'b1111110) begin
							zombies[i][j].xPos <= zombies[i][j].xPos - 4;
						end
					end
					
					if (makeZombie && zombies[zombieMoveCounter%5][zombieMoveCounter%7].exists == 1'b0) begin
						zombies[zombieMoveCounter%5][zombieMoveCounter%7].xPos <= (zombieMoveCounter%10)*4 + 640;
						zombies[zombieMoveCounter%5][zombieMoveCounter%7].exists <= 1'b1;
						if (randomCounterMax >= 500)
							randomCounterMax <= randomCounterMax - 200;
					end
				end
			end
			
			if (zombieMoveCounter != 7'b1111111)
				zombieMoveCounter <= zombieMoveCounter + 7'b0000001;
			else
				zombieMoveCounter <= 7'b0000000;
				
			if (zombieMoveCounter == 7'b0000000 || zombieMoveCounter == 7'b1000000)
				flipGameOver <= ~flipGameOver;
			
		end
	end
	
	
	
	
	
	
	// LOGIC FOR PEA STRUCT
	always_ff @ (posedge Reset or posedge vga_clk) begin
		if (Reset) begin
		
			// init struct
			for (int i = 0; i < 5; i++) begin
				for (int j = 0; j < 9; j++) begin
					myPeas[i][j].isVisible <= 0;
					myPeas[i][j].peaX <= PS_mouth + 64*j - 63;
					myPeas[i][j].peaY <= 38*4-1 + 64*i;
					myPeas[i][j].peashooterX <= PS_mouth + 64*j - 1;
				end
			end
			
		end else if (peaIsHere && zombieIsHere) begin
			if (zombies[zombI][zombJ].exists)
				myPeas[peaI][peaJ].isVisible <= 0;
		end else if (DrawY == 520 && DrawX == 0) begin
			
			for (int i = 0; i < 5; i++) begin
				for (int j = 0; j < 9; j++) begin
					
					if (myPeas[i][j].peaX != myPeas[i][j].peashooterX) begin
						if (myPeas[i][j].peaX < 639) begin
							myPeas[i][j].peaX <= myPeas[i][j].peaX + 2;
						end else begin
							myPeas[i][j].peaX <= 0;
							myPeas[i][j].isVisible <= 0;
						end
					end else begin
						if (peaCanShoot[i][j] && myLawn[i][j].plant_type == 1) begin
							myPeas[i][j].isVisible <= 1;
							myPeas[i][j].peaX <= myPeas[i][j].peaX + 2;
						end
					end
					
				end
			end
		end
	end
	
	// LOGIC FOR LAWN STRUCT
	always_ff @ (posedge Reset or posedge frame_clk) begin
	
		if (Reset) begin
			Sun_Count <= 50;
			purchasing <= 0;
			plantToPurchase <= 0;
			PS_cooldown <= PS_cooldownMax;
			SF_cooldown <= 0;
			WN_cooldown <= WN_cooldownMax;
			
			for (int i = 0; i < 5; i++) begin
				for (int j = 0; j < 9; j++) begin
					myLawn[i][j].plant_type = 0;
					myLawn[i][j].health = 0;
					myLawn[i][j].sprite = 0;
					myLawn[i][j].SFcounter = 1200; // start at 1000 so first sun comes quick
				end
			end
		// place a plant
		end else if (select && purchasing && CursorY >= aboveLawn) begin
			if (myLawn[(CursorY - (aboveLawn + 1))/64][(CursorX - (leftOfLawn + 1))/64].plant_type == 0) begin
				case (plantToPurchase)
					1: begin
						myLawn[(CursorY - (aboveLawn + 1))/64][(CursorX - (leftOfLawn + 1))/64].plant_type <= 1;
						myLawn[(CursorY - (aboveLawn + 1))/64][(CursorX - (leftOfLawn + 1))/64].health <= PS_health;
						Sun_Count -= 100;
						PS_cooldown <= PS_cooldownMax;
					end
					2: begin
						myLawn[(CursorY - (aboveLawn + 1))/64][(CursorX - (leftOfLawn + 1))/64].plant_type <= 2;
						myLawn[(CursorY - (aboveLawn + 1))/64][(CursorX - (leftOfLawn + 1))/64].health <= SF_health;
						Sun_Count -= 50;
						SF_cooldown <= SF_cooldownMax;
					end
					3: begin
						myLawn[(CursorY - (aboveLawn + 1))/64][(CursorX - (leftOfLawn + 1))/64].plant_type <= 3;
						myLawn[(CursorY - (aboveLawn + 1))/64][(CursorX - (leftOfLawn + 1))/64].health <= WN_health;
						Sun_Count -= 50;
						WN_cooldown <= WN_cooldownMax;
					end
					default: begin
					end
				endcase
				myLawn[(CursorY - (aboveLawn + 1))/64][(CursorX - (leftOfLawn + 1))/64].sprite <= 0;
				purchasing <= 0;
				plantToPurchase <= 0;
			end else begin
				purchasing <= 0;
				plantToPurchase <= 0;
			end
		// delete a plant
		end else if (delete && CursorY >= aboveLawn) begin
			myLawn[(CursorY - (aboveLawn + 1))/64][(CursorX - (leftOfLawn + 1))/64].plant_type <= 0;
			myLawn[(CursorY - (aboveLawn + 1))/64][(CursorX - (leftOfLawn + 1))/64].sprite <= 0;
			myLawn[(CursorY - (aboveLawn + 1))/64][(CursorX - (leftOfLawn + 1))/64].health <= 0;
			myLawn[(CursorY - (aboveLawn + 1))/64][(CursorX - (leftOfLawn + 1))/64].SFcounter <= 1500;
		// if you hover over a sun, collect the sun
		end else if (myLawn[(CursorY - (aboveLawn + 1))/64][(CursorX - (leftOfLawn + 1))/64].SFcounter == SFcounterMax) begin
			if (Sun_Count + 25 <= 975) // CHANGE BACK TO 25 AND 975
				Sun_Count <= Sun_Count + 25;
			else
				Sun_Count <= 975;
			myLawn[(CursorY - (aboveLawn + 1))/64][(CursorX - (leftOfLawn + 1))/64].SFcounter <= 0;
		// logic for if you can purchase a plant
		end else if (select && inMenu > 0) begin
			if ((purchasing == 0 && inMenu == 1 && Sun_Count >= 100 && PS_cooldown == 0) || 
				(purchasing == 0 && inMenu == 2 && Sun_Count >= 50 && SF_cooldown == 0) ||
				(purchasing == 0 && inMenu == 3 && Sun_Count >= 50 && WN_cooldown == 0)) begin
					purchasing <= 1;
					plantToPurchase <= inMenu;
			end else begin
				plantToPurchase <= 0;
				purchasing <= 0;
			end
		// always running 
		end else begin
			for (int i = 0; i < 5; i++) begin
				for (int j = 0; j < 9; j++) begin
				
					
					if (myLawn[i][j].plant_type > 0 && zombieGrid[i][j] != 1'b0) begin
						if (myLawn[i][j].health != 1) begin
							myLawn[i][j].health -= 1;
						end else begin
							// don't need to edit health
							myLawn[i][j].plant_type = 0;
							myLawn[i][j].sprite = 0;
							myLawn[i][j].SFcounter = 1200;
						end
					end
						
					// remove health from walnut and set walnut sprite based on health
					if (myLawn[i][j].plant_type == 3) begin
						myLawn[i][j].sprite = 1 - (myLawn[i][j].health / 671); // (was 3 -). also, denominator was ~(walnut health)/3.49 so 286. now it is /2.49 and (2 -)
						// myLawn[i][j].health -= 1;
					// increment a sunflower's counter
					end else if (myLawn[i][j].plant_type == 2 && myLawn[i][j].SFcounter != SFcounterMax) begin
						myLawn[i][j].SFcounter += 1;
					end
				end
			end
			
			// decrement cooldowns
			if (PS_cooldown != 0)
				PS_cooldown -= 1;
			if (SF_cooldown != 0)
				SF_cooldown -= 1;
			if (WN_cooldown != 0)
				WN_cooldown -= 1;
				
			if (sunCounter == 0)
				Sun_Count <= Sun_Count + 25;
		end
	end
	
	// set type and sprite of plant we are currently drawing
	always_comb begin
		curPlantType = myLawn[(DrawY - (aboveLawn + 1))/64][(DrawX - (leftOfLawn + 1))/64].plant_type;
		curPlantSprite = myLawn[(DrawY - (aboveLawn + 1))/64][(DrawX - (leftOfLawn + 1))/64].sprite;
		if (myLawn[(DrawY - (aboveLawn + 1))/64][(DrawX - (leftOfLawn + 1))/64].SFcounter == SFcounterMax)
			curPlantHasSun = 1'b1;
		else
			curPlantHasSun = 1'b0;
	end
	
	
	// set if we are currenty drawing a pea or not
	always_comb begin
		peaIsHere = 0;
		peaI = 4;
		peaJ = 8;
		for (int i = 0; i < 5; i++) begin
			for (int j = 0; j < 9; j++) begin
				if (myPeas[i][j].isVisible && myPeas[i][j].peaX >= DrawX && myPeas[i][j].peaX <= DrawX + 3 &&
					 myPeas[i][j].peaY >= DrawY && myPeas[i][j].peaY <= DrawY + 3) begin
					peaIsHere = 1;
					peaI = i;
					peaJ = j;
				end
			end
		end
	end
	
	// set if we are currenty drawing a zombie or not
	always_comb begin
		zombieIsHere = 0;
		curZombieXPos = 0;
		curZombieYPos = 0;
		zombI = 4;
		zombJ = 4;
		for (int i = 0; i < 5; i++) begin
			for (int j = 0; j < 5; j++) begin
				if (zombies[i][j].exists && blank && DrawX >= zombies[i][j].xPos && DrawX <= zombies[i][j].xPos + 11*4-1 &&
					 DrawY >= zombies[i][j].yPos && DrawY <= zombies[i][j].yPos + 16*4-1) begin
						zombieIsHere = 1;
						curZombieXPos = (DrawX - zombies[i][j].xPos) / 4;
						curZombieYPos = (DrawY - zombies[i][j].yPos) / 4;
						zombI = i;
						zombJ = j;
				end
			end
		end
	end

	// determine if a zombie is in front of a given pea shooter
	always_comb begin
		if (!frame_clk) begin
			for (int i = 0; i < 5; i++) begin
				for (int j = 0; j < 9; j++) begin
					peaCanShoot[i][j] = 0;
				end
			end
		end else begin
			for (int i = 0; i < 5; i++) begin
				for (int j = 0; j < 9; j++) begin
					if (zombieGrid[i][j]) begin
						for (int k = 0; k <= j; k++) begin
							peaCanShoot[i][k] = 1;
						end
					end
				end
			end
		end
	end
	
	
	
	
	
	
	
	// randomization
	
	always_ff @ (posedge Reset or posedge frame_clk) begin
	
		if (Reset) begin

			randomCounter <= 0;
			flipper <= 1;
			makeZombie <= 1'b0;
			
		end else begin
		
			flipper <= flipper * -1;
			
			if (keyWasPressed && randomCounter != 0) begin
					randomCounter <= randomCounter + flipper;
			end else if (randomCounter >= randomCounterMax) begin
				randomCounter <= 0;
				makeZombie <= 1'b1;
			end else if (makeZombie) begin
				makeZombie <= 1'b0;
			end else begin
				randomCounter <= randomCounter + 1;
			end
			
		end
	end
	
	
	always_ff @ (posedge Reset or posedge frame_clk) begin
		if (Reset) begin
			sunCounter <= 0;
		end else begin
			if (sunCounter < 2000) begin
				sunCounter <= sunCounter + 1;
			end else begin
				sunCounter <= 0;
			end
				
		end
	end
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	// CURSOR COLOR LOGIC
	always_ff @ (posedge Reset or posedge frame_clk) begin
		if (Reset) begin
			cursor_Red = 8'hff; // yellow
			cursor_Green = 8'heb;
			cursor_Blue = 8'h3b;
		end else if (spacebar == 1'b1 || purchasing == 1) begin
			cursor_Red = 8'hff; // light yellow
			cursor_Green = 8'hf4;
			cursor_Blue = 8'h9e;
		end else begin
			cursor_Red = 8'hff; // yellow
			cursor_Green = 8'heb;
			cursor_Blue = 8'h3b;
		end
	end
	
	// helper logic for drawing the sun count
	always_comb begin
		hundredsPlace = Sun_Count / 100;
		tensPlace = (Sun_Count - (hundredsPlace * 100)) / 10;
		onesPlace = Sun_Count - (tensPlace * 10) - (hundredsPlace * 100);
		
		if (DrawX < numbersXmin + 12) begin
			number = hundredsPlace;
			offset = 4'b0000;
		end else if (DrawX < numbersXmin + 32 && DrawX >= numbersXmin + 20) begin
			number = tensPlace;
			offset = 4'b0101;
		end else if (DrawX < numbersXmin + 52 && DrawX >= numbersXmin + 40) begin
			number = onesPlace;
			offset = 4'b1010;
		end else begin // draw blank number (10)
			number = 10;
			offset = 4'b0000;
		end
	end
	 
	always_comb
	begin: BG_Assignments
		if (DrawY <= aboveLawn) begin
			// DRAW "SUN" AND BLACK RECTANGLE BELOW IT
			if ((DrawX <= textXmax && DrawX >= textXmin && DrawY <= textYmax && DrawY >= textYmin) &&
				(SUN_TEXT[(DrawY-textYmin)/4][(DrawX-textXmin)/4])) begin
					BG_Red <= 8'h00; // black text
					BG_Green <= 8'h00;
					BG_Blue <= 8'h00;
			// DRAW SILLOUETTES
			end else if (DrawY >= sillouetteYmin && DrawY <= sillouetteYmax && DrawX >= PSXmin && DrawX <= WNXmax) begin
				if (DrawX <= PSXmax && (PEASHOOTER[(DrawY-sillouetteYmin)/4][(DrawX-PSXmin)/4] > 0)) begin
					if (PS_cooldown == 0 && plantToPurchase != 1) begin
						BG_Red <= 8'h00; // black
						BG_Green <= 8'h00;
						BG_Blue <= 8'h00;
					end else if ((((DrawY - sillouetteYmin)/4) <= ((PS_cooldown * 10) / PS_cooldownMax) + 3) || plantToPurchase == 1) begin
						BG_Red <= 8'ha3; // light light brown
						BG_Green <= 8'h7c;
						BG_Blue <= 8'h72;
					end else begin
						BG_Red <= 8'h00; // black
						BG_Green <= 8'h00;
						BG_Blue <= 8'h00;
					end
				end else if (DrawX >= SFXmin && DrawX <= SFXmax && (SUNFLOWER[(DrawY-sillouetteYmin)/4][(DrawX-SFXmin)/4] > 0)) begin
					if (SF_cooldown == 0 && plantToPurchase != 2) begin
						BG_Red <= 8'h00; // black
						BG_Green <= 8'h00;
						BG_Blue <= 8'h00;
					end else if ((((DrawY - sillouetteYmin)/4) <= ((SF_cooldown * 11) / SF_cooldownMax) + 2) || plantToPurchase == 2) begin
						BG_Red <= 8'ha3; // light light brown
						BG_Green <= 8'h7c;
						BG_Blue <= 8'h72;
					end else begin
						BG_Red <= 8'h00; // black
						BG_Green <= 8'h00;
						BG_Blue <= 8'h00;
					end
				end else if (DrawX >= WNXmin && (WALNUT[(DrawY-sillouetteYmin)/4][(DrawX-WNXmin)/4] > 0)) begin
					if (WN_cooldown == 0 && plantToPurchase != 3) begin
						BG_Red <= 8'h00; // black
						BG_Green <= 8'h00;
						BG_Blue <= 8'h00;
					end else if ((((DrawY - sillouetteYmin)/4) <= ((WN_cooldown * 11) / WN_cooldownMax) + 2) || plantToPurchase == 3) begin
						BG_Red <= 8'ha3; // light light brown
						BG_Green <= 8'h7c;
						BG_Blue <= 8'h72;
					end else begin
						BG_Red <= 8'h00; // black
						BG_Green <= 8'h00;
						BG_Blue <= 8'h00;
					end
				end else begin
					BG_Red <= 8'h85; // light brown
					BG_Green <= 8'h5d;
					BG_Blue <= 8'h52;
				end
			// DRAW SIGN AROUND SILLOUETTES
			end else if ((DrawX >= PSXmin - 8 && DrawX <= WNXmax + 8 && DrawY >= sillouetteYmin - 8 && DrawY <= sillouetteYmax + 8) ||
							(DrawY >= sillouetteYmax && 
							(((DrawX >= PSXmin + 4) && DrawX <= PSXmin + 15) || ((DrawX <= WNXmax - 4) && (DrawX >= WNXmax - 15))))) begin
								BG_Red <= 8'h3b; // dark brown
								BG_Green <= 8'h28;
								BG_Blue <= 8'h23;
			// DRAW SKY
			end else begin
				BG_Red <= 8'h21; // blue for the sky
				BG_Green <= 8'h95;
				BG_Blue <= 8'hf3;
			end
		// DRAW EDGES OUTSIDE OF LAWN
		end else if (DrawX <= leftOfLawn || (DrawY >= belowLawn && DrawY <= DrawYMax) || (DrawX >= rightOfLawn && DrawX <= DrawXMax)) begin
			BG_Red <= 8'h5d; // brown
			BG_Green <= 8'h40;
			BG_Blue <= 8'h37;
		end else begin
			// DRAW LAWN
			if ((((DrawY - (aboveLawn + 1)) / 64) % 2) == 0) begin
				if ((((DrawX - (leftOfLawn + 1)) / 64) % 2) == 0) begin
					BG_Red <= 8'h8b; // light green
					BG_Green <= 8'hc3;
					BG_Blue <= 8'h4a;
				end else begin
					BG_Red <= 8'h4c; // dark green
					BG_Green <= 8'haf;
					BG_Blue <= 8'h4f;
				end
			end else begin
				if ((((DrawX - (leftOfLawn + 1)) / 64) % 2) == 0) begin
					BG_Red <= 8'h4c; // dark green
					BG_Green <= 8'haf;
					BG_Blue <= 8'h4f;
				end else begin
					BG_Red <= 8'h8b; // light green
					BG_Green <= 8'hc3;
					BG_Blue <= 8'h4a;
				end
			end
		end
	end
			
	always_comb
	begin: RGB_Display
		// DRAW CURSOR
		if ((DrawX >= CursorX && DrawX < CursorX + 64 && DrawY >= CursorY && DrawY < CursorY + 64) &&
						(CURSOR_SPRITE[((DrawY-CursorY) % 64)/4 + 16*plantToPurchase][(DrawX-CursorX)/4])) begin
			FG_Red = cursor_Red;
			FG_Green = cursor_Green;
			FG_Blue = cursor_Blue;
		// DRAW FENCE
		end else if ((DrawY <= aboveLawn) &&
						(DrawY > aboveFence && DrawX <= DrawXMax) &&
						(FENCE_SPRITE[(DrawY - (aboveFence + 1))/4][(DrawX/4) % 7])) begin
							FG_Red = 8'hff; // white
							FG_Green = 8'hff;
							FG_Blue = 8'hff;
		// DRAW SUN COUNT
		end else if ((DrawY <= aboveLawn) &&
						(DrawX >= numbersXmin && DrawX <= numbersXmax && DrawY >= numbersYmin && DrawY <= numbersYmax) &&
						(NUMBERS[number*5 + (DrawY - numbersYmin)/4][(DrawX - numbersXmin - (offset*4))/4])) begin
							FG_Red = 8'hff; // white
							FG_Green = 8'hff;
							FG_Blue = 8'hff;
		// DRAW PLANTS
		end else if (curPlantType > 0) begin
			if (curPlantType == 1) begin
				case(PEASHOOTER[((DrawY-(aboveLawn + 1)) % 64)/4][((DrawX-(leftOfLawn + 1)) % 64)/4])
					2'b01: begin
						FG_Red = 8'ha5; // light light green
						FG_Green = 8'hd6;
						FG_Blue = 8'ha7;
					end
					2'b10: begin
						FG_Red = 8'h38; // dark dark green
						FG_Green = 8'h8e;
						FG_Blue = 8'h3d;
					end
					2'b11: begin
						FG_Red = 8'h00; // black
						FG_Green = 8'h00;
						FG_Blue = 8'h00;
					end
					default: begin
						FG_Red = BG_Red;
						FG_Green = BG_Green;
						FG_Blue = BG_Blue;
					end
				endcase
			end else if (curPlantType == 2) begin
				case(SUNFLOWER[((DrawY-(aboveLawn + 1)) % 64)/4][((DrawX-(leftOfLawn + 1)) % 64)/4])
					3'b001: begin
						FG_Red = 8'he6; // tan
						FG_Green = 8'ha7;
						FG_Blue = 8'h63;
					end
					3'b010: begin
						FG_Red = 8'h38; // dark dark green
						FG_Green = 8'h8e;
						FG_Blue = 8'h3d;
					end
					3'b011: begin
						FG_Red = 8'h00; // black
						FG_Green = 8'h00;
						FG_Blue = 8'h00;
					end
					3'b100: begin
						FG_Red = 8'hff; // yellow petal
						FG_Green = 8'heb;
						FG_Blue = 8'h3b;
					end
					default: begin
						FG_Red = BG_Red;
						FG_Green = BG_Green;
						FG_Blue = BG_Blue;
					end
				endcase
			end else if (curPlantType == 3) begin
				case(WALNUT[((DrawY-(aboveLawn + 1)) % 64)/4][((DrawX-(leftOfLawn + 1)) % 64)/4])
					2'b01: begin
						FG_Red = 8'hcf; // walnut color
						FG_Green = 8'h77;
						FG_Blue = 8'h13;
					end
					2'b10: begin
						FG_Red = 8'hff; // white
						FG_Green = 8'hff;
						FG_Blue = 8'hff;
					end
					2'b11: begin
						FG_Red = 8'h00; // black
						FG_Green = 8'h00;
						FG_Blue = 8'h00;
					end
					default: begin
						FG_Red = BG_Red;
						FG_Green = BG_Green;
						FG_Blue = BG_Blue;
					end
				endcase
			end else begin
				FG_Red = BG_Red;
				FG_Green = BG_Green;
				FG_Blue = BG_Blue;
			end
		end else begin
			FG_Red = BG_Red;
			FG_Green = BG_Green;
			FG_Blue = BG_Blue;
		end
	end 
	
	always_comb
	begin: Sun
		if (~blank) begin
			Red = 8'h00; // black
			Green = 8'h00;
			Blue = 8'h00;
		end else if (gameOver || enter) begin // CHANGE BACK
			case(ZOMBIE[(DrawY % 64)/4][((1-flipGameOver)*(DrawX % 44)/4) + (flipGameOver * (10 - (DrawX % 44)/4))])
				3'b001: begin
					Red = 8'h6d; // neon green
					Green = 8'hd9;
					Blue = 8'h00;
				end
				3'b010: begin
					Red = 8'h7c; // dark green
					Green = 8'h9e;
					Blue = 8'h02;
				end
				3'b011: begin
					Red = 8'hff; // red
					Green = 8'h17;
					Blue = 8'h45;
				end
				3'b100: begin
					Red = 8'h00; // black
					Green = 8'h00;
					Blue = 8'h00;
				end
				3'b101: begin
					Red = 8'h40; // dark purple
					Green = 8'h00;
					Blue = 8'h20;
				end
				3'b110: begin
					Red = 8'h0d; // blue pants
					Green = 8'h7f;
					Blue = 8'hd6;
				end
				default: begin
					Red = 8'ha6; // black
					Green = 8'h05;
					Blue = 8'h05;
				end
			endcase
//		end else if (zombieGridIsHere) begin
//			Red = 8'hFF; // black
//			Green = 8'hFF;
//			Blue = 8'hFF;
		end else if (peaIsHere) begin
			Red = 8'h00; // black
			Green = 8'h00;
			Blue = 8'h00;
		end else if (zombieIsHere) begin
			case(ZOMBIE[curZombieYPos][curZombieXPos])
				3'b001: begin
					Red = 8'h6d; // neon green
					Green = 8'hd9;
					Blue = 8'h00;
				end
				3'b010: begin
					Red = 8'h7c; // dark green
					Green = 8'h9e;
					Blue = 8'h02;
				end
				3'b011: begin
					Red = 8'hff; // red
					Green = 8'h17;
					Blue = 8'h45;
				end
				3'b100: begin
					Red = 8'h00; // black
					Green = 8'h00;
					Blue = 8'h00;
				end
				3'b101: begin
					Red = 8'h40; // dark purple
					Green = 8'h00;
					Blue = 8'h20;
				end
				3'b110: begin
					Red = 8'h0d; // blue pants
					Green = 8'h7f;
					Blue = 8'hd6;
				end
				default: begin
					Red = FG_Red;
					Green = FG_Green;
					Blue = FG_Blue;
				end
			endcase
		end else if (curPlantType == 2 && curPlantHasSun != 0) begin
			case(SUN[((DrawY-(aboveLawn + 1)) % 64)/4][((DrawX-(leftOfLawn + 1)) % 64)/4])
				3'b001: begin
					Red = (8'hff + FG_Red)/2; // yellow 1
					Green = (8'hd5 + FG_Green)/2;
					Blue = (8'h00 + FG_Blue)/2;
				end
				3'b010: begin
					Red = (8'hff + FG_Red)/2; // yellow 2
					Green = (8'hc8 + FG_Green)/2;
					Blue = (8'h00 + FG_Blue)/2;
				end
				3'b011: begin
					Red = (8'hff + FG_Red)/2; // yellow 3
					Green = (8'hb7 + FG_Green)/2;
					Blue = (8'h00 + FG_Blue)/2;
				end
				3'b100: begin
					Red = (8'hff + FG_Red)/2; // yellow 4
					Green = (8'ha6 + FG_Green)/2;
					Blue = (8'h00 + FG_Blue)/2;
				end
				default: begin
					Red = FG_Red;
					Green = FG_Green;
					Blue = FG_Blue;
				end
			endcase
		end else begin
			Red = FG_Red;
			Green = FG_Green;
			Blue = FG_Blue;
		end
	end
endmodule
