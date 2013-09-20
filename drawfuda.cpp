// drawfuda.cc

#include "kazesomiso.h"

//全角文字かどうか判定。SHIFT_JIS 限定
BOOL isZenkaku(unsigned char c){
  int cc = c;
  if(cc >= 0x81 && cc <= 0x9f || cc >= 0xe0 && cc <= 0xfc){
    return TRUE;
  }else{
    return FALSE;
  }
}
//漢字かどうか判定。SHIFT_JIS 限定
BOOL isKanji(unsigned char c){
  int cc = c;
  if( cc >= 0x88 && cc <= 0x9f || cc >= 0xe0 && cc <= 0xfc ){
    return TRUE;
  }else{
    return FALSE;
  }
}

const int yomimaxrownum = 9;

struct Furigana{
  int begin_pos;
  int end_pos;
  std::string furigana;
  struct MYRECT rect;
};

std::vector<std::string> ParseSimonoku(std::string);
std::vector<std::string> ParseYomi(std::string);
std::string ParseFurigana(const std::string &s,std::vector<struct Furigana> & );
void MyDrawTextToCenter(int,const HDC &, const char *, int, struct MYRECT *,BOOL,Gdiplus::GraphicsPath *);
void MyDrawTextToTop(const HDC &, std::string, struct MYRECT *, BOOL, Gdiplus::GraphicsPath *,int,std::vector<class Furigana> *);

void DrawBorder(const class BaseDesign *, const HDC &, const MYRECT &);
void DrawBackGround(const class BaseDesign *, const HDC &, const MYRECT &);
void DrawSimonoku(const class ToriDesign *, const HDC &,const MYRECT &,const std::string &, Gdiplus::GraphicsPath * );
void DrawKimariji(const class ToriDesign *, const HDC &, const MYRECT &, const std::string &, Gdiplus::GraphicsPath * );
void DrawBgimage(const class BaseDesign *, const HDC &,const MYRECT &,const std::string &);

void DrawYomi(const class YomiDesign *, const HDC &,const MYRECT &, const std::string &, Gdiplus::GraphicsPath *,std::vector<struct Furigana> *);
void DrawFurigana(const class YomiDesign *, const HDC &, Gdiplus::GraphicsPath *,const std::vector<struct Furigana> *);
void Myrect2Rect(const MYRECT & mr, RECT & r){
  r.left = (int)(mr.left);
  r.top  = (int)(mr.top);
  r.right= (int)(mr.right);
  r.bottom = (int)(mr.bottom);
}

