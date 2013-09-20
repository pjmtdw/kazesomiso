#include "kazesomiso.h"

BOOL fixaspectratioflag = TRUE;
BOOL initdialogmode = FALSE;

class BaseDesign * saved_design;

LRESULT CALLBACK BaseDesignProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK ToriDesignProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{

  HWND hMarginH = GetDlgItem(hDlg,IDC_MARGIN_H);
  HWND hMarginV = GetDlgItem(hDlg,IDC_MARGIN_V);

  HWND hKmSukimaH = GetDlgItem(hDlg,IDC_KIMARIJI_SUKIMA_HORIZONAL);
  HWND hKmSukimaV = GetDlgItem(hDlg,IDC_KIMARIJI_SUKIMA_VERTICAL);
  HWND hKmMarginH = GetDlgItem(hDlg,IDC_KIMARIJI_MARGIN_HORIZONAL);
  HWND hKmMarginV = GetDlgItem(hDlg,IDC_KIMARIJI_MARGIN_VERTICAL);


  BaseDesignProc(hDlg,message,wParam,lParam);

  switch (message)
  {
  case WM_INITDIALOG:
    {

      initdialogmode = TRUE;

      char buf[16];
      wsprintf(buf,"%d",currentdesign.tori->kimariji_sukima_h);
      SetWindowText(hKmSukimaH,buf);
      wsprintf(buf,"%d",currentdesign.tori->kimariji_sukima_v);
      SetWindowText(hKmSukimaV,buf);
      wsprintf(buf,"%d",currentdesign.tori->kimariji_margin_h);
      SetWindowText(hKmMarginH,buf);
      wsprintf(buf,"%d",currentdesign.tori->kimariji_margin_v);
      SetWindowText(hKmMarginV,buf);

      wsprintf(buf,"%d",currentdesign.tori->margin_left);
      SetWindowText(hMarginH,buf);

      wsprintf(buf,"%d",currentdesign.tori->margin_top);
      SetWindowText(hMarginV,buf);

      if(currentdesign.tori->show_kimariji){
        CheckDlgButton(hDlg,IDC_KIMARIJI_ON,BST_CHECKED);
      }

      if(currentdesign.tori->convert_dakuon){
        CheckDlgButton(hDlg,IDC_CONVERTDAKUON,BST_CHECKED);
      }

      initdialogmode = FALSE;

    }
    return TRUE;
  case WM_VSCROLL:
    {
      int iId = LOWORD(wParam);

      int id = GetDlgCtrlID((HWND)lParam);

      HWND hChange = NULL;
      int maximum = -1;
      int minimum = -1;
      if(id == IDC_SCRMARGIN_H){
        hChange = hMarginH;
        minimum = 0;
      }else if(id == IDC_SCRMARGIN_V){
        hChange = hMarginV;
        minimum = 0;
      }else if(id == IDC_KIMARIJI_SCR_SUKIMA_HORIZONAL){
        hChange = hKmSukimaH;
      }else if(id == IDC_KIMARIJI_SCR_SUKIMA_VERTICAL){
        hChange = hKmSukimaV;
      }else if(id == IDC_KIMARIJI_SCR_MARGIN_HORIZONAL){
        hChange = hKmMarginH;
      }else if(id == IDC_KIMARIJI_SCR_MARGIN_VERTICAL){
        hChange = hKmMarginV;
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
      case IDC_KIMARIJI_ON:
        {
          if(IsDlgButtonChecked(hDlg,IDC_KIMARIJI_ON) == BST_CHECKED){
            currentdesign.tori->show_kimariji = TRUE;
          }else{
            currentdesign.tori->show_kimariji = FALSE;
          }
        }
        break;
      case IDC_CONVERTDAKUON:
        {
          if(IsDlgButtonChecked(hDlg,IDC_CONVERTDAKUON) == BST_CHECKED){
            currentdesign.tori->convert_dakuon = TRUE;
          }else{
            currentdesign.tori->convert_dakuon = FALSE;
          }
        }
        break;
      case IDC_KIMARIJI_FONT:
        CallChooseFont(hDlg,currentdesign.tori->logfont_of_kimariji,TRUE);
        break;
      case IDC_KIMARIJI_COLOR:
        CallChooseColor(hDlg,currentdesign.tori->kimariji_color);
        break;
      case IDC_KIMARIJI_SUKIMA_HORIZONAL: case IDC_KIMARIJI_SUKIMA_VERTICAL:
      case IDC_KIMARIJI_MARGIN_HORIZONAL: case IDC_KIMARIJI_MARGIN_VERTICAL:
      case IDC_MARGIN_H: case IDC_MARGIN_V:
        {
          if(iEvent == EN_UPDATE){
            fixaspectratioflag = TRUE;
          }else if(iEvent == EN_CHANGE){
            int i;
            char buf[16];

            GetWindowText(hMarginH,buf,sizeof(buf)-1);
            if(buf[0] != '\0'){
              i = atoi(buf);
              currentdesign.tori->margin_left = i;
              currentdesign.tori->margin_right = i;
            }
            GetWindowText(hMarginV,buf,sizeof(buf)-1);
            if(buf[0] != '\0'){
              i = atoi(buf);
              currentdesign.tori->margin_top = i;
              currentdesign.tori->margin_bottom = i;
            }
            GetWindowText(hKmSukimaH,buf,sizeof(buf)-1);
            if(buf[0] != '\0'){
              i = atoi(buf);
              currentdesign.tori->kimariji_sukima_h = i;
            }
            GetWindowText(hKmSukimaV,buf,sizeof(buf)-1);
            if(buf[0] != '\0'){
              i = atoi(buf);
              currentdesign.tori->kimariji_sukima_v = i;
            }
            GetWindowText(hKmMarginH,buf,sizeof(buf)-1);
            if(buf[0] != '\0'){
              i = atoi(buf);
              currentdesign.tori->kimariji_margin_h = i;
            }
            GetWindowText(hKmMarginV,buf,sizeof(buf)-1);
            if(buf[0] != '\0'){
              i = atoi(buf);
              currentdesign.tori->kimariji_margin_v = i;
            }
          }
        }
        break;
      case IDOK: case IDCANCEL:
        {

          EndDialog(hDlg, LOWORD(wParam));
          return TRUE;
        }
        break;
      }
    }
    if(!initdialogmode && fixaspectratioflag){
      int iEvent = HIWORD(wParam);
      if(iEvent == CBN_SELCHANGE || iEvent == BN_CLICKED || iEvent == EN_CHANGE){

        HDC hdc = GetDC(hWndMain);
        LoadImageToDC(hdc,currentdesign);
        ReleaseDC(hWndMain,hdc);

        UpdateMainWindow(FALSE);

      }
    } 
    break;
  }
  return FALSE;
}

