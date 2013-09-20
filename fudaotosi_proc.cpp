#include "kazesomiso.h"

class BackStructure fudaotosi_structure;
WNDPROC oldFudaOtosiMessageProc;
HWND hMistakeButton = NULL;
BOOL nigateflag = FALSE;

LRESULT CALLBACK  FudaOtosiMessageProc(HWND, UINT, WPARAM, LPARAM);
std::string GetFudaOtosiInfo();

LRESULT CALLBACK FudaOtosiProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
  HWND hMessage = GetDlgItem(hDlg,IDC_MESSAGE);
  switch (message)
  {
  case WM_INITDIALOG:
    {

      fudaotosinum = 0;
      currentfuda = torifuda_of_fudaotosi[fudaotosinum];
      currentdesign.simonoku = currentfuda->simonoku;
      currentdesign.kimariji = currentfuda->kimariji;

      UpdateMainWindow(TRUE);

      hDlgWnd = hDlg; // メッセージループで使う
      EnableMenuAll(hWndMain,FALSE);
      RECT rect;
      GetWindowRect(hWndMain,&rect);
      SetWindowPos(hDlg,hWndMain,rect.right,rect.top,0,0,SWP_NOSIZE);
      oldFudaOtosiMessageProc = (WNDPROC)SetWindowLongPtr(hMessage,GWL_WNDPROC,(LONG)FudaOtosiMessageProc);
      std::string s = GetFudaOtosiInfo();
      SetWindowText(hMessage,s.c_str());
    }
    return TRUE;
    break;
  case WM_COMMAND:
    {
      int iEvent = HIWORD(wParam);
      int iId = LOWORD(wParam);
      switch(iId){
      case IDC_FUDAOTOSI_YES:
        {
          if(!nigateflag){
            currentfuda->nigatefuda = FALSE;
          }
          nigateflag = FALSE;

          if(fudaotosi_structure.hdc != NULL){
            DeleteDC(fudaotosi_structure.hdc);
            DeleteObject(fudaotosi_structure.hbmp);
            fudaotosi_structure.hdc = NULL;
          }
          if(front_structure.hdc != NULL){
            HDC hdc = GetDC(hWndMain);
            fudaotosi_structure.hdc = CreateCompatibleDC(hdc);
            fudaotosi_structure.hbmp = CreateCompatibleBitmap(hdc,currentdesign.width,currentdesign.height);
            HBITMAP hOld = (HBITMAP)SelectObject(fudaotosi_structure.hdc,fudaotosi_structure.hbmp);
            HBITMAP hOld2 = (HBITMAP)SelectObject(front_structure.hdc,front_structure.hbmp);
            BitBlt(fudaotosi_structure.hdc,0,0,currentdesign.width,currentdesign.height,front_structure.hdc,0,0,SRCCOPY);
            SelectObject(fudaotosi_structure.hdc,hOld);
            SelectObject(front_structure.hdc,hOld2);
            ReleaseDC(hWndMain,hdc);
          }

          fudaotosinum ++;
          if(fudaotosinum >= (int)torifuda_of_fudaotosi.size()){
            CreateFudaOtositorifuda();
            if(torifuda_of_fudaotosi.size() == 0){
              MessageBox(hDlg,"おめでとうございます。\r\n貴方は全ての札を覚えました。","(=^ω^=)",MB_OK);
              SendMessage(hDlg,WM_COMMAND,(WPARAM)IDOK,(LPARAM)0);
              return 0;
            }
            if(IDYES != MessageBox(hDlg,"一通り終了しました。\r\n分からなかった札だけをもう一周やりますか？","確認",MB_YESNO)){
              SendMessage(hDlg,WM_COMMAND,(WPARAM)IDOK,(LPARAM)0);
              return 0;
            }
            fudaotosinum = 0;
            DestroyWindow(hMistakeButton);
            hMistakeButton = NULL;
            std::string s = GetFudaOtosiInfo();
            SetWindowText(hMessage,s.c_str());
          }
          currentfuda = torifuda_of_fudaotosi[fudaotosinum];
          currentdesign.simonoku = currentfuda->simonoku;
          currentdesign.kimariji = currentfuda->kimariji;

          UpdateMainWindow(FALSE);

          InvalidateRect(hMessage,NULL,TRUE);
          UpdateWindow(hMessage);
        }
        break;
      case IDC_FUDAOTOSI_NO:
        {
          if(fudaotosinum >= (int)torifuda_of_fudaotosi.size()){
            return 0;
          }
          nigateflag = TRUE;
          DeleteDC(fudaotosi_structure.hdc);
          DeleteObject(fudaotosi_structure.hbmp);
          fudaotosi_structure.hdc = NULL;
          InvalidateRect(hMessage,NULL,TRUE);
          UpdateWindow(hMessage);
          DestroyWindow(hMistakeButton);
          hMistakeButton = NULL;
          class torifuda * p = torifuda_of_fudaotosi[fudaotosinum];
          std::string buf;
          buf = GetFudaOtosiInfo();
          buf += "\r\n";
          buf += "[札の説明]\r\n";
          buf += p->kaminoku;
          buf += "\r\n";
          buf += p->simonoku;
          buf += "\r\n・決まり字は「";
          buf += p->kimariji;
          buf += "」\r\n";
          std::string s = p->explanation;
          while(1){
            std::string::size_type pos;
            pos = s.find("//");
            if(pos == std::string::npos){
              if(s.length() > 0){
                buf += "・";
                buf += s;
              }
              break;
            }
            buf += "・";
            buf += s.substr(0,pos);
            buf += "\r\n";
            s = s.substr(pos+2);
          }
          SetWindowText(hMessage,buf.c_str());
        }
        break;
      case IDOK:case IDCANCEL:
        {
          EnableMenuAll(hWndMain,TRUE);
          DrawMenuBar(hWndMain);
          DestroyWindow(hMistakeButton);
          hMistakeButton = NULL;
          DeleteDC(fudaotosi_structure.hdc);
          DeleteObject(fudaotosi_structure.hbmp);
          fudaotosi_structure.hdc = NULL;
          hDlgWnd = NULL;
          EndDialog(hDlg,0);
        }
        break;
      }
    }
    return FALSE;
    break;
  }
  return FALSE;
}
LRESULT CALLBACK FudaOtosiMessageProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam){
  PAINTSTRUCT ps;
  switch(message){
  case WM_COMMAND:
    {
      if(LOWORD(wParam) == IDC_MISTAKE && fudaotosinum > 0){
        torifuda_of_fudaotosi[fudaotosinum-1]->nigatefuda = TRUE;
        InvalidateRect(hWnd,NULL,TRUE);
        UpdateWindow(hWnd);
      }
    }
    break;
  case WM_PAINT:
    {
      if(fudaotosi_structure.hdc != NULL && fudaotosinum > 0 && fudaotosinum <= (int)torifuda_of_fudaotosi.size()){
        BeginPaint(hWnd,&ps);
        HDC hdc = GetDC(hWnd);
        RECT rect;
        GetClientRect(hWnd,&rect);
        int BUTTONWIDTH = 135;
        int BUTTONHEIGHT = 40;
        int w = currentdesign.width/3;
        int h = currentdesign.height/3;
        if(!gdiplus_useable){
          HBITMAP hOld = (HBITMAP)SelectObject(fudaotosi_structure.hdc,fudaotosi_structure.hbmp);
          SetStretchBltMode(hdc,HALFTONE);
          StretchBlt(hdc,rect.right-w,rect.bottom-h,w,h,fudaotosi_structure.hdc,0,0,currentdesign.width,currentdesign.height,SRCCOPY);
          SelectObject(fudaotosi_structure.hdc,hOld);
        }else{
          Gdiplus::Bitmap gbmp(fudaotosi_structure.hbmp,Gdiplus::Graphics::GetHalftonePalette());
          Gdiplus::Graphics g(hdc);
          g.SetPixelOffsetMode(Gdiplus::PixelOffsetModeHighQuality);
          g.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);
          g.DrawImage(&gbmp,rect.right-w,rect.bottom-h,w,h);
        }
        std::string buf;
        RECT rect2;
        memcpy(&rect2,&rect,sizeof(RECT));
        buf = GetFudaOtosiInfo();
        int dh = DrawText(hdc,buf.c_str(),(int)buf.length(),&rect2,DT_CALCRECT);
        rect2.bottom = dh;
        rect2.top = 1;
        rect2.left = 1;
        rect2.right = rect.right;
        DrawText(hdc,buf.c_str(),(int)buf.length(),&rect2,DT_LEFT);
        buf = "";
        buf += "前の札は \r\n「";
        buf += torifuda_of_fudaotosi[fudaotosinum-1]->kimariji;
        buf += "」\r\nでした→ ";
        dh = DrawText(hdc,buf.c_str(),(int)buf.length(),&rect2,DT_CALCRECT);
        rect2.bottom = rect.bottom - BUTTONHEIGHT - 10;
        rect2.top = rect2.bottom - dh;
        rect2.left = 0;
        rect2.right = rect.right - w;
        DrawText(hdc,buf.c_str(),(int)buf.length(),&rect2,DT_RIGHT);
        if(hMistakeButton == NULL){
          hMistakeButton = CreateWindow("BUTTON","間違えました",WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, rect2.right - BUTTONWIDTH - 5 , rect.bottom - BUTTONHEIGHT - 5, BUTTONWIDTH, BUTTONHEIGHT, hWnd, (HMENU) IDC_MISTAKE, hInst, NULL);
        }
        EndPaint(hWnd,&ps);
      }
      break;
    }
  }
  return CallWindowProc(oldFudaOtosiMessageProc,hWnd,message,wParam,lParam);
}

std::string GetFudaOtosiInfo(){
  std::string buf;
  char b[16];
  wsprintf(b,"%d",fudaotosinum + 1);
  buf += "現在 ";
  buf += b;
  buf += " 枚目 / 全 ";
  wsprintf(b,"%d",torifuda_of_fudaotosi.size());
  buf += b;
  buf += " 枚\r\n";
  buf += "覚えていない札はあと ";
  int count = 0;
  std::vector<class torifuda*>::iterator it;
  for(it = torifuda_of_fudaotosi.begin(); it != torifuda_of_fudaotosi.end(); it++){
    if((*it)->nigatefuda){
      count ++;
    }
  }
  wsprintf(b,"%d",count);
  buf += b;
  buf += " 枚です";
  return buf;
}