// 枠を描く
void DrawBorder(const class BaseDesign * design, const HDC & hdc, const MYRECT & rect_prev){
  if(design->border_top_bottom_width <= 0 && design->border_left_right_width <= 0){
    return;
  }

  MYRECT rect;
  memcpy(&rect,&rect_prev,sizeof(MYRECT));
  if(!gdiplus_useable){
    RECT r;
    Myrect2Rect(rect,r);
    HBRUSH hbr = CreateSolidBrush(design->border_color);
    FillRect(hdc,&r,hbr);
    DeleteObject(hbr);
  }else{

    // //太い方の幅のペン
    // double penw = 0.0;
    // if(design->border_top_bottom_width > design->border_left_right_width){
    //   penw = design->border_top_bottom_width;
    // }else{
    //   penw = design->border_left_right_width;
    // }
    // Gdiplus::RectF rct(rect.left+penw/2.0,rect.top+penw/2.0,rect.right-rect.left-penw,rect.bottom-rect.top-penw);
    // Gdiplus::Color clr;
    // clr.SetFromCOLORREF(design->border_color);
    // Gdiplus::Pen pen(clr,penw);
    // MyGraphics g(hdc);
    // g.DrawRectangle(&pen,rct);
    
    Gdiplus::Rect rct(rect.left,rect.top,rect.right-rect.left,rect.bottom-rect.top);
    Gdiplus::Color clr;
    clr.SetFromCOLORREF(design->border_color);
    Gdiplus::SolidBrush br(clr);
    MyGraphics g(hdc);
    g.FillRectangle(&br,rct);

  }
}
// 背景色を描く
void DrawBackGround(const class BaseDesign * design, const HDC & hdc, const MYRECT & rect_prev){
  MYRECT rect;
  memcpy(&rect,&rect_prev,sizeof(MYRECT));
  rect.bottom -= design->border_top_bottom_width;
  rect.top += design->border_top_bottom_width;
  rect.right -= design->border_left_right_width;
  rect.left += design->border_left_right_width;

  if(!gdiplus_useable){
    RECT r;
    Myrect2Rect(rect,r);
    HBRUSH hbr = CreateSolidBrush(design->background_color);
    FillRect(hdc,&r,hbr);
    DeleteObject(hbr);
  }else{
    Gdiplus::Rect rct(rect.left,rect.top,rect.right-rect.left,rect.bottom-rect.top);
    Gdiplus::Color clr;
    clr.SetFromCOLORREF(design->background_color);
    Gdiplus::SolidBrush br(clr);
    MyGraphics g(hdc);
    g.FillRectangle(&br,rct);
  }
}
// 下の句を描く
void DrawSimonoku(const class ToriDesign * design, const HDC & hdc,const MYRECT & rect_prev, const std::string & simonoku, Gdiplus::GraphicsPath * gpath){
  if(simonoku.length() == 0){
    return;
  }
  const char * DAKUON = "が ぎ ぐ げ ご ざ じ ず ぜ ぞ だ ぢ づ で ど ば び ぶ べ ぼ ぱ ぴ ぷ ぺ ぽ";
  const char * SEION =  "か き く け こ さ し す せ そ た ち つ て と は ひ ふ へ ほ は ひ ふ へ ほ";
  MYRECT rect;
  memcpy(&rect,&rect_prev,sizeof(MYRECT));
  rect.bottom -= design->border_top_bottom_width;
  rect.bottom -= design->margin_bottom;
  rect.top += design->border_top_bottom_width;
  rect.top += design->margin_top;
  rect.right -= design->border_left_right_width;
  rect.right -= design->margin_right;
  rect.left += design->border_left_right_width;
  rect.left += design->margin_left;
  HFONT hf = CreateFontIndirect(&(design->logfont));
  HFONT hf_old = (HFONT)SelectObject(hdc,hf);
  int ONEROW = 5; // 一行の文字数 「いまこ」だけ特別なのでconstではない
  const int MAXROW = 3; // 全部で何行か
  std::vector<std::string> rsimonoku = ParseSimonoku(simonoku);
  double next_x = rect.right;
  double next_y = rect.top;
  COLORREF color_old = SetTextColor(hdc,design->font_color);
  if(color_old == CLR_INVALID){
    color_old = RGB(0,0,0);
  }
  int cols = 0;
  for(std::string::size_type i = 0; i < rsimonoku.size(); i++){
    struct MYRECT r;
    if( (rsimonoku.size() >= 16 && i >= 10) || (rsimonoku.size() >= 17 && i >= 5) || rsimonoku.size() >= 18){
      ONEROW = 6; // 「いまこ」は特別
    }
    double width = (rect.right - rect.left) / (double)MAXROW;
    double height = (rect.bottom - rect.top) / (double)ONEROW;
    r.right = next_x;
    r.top = next_y;
    r.left = r.right - width;
    r.bottom = r.top + height;
    std::string s = rsimonoku[i];
    if(design->convert_dakuon){
      char * p = (char *)strstr(DAKUON,s.c_str());
      if(p != NULL){
        s = SEION + (p - DAKUON);
        s = s.substr(0,2);
      }
    }
    MyDrawTextToCenter(255,hdc,s.c_str(),(int)s.length(),&r,design->logfont.lfFaceName[0] == '@',gpath);
    next_y += height;
    if( cols >= ONEROW -1){
      next_y = rect.top;
      next_x -= width;
      cols = 0;
    }else{
      cols++;
    }
  }
  SelectObject(hdc,hf_old);
  DeleteObject(hf);
  SetTextColor(hdc,color_old);
}

// 読み札の文字を描く
void DrawYomi(const class YomiDesign * design, const HDC & hdc,const MYRECT & rect_prev, const std::string & yomi, Gdiplus::GraphicsPath * gpath, std::vector<struct Furigana> * furigana){
  MYRECT rect;
  memcpy(&rect,&rect_prev,sizeof(MYRECT));
  rect.bottom -= design->border_top_bottom_width;
  rect.top += design->border_top_bottom_width;
  rect.right -= design->border_left_right_width;
  rect.right -= design->margin_right;
  rect.left += design->border_left_right_width;
  rect.left += design->margin_left;

  HFONT hf = CreateFontIndirect(&(design->logfont));
  HFONT hf_old = (HFONT)SelectObject(hdc,hf);

  std::vector<std::string> ryomi = ParseYomi(yomi);

  int cols = 5;
  COLORREF color_old = SetTextColor(hdc,design->font_color);
  if(color_old == CLR_INVALID){
    color_old = RGB(0,0,0);
  }

  for(int i = 0; i < ryomi.size(); i++){
    int margin_top;
    if(i >= yomimaxrownum){
      margin_top = design->margin_top[yomimaxrownum-1];
    }else{
      margin_top = design->margin_top[i];
    }

    std::vector<class Furigana> furi;
    std::string b = ParseFurigana(ryomi[i],furi);
    
    double width = (rect.right - rect.left) / double(ryomi.size());
    double height = (rect.bottom - rect.top);

    double x = rect.right - width * ( i + 1);
    struct MYRECT r;
    r.left = x;
    r.right = x + width;

    int align = 0;
    if(margin_top > 0){
      //上揃え
      r.top = rect.top + margin_top; 
      r.bottom = rect.bottom; 
      align = 0;
    }else if(margin_top == 0){
      //中央揃え
      r.top = rect.top;
      r.bottom = rect.bottom;
      align = 1;
    }else{
      //下揃え
      r.top = rect.top;
      r.bottom = rect.bottom + margin_top;
      align = 2;
    }
    if(furigana != NULL){
      MyDrawTextToTop(hdc, b, &r,design->logfont.lfFaceName[0] == '@', gpath,align,&furi);
      furigana->insert(furigana->begin(),furi.begin(),furi.end());
    }else{
      MyDrawTextToTop(hdc, b, &r,design->logfont.lfFaceName[0] == '@', gpath,align,NULL);
    }
  }
  SelectObject(hdc,hf_old);
  DeleteObject(hf);
  SetTextColor(hdc,color_old);

}


