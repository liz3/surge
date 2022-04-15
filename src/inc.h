#pragma once
#include <iostream>
#include <vector>
#include <windows.h>
#include <Winuser.h>
#include <Shellapi.h>
#include <wingdi.h>
#include <gdiplus.h>
#include <gdiplusheaders.h>
#include <tesseract/baseapi.h>
#include <leptonica/allheaders.h>
#include <thread>
#include <chrono>
#include <algorithm>
#include <cctype>
#include <locale>
#pragma comment(lib, "Gdiplus.lib")
#pragma comment(lib, "winmm.lib")
using namespace Gdiplus;
const char * lpszClass = "__hidden__";
bool g_stop = false;
UINT
const WMAPP_NOTIFYCALLBACK = WM_APP + 1;
static inline void ltrim(std::string & s) {
  s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
    return !std::isspace(ch);
  }));
}
static inline void rtrim(std::string & s) {
  s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
    return !std::isspace(ch);
  }).base(), s.end());
}

static inline void trim(std::string & s) {
  ltrim(s);
  rtrim(s);
}
struct Entry {
  uint8_t r, g, b;
};

Entry createEntry(uint8_t r, uint8_t g, uint8_t b) {
  Entry e;
  e.r = r;
  e.g = g;
  e.b = b;
  return e;
}
bool isSameColor(Entry entry, Color color) {
  return entry.r == color.GetR() && entry.g == color.GetG() && entry.b == color.GetB();
}
int GetEncoderClsid(const WCHAR * format, CLSID * pClsid) {
  UINT num = 0;
  UINT size = 0;
  ImageCodecInfo * pImageCodecInfo = NULL;
  GetImageEncodersSize( & num, & size);
  if (size == 0)
    return -1;

  pImageCodecInfo = (ImageCodecInfo * )(malloc(size));
  if (pImageCodecInfo == NULL)
    return -1; // Failure

  GetImageEncoders(num, size, pImageCodecInfo);
  for (UINT j = 0; j < num; ++j) {
    if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0) {
      * pClsid = pImageCodecInfo[j].Clsid;
      free(pImageCodecInfo);
      return j;
    }
  }
  free(pImageCodecInfo);
  return -1; 
}
void playWavSound(const char * path) {
  PlaySound(TEXT(path), NULL, SND_FILENAME | SND_ASYNC);
}
Color getPixel(int x, int y, Bitmap * map) {
  Color color;
  map -> GetPixel(x, y, & color);
  return color;
}
void savePng(const WCHAR * path, Bitmap * map) {
  CLSID pngClsid;
  int result = GetEncoderClsid(L"image/png", & pngClsid);
  map -> Save(path, & pngClsid, NULL);
}