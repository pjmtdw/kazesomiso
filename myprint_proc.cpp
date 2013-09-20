// myprint_proc.cpp

#include "kazesomiso.h"

HWND hDialog = NULL;

HDC hPrintDC = NULL;
HGLOBAL hDevMode = NULL;
HGLOBAL hDevNames = NULL;

WNDPROC oldMyPrintPreviewProc;
LRESULT CALLBACK MyPrintPreviewProc(HWND, UINT, WPARAM, LPARAM);
void GetPrintMargin(HWND,int *, int *, int *, int *);
void CountOnePageFudaNum(HWND, HDC, int *, int *);
void GetFudaSukima(HWND,HDC,int *, int *);
double GetFudaBairitu(HWND);
int CountFudaToPrint();
int CountPages(HWND, HDC);
int GetCurrentPrinterCopies();

int GetSizeOfDevMode(HGLOBAL hdv){
  DEVMODE * pdv = (DEVMODE *)GlobalLock(hdv);
  int r = pdv->dmSize + pdv->dmDriverExtra;
  GlobalUnlock(hdv);
  return r;
}
int GetSizeOfDevNames(HGLOBAL hdn){
  DEVNAMES * pdn = (DEVNAMES *)GlobalLock(hdn);
  int r = pdn->wOutputOffset + lstrlen((char *)pdn + pdn->wOutputOffset) + 1;
  GlobalUnlock(hdn);
  return r;
}

void GlobalCpy(HGLOBAL to,HGLOBAL from,int size){
  void * pto = GlobalLock(to);
  void * pfrom = GlobalLock(from);
  memcpy(pto,pfrom,size);
  GlobalUnlock(to);
  GlobalUnlock(from);
}

void BeginPrinter(){
  PRINTDLG pd;
  ZeroMemory(&pd,sizeof(PRINTDLG));
  pd.lStructSize = sizeof(PRINTDLG);
  pd.hwndOwner = NULL;

  pd.hDevMode = NULL;
  pd.hDevNames = NULL;
  pd.Flags = PD_USEDEVMODECOPIESANDCOLLATE | PD_RETURNDC | PD_RETURNDEFAULT;
  pd.nCopies = 1;

  if(PrintDlg(&pd)){
    hDevMode = pd.hDevMode;
    hDevNames = pd.hDevNames;
    hPrintDC = pd.hDC;
  }
}

void SelPrinter(HWND hDlg){
  PRINTDLG pd;
  ZeroMemory(&pd,sizeof(PRINTDLG));
  pd.lStructSize = sizeof(PRINTDLG);
  pd.hwndOwner = hDlg;

  HGLOBAL hNewDevMode = NULL;
  HGLOBAL hNewDevNames = NULL;
  if(hDevMode != NULL && hDevNames != NULL){
    int size = GetSizeOfDevMode(hDevMode);
    hNewDevMode = GlobalAlloc(GHND,size);
    GlobalCpy(hNewDevMode,hDevMode,size);
    size = GetSizeOfDevNames(hDevNames);
    hNewDevNames = GlobalAlloc(GHND,size);
    GlobalCpy(hNewDevNames,hDevNames,size);
  }
  pd.hDevMode = hNewDevMode;
  pd.hDevNames = hNewDevNames;
  pd.Flags = PD_USEDEVMODECOPIESANDCOLLATE | PD_RETURNDC | 
             PD_NOPAGENUMS | PD_NOSELECTION | PD_HIDEPRINTTOFILE;
  pd.nCopies = GetCurrentPrinterCopies();
  pd.nFromPage = 1;
  pd.nToPage = 1;
  pd.nMinPage = 1;
  pd.nMaxPage = 1;

  if(PrintDlg(&pd)){
    if(hPrintDC != NULL){
      DeleteDC(hPrintDC);
    }
    hPrintDC = pd.hDC;

    int size = GetSizeOfDevMode(pd.hDevMode);
    if(hDevMode != NULL){
      GlobalReAlloc(hDevMode,size,0);
    }else{
      hDevMode = GlobalAlloc(GHND,size);
    }
    GlobalCpy(hDevMode,pd.hDevMode,size);
    
    size = GetSizeOfDevNames(pd.hDevNames);
    if(hDevNames != NULL){
      GlobalReAlloc(hDevNames,size,0);
    }else{
      hDevNames = GlobalAlloc(GHND,size);
    }
    GlobalCpy(hDevNames,pd.hDevNames,size);

  }
  SetCurrentDirectoryToExePath();

  if(pd.hDevMode != NULL){
    GlobalFree(pd.hDevMode);
  }

  if(pd.hDevMode != NULL){
    GlobalFree(pd.hDevMode);
  }
}