void DrawFurigana(const class YomiDesign * design, const HDC & hdc, Gdiplus::GraphicsPath * gpath,const std::vector<struct Furigana> * furigana){
  if(furigana == NULL){
    return;
  }

  HFONT hftest = CreateFontIndirect(&(design->logfont_of_furigana));
  HFONT fontold = (HFONT)SelectObject(hdc,hftest);
  SIZE s;
  GetTextExtentPoint32(hdc,"た",2,&s);
  COLORREF color_old = SetTextColor(hdc,design->furigana_color);
  if(color_old == CLR_INVALID){
    color_old = RGB(0,0,0);
  }
  int width;
  if(design->logfont_of_furigana.lfFaceName[0] == '@'){
    width = s.cy;
  }else{
    width = s.cx; 
  }
  for(int i = 0; i < furigana->size(); i++){
    struct MYRECT r;
    r.left = design->furigana_sukima + (*furigana)[i].rect.right;
    r.top = (*furigana)[i].rect.top;
    r.bottom = (*furigana)[i].rect.bottom;
    r.right = r.left + width;
    MyDrawTextToTop(hdc, (*furigana)[i].furigana, &r, design->logfont_of_furigana.lfFaceName[0] == '@', gpath, 1 , NULL);


  }
  SelectObject(hdc,fontold);
  DeleteObject(hftest);
  SetTextColor(hdc,color_old);

}


