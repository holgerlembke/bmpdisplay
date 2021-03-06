//=====================================================================================================
// def: https://de.wikipedia.org/wiki/Windows_Bitmap
void displayBMP(String filename) // only 8-bit bmps
{
  int i;
  File f = LittleFS.open(filename, "r");

  // Soso.
  if (!f) {
    serial.print(F("Panic: BMP-File-Not-Found "));
    serial.print(filename);
    serial.println();
    return;
  }

#pragma pack(push, 1)
  struct {
    uint16_t bfType;
    uint32_t bfSize;
    uint32_t bfReserved;
    uint32_t bfOffBits;
  } header;

  struct {
    uint32_t biSize;
    int32_t biWidth;
    int32_t biHeight;
    uint16_t biPlanes;
    uint16_t biBitCount;
    uint32_t biCompression;
    uint32_t biSizeImage;
    int32_t biXPelsPerMeter;
    int32_t biYPelsPerMeter;
    uint32_t biClrUsed;
    uint32_t biClrImportant;
  } info;
#pragma pack(pop)

  if (sizeof(header) != 14) {
    serial.println(F("Headerpanic"));
    serial.println(sizeof(header));
  }
  if (sizeof(info) != 40) {
    serial.println(F("Infopanic"));
  }

  f.read(reinterpret_cast<byte*>(&header), sizeof(header));

  if (header.bfType == 0x4D42) { // "BM" 0x42 0x4D

    f.read(reinterpret_cast<byte*>(&info), sizeof(info));

    if ( (info.biWidth <= 64) && (info.biHeight <= 48) && (info.biBitCount == 8) && (info.biCompression == 0) && (info.biClrUsed < 256) ) {
      // etwas Aufwand, um Speicherfehler zu vermeiden
      struct color_t {
        byte r, g, b, u;
      };
      int cts = 256; // maximalgröße
      if ( (info.biClrUsed != 0) && (info.biClrUsed < 256) ) {
        cts = info.biClrUsed;
      }
      color_t * colors = new color_t[cts];
      f.read(reinterpret_cast<byte*>(colors), cts * sizeof(color_t));

      byte xofs = (64 - info.biWidth) / 2;
      byte yofs = (48 - info.biHeight) / 2;
      /*
        serial.println("xofs: " + String(xofs));
        serial.println("yofs: " + String(yofs));
      */

      f.seek(header.bfOffBits, SeekSet);

      // Zeilenlänge
      // BI_RGB
      // Jede Bildzeile ist durch rechtsseitiges Auffüllen mit Nullen auf ein ganzzahliges
      // Vielfaches von 4 Bytes ausgerichtet
      int zlen = (info.biWidth % 4 == 0) ? info.biWidth : info.biWidth + (4 - info.biWidth % 4);

      // Zeilenweises lesen
      byte * line = new byte[zlen];

      int y = 0;
      while (f.available() > 0) {
        f.read((uint8_t *)line, zlen);
        for (int x = 0; x < info.biWidth; x++) {
          int color = ((int)colors[line[x]].r + (int)colors[line[x]].g + (int)colors[line[x]].b) / 3;
          if (color < 127) {
            oleddisplay->drawPixel(x + xofs, 47 - (y + yofs), WHITE);
          }
        }
        y++;
      }

      delete colors;
      delete line;
    } else {
      serial.print(F("Panic: BMP-Format-Fehler in "));
      serial.print(filename);
      serial.print(" w*h*depth*compression*colors: ");
      serial.print(info.biWidth);
      serial.print("*");
      serial.print(info.biHeight);
      serial.print("*");
      serial.print(info.biBitCount);
      serial.print("*");
      serial.print(info.biCompression);
      serial.print("*");
      serial.print(info.biClrUsed);
      serial.println();
    }
  }

  f.close();
}