void EndPrinter(){
  if(hPrintDC != NULL){
    DeleteDC(hPrintDC);
    hPrintDC = NULL;
  }
  if(hDevMode != NULL){
    GlobalFree(hDevMode);
    hDevMode = NULL;
  }
  if(hDevNames != NULL){
    GlobalFree(hDevNames);
    hDevNames = NULL;
  }
}

void GetCurrentPrinterName(char * buf){
  *buf = '\0';
  if(hDevNames == NULL){
    return;
  }
  DEVNAMES * pdn = (DEVNAMES *)GlobalLock(hDevNames);
  lstrcpy(buf,(char *)pdn + pdn->wDeviceOffset);
  GlobalUnlock(hDevNames);
}

int GetCurrentPrinterCopies(){
  if(hDevMode == NULL){
    return 1;
  }
  DEVMODE * pdv = (DEVMODE *)GlobalLock(hDevMode);
  int n = 1;
  if(pdv->dmFields & DM_COPIES){
    n = pdv->dmCopies;
  }
  GlobalUnlock(hDevMode); 
  return n;
}

LRESULT CALLBACK MyPrintProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam){
  switch(message){
  case WM_INITDIALOG:
    {
      hDialog = hDlg;

      SetDlgItemText(hDlg,IDC_PM_LEFT,"15");
      SetDlgItemText(hDlg,IDC_PM_RIGHT,"15");
      SetDlgItemText(hDlg,IDC_PM_TOP,"21");
      SetDlgItemText(hDlg,IDC_PM_BOTTOM,"21");
      SetDlgItemText(hDlg,IDC_BAIRITU,"100");
      HWND hOk = GetDlgItem(hDlg,IDOK);
      if(hPrintDC == NULL){
        EnableWindow(hOk,FALSE);
      }
      SendDlgItemMessage(hDlg,IDOK,WM_COMMAND,(WPARAM)(BN_DISABLE),(LPARAM)0);
      HWND hPreview = GetDlgItem(hDlg,IDC_PREVIEW);
      oldMyPrintPreviewProc = (WNDPROC)SetWindowLongPtr(hPreview,GWL_WNDPROC,(LONG)MyPrintPreviewProc);
      char printername[256];
      GetCurrentPrinterName(printername);
      SetDlgItemText(hDlg,IDC_SELECTEDPRINTER,printername);
    }
    break;
  case WM_VSCROLL:
    {
      int iId = LOWORD(wParam);

      int id = GetDlgCtrlID((HWND)lParam);

      HWND hChange = NULL;
      int maximum = -1;
      int minimum = -1;
      if(id == IDC_SCR_BAIRITU){
        hChange = GetDlgItem(hDlg,IDC_BAIRITU);
        minimum = 1;
      }else if(id == IDC_PM_SCR_LEFT){
        hChange = GetDlgItem(hDlg,IDC_PM_LEFT);
        minimum = 0;
      }else if(id == IDC_PM_SCR_RIGHT){
        hChange = GetDlgItem(hDlg,IDC_PM_RIGHT);
        minimum = 0;
      }else if(id == IDC_PM_SCR_TOP){
        hChange = GetDlgItem(hDlg,IDC_PM_TOP);
        minimum = 0;
      }else if(id == IDC_PM_SCR_BOTTOM){
        hChange = GetDlgItem(hDlg,IDC_PM_BOTTOM);
        minimum = 0;
      }
      if(hChange != NULL){
        DoWhenVscroll(hChange,iId,minimum,maximum);
      }
    }
    break;
  case WM_COMMAND:
    {
      int iEvent = HIWORD(wParam);
      int iId = LOWORD(wParam);
      switch(iId){
      case IDC_SETTOKYOUGI:
        {
          char buf[32];
          double bai = 74.0 / (double)currentdesign.height * 600.0;
          sprintf(buf,"%.4f",bai);
          SetDlgItemText(hDlg,IDC_BAIRITU,buf);
        }
        break;
      case IDC_PM_LEFT: case IDC_PM_RIGHT: case IDC_PM_TOP: case IDC_PM_BOTTOM:
      case IDC_BAIRITU:
        {
          if(iEvent == EN_CHANGE && hPrintDC != NULL){
            HWND h = GetDlgItem(hDlg,IDC_PREVIEW);
            InvalidateRect(h,NULL,FALSE);
            UpdateWindow(h);
            int pages = CountPages(hDlg,hPrintDC);
            char buf[16];
            wsprintf(buf,"%d",pages);
            SetDlgItemText(hDlg,IDC_PAGENUM,buf);
          }
        }
        break;
      case IDC_SELPRINTER:
        {
          SelPrinter(hDlg);

          HWND hPreview = GetDlgItem(hDlg,IDC_PREVIEW);
          InvalidateRect(hPreview,NULL,TRUE);
          UpdateWindow(hPreview);

          HWND hOk = GetDlgItem(hDlg,IDOK);
          if(hPrintDC != NULL){
            EnableWindow(hOk,TRUE);
            int pages = CountPages(hDlg,hPrintDC);
            char buf[16];
            wsprintf(buf,"%d",pages);
            SetDlgItemText(hDlg,IDC_PAGENUM,buf);
          }else{
            EnableWindow(hOk,FALSE);
          }
          char printername[256];
          GetCurrentPrinterName(printername);
          SetDlgItemText(hDlg,IDC_SELECTEDPRINTER,printername);
        }
        break;
      case IDOK:
        {
          if(hPrintDC == NULL){
            break;
          }
          char buf[512];
          char printername[256];
          GetCurrentPrinterName(printername);
          int pages = CountPages(hDlg,hPrintDC);
          int nCopies = GetCurrentPrinterCopies();
          wsprintf(buf,"プリンタ名: [ %s ]\r\n一部当たり [ %d ページ ] で\r\n全部で [ %d 部 ] 印刷します\r\nよろしいですか？",printername,pages,nCopies);
          if(MessageBox(hDlg,buf,"確認",MB_YESNO)==IDYES){

            ChangeDisplayColorToMax();
            DOCINFO di;
            ZeroMemory(&di,sizeof(DOCINFO));
            di.cbSize = sizeof(DOCINFO);
            di.lpszDocName = "KAZESOMISO";
            double dotpermm_x = GetDeviceCaps(hPrintDC,LOGPIXELSX) / 25.4;
            double dotpermm_y = GetDeviceCaps(hPrintDC,LOGPIXELSY) / 25.4;
            double bairitu = GetFudaBairitu(hDlg);

            fudascale_x = bairitu * dotpermm_x;
            fudascale_y = bairitu * dotpermm_y;

            if(!gdiplus_useable){
              fudascale = fudascale_y;
            }else{
              fudascale = bairitu;
            }
            LoadImageToDC(hPrintDC,currentdesign);

            fudaunit = Gdiplus::UnitMillimeter;

            StartDoc(hPrintDC,&di);
            int rows,cols,sukima_x,sukima_y;
            CountOnePageFudaNum(hDlg,hPrintDC,&cols,&rows);
            GetFudaSukima(hDlg,hPrintDC,&sukima_x,&sukima_y);
            int left,right,top,bottom;
            GetPrintMargin(hDlg,&left,&top,&right,&bottom);

            int maxfudanum = 0;
            if(tori_or_yomi == TORI){
              maxfudanum = torifuda.size();
            }else{
              maxfudanum = yomifuda.size();
            }
            int fudanum = 0;
            for(int p = 0; p < pages; p++){
              StartPage(hPrintDC);
              for(int i = cols -1 ; i >= 0; i --){
                for(int j = 0; j < rows; j ++){
                  for(; fudanum < maxfudanum; fudanum++){
                    if(tori_or_yomi == TORI && torifuda[fudanum].saveasimage ||
                       tori_or_yomi == YOMI && yomifuda[fudanum].saveasimage
                    ){
                      break;
                    }
                  }
                  if(fudanum >= maxfudanum){
                    break;
                  }
                  class ToriYomiDesign design;
                  if(tori_or_yomi == TORI){
                    design.simonoku = torifuda[fudanum].simonoku;
                    design.kimariji = torifuda[fudanum].kimariji;
                    design.tori = currentdesign.tori;
                  }else{
                    design.waka = yomifuda[fudanum].waka;
                    design.yomi = currentdesign.yomi;
                  }
                  design.UpdateSize();
                  double x,y;
                  if(!gdiplus_useable){
                    x = left / bairitu + i * ( design.width + sukima_x / (bairitu * dotpermm_x) );
                    y = top / bairitu + j * ( design.height + sukima_y / (bairitu * dotpermm_y));
                  }else{
                    x = left / fudascale+ i * ( design.width + sukima_x / (dotpermm_x*fudascale) );
                    y = top / fudascale + j * ( design.height + sukima_y / (dotpermm_y*fudascale));
                  }
                  MYRECT rect;
                  rect.left = x;
                  rect.top = y;
                  rect.right = -1;
                  rect.bottom = -1;
                  DrawFuda(design,hPrintDC,&rect);
                  fudanum++;
                }
              }
              EndPage(hPrintDC);
            }

            EndDoc(hPrintDC);

            fudaunit = Gdiplus::UnitDisplay;
            fudascale = -1.0;
            HDC hdc = GetDC(hWndMain);
            LoadImageToDC(hdc,currentdesign);
            ReleaseDC(hWndMain,hdc);

            MessageBox(hDlg,"プリンタに出力しました。","完了",MB_OK);
          }
        }
        break;
      case IDCANCEL:
        {
          EndDialog(hDlg,0);
        }
        break;
      }
    }
    break;
  }
  return FALSE;
}

