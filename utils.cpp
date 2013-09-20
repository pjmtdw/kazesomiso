// utils.cc

#include "kazesomiso.h"

void EnableMenuAll(HWND hwnd, BOOL flag){
  HMENU hmenu = GetMenu(hwnd);
  int count = GetMenuItemCount(hmenu);

  UINT enable = MF_BYPOSITION;;
  if(flag){
    enable |= MF_ENABLED;
  }else{
    enable |= MF_GRAYED;
  }

  for(int i = 0; i < count; i++){
    EnableMenuItem(hmenu,i,enable);
  }
}
// 0 以上 max 未満の乱数を返す
int myrandint(int max){
  if(max <= 0){
    return 0;
  }
  int r = int((double)max * ((double)rand() / ((double)RAND_MAX + 1.0)));
  if( r < 0 || r >= max ){
    return 0; //ありえないはず。本当はassertしたい
  }
  return r;
}


void SetCurrentDirectoryToExePath(){
  char path[MAX_PATH];
  GetModuleFileName(NULL,path,MAX_PATH);
  char * p = strrchr(path,'\\');
  if(p != NULL){
    * p = '\0';
    SetCurrentDirectory(path);
  }
}


int cfhookproc_flag = 0;

LOGFONT * choosefontlog = NULL;

struct fontfacelist_cell{
  char facename[LF_FACESIZE];
  DWORD itemdata;
};

BOOL setfontsizelistflag = TRUE;
void SetFontSizeList(HWND hDlg){
  if(!setfontsizelistflag){
    return;
  }
  const int MAXIMUM = 128;
  HWND h = GetDlgItem(hDlg,cmb3);

  char currenttext[16];
  GetWindowText(h,currenttext,sizeof(currenttext)-1);
  char buf[16];
  int count = SendMessage(h,CB_GETCOUNT,(WPARAM)0,(LPARAM)0);
  SendMessage(h,CB_GETLBTEXT,(WPARAM)0,(LPARAM)buf);
  int currentminimum = atoi(buf);
  SendMessage(h,CB_GETLBTEXT,(WPARAM)count-1,(LPARAM)buf);
  int currentmaximum = atoi(buf);
  if(currentminimum < 0){currentminimum = 0;};
  if(currentmaximum < 0){currentmaximum = 0;};
  SendMessage(h,CB_RESETCONTENT,(WPARAM)0,(LPARAM)0);
  int icurrenttext = atoi(currenttext);
  BOOL foundflag = FALSE;
  int idivmin = -1;
  int divmin = 9999;
  for(int i = currentmaximum; i >= currentminimum; i -= 6){
    char b[16];
    wsprintf(b,"%d",i);
    SendMessage(h,CB_INSERTSTRING,(WPARAM)0,(LPARAM)b);
    if(icurrenttext == i){
      foundflag = TRUE;
    }else if(icurrenttext - i > 0 && icurrenttext - i < divmin){
      divmin = icurrenttext - i;
      idivmin = i;
    }
  }
  for(int i = currentmaximum + 6; i <= MAXIMUM; i+= 6){
    char b[16];
    wsprintf(b,"%d",i);
    SendMessage(h,CB_INSERTSTRING,(WPARAM)-1,(LPARAM)b);
    if(icurrenttext == i){
      foundflag = TRUE;
    }else if(icurrenttext - i > 0 && icurrenttext - i < divmin){
      divmin = icurrenttext - i;
      idivmin = i;
    }
  }
  if(!foundflag){
    char b[16];
    wsprintf(b,"%d",idivmin);
    int pos = SendMessage(h,CB_FINDSTRING,(WPARAM)-1,(LPARAM)b);
    if(pos != CB_ERR){
      SendMessage(h,CB_INSERTSTRING,(WPARAM)pos+1,(LPARAM)currenttext);
    }
  }
  SetWindowText(h,currenttext);
  int pos = SendMessage(h,CB_FINDSTRING,(WPARAM)-1,(LPARAM)currenttext);
  if(pos != CB_ERR){
    SendMessage(h,CB_SETCURSEL,(WPARAM)pos,(LPARAM)0);
  }
}

std::vector<struct fontfacelist_cell> fontfacelist;
std::vector<std::string> vertfontfacelist;

// vetfontfacelist から縦書きフォント一覧を取得して
// 縦書きフォントが存在しているかチェックして、存在していたら縦書きに変換する
// vetfontfacelist は CFHookProc の WM_INITDIALOG で作られる
void ConvertToTateGaki(LOGFONT & lf){
  if(lf.lfFaceName[0] == '@'){
    lf.lfEscapement = 2700;
    lf.lfOrientation = 2700;
    return;
  }
  char buf[LF_FACESIZE] = "@";
  lstrcat(buf,lf.lfFaceName);
  if(std::find(vertfontfacelist.begin(),vertfontfacelist.end(),std::string(buf)) == vertfontfacelist.end()){
    return;
  }
  lstrcpy(lf.lfFaceName,buf);
  lf.lfEscapement = 2700;
  lf.lfOrientation = 2700;
}

