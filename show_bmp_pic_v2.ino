// IMPORTANT: LCDWIKI_KBV LIBRARY MUST BE SPECIFICALLY
// CONFIGURED FOR EITHER THE TFT SHIELD OR THE BREAKOUT BOARD.

//This program is a demo of how to show a bmp picture from SD card.

//Set the pins to the correct ones for your development shield or breakout board.
//the 16bit mode only use in Mega.you must modify the mode in the file of lcd_mode.h
//when using the BREAKOUT BOARD only and using these 16 data lines to the LCD,
//pin usage as follow:
//             CS  CD  WR  RD  RST  D0  D1  D2  D3  D4  D5  D6  D7  D8  D9  D10  D11  D12  D13  D14  D15 
//Arduino Mega 40  38  39  43  41   37  36  35  34  33  32  31  30  22  23  24   25   26   27   28   29
//             TP_IRQ  MOSI  MISO  TP_CS  EX_CLK
//Arduino Mega   44    51     50    53      52

//when using the BREAKOUT BOARD only and using these 8 data lines to the LCD,
//pin usage as follow:
//             CS  CD  WR  RD  RST  D0  D1  D2  D3  D4  D5  D6  D7  D8  D9  D10  D11  D12  D13  D14  D15 
//Arduino Mega 40  38  39  43  41   37  36  35  34  33  32  31  30  /   /    /    /    /    /    /    /
//             TP_IRQ  MOSI  MISO  TP_CS  EX_CLK
//Arduino Mega   44    51     50    53      52

//Remember to set the pins to suit your display module!

/**********************************************************************************
* @attention
*
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
* TIME. AS A RESULT, QD electronic SHALL NOT BE HELD LIABLE FOR ANY
* DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
* FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE 
* CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
**********************************************************************************/

#include <SD.h>
#include <SPI.h>
#include <LCDWIKI_GUI.h> //Core graphics library
#include <LCDWIKI_KBV.h> //Hardware-specific library

//the definiens of 8bit mode as follow:
//if the IC model is known or the modules is unreadable,you can use this constructed function
LCDWIKI_KBV my_lcd(NT35510,40,38,39,43,41); //model,cs,cd,wr,rd,reset


#define  BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF

#define PIXEL_NUMBER  (my_lcd.Get_Display_Width()/4)
#define SCREEN_WIDTH (my_lcd.Get_Display_Width())
#define SCREEN_HEIGHT (my_lcd.Get_Display_Height())
#define FILE_NUMBER 4
#define FILE_NAME_SIZE_MAX 20

typedef struct {
    uint32_t bV4Size;             // Size of the header (40 bytes for BITMAPV4HEADER)
    int32_t  bV4Width;            // Width of the bitmap in pixels
    int32_t  bV4Height;           // Height of the bitmap in pixels
    uint16_t bV4Planes;           // Number of color planes (must be 1)
    uint16_t bV4BitCount;         // Number of bits per pixel
    uint32_t bV4Compression;      // Compression method (3 for BI_BITFIELDS)
    uint32_t bV4SizeImage;        // Size of the image data in bytes (can be 0 for BI_RGB)
    int32_t  bV4XPelsPerMeter;    // Horizontal resolution (pixels per meter)
    int32_t  bV4YPelsPerMeter;    // Vertical resolution (pixels per meter)
    uint32_t bV4ClrUsed;          // Number of colors in the color palette (0 for full color images)
    uint32_t bV4ClrImportant;     // Number of important colors used (0 for all colors)
    uint32_t bV4RedMask;          // Bit mask for the red channel (valid only for BI_BITFIELDS compression)
    uint32_t bV4GreenMask;        // Bit mask for the green channel (valid only for BI_BITFIELDS compression)
    uint32_t bV4BlueMask;         // Bit mask for the blue channel (valid only for BI_BITFIELDS compression)
    uint32_t bV4AlphaMask;        // Bit mask for the alpha channel (valid only for BI_BITFIELDS compression)
    uint32_t bV4CSType;           // Color space type (0 for LCS_CALIBRATED_RGB)
    uint32_t bV4Endpoints;        // CIEXYZTRIPLE structure that specifies the XYZ coordinates of the three colors that correspond to the red, green, and blue endpoints for the logical color space associated with the bitmap
    uint32_t bV4GammaRed;         // Tone response curve for red channel
    uint32_t bV4GammaGreen;       // Tone response curve for green channel
    uint32_t bV4GammaBlue;        // Tone response curve for blue channel
    uint32_t bV4Intent;           // Rendering intent for the bitmap (0 for LCS_GM_ABS_COLORIMETRIC)
    uint32_t bV4ProfileData;      // Offset to the start of the profile data from the beginning of the header
    uint32_t bV4ProfileSize;      // Size of embedded profile data
    uint32_t bV4Reserved;         // Reserved (set to 0)
} BITMAPV4HEADER;

uint32_t bmp_offset = 0;
uint32_t bmp_size = 0;
uint16_t s_width = my_lcd.Get_Display_Width();  
uint16_t s_heigh = my_lcd.Get_Display_Height();
//int16_t PIXEL_NUMBER;

char file_name [FILE_NAME_SIZE_MAX];