LRESULT CALLBACK MyPrintPreviewProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam){
  switch(message){
  case WM_PAINT:
    {
      PAINTSTRUCT ps;
      HDC hdc = BeginPaint(hWnd,&ps);
      if(hPrintDC == NULL){
        RECT r;
        GetClientRect(hWnd,&r);
        HBRUSH hbr = CreateSolidBrush(RGB(255,255,255));
        FillRect(hdc,&r,hbr);
        DeleteObject(hbr);
        DrawText(hdc,"プリンタを選択して下さい",-1,&r,DT_CENTER | DT_VCENTER | DT_SINGLELINE);
      }else{
        RECT r;
        GetClientRect(hWnd,&r);

        HBRUSH hbr = CreateSolidBrush(RGB(127,127,127));
        FillRect(hdc,&r,hbr);
        DeleteObject(hbr);

        int hr = GetDeviceCaps(hPrintDC,HORZRES);
        int vr = GetDeviceCaps(hPrintDC,VERTRES);
        double w,h;

        hbr = CreateSolidBrush(RGB(255,255,255));
        HPEN hpen = CreatePen(PS_SOLID,1,RGB(0,0,0));
        HBRUSH hbold = (HBRUSH)SelectObject(hdc,hbr);
        HPEN hpold = (HPEN)SelectObject(hdc,hpen);
        if(vr > hr){
          h = r.bottom - r.top;
          w = hr / (double)vr * h;
        }else{
          w = r.right - r.left;
          h = vr / (double)hr * w;
        }
        w *= 0.9;
        h *= 0.9;

        double scale = h / (double)vr;

        double x = (r.left + r.right)/2.0 - w / 2.0;
        double y = (r.top + r.bottom)/2.0 - h / 2.0;
        Rectangle(hdc,x,y,x+w,y+h);
        SelectObject(hdc,hpold);
        DeleteObject(hpen);

        //余白枠を描く

        double dotpermm_x = GetDeviceCaps(hPrintDC,LOGPIXELSX) / 25.4;
        double dotpermm_y = GetDeviceCaps(hPrintDC,LOGPIXELSY) / 25.4;

        hpen = CreatePen(PS_DOT,1,RGB(0,0,0));
        hpold = (HPEN)SelectObject(hdc,hpen);

        int pm_left, pm_top, pm_right, pm_bottom;
        GetPrintMargin(hDialog,&pm_left,&pm_top,&pm_right,&pm_bottom);

        double left = x + dotpermm_x * scale * pm_left;
        double right = x + w - dotpermm_x * scale * pm_right;
        double top = y + dotpermm_y * scale * pm_top;
        double bottom = y +h - dotpermm_y * scale * pm_bottom;
        Rectangle(hdc,left,top,right,bottom);

        SelectObject(hdc,hpold);
        DeleteObject(hpen);

        SelectObject(hdc,hbold);
        DeleteObject(hbr);

        //札の代わりとなる長方形を描く
        double bairitu = GetFudaBairitu(hDialog);

        double fudaw = currentdesign.width * bairitu * dotpermm_x * scale;
        double fudah = currentdesign.height * bairitu * dotpermm_y * scale;
        hpen = CreatePen(PS_SOLID,3,RGB(106,172,106));
        hbr = CreateSolidBrush(RGB(255,185,185));
        hbold = (HBRUSH)SelectObject(hdc,hbr);
        hpold = (HPEN)SelectObject(hdc,hpen);
        int cols,rows,sukima_x,sukima_y;
        CountOnePageFudaNum(hDialog,hPrintDC,&cols,&rows);
        GetFudaSukima(hDialog,hPrintDC,&sukima_x,&sukima_y);
        if(cols < 100 && rows < 100){
          for( int i = 0; i < cols; i++){
            for( int j = 0; j < rows; j++){
              double xx = left + i * (fudaw + sukima_x * scale);
              double yy = top + j * (fudah + sukima_y * scale);
              Rectangle(hdc,xx+1,yy+1,xx+fudaw-1,yy+fudah-1);
            }
          }
        }
        SelectObject(hdc,hpold);
        DeleteObject(hpen);
        SelectObject(hdc,hbold);
        DeleteObject(hbr);
        
      }
      EndPaint(hWnd,&ps);
    }
    break;

  }
  return CallWindowProc(oldMyPrintPreviewProc,hWnd,message,wParam,lParam);
}

