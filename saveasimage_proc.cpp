#include "kazesomiso.h"

char initial_browsedir[MAX_PATH] = "";
void SetToriYomi(HWND, HWND);
void UpdateSaveAsImage(HWND);

LRESULT CALLBACK  MyPrintProc(HWND, UINT, WPARAM, LPARAM);


int CALLBACK BrowseCallbackProc(HWND hwnd,UINT uMsg,LPARAM lp, LPARAM pData)
{
    switch (uMsg)
    {
        case BFFM_INITIALIZED:
            SendMessage(hwnd, BFFM_SETSELECTION, TRUE, (LPARAM)initial_browsedir);
            break;
        default:
            break;
    }
    return 0;
}

void UpdateMainDC(){
  HDC hdc = GetDC(hWndMain);
  LoadImageToDC(hdc,currentdesign);
  ReleaseDC(hWndMain,hdc);
}

LRESULT CALLBACK SaveAsImageProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
  HWND hQuality = GetDlgItem(hDlg,IDC_QUALITY);
  HWND hFileType = GetDlgItem(hDlg,IDC_FILETYPE);
  HWND hFilePrefix = GetDlgItem(hDlg,IDC_FILEPREFIX);
  HWND hFudaList = GetDlgItem(hDlg,IDC_FUDALIST);
  HWND hSelectedNum = GetDlgItem(hDlg,IDC_SELECTEDNUM);
  HWND hToriYomi = GetDlgItem(hDlg,IDC_TORIORYOMI);
  HWND hImageScale = GetDlgItem(hDlg,IDC_IMAGE_SCALE);
  switch (message)
  {
  case WM_INITDIALOG:
    {

      MoveWindowToRight(hDlg);
      SetWindowText(hQuality,"80");
      SetWindowText(hImageScale,"100");

      SendMessage(hFileType,CB_INSERTSTRING,(WPARAM)0,(LPARAM)"PNG");
      SendMessage(hFileType,CB_INSERTSTRING,(WPARAM)1,(LPARAM)"JPG");
      SendMessage(hFileType,CB_INSERTSTRING,(WPARAM)2,(LPARAM)"GIF");
      SendMessage(hFileType,CB_INSERTSTRING,(WPARAM)3,(LPARAM)"BMP");
      SendMessage(hFileType,CB_SETCURSEL,(WPARAM)0,(LPARAM)0);

      SendMessage(hToriYomi,CB_INSERTSTRING,(WPARAM)0,(LPARAM)"取札");
      SendMessage(hToriYomi,CB_INSERTSTRING,(WPARAM)1,(LPARAM)"詠札");
      if(tori_or_yomi == TORI){
        SendMessage(hToriYomi,CB_SETCURSEL,(WPARAM)0,(LPARAM)0);
      }else{
        SendMessage(hToriYomi,CB_SETCURSEL,(WPARAM)1,(LPARAM)0);
      }

      SetToriYomi(hFilePrefix,hFudaList);

      int r = SendMessage(hFudaList,LB_GETSELCOUNT,(WPARAM)0,(LPARAM)0);
      char buf[16];
      wsprintf(buf,"%d",r);
      SetWindowText(hSelectedNum,buf);
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
      if(id == IDC_SCR_QUALITY){
        hChange = hQuality;
        minimum = 0;
        maximum = 100;
      }else if(id == IDC_SCR_IMAGE_SCALE){
        hChange = hImageScale;
        minimum = 1;
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
      case IDC_PRINTFUDA:
        {
          UpdateSaveAsImage(hFudaList);
          DialogBox(hInst,MAKEINTRESOURCE(IDD_MYPRINTDLG),hDlg,(DLGPROC)MyPrintProc);
        }
        break;
      case IDC_TORIORYOMI:
        {
          if(iEvent == CBN_SELCHANGE){
            int r = SendMessage(hToriYomi,CB_GETCURSEL,(WPARAM)0,(LPARAM)0);
            if(r == 0){
              ChangeToriYomi(TORI);
            }else{
              ChangeToriYomi(YOMI);
            }
            SendMessage(hFudaList,LB_RESETCONTENT,(WPARAM)0,(LPARAM)0);
            SetToriYomi(hFilePrefix, hFudaList);
            r = SendMessage(hFudaList,LB_GETSELCOUNT,(WPARAM)0,(LPARAM)0);
            char buf[16];
            wsprintf(buf,"%d",r);
            SetWindowText(hSelectedNum,buf);
          }
        }
        break;
      case IDC_SELECTALL: case IDC_CANCELALL:
        {
          BOOL flag = TRUE;
          if(iId == IDC_SELECTALL){
            flag = TRUE;
          }else if(iId == IDC_CANCELALL){
            flag = FALSE;
          }
          int r = SendMessage(hFudaList,LB_GETCOUNT,(WPARAM)0,(LPARAM)0);
          for(int i=0; i < r; i++){
            SendMessage(hFudaList,LB_SETSEL,(WPARAM)flag,(LPARAM)i);
          }
          r = SendMessage(hFudaList,LB_GETSELCOUNT,(WPARAM)0,(LPARAM)0);
          char buf[16];
          wsprintf(buf,"%d",r);
          SetWindowText(hSelectedNum,buf);
        }
        break;
      case IDC_FUDALIST:
        {
          if(iEvent == LBN_SELCHANGE){
            int r = SendMessage(hFudaList,LB_GETSELCOUNT,(WPARAM)0,(LPARAM)0);
            char buf[16];
            wsprintf(buf,"%d",r);
            SetWindowText(hSelectedNum,buf);
          }
        }
        break;
      case IDOK:
        {
           char buf[32];
           GetDlgItemText(hDlg,IDC_IMAGE_SCALE,buf,sizeof(buf)-1);
           double scale;
           if(!strcmp(buf,"100")){
             scale = 1.0;
           }else{
             scale = atof(buf) / 100.0;
           }

           UpdateSaveAsImage(hFudaList);
           const char * ext[] = {"png","jpg","gif","bmp"};
           int filetype = 0;
           filetype = SendMessage(hFileType,CB_GETCURSEL,(WPARAM)0,(LPARAM)0);

           BROWSEINFO bi;
           ZeroMemory(&bi,sizeof(bi));
           bi.hwndOwner = hDlg;
           bi.lpszTitle = "保存先の選択";
           bi.lpfn = BrowseCallbackProc;
           bi.ulFlags = BIF_RETURNONLYFSDIRS;
           LPITEMIDLIST pidl;
           LPMALLOC pMalloc;
           char dirname[MAX_PATH];
           dirname[0] = '\0';
           if((pidl = SHBrowseForFolder(&bi))!=NULL){
             ChangeDisplayColorToMax();
             HDC hdc = CreateDC("DISPLAY",NULL,NULL,NULL);
             fudascale = scale;
             fudascale_x = scale;
             fudascale_y = scale;
             LoadImageToDC(hdc,currentdesign);
             fudascale = -1.0;
             DeleteDC(hdc);

             if(SHGetMalloc(&pMalloc) != E_FAIL && SHGetPathFromIDList(pidl,dirname)){
               pMalloc->Free(pidl);
               pMalloc->Release();
             }
             if(dirname[0] == '\0'){
               dirname[0] = '.';
               dirname[1] = '\0';
             }
             lstrcpy(initial_browsedir,dirname);
             if(dirname[lstrlen(dirname)-1] != '\\'){
               lstrcat(dirname,"\\");
             }
             int loopsize = 0;
             if(tori_or_yomi == TORI){
               loopsize = torifuda.size();
             }else{
               loopsize = yomifuda.size();
             }
             for(std::vector<std::string>::size_type i = 0; i < loopsize; i++){
               if(tori_or_yomi == TORI && torifuda[i].saveasimage == FALSE ||
                  tori_or_yomi == YOMI && yomifuda[i].saveasimage == FALSE
                 ){
                 continue;
               }
               const char * BMP_TEMP = "kazesomiso_temp.bmp";
               const char * GIF_TEMP = "kazesomiso_temp.gif";
               const char * JPG_TEMP = "kazesomiso_temp.jpg";
               const char * PNG_TEMP = "kazesomiso_temp.png";

               char filename[MAX_PATH];
               char prefix[64];
               GetWindowText(hFilePrefix,prefix,sizeof(prefix)-1);
               wsprintf(filename,"%s\\%s%03d.%s",dirname,prefix,i+1,ext[filetype]);

               if(GetFileAttributes(filename) != -1){
                 char buf[200];
                 wsprintf(buf,"%s%03d.%s は存在します。上書きしますか？",prefix,i+1,ext[filetype]);
                 int r = MessageBox(hDlg,buf,"確認",MB_YESNOCANCEL);
                 if(r == IDNO){
                   continue;
                 }else if(r == IDCANCEL){
                   UpdateMainDC();
                   return FALSE;
                 }
               }

               if(filetype == 0 || filetype == 1 || filetype == 2){
                 SaveFudaToBitmap(i,BMP_TEMP,currentdesign,scale);
                 BOOL r;
                 char * p;
                 if(filetype == 0){
                   r = Bmp2Png(BMP_TEMP,PNG_TEMP);
                   p = (char *)PNG_TEMP;
                 }else if(filetype == 1){
                   char buf[16];
                   GetWindowText(hQuality,buf,sizeof(buf)-1);
                   r = Bmp2Jpg(BMP_TEMP,JPG_TEMP,atoi(buf));
                   p = (char *)JPG_TEMP;
                 }else if(filetype == 2){
                   r = Bmp2Gif(BMP_TEMP,GIF_TEMP);
                   p = (char *)GIF_TEMP;
                 }
                 DeleteFile(BMP_TEMP);

                 if(!r || GetFileAttributes(p) == -1){
                   MessageBox(hDlg,"エラー: 画像を作成できませんでした。","エラー",MB_OK);
                   UpdateMainDC();
                   return FALSE;
                 }

                 DeleteFile(filename);
                 if(!MoveFile(p,filename)){
                   MessageBox(hDlg,"エラー: 作成した画像をフォルダに移動できませんでした。失敗です。","エラー",MB_OK);
                   DeleteFile(p);
                   UpdateMainDC();
                   return FALSE;
                 }
               }else{
                 SaveFudaToBitmap(i,filename,currentdesign,scale);
               }
             }
             HWND ownerwindow = GetParent(hWndMain);
             InvalidateRect(ownerwindow,NULL,TRUE);
             UpdateWindow(ownerwindow);
             MessageBox(hDlg,"完了しました","完了",MB_OK);
             UpdateMainDC();

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
    return FALSE;
    break;
  }
  return FALSE;
}

void SetToriYomi(HWND hFilePrefix, HWND hFudaList){

  if(tori_or_yomi == TORI){
    SetWindowText(hFilePrefix,"tori_");
    for(int i = 0; i < torifuda.size(); i++){
      int r = SendMessage(hFudaList,LB_ADDSTRING,(WPARAM)0,(LPARAM)torifuda[i].kimariji.c_str());
      SendMessage(hFudaList,LB_SETITEMDATA,(WPARAM)r,(LPARAM)i);
      SendMessage(hFudaList,LB_SETSEL,(WPARAM)TRUE,(LPARAM)r);
      torifuda[i].saveasimage = TRUE;
    }
  }else{
    SetWindowText(hFilePrefix,"yomi_");
    for(int i = 0; i < yomifuda.size(); i++){
      std::string ss = RemoveFuriganaAndTrunc(yomifuda[i].waka);

      int r = SendMessage(hFudaList,LB_INSERTSTRING,(WPARAM)-1,(LPARAM)ss.c_str());
      SendMessage(hFudaList,LB_SETITEMDATA,(WPARAM)r,(LPARAM)i);
      SendMessage(hFudaList,LB_SETSEL,(WPARAM)TRUE,(LPARAM)r);
      yomifuda[i].saveasimage = TRUE;
    }
  }
}


void UpdateSaveAsImage(HWND hFudaList){
  int r = SendMessage(hFudaList,LB_GETCOUNT,(WPARAM)0,(LPARAM)0);
  for(int i = 0; i < r; i++){
    int rr = SendMessage(hFudaList,LB_GETSEL,(WPARAM)i,(LPARAM)0);
    int index = SendMessage(hFudaList,LB_GETITEMDATA,(WPARAM)i,(LPARAM)0);
    if(tori_or_yomi == TORI){
      if(rr > 0){
        torifuda[index].saveasimage = TRUE;
      }else{
        torifuda[index].saveasimage = FALSE;
      }
    }else{
      if(rr > 0){
        yomifuda[index].saveasimage = TRUE;
      }else{
        yomifuda[index].saveasimage = FALSE;
      }
    }
  }
}