LRESULT CALLBACK YomiDesignProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{

  HWND hMargintop = GetDlgItem(hDlg,IDC_MARGINTOP);
  HWND hMarginh = GetDlgItem(hDlg,IDC_MARGINH);
  HWND hRownum = GetDlgItem(hDlg,IDC_ROWNUM);
  HWND hFuriganaSukima = GetDlgItem(hDlg,IDC_FURIGANA_SUKIMA);

  BaseDesignProc(hDlg,message,wParam,lParam);

  switch (message)
  {
  case WM_INITDIALOG:
    {

      initdialogmode = TRUE;

      if(currentdesign.yomi->show_furigana){
        CheckDlgButton(hDlg,IDC_FURIGANA_ON,BST_CHECKED);
      }

      char buf[16];
      for(int i = 0; i < yomimaxrownum; i ++){
        wsprintf(buf,"%d行目",i+1);
        SendMessage(hRownum,CB_INSERTSTRING,(WPARAM)i,(LPARAM)buf);
      }

      SendMessage(hRownum,CB_INSERTSTRING,(WPARAM)yomimaxrownum,(LPARAM)"全部");
      SendMessage(hRownum,CB_SETCURSEL,(WPARAM)0,(LPARAM)0);

      HWND hFuriganaOn = GetDlgItem(hDlg,IDC_FURIGANA_ON);
      SendMessage(hFuriganaOn,CB_INSERTSTRING,(WPARAM)0,(LPARAM)"表示しない");
      SendMessage(hFuriganaOn,CB_INSERTSTRING,(WPARAM)1,(LPARAM)"文字枠なし");
      SendMessage(hFuriganaOn,CB_INSERTSTRING,(WPARAM)2,(LPARAM)"文字枠あり");
      SendMessage(hFuriganaOn,CB_SETCURSEL,(WPARAM)currentdesign.yomi->show_furigana,(LPARAM)0);

      wsprintf(buf,"%d",currentdesign.yomi->margin_top[0]);
      SetWindowText(hMargintop,buf);
      wsprintf(buf,"%d",currentdesign.yomi->margin_left);
      SetWindowText(hMarginh,buf);
      wsprintf(buf,"%d",currentdesign.yomi->furigana_sukima);
      SetWindowText(hFuriganaSukima,buf);

      initdialogmode = FALSE;

    }
    return TRUE;
  case WM_VSCROLL:
    {
      int iId = LOWORD(wParam);
      int id = GetDlgCtrlID((HWND)lParam);
      HWND hChange = NULL;

      int maximum = -1;
      int minimum = -1;
      if(id == IDC_SCRMARGINTOP){
        hChange = hMargintop;
      }else if(id == IDC_SCRMARGINH){
        hChange = hMarginh;
        minimum = 0;
      }else if(id == IDC_FURIGANA_SCRSUKIMA){
        hChange = hFuriganaSukima;
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
       case IDC_FURIGANA_ON:
        {
          if(iEvent == CBN_SELCHANGE){
            HWND hFuriganaOn = GetDlgItem(hDlg,IDC_FURIGANA_ON);
            int r = SendMessage(hFuriganaOn,CB_GETCURSEL,(WPARAM)0,(LPARAM)0);
            currentdesign.yomi->show_furigana = r;
          }
        }
        break;

      case IDC_FURIGANA_FONT:
        CallChooseFont(hDlg,currentdesign.yomi->logfont_of_furigana,FALSE);
        break;
      case IDC_FURIGANA_COLOR:
        CallChooseColor(hDlg,currentdesign.yomi->furigana_color);
        break;

       case IDC_ROWNUM:
        {
          if(iEvent == CBN_SELCHANGE){
            int r = SendMessage(hRownum,CB_GETCURSEL,(WPARAM)0,(LPARAM)0);
            char buf[16];
            if(r >= yomimaxrownum){
              GetWindowText(hMargintop,buf,sizeof(buf)-1);
            }else{
              wsprintf(buf,"%d",currentdesign.yomi->margin_top[r]);
            }
            SetWindowText(hMargintop,buf);
          }
        }
        break;
       case IDC_MARGINTOP: case IDC_MARGINH: case IDC_FURIGANA_SUKIMA:
        {

          if(iEvent == EN_CHANGE){
            int i;
            char buf[16];

            GetWindowText(hMargintop,buf,sizeof(buf)-1);
            int r = SendMessage(hRownum,CB_GETCURSEL,(WPARAM)0,(LPARAM)0);
            if(buf[0] != '\0'){
              i = atoi(buf);
              if(r < yomimaxrownum){
                currentdesign.yomi->margin_top[r] = i;
              }else{
                for(int j = 0; j < yomimaxrownum; j++){
                  currentdesign.yomi->margin_top[j] = i;
                }
              }
            }

            GetWindowText(hMarginh,buf,sizeof(buf)-1);
            if(buf[0] != '\0'){
              i = atoi(buf);
              currentdesign.yomi->margin_left = i;
              currentdesign.yomi->margin_right = i;
            }

            GetWindowText(hFuriganaSukima,buf,sizeof(buf)-1);
            if(buf[0] != '\0'){
              i = atoi(buf);
              currentdesign.yomi->furigana_sukima = i;
            }
          }
        }
        break;
       case IDOK: case IDCANCEL:
        {

          EndDialog(hDlg, LOWORD(wParam));
          return TRUE;
        }
        break;
      }
    }
    if(!initdialogmode && fixaspectratioflag){
      int iEvent = HIWORD(wParam);
      if(iEvent == CBN_SELCHANGE || iEvent == BN_CLICKED || iEvent == EN_CHANGE){

        HDC hdc = GetDC(hWndMain);
        LoadImageToDC(hdc,currentdesign);
        ReleaseDC(hWndMain,hdc);

        UpdateMainWindow(FALSE);

      }
    } 
    break;
  }
  return FALSE;
}


