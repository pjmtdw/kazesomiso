#include "kazesomiso.h"

struct BackStructure backstructure[20];
void ShowSavedSetting(const HWND &,const RECT &,const int &);

LRESULT CALLBACK SettingsProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
  const int WIDTH = 205;
  const int HEIGHT = 200;
  const int MARGIN_TOP = 10;
  const int MARGIN_LEFT = 10;
  const int SUKIMA_H = 10;
  const int SUKIMA_V = 10;
  const int ROWS = 3;
  const int COLS = 4;
  const int BUTTONWIDTH = 56;
  const int BUTTONHEIGHT = 30;
  switch (message)
  {
  case WM_INITDIALOG:
    {
      if(tori_or_yomi == TORI){
        SetWindowText(hDlg,"設定の保存／読込： 取札");
      }else{
        SetWindowText(hDlg,"設定の保存／読込： 詠札");
      }
      RECT rect1;
      GetWindowRect(hDlg,&rect1);
      RECT rect2;
      GetClientRect(hDlg,&rect2);
      int dw = ( rect1.right - rect1.left ) - (rect2.right - rect2.left);
      int dh = ( rect1.bottom - rect1.top ) - (rect2.bottom - rect2.top);

      int cw = (WIDTH + SUKIMA_H)*COLS+MARGIN_LEFT*2;
      int ch = (HEIGHT+SUKIMA_V)*ROWS+MARGIN_TOP*2;

      SetWindowPos(hDlg,NULL,10,10,dw+cw,dh+ch,SWP_NOZORDER);
      RECT rect;
      rect.top = 0;
      rect.left = 0;
      rect.bottom = HEIGHT;
      rect.right = WIDTH;
      for(int i = 0; i < ROWS*COLS ; i++){
        backstructure[i].hbmp = NULL;
        backstructure[i].hdc = NULL;
        ShowSavedSetting(hDlg,rect,i-1);
      }
      for(int i = 0; i < ROWS*COLS ; i++){
        int rows = i / COLS;
        int cols = i % COLS;
        int x = MARGIN_LEFT + cols * WIDTH + cols * SUKIMA_H + WIDTH - 5;
        int y = MARGIN_TOP + rows * HEIGHT + rows * SUKIMA_V + HEIGHT - 5;
        CreateWindow("BUTTON","読込",WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, x - BUTTONWIDTH , y - BUTTONHEIGHT , BUTTONWIDTH, BUTTONHEIGHT, hDlg, (HMENU) (IDC_SETTING_LOAD + i), hInst, NULL);
        y -= BUTTONHEIGHT + 5;
        if(i >= 2){
          CreateWindow("BUTTON","保存",WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, x - BUTTONWIDTH , y - BUTTONHEIGHT , BUTTONWIDTH, BUTTONHEIGHT, hDlg, (HMENU) (IDC_SETTING_SAVE + i), hInst, NULL);
        }
        
      }


    }
    return TRUE;
    break;
  case WM_PAINT:
    {
       PAINTSTRUCT ps;
       BeginPaint(hDlg,&ps);
       HDC hdc = GetDC(hDlg);
       for(int i = 0; i < ROWS*COLS; i++){
         if(backstructure[i].hdc != NULL){
            int rows = i / COLS;
            int cols = i % COLS;
            int x = MARGIN_LEFT + cols * WIDTH + cols * SUKIMA_H;
            int y = MARGIN_TOP + rows * HEIGHT + rows * SUKIMA_V;
            HDC h = backstructure[i].hdc;
            HBITMAP hb = backstructure[i].hbmp;
            HBITMAP hold = (HBITMAP)SelectObject(h,hb);
            BitBlt(hdc,x,y,WIDTH,HEIGHT,h,0,0,SRCCOPY);
            SelectObject(h,hold);

            RECT rect;
            rect.left = x;
            rect.top = y;
            rect.right = x + WIDTH;
            rect.bottom = y + HEIGHT;
            DrawEdge(hdc,&rect,EDGE_SUNKEN,BF_RECT);
            HWND hwnd = GetDlgItem(hDlg,IDC_SETTING_LOAD+i);
            if(hwnd != NULL){
              SetFocus(hwnd);
            }
            hwnd = GetDlgItem(hDlg,IDC_SETTING_SAVE+i);
            if(hwnd != NULL){
              SetFocus(hwnd);
            }
         }
       }
       EndPaint(hDlg,&ps);
    }
    break;
  case WM_COMMAND:
    {
      int iEvent = HIWORD(wParam);
      int iId = LOWORD(wParam);
      if(iId >= IDC_SETTING_LOAD && iId < IDC_SETTING_LOAD + ROWS * COLS){
        SetWindowText(hDlg,"設定の読込／保存");
        if(IDYES != MessageBox(hDlg,"読み込みますか？","確認",MB_YESNO)){
          break;
        }
        int i = iId - IDC_SETTING_LOAD -1;
        class ToriYomiDesign setting;

        if(tori_or_yomi == TORI){
          class ToriDesign newtori;
          setting.tori = &newtori;
          std::vector<int> notnigate;
          if( i != -1){
            LoadFromIniFile(i, setting,&notnigate);
          }
          memcpy(currentdesign.tori,setting.tori,sizeof(class ToriDesign));
          ClearNigateFuda();
          std::vector<int>::iterator it;
          for(it = notnigate.begin(); it != notnigate.end(); it++){
            torifuda[*it].nigatefuda = FALSE;
          }
        }else{
          class YomiDesign newyomi;
          setting.yomi = &newyomi;
          if( i != -1){
            LoadFromIniFile(i, setting,NULL);
          }
          memcpy(currentdesign.yomi,setting.yomi,sizeof(class YomiDesign));
        }

        SetWindowText(hDlg,"☆読み込み完了☆");

        FixWindowSizeToFudaSize();
        HDC hdc = GetDC(hWndMain);
        LoadImageToDC(hdc,currentdesign);
        ReleaseDC(hWndMain,hdc);

        UpdateMainWindow(TRUE);

      }else if(iId >= IDC_SETTING_SAVE && iId < IDC_SETTING_SAVE + ROWS * COLS){
        int i = iId - IDC_SETTING_SAVE -1;
        int r = myrandint(torifuda.size());

        if(GetKeyState(VK_SHIFT) < 0 && GetKeyState(VK_CONTROL) < 0){
          if(IDYES != MessageBox(hDlg,"この設定を削除しますか？","確認",MB_YESNO)){
            break;
          }
          SaveToIniFile(i, NULL);
        }else{
          if(IDYES != MessageBox(hDlg,"現在の設定で上書き保存してもよろしいですか？","確認",MB_YESNO)){
            break;
          }
          int r = myrandint(torifuda.size());
          currentdesign.simonoku = torifuda[r].simonoku;
          currentdesign.kimariji = torifuda[r].kimariji;
          r = myrandint(yomifuda.size());
          currentdesign.waka = yomifuda[r].waka;
          SaveToIniFile(i, &currentdesign);
        }
        int rows = i / COLS;
        int cols = i % COLS;
        int x = MARGIN_LEFT + cols * WIDTH + cols * SUKIMA_H;
        int y = MARGIN_TOP + rows * HEIGHT + rows * SUKIMA_V;

        RECT rect;
        rect.left = 0;
        rect.top = 0;
        rect.right = WIDTH;
        rect.bottom = HEIGHT;

        ShowSavedSetting(hDlg,rect,i);
        InvalidateRect(hDlg,NULL,TRUE);
        UpdateWindow(hDlg);
      }else{
        switch(iId){
        case IDOK: case IDCANCEL:
          {
            for(int i = 0; i < sizeof(backstructure) / sizeof(*backstructure); i++){
              if(backstructure[i].hdc != NULL){
                DeleteObject(backstructure[i].hbmp);
                DeleteDC(backstructure[i].hdc);
              }
              backstructure[i].hbmp = NULL;
              backstructure[i].hdc = NULL;
            }
            EndDialog(hDlg,0);
          }
        }
      }
      break;
    }
    return FALSE;
    break;
  }
  return FALSE;
}
void ShowSavedSetting(const HWND & hWnd, const RECT & rect, const int & num){

  BOOL loadedflag = FALSE;
  class ToriYomiDesign design;
  class ToriDesign toridesign;
  class YomiDesign yomidesign;
  design.tori = &toridesign;
  design.yomi = &yomidesign;

  std::string simonoku, kimariji,waka;
  if(backstructure[num+1].hdc != NULL){
    DeleteObject(backstructure[num+1].hdc);
    DeleteObject(backstructure[num+1].hbmp);
  }
  std::vector<int> notnigate;
  if(num==-1){
    design.simonoku = std::string("初期設定に戻したい時はこれを読み込む");
    design.kimariji = std::string("きまりじ");
    design.waka = std::string("初期(しょき)設定(せってい)に\n戻(もど)したい時(とき)は\nこれを読(よ)み込(こ)む");
    loadedflag = TRUE;
  }else{
    loadedflag = LoadFromIniFile(num, design,&notnigate);
    if(num == 0){
       design.simonoku = std::string("今回起動したときの設定に戻すにはこれ");
       design.waka = std::string("今回(こんかい)起動(きどう)した\nときの設定(せってい)に\n戻(もど)すにはこれ");
       design.kimariji = std::string("きまりじ");
    }
  }
  design.UpdateSize();

  int width = rect.right - rect.left;
  int height = rect.bottom - rect.top;

  HDC hwnddc = GetDC(hWnd);

  LoadImageToDC(hwnddc,design);

  HDC hdc1 = CreateCompatibleDC(hwnddc);
  
  HDC hdc = CreateCompatibleDC(hwnddc);
  HBITMAP hbmp = CreateCompatibleBitmap(hwnddc,width,height);
  HBITMAP hold = (HBITMAP)SelectObject(hdc,hbmp);

  HBRUSH hbr = CreateSolidBrush(RGB(245,232,187));
  FillRect(hdc,&rect,hbr);
  DeleteObject(hbr);

  if(loadedflag){
    HBITMAP hbmp1 = CreateCompatibleBitmap(hwnddc,design.width,design.height);
    HBITMAP hold1 = (HBITMAP)SelectObject(hdc1,hbmp1);
    DrawFuda(design, hdc1,NULL);
    int w = int( (height -10)*(double(design.width) / double(design.height)));

    if(!gdiplus_useable){
      SetStretchBltMode(hdc,HALFTONE);
      StretchBlt(hdc,5,5,w,height-10,hdc1,0,0,design.width,design.height,SRCCOPY);
    }else{
      Gdiplus::Graphics g(hdc);
      g.SetPixelOffsetMode(Gdiplus::PixelOffsetModeHighQuality);
      g.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);
      Gdiplus::Bitmap gbmp(hbmp1,Gdiplus::Graphics::GetHalftonePalette());
      g.DrawImage(&gbmp,5,5,w,height-10);
    }
    SelectObject(hdc1,hold1);
    DeleteDC(hdc1);
    DeleteObject(hbmp1);
  }else{
    COLORREF bordercolor;
    COLORREF ellipsecolor;
    int bgimage;
    if(tori_or_yomi == TORI){
      bordercolor = RGB(245,163,155);
      ellipsecolor = RGB(245,163,155);
      bgimage = IDB_CHERRY;
    }else{
      bordercolor = RGB(252,156,130);
      ellipsecolor = RGB(252,156,130);
      bgimage = IDB_KAEDE;
    }
    
    int w = int( (height -10) * (53.0 / 74.0));
    LOGBRUSH lbr;
    HBRUSH hbr;
    RECT r;
    r.left = 5;
    r.top = 5;
    r.right = 5 + w;
    r.bottom = 5 + height -10;

    hbr = CreateSolidBrush(bordercolor);
    FillRect(hdc,&r,hbr);
    DeleteObject(hbr);

    r.left += 5;
    r.top += 5;
    r.right -= 5;
    r.bottom -= 5;
    lbr.lbStyle = BS_PATTERN;
    lbr.lbColor = RGB(255,255,255);
    HBITMAP hbmp1 = (HBITMAP)LoadImage(hInst,MAKEINTRESOURCE(bgimage),IMAGE_BITMAP,0,0,LR_DEFAULTCOLOR);
    lbr.lbHatch = (LONG)hbmp1;
    hbr = CreateBrushIndirect(&lbr);
    FillRect(hdc,&r,hbr);
    DeleteObject(hbr);
    
    hbr = CreateSolidBrush(ellipsecolor);
    HBRUSH hbold = (HBRUSH)SelectObject(hdc,hbr);
    HPEN hpen = CreatePen(PS_DASH,1,RGB(255,255,255));
    HPEN hpenold = (HPEN)SelectObject(hdc,hpen);
    Ellipse(hdc,r.left + 30 ,r.top + 40 ,r.right - 30 ,r.bottom -40);
    SelectObject(hdc,hbold);
    SelectObject(hdc,hpenold);
    DeleteObject(hpen);
    DeleteObject(hbr);
    DeleteObject(hbmp1);
  }

  if(tori_or_yomi == TORI){
    COLORREF oldcolor = SetBkColor(hdc,RGB(245,232,187));
    char buf[32];
    wsprintf(buf,"残り:\r\n%d枚",torifuda.size() - notnigate.size());
    RECT r;
    r.left = 0;
    r.top = 5;
    r.right = rect.right -5;
    r.bottom = rect.bottom - 5;
    DrawText(hdc,buf,lstrlen(buf),&r,DT_RIGHT);
    SetBkColor(hdc,oldcolor);
  }
  ReleaseDC(hWnd,hwnddc);
	
  SelectObject(hdc,hold);
  backstructure[num+1].hdc = hdc;
  backstructure[num+1].hbmp = hbmp;
}