void GetPrintMargin(HWND hDlg,int * left, int * top, int * right, int * bottom){
  char buf[16];
  GetDlgItemText(hDialog,IDC_PM_LEFT,buf,sizeof(buf)-1);
  *left = atoi(buf);
  GetDlgItemText(hDialog,IDC_PM_TOP,buf,sizeof(buf)-1);
  *top = atoi(buf);
  GetDlgItemText(hDialog,IDC_PM_RIGHT,buf,sizeof(buf)-1);
  *right = atoi(buf);
  GetDlgItemText(hDialog,IDC_PM_BOTTOM,buf,sizeof(buf)-1);
  *bottom = atoi(buf);
}

void CountOnePageFudaNum(HWND hDlg, HDC hdc, int * cols, int * rows){
  double dotpermm_x = GetDeviceCaps(hdc,LOGPIXELSX) / 25.4;
  double dotpermm_y = GetDeviceCaps(hdc,LOGPIXELSY) / 25.4;
  int hr = GetDeviceCaps(hPrintDC,HORZRES);
  int vr = GetDeviceCaps(hPrintDC,VERTRES);
  int l,t,r,b;
  GetPrintMargin(hDlg,&l,&t,&r,&b);
  double w = hr / dotpermm_x - l - r;
  double h = vr / dotpermm_y - t - b;
  double bairitu = GetFudaBairitu(hDlg);
  double fudaw = currentdesign.width * bairitu;
  double fudah = currentdesign.height * bairitu;
  double cls = w / fudaw;
  double rws = h / fudah;
  * cols = int(cls);
  if( cls - * cols == 0.0 ){
    (* cols) ++;
  }
  * rows = int(rws);
  if( rws - * rows == 0.0 ){
    (* rows) ++;
  }
  if( * cols <= 0 ){
    * cols = 1;
  }
  if( * rows <= 0 ){
    * rows = 1;
  }
}