// 決まり字を描く
void DrawKimariji(const class ToriDesign * design, const HDC & hdc, const MYRECT & rect, const std::string & kimariji, Gdiplus::GraphicsPath * gpath){
  if(kimariji.length() == 0){
    return;
  }
  std::string::size_type pos = kimariji.find(' ');
  std::string s;
  if(pos == std::string::npos){
    s = kimariji;
  }else{
    s = kimariji.substr(0,pos);
  }
  std::vector<std::string> ss = ParseSimonoku(s);
  const int WARIATE[6][3] = {
  { 1 , 0 , 0 }, // 1字決まりなら1文字そのまま表示
  { 2 , 0 , 0 },
  { 3 , 0 , 0 },
  { 2 , 2 , 0 }, // 4字決まりなら2文字ずつに分けて二行に表示
  { 2 , 3 , 0 },
  { 3 , 3 , 0 }
  };
  if( ss.size() > (sizeof(WARIATE) / sizeof(*WARIATE))){
    return;
  }
  int wariate[3];
  memcpy(wariate,WARIATE[ss.size()-1],sizeof(wariate));
  int cols = 0;
  int rows = 0;
  for(int i = 0; i < sizeof(wariate)/sizeof(*wariate); i++){
    if(wariate[i] > 0){
      rows ++;
    }
    if(wariate[i] > cols){
      cols = wariate[i];
    }
  }
  double width = (rect.right - rect.left) - design->kimariji_margin_h * 2;
  double height = (rect.bottom - rect.top) - design->kimariji_margin_v * 2;
  double onemoji_width = (width - design->kimariji_sukima_h * (rows-1)) / rows;
  double onemoji_height = (height - design->kimariji_sukima_v * (cols-1)) / cols;
  // 大きすぎる場合はフォントを小さくする
  LOGFONT lf;
  memcpy(&lf,&(design->logfont_of_kimariji),sizeof(LOGFONT));
  lf.lfWidth = 0;
  int fontheight = onemoji_height;
  for(int i=0;i<5;i++){ // 5回で挫折
    lf.lfHeight = fontheight;
    HFONT hftest = CreateFontIndirect(&lf);
    HFONT fontold = (HFONT)SelectObject(hdc,hftest);
    SIZE s;
    GetTextExtentPoint32(hdc,"た",2,&s);
    SelectObject(hdc,fontold);
    DeleteObject(hftest);
    int newheight = s.cy;
    int newwidth = s.cx;
    if(newwidth > onemoji_width){
      double ratio = (double(newheight) / double(newwidth));
      int fontheight1 =  int(onemoji_width * ratio);
      int fontheight2 = int(onemoji_height / ratio);
      fontheight = (fontheight1 > fontheight2) ? fontheight2 : fontheight1;
    }else{
      break;
    }
  }
  HDC h;
  HBITMAP hbmp,hold;
  HFONT hfold;
  COLORREF txtold,bgold;
  lf.lfHeight = fontheight;
  HFONT hf = CreateFontIndirect(&lf);
  if(!gdiplus_useable){
    MYRECT r1;
    r1.left = 0;
    r1.top = 0;
    r1.right = width;
    r1.bottom = height;
    RECT r2;
    Myrect2Rect(r1,r2);

    h = CreateCompatibleDC(hdc);
    hbmp = CreateCompatibleBitmap(hdc,width,height);
    hold = (HBITMAP)SelectObject(h,hbmp);
    hfold = (HFONT)SelectObject(h,hf);
    SetTextColor(h,design->kimariji_color);
    SetBkColor(h,RGB(255,255,255));
    HBRUSH hbr = (HBRUSH) CreateSolidBrush(RGB(255,255,255));
    FillRect(h,&r2,hbr);
    DeleteObject(hbr);
  }else{
    hfold = (HFONT)SelectObject(hdc,hf);
    txtold = SetTextColor(hdc,design->kimariji_color);
    bgold = SetBkColor(hdc,RGB(255,255,255));
  }
  

  int n = 0;
  for(int i = 0; i < sizeof(wariate)/sizeof(*wariate); i++){
    for(int j = 0; j < wariate[i]; j ++){
      if(n >= (int)ss.size()){
        break;
      }
      double x,y;
      if(!gdiplus_useable){
        x = width - i * onemoji_width - i * design->kimariji_sukima_h;
        y = j * onemoji_height + j * design->kimariji_sukima_v;
        struct MYRECT r;
        r.right = x;
        r.top = y;
        r.left = x - onemoji_width;
        r.bottom = y + onemoji_height;
        MyDrawTextToCenter(100,h,ss[n].c_str(),(int)ss[n].length(),&r,design->logfont_of_kimariji.lfFaceName[0] == '@',gpath);
      }else{
        x = rect.left + width - i * onemoji_width - i * design->kimariji_sukima_h;
        y = rect.top + j * onemoji_height + j * design->kimariji_sukima_v;
        struct MYRECT r;
        r.right = x + design->kimariji_margin_h;
        r.top = y + design->kimariji_margin_v;
        r.left = r.right - onemoji_width;
        r.bottom = r.top + onemoji_height;
        MyDrawTextToCenter(100,hdc,ss[n].c_str(),(int)ss[n].length(),&r,design->logfont_of_kimariji.lfFaceName[0] == '@',gpath);
      }
      n++;
    }
  }
  if(!gdiplus_useable){
    BitBlt(hdc,rect.left + design->kimariji_margin_h, rect.top + design->kimariji_margin_v,width,height,h,0,0,SRCAND);
  }
  if(!gdiplus_useable){
    SelectObject(h,hold);
    SelectObject(h,hfold);
    DeleteDC(h);
    DeleteObject(hbmp);
  }else{
    SelectObject(hdc,hfold);
    SetTextColor(hdc,txtold);
    SetBkColor(hdc,bgold);
  }
  DeleteObject(hf);
}