struct CF_CUSTDATA{
  LOGFONT * lf;
  BOOL largefont;
};
UINT_PTR CALLBACK CFHookProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam){
  switch(message){
    case WM_INITDIALOG:
      {

        struct CF_CUSTDATA * p = (struct CF_CUSTDATA *)(((CHOOSEFONT *) lParam) -> lCustData);
        choosefontlog = p->lf;
        setfontsizelistflag = p->largefont;

        //フォント一覧をfontfacelistに蓄える。
        //@で始まるフォントはvertfontfacelistに蓄え、コンボボックスからは削除
        HWND h = GetDlgItem(hDlg,cmb1);
        int count = SendMessage(h,CB_GETCOUNT,(WPARAM)0,(LPARAM)0);
        fontfacelist.clear();
        vertfontfacelist.clear();
        
        for(int i =0; i < count ; i++){
          DWORD id = SendMessage(h,CB_GETITEMDATA,(WPARAM)i,(LPARAM)0);
          char buf[LF_FACESIZE];
          if(CB_ERR == SendMessage(h,CB_GETLBTEXT,(WPARAM)i,(LPARAM)buf)){
            break;
          }
          if(buf[0] != '@'){
            struct fontfacelist_cell fc;
            lstrcpy(fc.facename,buf);
            fc.itemdata = id;
            fontfacelist.push_back(fc);
          }else{
            vertfontfacelist.push_back(std::string(buf));
            count = SendMessage(h,CB_DELETESTRING,(WPARAM)i,(LPARAM)0);
            i --;
          }
        }
        if(choosefontlog->lfFaceName[0] == '@')
        SetWindowText(h,choosefontlog->lfFaceName + 1);
        int pos = SendMessage(h,CB_FINDSTRING,(WPARAM)-1,(LPARAM)choosefontlog->lfFaceName + 1);
        if(pos != CB_ERR){
         SendMessage(h,CB_SETCURSEL,(WPARAM)pos,(LPARAM)0);
        }


        SetFontSizeList(hDlg);
      }
      return TRUE;
      break;
    case WM_VSCROLL:
      {
        int iId = LOWORD(wParam);
  
        int id = GetDlgCtrlID((HWND)lParam);
  
        HWND hChange = NULL;
        int maximum = -1;
        int minimum = -1;
        if(id == IDC_SCR_FONTSIZE){
          hChange = GetDlgItem(hDlg,cmb3);
          minimum = 1;
        }
        if(hChange != NULL){
          DoWhenVscroll(hChange,iId,minimum,maximum);
        }
        if(id == IDC_SCR_FONTSIZE){
          SendMessage(hDlg,WM_COMMAND,MAKEWPARAM(cmb3,CBN_EDITCHANGE),(LPARAM)hChange);
        }
      }
      break;
    case WM_COMMAND:
      {
        int iId = LOWORD(wParam);
        int iEvent = HIWORD(wParam);
        if(
          (iEvent == CBN_SELCHANGE || iEvent == CBN_EDITCHANGE) && 
          (iId == cmb1 || iId == cmb2 || iId == cmb3)
        ){
            if(cfhookproc_flag == 0 && choosefontlog != NULL){
              cfhookproc_flag = 1;
              DLGPROC proc = (DLGPROC)GetWindowLongPtr(hDlg,DWL_DLGPROC);
              if(proc != NULL){
                CallWindowProc((WNDPROC)proc,hDlg,message,wParam,lParam);

                cfhookproc_flag = 0;

                LOGFONT curlogfont;
                SendMessage(hDlg,WM_CHOOSEFONT_GETLOGFONT,(WPARAM)0,(LPARAM)&curlogfont);
                memcpy(choosefontlog,&curlogfont,sizeof(LOGFONT));

                choosefontlog -> lfQuality = ANTIALIASED_QUALITY;
                ConvertToTateGaki(*choosefontlog);

                UpdateMainWindow(FALSE);

                if(iId == cmb1 && iEvent == CBN_SELCHANGE){
                  SetFontSizeList(hDlg);
                }
              }else{
                return FALSE;
              }
              return TRUE;
            }
          return FALSE;

        }else if(iId == IDC_FONTRANDOM){
          HWND h = GetDlgItem(hDlg,cmb1);
          int count = SendMessage(h,CB_GETCOUNT,(WPARAM)0,(LPARAM)0);
          int r = myrandint(count);
          SendMessage(h,CB_SETCURSEL,(WPARAM)r,(LPARAM)0);
          SendMessage(hDlg,WM_COMMAND,MAKEWPARAM(cmb1,CBN_SELCHANGE),(LPARAM)h);
        }else if(iId == IDC_FONTSEARCH){
          if(iEvent == EN_CHANGE){
            HWND h = GetDlgItem(hDlg,cmb1);
            SendMessage(h,CB_RESETCONTENT,(WPARAM)0,(LPARAM)0);
            std::vector<struct fontfacelist_cell>::iterator it;
            char buf[128];
            GetDlgItemText(hDlg,IDC_FONTSEARCH,buf,sizeof(buf)-1);
            for(it = fontfacelist.begin(); it != fontfacelist.end(); it++){
              if(*buf == '\0' || NULL != strstr(it->facename,buf)){
                int index = SendMessage(h,CB_INSERTSTRING,(WPARAM)-1,(LPARAM)(it->facename));
                SendMessage(h,CB_SETITEMDATA,(WPARAM)index,(LPARAM)(it->itemdata));
              }
            }
          }
        }
      }
      break;
  }
  return FALSE;
}


