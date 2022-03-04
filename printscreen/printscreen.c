#include <windows.h>
#define print 1
#define save 2
int x,y;HWND parent,hcwnd;HDC hwdc,hmdcA,hmdc;HBITMAP hbitmapA,hbitmap;HINSTANCE hinstance;POINT start,end;
LRESULT CALLBACK WndProc(HWND hwnd,UINT message,WPARAM wparam,LPARAM lparam);
LRESULT CALLBACK printproc(HWND hwnd,UINT message,WPARAM wparam,LPARAM lparam);
int APIENTRY WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,LPSTR lpCmdLine,int nCmdShow)
{
	HWND hwnd;WNDCLASS wndclass;MSG msg;HDC hdc;LOGFONT logfont;HFONT hfont;RECT rect;HMENU hmenu;
	char string[]="选择新建,按下鼠标左键拖动截屏,按下鼠标右键取消截屏.\n截屏完成后会自动放入剪贴板中.";
	memset(&wndclass,0,sizeof(WNDCLASS));
	wndclass.hbrBackground=GetStockObject(WHITE_BRUSH);
	wndclass.hCursor=LoadCursor(0,IDC_ARROW);
	wndclass.hIcon=LoadIcon(0,IDI_APPLICATION);
	wndclass.hInstance=hInstance;
	wndclass.lpfnWndProc=WndProc;
	wndclass.lpszClassName="printscreen";
	wndclass.style=CS_HREDRAW|CS_VREDRAW;
	RegisterClass(&wndclass);
	wndclass.lpfnWndProc=printproc;
	wndclass.lpszClassName="printwindow";
	RegisterClass(&wndclass);
	hinstance = hInstance;
	x = GetSystemMetrics(SM_CXSCREEN);
	y = GetSystemMetrics(SM_CYSCREEN);
	parent=hwnd=CreateWindow("printscreen","printscreen",WS_OVERLAPPEDWINDOW|WS_VSCROLL|WS_HSCROLL,
		x*2/5,y*2/5,x/5,y/5,0,0,hInstance,0);
	hmenu=CreateMenu();
	AppendMenu(hmenu,MF_STRING,print,"新建");
	AppendMenu(hmenu,MF_STRING|MF_GRAYED,save,"保存");
	SetMenu(hwnd,hmenu);
	ShowWindow(hwnd,nCmdShow);
	hdc=GetDC(hwnd);
	GetObject(GetStockObject(SYSTEM_FONT),sizeof(LOGFONT),&logfont);
	lstrcpy(logfont.lfFaceName,"楷体");
	hfont=CreateFontIndirect(&logfont);
	SelectObject(hdc,hfont);
	GetClientRect(hwnd, &rect);
	DrawText(hdc,string,-1,&rect,DT_WORDBREAK);
	ReleaseDC(hwnd,hdc);
	while(GetMessage(&msg,0,0,0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return 0;
}
absolute(RECT *temp)
{
	temp->left=start.x>end.x?end.x:start.x;
	temp->right=start.x>end.x?start.x:end.x;
	temp->top=start.y>end.y?end.y:start.y;
	temp->bottom=start.y>end.y?start.y:end.y;
}
printscreen()
{
	int i,*color;unsigned char *rgb;HWND hdesk;HDC hdc;BITMAP bitmap;
	hdesk=GetDesktopWindow();
	hdc=GetDC(hdesk);
	if(!hbitmapA)
	{
		hmdc=CreateCompatibleDC(hdc);
		hmdcA=CreateCompatibleDC(hdc);
		hbitmap=CreateCompatibleBitmap(hdc,x,y);
		hbitmapA=CreateCompatibleBitmap(hdc,x,y);
		SelectObject(hmdc,hbitmap);
		SelectObject(hmdcA,hbitmapA);
	}
	Sleep(250);
	BitBlt(hmdc,0,0,x,y,hdc,0,0,SRCCOPY);
	ReleaseDC(hdesk,hdc);
	BitBlt(hmdcA,0,0,x,y,hmdc,0,0,SRCCOPY);
	GetObject(hbitmap,sizeof(BITMAP),&bitmap);
	color=calloc(bitmap.bmHeight*bitmap.bmWidth,4);
	GetBitmapBits(hbitmap,bitmap.bmHeight*bitmap.bmWidth*4,color);
	for(i=0;i<bitmap.bmHeight*bitmap.bmWidth;i++)
	{
		rgb=&(char)color[i];
		rgb[0]=rgb[0]+(255-rgb[0])/2;
		rgb[1]=rgb[1]+(255-rgb[1])/2;
		rgb[2]=rgb[2]+(255-rgb[2])/2;
	}
	SetBitmapBits(hbitmap,bitmap.bmHeight*bitmap.bmWidth*4,color);
	free(color);
	if(!hcwnd)
		hcwnd=CreateWindow("printwindow",0,WS_POPUP,0,0,x,y,0,0,hinstance,0);
	ShowWindow(hcwnd,SW_SHOW);
	SetWindowPos(hcwnd,HWND_TOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);
	hwdc=GetDC(hcwnd);
	BitBlt(hwdc,0,0,x,y,hmdc,0,0,SRCCOPY);
}

char savebitmap()
{
	int i,cx,cy;char path[MAX_PATH];unsigned long bytes,write;RECT temp;HDC hdc,hdcA;HWND hdesk;unsigned char *color;
	BITMAPFILEHEADER file;OPENFILENAME filename;HANDLE hfile;HBITMAP hbmp;BITMAPINFO info;
	memset(&filename,0,sizeof(filename));
	memset(path,0,MAX_PATH);
	filename.lStructSize=sizeof(OPENFILENAME);
	filename.hwndOwner=parent;
	filename.lpstrFilter="位图文件(*.bmp)\0*.bmp\0所有文件(*.*)\0*.*\0\0";
	filename.lpstrFile=path;
	filename.nMaxFile=MAX_PATH;
	filename.lpstrDefExt="bmp";
	filename.Flags=OFN_OVERWRITEPROMPT;
	memset(&info,0,sizeof(info));
	absolute(&temp);
	cx=temp.right-temp.left+1;
	cy=temp.bottom-temp.top+1;
	info.bmiHeader.biSize=sizeof(info.bmiHeader);
	info.bmiHeader.biWidth=cx;
	info.bmiHeader.biHeight=cy;
	info.bmiHeader.biPlanes=1;
	info.bmiHeader.biBitCount=32;
	info.bmiHeader.biCompression=BI_RGB;
	info.bmiHeader.biSizeImage=cx*cy*4;
	memset(&file,0,sizeof(file));
	file.bfType=0x4D42;
	file.bfOffBits=sizeof(file)+sizeof(info.bmiHeader);
	file.bfSize=file.bfOffBits+info.bmiHeader.biSizeImage;
	if(!GetSaveFileName(&filename))
		return 2;
	hdesk=GetDesktopWindow();
	hdc=GetDC(hdesk);
	hdcA=CreateCompatibleDC(hdc);
	hbmp=CreateCompatibleBitmap(hdc,cx,cy);
	ReleaseDC(hdesk,hdc);
	SelectObject(hdcA,hbmp);
	BitBlt(hdcA,0,0,cx,cy,hmdcA,temp.left,temp.top,SRCCOPY);
	color=calloc(cx*cy*4,1);
	GetDIBits(hdcA,hbmp,0,cy,color,&info,DIB_RGB_COLORS);
	DeleteDC(hdcA);
	DeleteObject(hbmp);
	if(INVALID_HANDLE_VALUE==(hfile=CreateFile(filename.lpstrFile,GENERIC_WRITE,0,0,CREATE_ALWAYS,0,0)))
		return 0;
	for(i=0;i<3;i++)
	{
		if(!i)
			WriteFile(hfile,&file,write=sizeof(file),&bytes,0);
		else if(i==1)
			WriteFile(hfile,&info.bmiHeader,write=sizeof(info.bmiHeader),&bytes,0);
		else if(i==2)
			WriteFile(hfile,color,write=info.bmiHeader.biSizeImage,&bytes,0);
		if(bytes!=write)
		{
			CloseHandle(hfile);
			free(color);
			return 0;
		}
	}	
	CloseHandle(hfile);
	free(color);
	return 1;
}
LRESULT CALLBACK printproc(HWND hwnd,UINT message,WPARAM wparam,LPARAM lparam)
{
	int cx,cy;HWND hdesk;HDC hdc,hdcA;HBITMAP hbmp;static char capture;RECT temp;
	switch(message)
	{
	case WM_LBUTTONDOWN:
		capture=1;
		start.x=LOWORD(lparam);
		start.y=HIWORD(lparam);
		return 0;
	case WM_MOUSEMOVE:
		if(capture)
		{
			end.x=LOWORD(lparam);
			end.y=HIWORD(lparam);
			hdc=CreateCompatibleDC(hwdc);
			hbmp=CreateCompatibleBitmap(hwdc,x,y);
			SelectObject(hdc,hbmp);
			BitBlt(hdc,0,0,x,y,hmdc,0,0,SRCCOPY);
			BitBlt(hdc,start.x,start.y,end.x-start.x,end.y-start.y,hmdcA,start.x,start.y,SRCCOPY);
			POINT line[5]={start.x,start.y,start.x,end.y,end.x,end.y,end.x,start.y,start.x,start.y};
			Polyline(hdc,line,5);
			BitBlt(hwdc,0,0,x,y,hdc,0,0,SRCCOPY);
			DeleteDC(hdc);
			DeleteObject(hbmp);
		}
		return 0;
	case WM_RBUTTONUP:
	case WM_LBUTTONUP:
		capture=0;
		if(message==WM_RBUTTONUP)
		{
			hbitmapA=0;
			DeleteDC(hmdc);
			DeleteDC(hmdcA);
			DeleteObject(hbitmap);
			DeleteObject(hbitmapA);
			MoveWindow(parent,x*2/5,y*2/5,x/5,y/5,TRUE);
		}
		else
		{
			EnableMenuItem(GetMenu(parent),save,MF_ENABLED);
			absolute(&temp);
			cx=temp.right-temp.left;
			cy=temp.bottom-temp.top;
			hdesk=GetDesktopWindow();
			hdc=GetDC(hdesk);
			hdcA=CreateCompatibleDC(hdc);
			hbmp=CreateCompatibleBitmap(hdc,cx,cy);
			ReleaseDC(hdesk,hdc);
			SelectObject(hdcA,hbmp);
			BitBlt(hdcA,0,0,cx,cy,hmdcA,temp.left,temp.top,SRCCOPY);
			DeleteDC(hdcA);
			OpenClipboard(hwnd);
			EmptyClipboard();
			SetClipboardData(CF_BITMAP,hbmp);
			CloseClipboard();
			MoveWindow(parent,x/8,y/8,x*3/4,y*3/4,TRUE);
		}
		ShowWindow(hcwnd,SW_HIDE);
		ShowWindow(parent,SW_SHOW);
		return 0;
	}
	return DefWindowProc(hwnd,message,wparam,lparam);
}
LRESULT CALLBACK WndProc(HWND hwnd,UINT message,WPARAM wparam,LPARAM lparam)
{
	HDC hdc;PAINTSTRUCT ps;RECT temp;static short wx,wy;SCROLLINFO info;int hscroll,vscroll;
	switch(message)
	{
	case WM_SIZE:
	case WM_SHOWWINDOW:
		if(message==WM_SIZE)
		{
			wx=LOWORD(lparam);
			wy=HIWORD(lparam);
		}
		info.cbSize=sizeof(SCROLLINFO);
		info.fMask=SIF_PAGE|SIF_RANGE;
		info.nMin=info.nMax=0;
		info.nPage=wx;
		if(hbitmapA)
			info.nMax=end.x-start.x>0?end.x-start.x:start.x-end.x;
		SetScrollInfo(hwnd,SB_HORZ,&info,TRUE);
		info.nPage=wy;
		if(hbitmapA)
			info.nMax=end.y-start.y>0?end.y-start.y:start.y-end.y;
		SetScrollInfo(hwnd,SB_VERT,&info,TRUE);
		return 0;
	case WM_COMMAND:
		switch(LOWORD(wparam))
		{
		case print:
			EnableMenuItem(GetMenu(parent),save,MF_GRAYED);
			ShowWindow(hwnd,SW_HIDE);
			printscreen();
			break;
		case save:
			if(!savebitmap())
				MessageBox(hwnd,"保存文件失败!","保存",MB_ICONERROR|MB_OK);
			break;
		}
		return 0;
		case WM_HSCROLL:
			info.cbSize=sizeof(SCROLLINFO);
			info.fMask=SIF_POS|SIF_TRACKPOS;
			GetScrollInfo(hwnd,SB_HORZ,&info);
			switch(LOWORD(wparam))
			{
			case SB_LINELEFT:
				info.nPos-=10;
				break;
			case SB_LINERIGHT:
				info.nPos+=10;
				break;
			case SB_THUMBTRACK:
				info.nPos=info.nTrackPos;
				break;
			}
			info.fMask=SIF_POS;
			SetScrollInfo(hwnd,SB_HORZ,&info,TRUE);
			InvalidateRect(hwnd,0,0);
			return 0;
			case WM_VSCROLL:
				info.cbSize=sizeof(SCROLLINFO);
				info.fMask=SIF_POS|SIF_TRACKPOS;
				GetScrollInfo(hwnd,SB_VERT,&info);
				switch(LOWORD(wparam))
				{
				case SB_LINEUP:
					info.nPos-=10;
					break;
				case SB_LINEDOWN:
					info.nPos+=10;
					break;
				case SB_THUMBTRACK:
					info.nPos=info.nTrackPos;
					break;
				}
				info.fMask=SIF_POS;
				SetScrollInfo(hwnd,SB_VERT,&info,TRUE);
				InvalidateRect(hwnd,0,0);
				return 0;
				case WM_PAINT:
					hdc=BeginPaint(hwnd,&ps);
					if(hbitmapA)
					{
						absolute(&temp);
						info.cbSize=sizeof(SCROLLINFO);
						info.fMask=SIF_POS;
						GetScrollInfo(hwnd,SB_HORZ,&info);
						hscroll=info.nPos;
						GetScrollInfo(hwnd,SB_VERT,&info);
						vscroll=info.nPos;
						BitBlt(hdc,wx>temp.right-temp.left?(wx-temp.right+temp.left)/2:0-hscroll,
							wy>temp.bottom-temp.top?(wy-temp.bottom+temp.top)/2:0-vscroll,
							temp.right-temp.left,temp.bottom-temp.top,hmdcA,temp.left,temp.top,SRCCOPY);
					}
					EndPaint(hwnd,&ps);
					return 0;
				case WM_DESTROY:
					PostQuitMessage(0);
					return 0;
	}
	return DefWindowProc(hwnd,message,wparam,lparam);
}