uint16_t read_16(File fp)
{
    uint8_t low;
    uint16_t high;
    low = fp.read();
    high = fp.read();
    return (high<<8)|low;
}

uint32_t read_32(File fp)
{
    uint16_t low;
    uint32_t high;
    low = read_16(fp);
    high = read_16(fp);
    return (high<<16)|low;   
 }
 
void drawBMP(File bmpFile) {
  if (bmpFile) {
    
    BITMAPV4HEADER bmpHeader;
    // Read and validate BMP file signature
    if(read_16(bmpFile) != 0x4D42)
    {
       Serial.println("No 0x4D42");
      return false;  
    }
    // Read BITMAPV4HEADER structure
    uint32_t size = read_32(bmpFile);
    uint32_t creator_info = read_32(bmpFile);
    uint32_t offset = read_32(bmpFile);
    bmpFile.read(reinterpret_cast<char*>(&bmpHeader), sizeof(BITMAPV4HEADER));
    // bmpHeader.bV4Size = read_32(bmpFile);
    // bmpHeader.bV4Width = read_32(bmpFile);
    // bmpHeader.bV4Height = read_32(bmpFile);
    // bmpHeader.bV4Planes = read_16(bmpFile);
    // bmpHeader.bV4BitCount = read_16(bmpFile);

    Serial.println((String)"Size is "+ size);
    Serial.println((String)"Creator Info is "+ creator_info);
    Serial.println((String)"Offset is "+offset);
    Serial.println((String)"bmpHeader.bV4Size is "+bmpHeader.bV4Size);
    Serial.println((String)"Width is "+bmpHeader.bV4Width);
    Serial.println((String)"Height is "+bmpHeader.bV4Height);
    Serial.println((String)"Red bitmask is "+bmpHeader.bV4RedMask);
    Serial.println((String)"Blue bitmask is "+bmpHeader.bV4BlueMask);
    Serial.println((String)"Green bitmask is "+bmpHeader.bV4GreenMask);

    // Skip to image data
    bmpFile.seek(offset-1);

    // Read image data
    int padding = (4 - (bmpHeader.bV4Width * bmpHeader.bV4BitCount / 8) % 4) % 4;
    for (int y = 0; y < abs(bmpHeader.bV4Height); ++y) {
      for (int x = 0; x < bmpHeader.bV4Width; ++x) {
        // Read pixel color based on the bV4BitCount (assuming 24 bits per pixel)
        uint8_t pixel[3] = {0};  // Assuming 32 bits per pixel (including alpha channel)
        bmpFile.read(reinterpret_cast<char*>(&pixel), bmpHeader.bV4BitCount / 8);
        // pixel[0] = bmpFile.read();
        // pixel[1] = bmpFile.read();
        // pixel[2] = bmpFile.read();
        // pixel[3] = bmpFile.read();

        // Extract RGB values (assuming little-endian)
        uint8_t blue = pixel[1] ;
        uint8_t green = pixel[2] ;
        uint8_t red = pixel[3] ;

        // Serial.println((String) blue);
        // Serial.println((String) green);
        // Serial.println((String) red);
        // break;
        
        // Combine color components into a 16-bit color (assuming RGB565 format)
        uint16_t color = ((red >> 3) << 11) | ((green >> 2) << 5) | (blue >> 3);
        // Serial.println((String) color);
        // break;

        // Draw the pixel on the LCD
        //drawPixel(x, y, color);
        //Serial.println(color);
        my_lcd.Set_Draw_color(red, green, blue);
        my_lcd.Draw_Pixel(x,y);
      }
      // Skip padding
      //bmpFile.seek(bmpFile.position() + padding);
      //break;
    }

    Serial.println("BMP image drawn on the LCD.");
    bmpFile.close();
  } else {
    Serial.println("Error opening BMP file.");
  }
}

void setup() 
{
    Serial.begin(9600);
   my_lcd.Init_LCD();
   my_lcd.Fill_Screen(BLUE);
   //s_width = my_lcd.Get_Display_Width();  
   //s_heigh = my_lcd.Get_Display_Height();
   //PIXEL_NUMBER = my_lcd.Get_Display_Width()/4;
   strcpy(file_name,"hakbal.bmp");
  //Init SD_Card
   pinMode(48, OUTPUT);
   
    if (!SD.begin(48)) 
    {
      my_lcd.Set_Text_Back_colour(BLUE);
      my_lcd.Set_Text_colour(WHITE);    
      my_lcd.Set_Text_Size(1);
      my_lcd.Print_String("SD Card Init fail!",0,0);
    }
}


void loop() 
{
    File bmp_file;
    Serial.println(file_name);
    bmp_file = SD.open(file_name, (O_READ | O_WRITE));
    if(!bmp_file){
      my_lcd.Set_Text_Back_colour(BLUE);
      my_lcd.Set_Text_colour(WHITE);    
      my_lcd.Set_Text_Size(1);
      my_lcd.Print_String("didnt find BMPimage!",0,10);
      Serial.println("didnt find BMPimagge!");
      while(1);
    }   
    drawBMP(bmp_file);
    bmp_file.close(); 
    while(1);
}