#define COLOR_CURRENT 709 // ColorDlg.hからもらってきた

COLORREF * cchoosecolor = NULL;
UINT_PTR CALLBACK CCHookProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam){
  if(message == WM_INITDIALOG){
    cchoosecolor = (COLORREF *) (((CHOOSECOLOR *)lParam) -> lCustData);
    return TRUE;
  }else if(message == WM_CTLCOLORSTATIC && cchoosecolor != NULL){
    HWND h = GetDlgItem(hDlg,COLOR_CURRENT);
    if( h == NULL ){
      return FALSE;
    }
    if((HWND)lParam == h){
      HDC hdc = GetDC(h);
      COLORREF c = GetPixel(hdc,0,0);
      ReleaseDC(h,hdc);
    *   cchoosecolor = c;
      UpdateMainWindow(FALSE);
    }
  }
  return FALSE;
}

void CallChooseColor(HWND hDlg,COLORREF & cr){
  CHOOSECOLOR cc;
  COLORREF prev = cr;
  ZeroMemory(&cc,sizeof(cc));
  cc.lStructSize = sizeof(CHOOSECOLOR);
  cc.Flags = CC_RGBINIT | CC_FULLOPEN | CC_ENABLEHOOK;
  cc.lpfnHook = CCHookProc;
  cc.lCustData = (LPARAM)&(cr);
  cc.rgbResult = prev;
  cc.hwndOwner = hDlg;
  DWORD dwCust[16];
  ZeroMemory(dwCust,sizeof(dwCust));
  cc.lpCustColors = dwCust;
  if(!ChooseColor(&cc)){
    cr = prev;
  }else{
    cr = cc.rgbResult;
  }
}

void CallChooseFont(HWND hDlg,LOGFONT & logfont,BOOL largefont){
  CHOOSEFONT cf;
  LOGFONT lf, lfbkup;
  memcpy(&lf,&logfont,sizeof(LOGFONT));
  memcpy(&lfbkup,&logfont,sizeof(LOGFONT));
  struct CF_CUSTDATA ccd;
  ccd.lf = &logfont;
  ccd.largefont = largefont;
  ZeroMemory(&cf,sizeof(cf));
  cf.lStructSize = sizeof(cf);
  cf.hwndOwner = hDlg;
  cf.lpLogFont = &lf;
  cf.Flags = CF_SCREENFONTS | CF_SELECTSCRIPT | CF_INITTOLOGFONTSTRUCT | CF_ENABLEHOOK | CF_ENABLETEMPLATE;
  cf.hInstance = hInst;
  cf.lpTemplateName = MAKEINTRESOURCE(IDD_MYCHOOSEFONT);
  cf.lpfnHook = CFHookProc;
  cf.lCustData = (LPARAM)&ccd;
  cf.nFontType = SCREEN_FONTTYPE;
  if(ChooseFont(&cf)){
    memcpy(&logfont,&lf,sizeof(LOGFONT));
  }else{
    memcpy(&logfont,&lfbkup,sizeof(LOGFONT));
  }
  logfont.lfQuality = ANTIALIASED_QUALITY;
  ConvertToTateGaki(logfont);
}