// BaseDesignProc
LRESULT CALLBACK BaseDesignProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
  HWND hWidth = GetDlgItem(hDlg,IDC_WIDTH);
  HWND hHeight = GetDlgItem(hDlg,IDC_HEIGHT);
  HWND hBorderSize = GetDlgItem(hDlg,IDC_BORDERSIZE);
  HWND hBgType = GetDlgItem(hDlg,IDC_BGIMAGETYPE);
  HWND hBgTrans = GetDlgItem(hDlg,IDC_BGIMAGE_TRANSPARENT);
  HWND hBgOutline = GetDlgItem(hDlg,IDC_MOJIOUTLINE);

  class BaseDesign * design;
  if(tori_or_yomi == TORI){
    design = currentdesign.tori;
  }else{
    design = currentdesign.yomi;
  }
  switch(message){
  case WM_INITDIALOG:
    {
      initdialogmode = TRUE;

      if(tori_or_yomi == TORI){
        saved_design = (class ToriDesign *)malloc(sizeof(class ToriDesign));
        memcpy(saved_design,design,sizeof(class ToriDesign));
      }else{
        saved_design = (class YomiDesign *)malloc(sizeof(class YomiDesign));
        memcpy(saved_design,currentdesign.yomi,sizeof(class YomiDesign));
      }

      EnableMenuAll(hWndMain,FALSE);

      MoveWindowToRight(hDlg);

      char buf[16];
      wsprintf(buf,"%d",design->width);
      SetWindowText(hWidth,buf);
      wsprintf(buf,"%d",design->height);
      SetWindowText(hHeight,buf);
      wsprintf(buf,"%d",design->border_left_right_width);
      SetWindowText(hBorderSize,buf);
      CheckDlgButton(hDlg,IDC_FIXASPECTRATIO,BST_CHECKED);
      wsprintf(buf,"%d",design->bgimage_transparency);
      SetWindowText(hBgTrans,buf);
      fixaspectratioflag = TRUE;
      SendMessage(hBgType,CB_INSERTSTRING,(WPARAM)0,(LPARAM)"表示しない");
      SendMessage(hBgType,CB_INSERTSTRING,(WPARAM)1,(LPARAM)"並べて表示");
      SendMessage(hBgType,CB_INSERTSTRING,(WPARAM)2,(LPARAM)"拡大縮小");
      SendMessage(hBgType,CB_INSERTSTRING,(WPARAM)3,(LPARAM)"中央");
      SendMessage(hBgType,CB_INSERTSTRING,(WPARAM)4,(LPARAM)"左上");
      SendMessage(hBgType,CB_INSERTSTRING,(WPARAM)5,(LPARAM)"右上");
      SendMessage(hBgType,CB_INSERTSTRING,(WPARAM)6,(LPARAM)"左下");
      SendMessage(hBgType,CB_INSERTSTRING,(WPARAM)7,(LPARAM)"右下");
      SendMessage(hBgType,CB_SETCURSEL,(WPARAM)design->bgimage_mode,(LPARAM)0);

      SendMessage(hBgOutline,CB_INSERTSTRING,(WPARAM)0,(LPARAM)"なし");
      SendMessage(hBgOutline,CB_INSERTSTRING,(WPARAM)1,(LPARAM)"極細");
      SendMessage(hBgOutline,CB_INSERTSTRING,(WPARAM)2,(LPARAM)"細い");
      SendMessage(hBgOutline,CB_INSERTSTRING,(WPARAM)3,(LPARAM)"普通");
      SendMessage(hBgOutline,CB_INSERTSTRING,(WPARAM)4,(LPARAM)"太い");
      SendMessage(hBgOutline,CB_INSERTSTRING,(WPARAM)5,(LPARAM)"極太");
      SendMessage(hBgOutline,CB_SETCURSEL,(WPARAM)design->bgimage_mojioutline,(LPARAM)0);

      HWND hFudaNum = GetDlgItem(hDlg,IDC_FUDANUM);
      int currentfudanum = 0;
      
      if(tori_or_yomi == TORI){
        for(int i = 0; i < torifuda.size(); i++){
          SendMessage(hFudaNum,CB_INSERTSTRING,(WPARAM)i,(LPARAM)torifuda[i].kimariji.c_str());
          if(currentfuda == &(torifuda[i])){
            currentfudanum = i;
          }
        }
        currentdesign.simonoku = torifuda[currentfudanum].simonoku;
        currentdesign.kimariji = torifuda[currentfudanum].kimariji;
      }else{

        for(int i = 0; i < yomifuda.size(); i++){
          std::string ss = RemoveFuriganaAndTrunc(yomifuda[i].waka);
          SendMessage(hFudaNum,CB_INSERTSTRING,(WPARAM)i,(LPARAM)ss.c_str());
        }
        currentdesign.waka = yomifuda[currentfudanum].waka;
      }
      SendMessage(hFudaNum,CB_SETCURSEL,(WPARAM)currentfudanum,(LPARAM)0);
      UpdateMainWindow(FALSE);

      initdialogmode = FALSE;

    }
    break;
  case WM_VSCROLL:
    {
      int iId = LOWORD(wParam);

      int id = GetDlgCtrlID((HWND)lParam);

      HWND hChange = NULL;
      int maximum = -1;
      int minimum = -1;
      if(id == IDC_SCRWIDTH){
        hChange = hWidth;
        minimum = 10;
      }else if(id == IDC_SCRHEIGHT){
        hChange = hHeight;
        minimum = 10;
      }else if(id == IDC_SCRBORDERSIZE){
        hChange = hBorderSize;
        minimum = 0;
      }else if(id == IDC_BGIMAGE_SCR_TRANSPARENT){
        hChange = hBgTrans;
        minimum = 0;
        maximum = 100;
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

      case IDC_BGIMAGETYPE:
        {
          if(iEvent == CBN_SELCHANGE){
            design->bgimage_mode = SendMessage(hBgType,CB_GETCURSEL,(WPARAM)0,(LPARAM)0);
          }
        }
        break;
      case IDC_BGIMAGEFILE:
        {
          char filename[MAX_PATH];
          filename[0] = '\0';
          OPENFILENAME ofn;
          ZeroMemory(&ofn,sizeof(OPENFILENAME));
          ofn.lStructSize = sizeof(OPENFILENAME);
          ofn.hwndOwner = hDlg;
          ofn.lpstrFilter = "イメージファイル(*.bmp,*.jpg,*.gif,*.png)\0*.bmp;*.jpg;*.gif;*.png;\0\0";
          ofn.nMaxFile = MAX_PATH;
          ofn.lpstrFile = filename;
          ofn.Flags = OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
          if(GetOpenFileName(&ofn)){
            if(GetFileAttributes("bg_image") == -1){
              CreateDirectory("bg_image",NULL);
            }
            char * localfile;
            if(tori_or_yomi == TORI){
              localfile = "bg_image\\tori_current.img";
            }else{
              localfile = "bg_image\\yomi_current.img";
            }
            if(!CopyFile(filename,localfile,FALSE)){
              MessageBox(hDlg,"エラー:イメージファイルのbg_imageフォルダへのコピー失敗","エラー",MB_OK);
              return FALSE;
            }
            lstrcpy(design->background_image,localfile);
          }
        }
        break;
      case IDC_MOJIOUTLINE:
        {
          if(iEvent == CBN_SELCHANGE){
            design->bgimage_mojioutline = SendMessage(hBgOutline,CB_GETCURSEL,(WPARAM)0,(LPARAM)0);
          }
        }
        break;

      case IDC_SETBACKGROUNDCOLOR:
        CallChooseColor(hDlg,design->background_color);
        break;
      case IDC_SETBORDERCOLOR:
        CallChooseColor(hDlg,design->border_color);
        break;
      case IDC_SETFONT:
        CallChooseFont(hDlg,design->logfont,TRUE);
        break;
      case IDC_FONTCOLOR:
        CallChooseColor(hDlg,design->font_color);
        break;

      case IDOK: case IDCANCEL:
        {
          if(iId == IDCANCEL){
            if(tori_or_yomi == TORI){
              memcpy(design,saved_design,sizeof(class ToriDesign));
            }else{
              memcpy(currentdesign.yomi,saved_design,sizeof(class YomiDesign));
            }
            free(saved_design);
            HDC hdc = GetDC(hWndMain);
            LoadImageToDC(hdc,currentdesign);
            ReleaseDC(hWndMain,hdc);
            FixWindowSizeToFudaSize();
            UpdateMainWindow(TRUE);
          }
          EnableMenuAll(hWndMain,TRUE);
          DrawMenuBar(hWndMain);
        }
        break;

      case IDC_FUDANUM:
        {
          if(iEvent == CBN_SELCHANGE){

            HWND hFudaNum = GetDlgItem(hDlg,IDC_FUDANUM);
            int i = SendMessage(hFudaNum,CB_GETCURSEL,(WPARAM)0,(LPARAM)0);
            if(tori_or_yomi == TORI){
              currentdesign.simonoku = torifuda[i].simonoku;
              currentdesign.kimariji = torifuda[i].kimariji;
              currentfuda = &(torifuda[i]);
            }else{
              currentdesign.waka = yomifuda[i].waka;
            }
          }
          
        }
        break;

      case IDC_WIDTH: case IDC_HEIGHT: case IDC_BORDERSIZE: case IDC_BGIMAGE_TRANSPARENT:
      {
          if((iId == IDC_WIDTH || iId == IDC_HEIGHT) && iEvent == EN_UPDATE && IsDlgButtonChecked(hDlg,IDC_FIXASPECTRATIO) == BST_CHECKED && fixaspectratioflag){
            char buf[16];
            GetWindowText(hWidth,buf,sizeof(buf)-1);
            int w = atoi(buf);
            GetWindowText(hHeight,buf,sizeof(buf)-1);
            int h = atoi(buf);
            if(iId == IDC_WIDTH){
              h = int(w / design->aspect_ratio);
              wsprintf(buf,"%d",h);
              fixaspectratioflag = FALSE;
              SetWindowText(hHeight,buf);
            }else if(iId == IDC_HEIGHT){
              w = int(h * design->aspect_ratio);
              wsprintf(buf,"%d",w);
              fixaspectratioflag = FALSE;
              SetWindowText(hWidth,buf);
            }
          }else if(iEvent == EN_UPDATE){
            fixaspectratioflag = TRUE;
          }else if(iEvent == EN_CHANGE){
             int i;
            char buf[16];

            GetWindowText(hWidth,buf,sizeof(buf)-1);
            if(buf[0] != '\0'){
              i = atoi(buf);
              design->width = i;
            }
            GetWindowText(hHeight,buf,sizeof(buf)-1);
            if(buf[0] != '\0'){
              i = atoi(buf);
              design->height = i;
            }
            GetWindowText(hBorderSize,buf,sizeof(buf)-1);
            if(buf[0] != '\0'){
              i = atoi(buf);
              design->border_left_right_width = i;
              design->border_top_bottom_width = i;
            }
            GetWindowText(hBgTrans,buf,sizeof(buf)-1);
            if(buf[0] != '\0'){
              i = atoi(buf);
              design->bgimage_transparency = i;
            }
            if(IsDlgButtonChecked(hDlg,IDC_FIXASPECTRATIO) == BST_UNCHECKED){
              if(design->height == 0 || design->width == 0){
                design->aspect_ratio = 1.0;
              }else{
                design->aspect_ratio = double(design->width) / double(design->height);
              }
            }
            FixWindowSizeToFudaSize();

          }
      }
      }
      break;
    }
    break;
  }
  return FALSE;
}

