#include "helper.h"
#include "periph.h"
#include "serialATmega.h"
#include "spiAVR.h"
#include "ST7735.h"
#include "timerISR.h"

#define NUM_OBSTACLES 5 // Number of obstacles in the games

// Function to draw a bitmap
void drawBitmap(const unsigned char *bitmap, int x, int y, int height, int width) {

    // Set the address window to the area for bitmap
    setAddrWindow(x, y, x + width - 1, y + height - 1);

    // Iterate through each pixel in the bitmap
    for (int j = 0; j < height; j++) {
        for (int i = 0; i < width; i++) {
            // Calculate byte index and bit position within byte
            int byteIndex = (j * width + i) / 8;
            int bitPosition = 7 - (i % 8);

            // Read byte from bitmap array
            int byte = bitmap[byteIndex];

            // Determine the color of the pixel (1 for white, 0 for black)
            int color = (byte & (1 << bitPosition)) ? 0xFFFF : 0x0000; // RGB565 format

            // Send the color data
            Send_Data(color >> 8);   // High byte
            Send_Data(color & 0xFF); // Low byte
        }
    }
}

// Function to "delete" a bitmap by drawing a blank rectangle over it
void deleteBitmap(int x, int y, int height, int width) {
    // Set the address window to the area for the blank rectangle
    setAddrWindow(x, y, x + height - 1, y + width - 1);

    // Iterate through each pixel in the rectangle
    for (int j = 0; j < width; j++) {
        for (int i = 0; i < height; i++) {
            Send_Data(0xFF);   // High byte
            Send_Data(0xFF);   // Low byte
        }
    }
}

// Function to check if two bitmaps intersect or touch
bool checkCollision(int x1, int y1, int width1, int height1, int x2, int y2, int width2, int height2) {
    if (x1 + width1 <= x2 || x2 + width2 <= x1) {
        return false;
    }
    if (y1 + height1 <= y2 || y2 + height2 <= y1) {
        return false;
    }
    serial_println("Collision detected!");
    return true;
}

// Dinosaur bitmap, 30x24px
const unsigned char dinosaur_map [] = {
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf8, 0x03, 0xff, 0xf2, 0x01, 0xff, 0xf0, 0x01, 0xff, 
	0xf0, 0x01, 0xff, 0xf0, 0x01, 0xff, 0xf0, 0x3f, 0xff, 0xf0, 0x3f, 0xff, 0xf0, 0x07, 0xff, 0xe0, 
	0x7f, 0xbf, 0xc0, 0x7f, 0xbf, 0x80, 0x0f, 0x9f, 0x00, 0x4f, 0x9f, 0x00, 0x4f, 0x86, 0x00, 0x7f, 
	0x80, 0x00, 0x7f, 0x80, 0x00, 0xff, 0xc0, 0x00, 0xff, 0xe0, 0x01, 0xff, 0xf8, 0x03, 0xff, 0xf8, 
	0x03, 0xff, 0xfc, 0x07, 0xff, 0xfe, 0x07, 0xff, 0xfe, 0x27, 0xff, 0xfe, 0x77, 0xff, 0xfe, 0xf7, 
	0xff, 0xfe, 0x73, 0xff, 0xfe, 0x73, 0xff, 0xff, 0xff, 0xff
};

// Small cactus bitmap, 16x24px
const unsigned char cactus_map [] = {
	0xff, 0xff, 0xfc, 0x3f, 0xfc, 0x3f, 0xfc, 0x3f, 0xec, 0x3f, 0xe4, 0x3f, 0xe4, 0x3f, 0xe4, 0x37, 
	0xe4, 0x27, 0xe4, 0x27, 0xe0, 0x27, 0xf0, 0x27, 0xfc, 0x27, 0xfc, 0x07, 0xfc, 0x0f, 0xfc, 0x3f, 
	0xfc, 0x3f, 0xfc, 0x3f, 0xfc, 0x3f, 0xfc, 0x3f, 0xfc, 0x3f, 0xfc, 0x3f, 0xfc, 0x3f, 0xfc, 0x3f
};

// Large cacti bitmap, 24x24px
const unsigned char cacti_map [] = {
	0xf9, 0xff, 0xff, 0xf0, 0xff, 0xff, 0xf0, 0xff, 0xff, 0xf0, 0xff, 0x87, 0xf0, 0x8f, 0x87, 0xf0, 
	0x8e, 0x87, 0xb0, 0x8e, 0x07, 0x10, 0x8e, 0x07, 0x10, 0x8e, 0x01, 0x10, 0x8e, 0x01, 0x10, 0x92, 
	0x01, 0x10, 0x12, 0x01, 0x10, 0x33, 0x81, 0x90, 0xd2, 0x01, 0xc0, 0x92, 0x01, 0xf0, 0x92, 0x03, 
	0xf0, 0x92, 0x07, 0xf0, 0xc0, 0x07, 0xf0, 0xe0, 0x87, 0xf0, 0xf3, 0x87, 0xf1, 0xf3, 0x87, 0xf1, 
	0xf3, 0x87, 0xf1, 0xf3, 0x87, 0xe0, 0x00, 0x03
};

// Bird bitmap, 16x24px
const unsigned char bird_map [] = {
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x7f, 0xff, 0xff, 0x3f, 0xff, 0xf9, 
	0x9f, 0xff, 0xf8, 0x8f, 0xff, 0xf4, 0x87, 0xff, 0xc0, 0x07, 0xff, 0xff, 0x03, 0xff, 0xff, 0x80, 
	0xff, 0xff, 0xc0, 0x7f, 0xff, 0xe0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
};

// Character struct
typedef struct {
    int x;           // X-coordinate of the character
    int y;           // Y-coordinate of the character
    int height;       // Height of the dinosaur
    int width;      // Width of the dinosaur
    bool alive;      // Status of the character, true if alive
} Character;

// Obstacle struct
typedef struct {
    int x;       // X-coordinate of the obstacle
    int y;       // Y-coordinate of the obstacle
    int height;   // Width of the obstacle
    int width;  // Height of the obstacle
    bool active; // Status of the obstacle, true if active
} Obstacle;

// Draw dinosaur
void drawDinosaur(Character dino) {
    drawBitmap(dinosaur_map, dino.x, dino.y, 30, 24);
}

// Draw bird
void drawBird(Obstacle bird) {
    drawBitmap(bird_map, bird.x, bird.y, 16, 24);
}

// Draw cactus
void drawCactus(Obstacle cactus) {
    drawBitmap(cactus_map, cactus.x, cactus.y, 24, 16);
}

// Large cacti
void drawCacti(Obstacle cacti) {
    drawBitmap(cacti_map, cacti.x, cacti.y, 24, 24);
}