// 背景画像の設定
void DrawBgimage(const class BaseDesign * design, const HDC & hdc, const MYRECT & rect,const std::string &s){
  if(bgimage_structure.hdc == NULL){
    return;
  }
  double x = rect.left + design->border_left_right_width;
  double y = rect.top + design->border_top_bottom_width;
  double w = rect.right - rect.left - design->border_left_right_width*2;
  double h = rect.bottom - rect.top - design->border_top_bottom_width*2;

  HBITMAP hold;
  if(!gdiplus_useable){
    BITMAP bmp;
    GetObject(bgimage_structure.hbmp,sizeof(BITMAP),&bmp);
    int bw = bmp.bmWidth;
    int bh = bmp.bmHeight;

    hold = (HBITMAP) SelectObject(bgimage_structure.hdc,bgimage_structure.hbmp);
    if( w == bw && h == bh){
      BitBlt(hdc,x,y,w,h,bgimage_structure.hdc,0,0,SRCCOPY);
    }else{
      SetStretchBltMode(hdc,HALFTONE);
      StretchBlt(hdc,x,y,w,h,bgimage_structure.hdc,0,0,bw,bh,SRCCOPY);
    }
  }else{
    Gdiplus::Bitmap * b = bgimage_structure.gbmp;
    MyGraphics g(hdc);
    Gdiplus::REAL xx = x;
    Gdiplus::REAL yy = y;
    Gdiplus::REAL ww = w;
    Gdiplus::REAL hh = h;
    g.DrawImage(b,xx,yy,ww,hh);
  }
  if(design->bgimage_mojioutline ){
    
    double basicpensize = h / 65.0;
    const double bairitu[] = {
      0.0, 0.3, 0.5 , 1.0 , 1.5, 2.0
    };
    int index = design->bgimage_mojioutline;
    if(design->bgimage_mojioutline >= sizeof(bairitu) / sizeof(double)){
      index = sizeof(bairitu) / sizeof(double) - 1;
    }
    if(!gdiplus_useable){
      int pensize = int(basicpensize * bairitu[index]);
      HPEN hpen = CreatePen(PS_SOLID,pensize,design->background_color);
      HBRUSH hbr = CreateSolidBrush(design->background_color);
      HPEN hpold = (HPEN)SelectObject(hdc,hpen);
      HBRUSH hbold = (HBRUSH)SelectObject(hdc,hbr);
      BeginPath(hdc);
      if(tori_or_yomi == TORI){
        DrawSimonoku((const ToriDesign *)design,hdc,rect,s,NULL);
      }else{
        std::vector<class Furigana> v;
        DrawYomi((const YomiDesign *)design,hdc,rect,s,NULL,&v);
        if(((const YomiDesign *)design)->show_furigana == 2){
          DrawFurigana((const YomiDesign *)design, hdc, NULL,&v);
        }
      }
      EndPath(hdc);
      StrokeAndFillPath(hdc);
      SelectObject(hdc,hpold);
      SelectObject(hdc,hbold);
      DeleteObject(hpen);
      DeleteObject(hbr);
    }else{
      double pensize = basicpensize * bairitu[index];
      Gdiplus::GraphicsPath gp;
      if(tori_or_yomi == TORI){
        DrawSimonoku((const ToriDesign *)design,hdc,rect,s,&gp);
      }else{
        std::vector<class Furigana> v;
        DrawYomi((const YomiDesign *)design,hdc,rect,s,&gp,&v);
        if(((const YomiDesign *)design)->show_furigana == 2){
          DrawFurigana((const YomiDesign *)design, hdc, &gp,&v);
        }
      }
      MyGraphics g(hdc);

      Gdiplus::Color clr;
      clr.SetFromCOLORREF(design->background_color);
      Gdiplus::Pen pen(clr,pensize);
      pen.SetLineJoin(Gdiplus::LineJoinRound);

      g.DrawPath(&pen,&gp);
      Gdiplus::SolidBrush brush(clr);
      g.FillPath(&brush,&gp);
    }
  
  }
  if(!gdiplus_useable){
    SelectObject(bgimage_structure.hdc,hold);
  }
}
void DrawFuda(const class ToriYomiDesign & s_design, const HDC & hdc,MYRECT * base_rect)
{
  MYRECT rect;
  class BaseDesign * design;
  if(tori_or_yomi == TORI){
    design = s_design.tori;
  }else{
    design = s_design.yomi;
  }
  if(base_rect == NULL){
    rect.left = 0;
    rect.top = 0;
  }else{
    rect.left = base_rect->left;
    rect.top = base_rect->top;
  }
  rect.right = rect.left + design->width;
  rect.bottom = rect.top + design->height;

  class BaseDesign * bkup;
  if(!gdiplus_useable && fudascale > 0.0){
    rect.left *= fudascale_x;
    rect.right *= fudascale_x;
    rect.top *= fudascale_y;
    rect.bottom *= fudascale_y;
    if(tori_or_yomi == TORI){
      bkup = new ToriDesign();
      memcpy(bkup,design,sizeof(class ToriDesign));
    }else{
      bkup = new YomiDesign();
      memcpy(bkup,design,sizeof(class YomiDesign));
    }
    design->margin_left *= fudascale_x;
    design->margin_right *= fudascale_x;
    design->margin_bottom *= fudascale_y;
    design->logfont.lfHeight *= fudascale_y;
    design->border_left_right_width *= fudascale_x;
    design->border_top_bottom_width *= fudascale_y;
    if(tori_or_yomi == TORI){
      class ToriDesign * p = (class ToriDesign *)design;
      p->margin_top *= fudascale_y;
      p->kimariji_sukima_h *= fudascale_x;
      p->kimariji_sukima_v *= fudascale_y;
      p->kimariji_margin_h *= fudascale_x;
      p->kimariji_margin_v *= fudascale_y;
      p->logfont_of_kimariji.lfHeight *= fudascale_y;
    }else{
      class YomiDesign * p = (class YomiDesign *)design;
      for(int i = 0; i < 16; i ++){
        p->margin_top[i] *= fudascale_y;
      }
      p->furigana_sukima *= fudascale_x;
      p->logfont_of_furigana.lfHeight *= fudascale_y;
    }
  }

  DrawBorder(design,hdc,rect);
  if(!design->bgimage_mode){
    DrawBackGround(design,hdc,rect);
  }else{
    if(tori_or_yomi == TORI){
      DrawBgimage(design,hdc,rect,s_design.simonoku);
    }else{
      DrawBgimage(design,hdc,rect,s_design.waka);
    }
  }
  if(tori_or_yomi == TORI){
    DrawSimonoku((const ToriDesign *)design,hdc,rect,s_design.simonoku,NULL);
    if(s_design.tori->show_kimariji){
      DrawKimariji((const ToriDesign *)design,hdc,rect,s_design.kimariji,NULL);
    }
  }else{
    std::vector<class Furigana> v;
    DrawYomi((const YomiDesign *)design,hdc,rect,s_design.waka,NULL,&v);
    if(s_design.yomi->show_furigana){
      DrawFurigana((const YomiDesign *)design, hdc, NULL,&v);
    }
  }
  if(!gdiplus_useable && fudascale > 0.0){
    if(tori_or_yomi == TORI){
      memcpy(design,bkup,sizeof(class ToriDesign));
    }else{
      memcpy(design,bkup,sizeof(class YomiDesign));
    }
    delete bkup;
  }

}