void GetFudaSukima(HWND hDlg,HDC hdc,int * sukima_x, int * sukima_y){
  double dotpermm_x = GetDeviceCaps(hdc,LOGPIXELSX) / 25.4;
  double dotpermm_y = GetDeviceCaps(hdc,LOGPIXELSY) / 25.4;
  int hr = GetDeviceCaps(hPrintDC,HORZRES);
  int vr = GetDeviceCaps(hPrintDC,VERTRES);
  int l,t,r,b;
  GetPrintMargin(hDlg,&l,&t,&r,&b);
  double w = hr - (l+r) * dotpermm_x;
  double h = vr - (t+b) * dotpermm_y;
  double bairitu = GetFudaBairitu(hDlg);
  double fudaw = currentdesign.width * bairitu * dotpermm_x;
  double fudah = currentdesign.height * bairitu * dotpermm_y;
  int cols,rows;
  CountOnePageFudaNum(hDlg,hdc,&cols,&rows);
  if(cols <= 1){
    * sukima_x = 0;
  }else{
    * sukima_x = (w - fudaw * cols) / (double)(cols -1);
  }
  if(rows <= 1){
    * sukima_y = 0;
  }else{
    * sukima_y = (h - fudah * rows) / (double)(rows -1);
  }
}

double GetFudaBairitu(HWND hDlg){
  char buf[32];
  GetDlgItemText(hDlg,IDC_BAIRITU,buf,sizeof(buf)-1);
  double bairitu = atof(buf);
  if(bairitu <= 0.0){
    bairitu = 1.0;
  }
  return bairitu / 600.0;
}

int CountFudaToPrint(){
  int count = 0;
  if(tori_or_yomi == TORI){
    std::vector<class torifuda>::iterator it;
    for(it = torifuda.begin(); it != torifuda.end(); it++){
      if(it->saveasimage){
        count ++;
      }
    }
  }else{
    std::vector<class yomifuda>::iterator it;
    for(it = yomifuda.begin(); it != yomifuda.end(); it++){
      if(it->saveasimage){
        count ++;
      }
    }
  }
  return count;
}

int CountPages(HWND hDlg, HDC hdc){
  int cols, rows;
  CountOnePageFudaNum(hDlg,hdc, &cols, &rows);
  int count = CountFudaToPrint();
  int pages = count / (cols * rows);
  if( pages * cols * rows != count){
    pages ++;
  }
  if( pages == 0){
    pages = 1;
  }
  return pages;
}


