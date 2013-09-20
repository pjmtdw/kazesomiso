#ifndef _CLASSES_H_
#define _CLASSES_H_

#include <windows.h>
#include <vector>
#include <string>
#include <gdiplus.h>

enum TORIYOMI {TORI,YOMI};

struct MYRECT{
  double top;
  double left;
  double bottom;
  double right;
};

class BackStructure{
public:
  HBITMAP hbmp;
  HDC hdc;
  Gdiplus::Bitmap * gbmp;
  BackStructure() : hbmp(NULL), hdc(NULL), gbmp(NULL){
  }
};

class torifuda{
public:
  std::string kimariji;
  std::string kaminoku;
  std::string simonoku;
  std::string explanation;
  BOOL nigatefuda;
  BOOL saveasimage;
  torifuda():
   nigatefuda(TRUE),
   saveasimage(TRUE)
  {
  }
};

class yomifuda{
public:
  std::string waka;
  BOOL saveasimage;
};
class BaseDesign{
  public:
  int width;
  int height;
  int margin_left;
  int margin_right;
  int margin_bottom;
  int border_top_bottom_width;
  int border_left_right_width;
  COLORREF background_color;
  COLORREF border_color;
  COLORREF font_color;
  double aspect_ratio;
  char background_image[MAX_PATH];
  LOGFONT logfont;
  int bgimage_mode;
  int bgimage_mojioutline;
  int bgimage_transparency;
  BaseDesign():
    width(53*6),
    height(74*6),
    margin_left(20),
    margin_right(20),
    margin_bottom(20),
    border_top_bottom_width(10),
    border_left_right_width(10),
    background_color( RGB(255,255,255) ),
    border_color(RGB(106,172,106) ),
    font_color(RGB(0,0,0)),
    bgimage_mode(0),
    bgimage_mojioutline(0),
    bgimage_transparency(0)
   {
    background_image[0] = '\0';
    aspect_ratio = double(width) / double(height);
    ;
   };
};

class ToriDesign : public BaseDesign{
  public:
  int margin_top;
  int kimariji_sukima_h;
  int kimariji_sukima_v;
  int kimariji_margin_h;
  int kimariji_margin_v;
  BOOL convert_dakuon;
  COLORREF kimariji_color;
  LOGFONT logfont_of_kimariji;
  BOOL show_kimariji;

  ToriDesign():
    margin_top(22),
    kimariji_sukima_h(5),
    kimariji_sukima_v(5),
    kimariji_margin_h(25),
    kimariji_margin_v(25),
    convert_dakuon(TRUE),
    kimariji_color(RGB(255,81,168)),
    show_kimariji(FALSE)
  {
    ZeroMemory(&logfont,sizeof(logfont));
    logfont.lfHeight = -64;
    logfont.lfWeight = FW_NORMAL;
    logfont.lfCharSet = SHIFTJIS_CHARSET;
    logfont.lfQuality = ANTIALIASED_QUALITY;
    lstrcpy(logfont.lfFaceName,"‚l‚r –¾’©");
    ZeroMemory(&logfont_of_kimariji,sizeof(logfont_of_kimariji));
    logfont_of_kimariji.lfHeight = -64;
    logfont_of_kimariji.lfWeight = FW_NORMAL;
    logfont_of_kimariji.lfCharSet = SHIFTJIS_CHARSET;
    logfont_of_kimariji.lfQuality = ANTIALIASED_QUALITY;
    lstrcpy(logfont_of_kimariji.lfFaceName,"‚l‚r ƒSƒVƒbƒN");
    margin_left = 22;
    margin_right = 22;
    margin_bottom = 22;
  }
};


class YomiDesign: public BaseDesign{
  public:
  int margin_top[16];
  int furigana_sukima;
  COLORREF furigana_color;
  LOGFONT logfont_of_furigana;
  int show_furigana;

  YomiDesign():
    furigana_sukima(-1),
    furigana_color(RGB(0,0,0)),
    show_furigana(1)
  {
    ZeroMemory(&logfont,sizeof(logfont));
    logfont.lfHeight = -36;
    logfont.lfWeight = FW_NORMAL;
    logfont.lfCharSet = SHIFTJIS_CHARSET;
    logfont.lfQuality = ANTIALIASED_QUALITY;
    logfont.lfEscapement = 2700;
    logfont.lfOrientation = 2700;
    lstrcpy(logfont.lfFaceName,"@‚l‚r ‚o–¾’©");
    ZeroMemory(&logfont_of_furigana,sizeof(logfont_of_furigana));
    logfont_of_furigana.lfHeight = -16;
    logfont_of_furigana.lfWeight = FW_NORMAL;
    logfont_of_furigana.lfCharSet = SHIFTJIS_CHARSET;
    logfont_of_furigana.lfQuality = ANTIALIASED_QUALITY;
    logfont_of_furigana.lfEscapement = 2700;
    logfont_of_furigana.lfOrientation = 2700;
    lstrcpy(logfont_of_furigana.lfFaceName,"@‚l‚r ƒSƒVƒbƒN");
    margin_top[0] = 40;
    margin_top[1] = 70;
    margin_top[2] = 110;
    margin_top[3] = 65;
    margin_top[4] = 135;
    for(int i = 5; i < sizeof(margin_top) / sizeof(*margin_top); i++){
      margin_top[i] = 1;
    }
  }
};

class ToriYomiDesign{
public:
  class ToriDesign * tori;
  class YomiDesign * yomi;
  int width;
  int height;
  std::string simonoku;
  std::string kimariji;
  std::string waka;
  void UpdateSize();
};

class MyGraphics : public Gdiplus::Graphics{
  void Init();
public:
  MyGraphics(HDC hdc):Gdiplus::Graphics(hdc){
    Init();
  };
  MyGraphics(Gdiplus::Bitmap * b):Gdiplus::Graphics(b){
    Init();
  };
};
#endif //_CLASSES_H_