// DrawText の DT_CENTER | DT_VCENTER DT_SINGLELINE を TextOut で実現 (BeginPath, EndPathのため)
// 戻り値はフォントの高さ
void MyDrawTextToCenter(int alpha,const HDC & hdc, const char * s, int length, struct MYRECT * r, BOOL tategaki, Gdiplus::GraphicsPath * gpath){

  if(!gdiplus_useable){
    SIZE size;
    GetTextExtentPoint32(hdc,s,length,&size);
    double w,h,x,y;
    w = r->right - r->left;
    h = r->bottom - r->top;
    if ( tategaki ){
      x = r->left + (w - size.cy)/2.0 + size.cy;
      y = r->top + (h - size.cx)/2.0;
    }else{
      x = r->left + (w - size.cx)/2.0;
      y = r->top + (h - size.cy)/2.0;
    }
    int bkold = SetBkMode(hdc,TRANSPARENT);
    TextOut(hdc,x,y,s,length);
    SetBkMode(hdc,bkold);
  }else{
    WCHAR buf[16];
    MultiByteToWideChar(CP_ACP,0,s,-1,buf,sizeof(buf));
    HFONT hfont = (HFONT)GetCurrentObject(hdc,OBJ_FONT);
    Gdiplus::Font myfont(hdc,hfont);

    Gdiplus::FontFamily ff;
    myfont.GetFamily(&ff);
    //行間
    double ds = myfont.GetSize() * ff.GetCellDescent(myfont.GetStyle()) / ff.GetEmHeight(myfont.GetStyle()); 

    MyGraphics graphics(hdc);

    // 行間の分だけ右にずれるのでその分を補正
    Gdiplus::RectF rect(r->left-ds/2.0,r->top,r->right-r->left,r->bottom-r->top);
    Gdiplus::Color clr;
    clr.SetFromCOLORREF(GetTextColor(hdc));
    clr.SetValue(Gdiplus::Color::MakeARGB(alpha,clr.GetRed(),clr.GetGreen(),clr.GetBlue()));
    Gdiplus::SolidBrush brush(clr);

    Gdiplus::StringFormat format;
    format.SetAlignment(Gdiplus::StringAlignmentCenter);
    format.SetLineAlignment(Gdiplus::StringAlignmentCenter);
    format.SetFormatFlags(
      Gdiplus::StringFormatFlagsDirectionVertical 
      |Gdiplus::StringFormatFlagsNoWrap
      |Gdiplus::StringFormatFlagsNoClip
      |Gdiplus::StringFormatFlagsNoFitBlackBox
    );
    format.SetTrimming(Gdiplus::StringTrimmingNone);

    if(gpath == NULL){
      graphics.DrawString(buf,-1,&myfont,rect,&format,&brush);
    }else{
      Gdiplus::FontFamily fontfamily;
      myfont.GetFamily(&fontfamily);
      int style = myfont.GetStyle();
      double emsize = myfont.GetSize();
      gpath->AddString(buf,-1, &fontfamily,style,emsize,rect,&format);
    }
  }
}
// align = 0: top
// align = 1: center
// align = 2: bottom
void MyDrawTextToTop(const HDC & hdc, std::string s, struct MYRECT * r, BOOL tategaki, Gdiplus::GraphicsPath * gpath, int align,std::vector<class Furigana> * furi){
  if(!gdiplus_useable){
    SIZE size;
    double w,h,x,y;
    GetTextExtentPoint32(hdc,s.c_str(),s.length(),&size);
    w = r->right - r->left;
    h = r->bottom - r->top;
    if ( tategaki ){
      if(align == 0){
        y = r->top;
      }else if(align == 1){
        y = r->top + (h - size.cx)/2.0;
      }else if(align == 2){
        y = r->top + (h - size.cx);
      }
    }else{
      if(align == 0){
        y = r->top;
      }else if(align == 1){
        y = r->top + (h - size.cy*s.length()/2)/2.0; // 全て全角と仮定
      }else if(align == 2){
        y = r->top + (h - size.cy*s.length()/2);// 全て全角と仮定
      }
    }
    int bkold = SetBkMode(hdc,TRANSPARENT);

    char * p = (char *)s.c_str();
    while(*p != '\0'){
      int c = (unsigned char) * p;
      if( isZenkaku(c) ){
        //全角
        GetTextExtentPoint32(hdc,p,2,&size);
        if(tategaki){
          x = r->left + (w - size.cy)/2.0 + size.cy;
        }else{
          x = r->left + (w - size.cx)/2.0;
        }
        TextOut(hdc,x,y,p,2);
        
        if(furi != NULL){
          int pos = p - s.c_str();
          for(int i = 0; i < furi->size(); i++){
            if((*furi)[i].begin_pos == pos || (*furi)[i].begin_pos == pos + 1){
              if(tategaki){
                (*furi)[i].rect.left = x - size.cy;
                (*furi)[i].rect.top = y;
              }else{ 
                (*furi)[i].rect.left = x;
                (*furi)[i].rect.top = y;
              }
            }
            if((*furi)[i].end_pos-1 == pos || (*furi)[i].end_pos-1 == pos + 1){
              if(tategaki){
                (*furi)[i].rect.right = x;
                (*furi)[i].rect.bottom = y + size.cx;
              }else{
                (*furi)[i].rect.right = x+size.cx;
                (*furi)[i].rect.bottom = y+size.cy;
              }
            }
          }
        }
        if(tategaki){
          y += size.cx;
        }else{
          y += size.cy;
        }
        p++;
        if(*p == '\0')break;
      }else{
        //半角
        GetTextExtentPoint32(hdc,p,1,&size);
        if(tategaki){
          x = r->left + (w - size.cy)/2.0 + size.cy;
        }else{
          x = r->left + (w - size.cx)/2.0;
        }
        TextOut(hdc,x,y,p,1);
        
        if(furi != NULL){
          int pos = p - s.c_str();
          for(int i = 0; i < furi->size(); i++){
            if((*furi)[i].begin_pos == pos){
              if(tategaki){
                (*furi)[i].rect.left = x - size.cy;
                (*furi)[i].rect.top = y;
              }else{ 
                (*furi)[i].rect.left = x;
                (*furi)[i].rect.top = y;
              }
            }
            if((*furi)[i].end_pos-1 == pos){
              if(tategaki){
                (*furi)[i].rect.right = x;
                (*furi)[i].rect.bottom = y + size.cx;
              }else{
                (*furi)[i].rect.right = x+size.cx;
                (*furi)[i].rect.bottom = y+size.cy;
              }
            }
          }
        }

        if(tategaki){
          y += size.cx;
        }else{
          y += size.cy;
        }
      }
      p++;
    }

    SetBkMode(hdc,bkold);

  }else{

    WCHAR * wbuf = new WCHAR [s.length() + 1];
    MultiByteToWideChar(CP_ACP,0,s.c_str(),-1,wbuf,sizeof(WCHAR)*(s.length()+1));

    HFONT hfont = (HFONT)GetCurrentObject(hdc,OBJ_FONT);
    Gdiplus::Font myfont(hdc,hfont);
    Gdiplus::FontFamily ff;
    myfont.GetFamily(&ff);
    //行間
    double ds = myfont.GetSize() * ff.GetCellDescent(myfont.GetStyle()) / ff.GetEmHeight(myfont.GetStyle()); 

    MyGraphics graphics(hdc);
    // 行間の分だけ右にずれるのでその分を補正
    Gdiplus::RectF rect(r->left-ds/2.0,r->top,r->right-r->left,r->bottom-r->top);
    Gdiplus::Color clr;
    clr.SetFromCOLORREF(GetTextColor(hdc));
    Gdiplus::SolidBrush brush(clr);

    Gdiplus::StringFormat format;
    format.SetLineAlignment(Gdiplus::StringAlignmentCenter);

    if(align == 1){
      format.SetAlignment(Gdiplus::StringAlignmentCenter);
    }else if(align == 2){
      format.SetAlignment(Gdiplus::StringAlignmentFar);
    }

    format.SetFormatFlags(
      Gdiplus::StringFormatFlagsDirectionVertical 
      |Gdiplus::StringFormatFlagsNoWrap
      |Gdiplus::StringFormatFlagsNoClip
      |Gdiplus::StringFormatFlagsNoFitBlackBox
    );
    format.SetTrimming(Gdiplus::StringTrimmingNone);
    if(gpath == NULL){
      graphics.DrawString(wbuf,-1,&myfont,rect,&format,&brush);
    }else{
      Gdiplus::FontFamily fontfamily;
      myfont.GetFamily(&fontfamily);
      int style = myfont.GetStyle();
      double emsize = myfont.GetSize();
      gpath->AddString(wbuf,-1, &fontfamily,style,emsize,rect,&format);
    }
    if(furi != NULL){
      Gdiplus::CharacterRange * crs = new Gdiplus::CharacterRange[furi->size()];
      for(int i = 0; i < furi->size() ; i++){
        std::string ss;
        ss = s.substr(0,(*furi)[i].begin_pos);
        WCHAR * wb = new WCHAR [ss.length() + 1];
        int begin = MultiByteToWideChar(CP_ACP,0,ss.c_str(),-1,wb,sizeof(WCHAR)*(ss.length()+1))-1;
        delete[] wb;
        ss = s.substr(0,(*furi)[i].end_pos);
        wb = new WCHAR [ss.length() + 1];
        int end = MultiByteToWideChar(CP_ACP,0,ss.c_str(),-1,wb,sizeof(WCHAR)*(ss.length()+1))-1;
        delete[] wb;
        crs[i] = Gdiplus::CharacterRange(begin,end-begin);
      }
      format.SetMeasurableCharacterRanges(furi->size(), crs);
      INT count = format.GetMeasurableCharacterRangeCount();
      Gdiplus::Region * regions = new Gdiplus::Region [count];
      graphics.MeasureCharacterRanges(wbuf,-1,&myfont,rect,&format,count,regions);
      for(int i = 0; i < count ; i++){
        Gdiplus::RectF rct;
        regions[i].GetBounds(&rct,&graphics);
        double top = rct.GetTop();
        double bottom = rct.GetBottom();
        double left = rct.GetLeft();
        double right = rct.GetRight();
        (*furi)[i].rect.top = top;
        (*furi)[i].rect.bottom = bottom;
        (*furi)[i].rect.left = left;
        (*furi)[i].rect.right = right;
      }
      delete [] regions;
      delete [] crs;
    }
    delete [] wbuf;
  }
}

