#include "inc.h"

using namespace Gdiplus;

HBITMAP doScreenshot(HDC hdc) {
  int nScreenWidth = 3840;
  int nScreenHeight = 2160;
  HDC hCaptureDC = CreateCompatibleDC(hdc);
  HBITMAP hBitmap = CreateCompatibleBitmap(hdc, nScreenWidth, nScreenHeight);
  HGDIOBJ hOld = SelectObject(hCaptureDC, hBitmap);
  BOOL bOK = BitBlt(hCaptureDC, 0, 0, nScreenWidth, nScreenHeight, hdc, 0, 0, SRCCOPY | CAPTUREBLT);
  SelectObject(hCaptureDC, hOld);
  DeleteDC(hCaptureDC);
  return hBitmap;
}
int getStart(Bitmap * bmp,
  const std::vector < Entry > * search) {
  for (int line = 0; line < bmp -> GetWidth(); line++) {
    for (int col = 0; col < bmp -> GetHeight(); col++) {
      bool same = true;
      for (int i = 0; i < search -> size(); i++) {
        if (col + i == bmp -> GetHeight()) {
          same = false;
          break;
        }
        if (!isSameColor(( * search)[i], getPixel(line, col + i, bmp))) {
          same = false;
          break;
        }
      }
      if (same) {
        return line - 13;
      }
    }
  }
  return -1;
}
int getValue(Bitmap * bmp, tesseract::TessBaseAPI * api,
  const std::vector < Entry > * search) {
  int xStart = 1250;
  int yStart = 1830;
  int xEnd = 650;
  int yEnd = 80;
  int offset = 50;
  auto outMap = Bitmap(xEnd, yEnd);
  for (int x = xStart; x < xStart + xEnd; x++) {
    for (int y = yStart; y < yStart + yEnd; y++) {
      outMap.SetPixel(x - xStart, y - yStart, getPixel(x, y, bmp));
    }
  }
  auto outMap2 = Bitmap(xEnd, offset);
  for (int x = 0; x < xEnd; x++) {
    for (int y = 0; y < offset; y++) {
      outMap2.SetPixel(x, y, getPixel(x, y + (yEnd - offset), & outMap));
    }
  }
  int indexFound = getStart( & outMap2, search);
  if (indexFound != -1) {
    auto * rawData = new uint8_t[30 * 55 * 3];
    for (int y = 0; y < 30; y++) {
      for (int x = 0; x < 55; x++) {
        int target = (55 * y + x) * 3;
        Color c = getPixel(x + indexFound - 5, y, & outMap);
        if (c.GetG() > c.GetR() && c.GetG() > c.GetB()) {
          c = Color(0, 0, 0);
        } else {
          c = Color(255, 255, 255);
        }
        rawData[target] = c.GetR();
        rawData[target + 1] = c.GetG();
        rawData[target + 2] = c.GetB();

      }
    }

    api -> SetImage(rawData, 55, 30, 3, 55 * 3);
    int value = -2;
    const char * content = api -> GetUTF8Text();
    if (content) {
      const std::string str(content);
      int xIndex = str.find("X");
      if (xIndex > -1) {
        std::string sub = str.substr(xIndex + 1);
        trim(sub);
        bool save = true;
        for (char c: sub) {
          if (c < '0' || c > '9') {
            save = false;
            break;
          }
        }
        if (save)
          value = std::stoi(sub);
      }
      delete[] content;
    }
    delete[] rawData;
    return value;
  }

  return -1;
}
LRESULT CALLBACK WndProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam) {
  static NOTIFYICONDATA nid;
  switch (iMsg) {
  case WM_CREATE:
    std::memset( & nid, 0, sizeof(nid));
    nid.cbSize = sizeof(nid);
    nid.hWnd = hWnd;
    nid.uID = 0;
    nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    nid.uCallbackMessage = WM_APP + 1;
    nid.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    strcpy(nid.szTip, "Surge tool");
    Shell_NotifyIcon(NIM_ADD, & nid);
    Shell_NotifyIcon(NIM_SETVERSION, & nid);
    return 0;
  case WM_APP + 1:
    switch (lParam) {
    case WM_LBUTTONDBLCLK:
      Shell_NotifyIcon(NIM_DELETE, & nid);
      PostQuitMessage(0);
      g_stop = true;
      break;
    case WM_RBUTTONDOWN:
    case WM_CONTEXTMENU:

      break;
    }
    break;
  case WM_DESTROY:
    Shell_NotifyIcon(NIM_DELETE, & nid);
    PostQuitMessage(0);
    return 0;
  }
  return DefWindowProc(hWnd, iMsg, wParam, lParam);
}
void startLoop() {
  GdiplusStartupInput gdiplusStartupInput;
  ULONG_PTR gdiplusToken;
  const std::vector < Entry > searchEntries = {
    createEntry(19, 53, 15),
    createEntry(6, 23, 33),
    createEntry(12, 43, 63),
    createEntry(16, 51, 81),
    createEntry(16, 51, 81),
    createEntry(21, 83, 124),
    createEntry(30, 114, 164)
  };
  tesseract::TessBaseAPI * api = new tesseract::TessBaseAPI();
  if (api -> Init("assets/eng", "eng"))
    std::cerr << "init err\n";
  int lastValue = -1;
  GdiplusStartup( & gdiplusToken, & gdiplusStartupInput, NULL);
  HDC hScreenDC = GetDC(nullptr);
  while (!g_stop) {
    auto bm = doScreenshot(hScreenDC);
    auto bx = Bitmap::FromHBITMAP(bm, nullptr);
    int value = getValue(bx, api, & searchEntries);
    DeleteObject(bm);
    delete bx;
    if (lastValue > 0 && value > lastValue) {
      std::cout << "Stacks: " << value << "\n";
      switch (value) {
      case 18:
      case 19:
        playWavSound("assets/s1.wav");
        break;
      case 20:
        playWavSound("assets/s2.wav");
        break;
      default:
        break;
      }
    }
    if (value != -2)
      lastValue = value;
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
  }
  api -> End();
}
int main() {
  ShowWindow(GetConsoleWindow(), SW_HIDE);
  std::thread loop_thread( & startLoop);
  HINSTANCE hInstance = GetModuleHandle(nullptr);
  WNDCLASS wc;
  HWND hWnd;
  MSG msg;
  wc.cbClsExtra = 0;
  wc.cbWndExtra = 0;
  wc.hbrBackground = nullptr;
  wc.hCursor = nullptr;
  wc.hIcon = nullptr;
  wc.hInstance = hInstance;
  wc.lpfnWndProc = WndProc;
  wc.lpszClassName = lpszClass;
  wc.lpszMenuName = nullptr;
  wc.style = 0;
  RegisterClass( & wc);

  hWnd = CreateWindow(lpszClass, lpszClass, WS_OVERLAPPEDWINDOW,
    CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
    nullptr, nullptr, hInstance, nullptr);

  while (GetMessage( & msg, nullptr, 0, 0)) {
    TranslateMessage( & msg);
    DispatchMessage( & msg);
  }
  loop_thread.join();
  return 0;
}