void UpdateMainWindow(BOOL flag){
  if(front_structure.hdc != NULL){
    DeleteObject(front_structure.hbmp);
    DeleteDC(front_structure.hdc);
    front_structure.hdc = NULL;
  }

  if(front_structure.hdc == NULL){
    int width = currentdesign.width;
    int height = currentdesign.height;
    
    HDC hdc = GetDC(hWndMain);
    front_structure.hdc = CreateCompatibleDC(hdc);
    front_structure.hbmp = CreateCompatibleBitmap(hdc,width,height);
    HBITMAP hold = (HBITMAP)SelectObject(front_structure.hdc,front_structure.hbmp);
    DrawFuda(currentdesign, front_structure.hdc,NULL);
    SelectObject(front_structure.hdc,hold);
    ReleaseDC(hWndMain,hdc);
  }

  currentdesign.UpdateSize();
  InvalidateRect(hWndMain,NULL,flag);
  UpdateWindow(hWndMain);
}

void FixWindowSizeToFudaSize(){
  currentdesign.UpdateSize();
  RECT rect;
  GetWindowRect(hWndMain,&rect);
  RECT rect2;
  GetClientRect(hWndMain,&rect2);
  int w = ( rect.right - rect.left ) - (rect2.right - rect2.left);
  int h = ( rect.bottom - rect.top ) - (rect2.bottom - rect2.top);

  int cw = currentdesign.width;
  int ch = currentdesign.height;

  if( cw < 100 ){
    cw = 100;
  }
  if( ch < 100 ){
    ch = 100;
  }
  MoveWindow(hWndMain,rect.left,rect.top,cw+w,ch+h,TRUE);
}


void ChangeToriYomi(enum TORIYOMI newtoriyomi){
  if(newtoriyomi == tori_or_yomi){
    return;
  }
  tori_or_yomi = newtoriyomi;
  currentdesign.UpdateSize();
  FixWindowSizeToFudaSize();
  HDC hdc = GetDC(hWndMain);
  LoadImageToDC(hdc,currentdesign);
  ReleaseDC(hWndMain,hdc);
  UpdateMainWindow(TRUE);
}

void MoveWindowToRight(HWND hDlg){
  RECT rect1, rect2;
  GetWindowRect(hWndMain,&rect1);
  GetWindowRect(hDlg,&rect2);
  int y = rect1.top;
  int x = rect1.right;
  int scWidth = GetSystemMetrics(SM_CXSCREEN);
  if(scWidth > 0 && x + (rect2.right - rect2.left) > scWidth){
    x = scWidth - (rect2.right - rect2.left);
  }
  SetWindowPos(hDlg,NULL,x,y,0,0,SWP_NOSIZE|SWP_NOZORDER);
}

HWND prevChange = NULL;
WORD prevChangeId = 0;
int changecounter = 0;
void DoWhenVscroll(HWND hChange,int iId,int minimum,int maximum){
  if(hChange == NULL){
    return;
  }
  char buf[16];
  GetWindowText(hChange,buf,sizeof(buf)-1);
  int i = atoi(buf);
  int d = 1;
  if(prevChange != hChange || prevChangeId != iId){
    changecounter = 0;
  }
  if(changecounter < 10){
    d = 1;
  }else if(changecounter < 15){
    d = 2;
  }else if(changecounter < 20){
    d = 5;
  }else{
    d = 10;
  }
  if(iId == SB_LINEUP){
    i+=d;
  }else if(iId == SB_LINEDOWN){
    i-=d;
  }

  if(minimum != -1 && i < minimum)i=minimum;
  if(maximum != -1 && i > maximum)i=maximum;

  wsprintf(buf,"%d",i);
  SetWindowText(hChange,buf);
  prevChange = hChange;
  prevChangeId = iId;
  changecounter ++;

}

void ChangeDisplayColorToMax(){
  HDC hdc = CreateDC("DISPLAY",NULL,NULL,NULL);
  int curbpp = GetDeviceCaps(hdc,BITSPIXEL);
  DeleteDC(hdc);
  if( curbpp < 32 ){
    int maxbpp = -9999;
    for(int bpp = 32; bpp >= 16; bpp -= 8){
      DEVMODE dv;
      ZeroMemory(&dv,sizeof(DEVMODE));
      dv.dmSize = sizeof(DEVMODE);
      dv.dmFields = DM_BITSPERPEL;
      dv.dmBitsPerPel = bpp;
      if(ChangeDisplaySettings(&dv,CDS_TEST) == DISP_CHANGE_SUCCESSFUL){
        if(bpp > maxbpp){
          maxbpp = bpp;
          break;
        }
      }
    }
    if( maxbpp > 0 && curbpp < maxbpp){
      DEVMODE dv;
      ZeroMemory(&dv,sizeof(DEVMODE));
      dv.dmSize = sizeof(DEVMODE);
      dv.dmFields = DM_BITSPERPEL;
      dv.dmBitsPerPel = maxbpp;
      ChangeDisplaySettings(&dv,0);
    }
  }
}