// 下の句を空白を除いて一文字(2バイト)ごとに分解
std::vector<std::string> ParseSimonoku(std::string s){
  std::vector<std::string> r;
  while(1){
    std::string::size_type pos;
    pos = s.find(' ');
    if(pos == std::string::npos){
      break;
    }
    s.erase(pos,1);
  }
  std::string::size_type len = s.length();
  for(std::string::size_type i = 0; i < len ; i++){
    int c = (unsigned char)s[i];
    if(isZenkaku(c)){
      char b[3];
      b[0] = s[i];
      b[1] = s[i+1];
      b[2] = '\0';
      r.push_back(std::string(b));
      i++;
      if(i >= len){
        break;
      }
    }else{
      char b[2];
      b[0] = s[i];
      b[1] = '\0';
      r.push_back(std::string(b));
    }
  }
  return r;
}

// 和歌を "\n" ごとに一行と見て分解
std::vector<std::string> ParseYomi(std::string s){
  std::vector<std::string> r;
  std::string buf = s;
  while(1){
    std::string::size_type pos;
    pos = buf.find('\n');
    if(pos == std::string::npos){
      r.push_back(buf);
      break;
    }
    r.push_back(buf.substr(0,pos));
    buf = buf.substr(pos+1);
  }
  return r;
}


std::string ParseFurigana(const std::string & s, std::vector<struct Furigana> & furi){
  int furibegin = -1;
  int kanjibegin = 0;
  std::string buf = s;
  int prevwaskanji = 0;
  int forcekanji = 0;
  for(int i = 0; i < buf.length() ; i++){
    int c = (unsigned char)buf[i];
    if( isKanji(c)  ){
      //漢字
      if(prevwaskanji == 0){
        kanjibegin = i;
      }
      prevwaskanji = 1;
      i++;
      if(i >= buf.length()){
        break;
      }
    }else if( isZenkaku(c) ){
      //全角文字
      if(!forcekanji){
        prevwaskanji = 0;
      }else{
        prevwaskanji = 1;
      }
      i++;
      if(i >= buf.length()){
        break;
      }
    }else{
      if(!forcekanji){
        prevwaskanji = 0;
      }else{
        prevwaskanji = 1;
      }
      if( c == '('){
        furibegin = i + 1;
      }else if( c == ')'){
        if(furibegin >= 0){
          struct Furigana f;
          f.begin_pos = kanjibegin;
          f.end_pos = furibegin - 1;
          f.furigana = buf.substr(furibegin,i - furibegin);
          f.rect.top = 0;
          f.rect.bottom = 0;
          f.rect.left = 0;
          f.rect.right = 0;
          furi.push_back(f);
          buf.erase(furibegin-1,i-furibegin+2);
          i = furibegin-2;
          kanjibegin = i+1;
          furibegin = -1;
          forcekanji = 0;
          prevwaskanji = 0;
        }
      }else if( c == '#'){
        kanjibegin = i;
        forcekanji = 1;
        prevwaskanji = 1;
        buf.erase(i,1);
        i--;
      }
    }
  }
  return buf;
}

std::string RemoveFuriganaAndTrunc(std::string s){
  std::vector<struct Furigana> furi;
  std::string ss = "";
  std::string::size_type pos = 0;
  for(int loop = 0; loop < 10; loop++){
    pos = s.find('\n');
    if(pos == std::string::npos){
      ss += ParseFurigana(s, furi);
      break;
    }
    ss += ParseFurigana(s.substr(0,pos),furi);
    if(ss.length() > 30){
      break;
    }
    s = s.substr(pos+1);
  }
  return ss;
}
