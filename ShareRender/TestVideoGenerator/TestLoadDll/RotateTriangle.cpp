#include <d3d9.h>
#include < d3dx9.h>
#include < windows.h>
#include < stdlib.h>
#include < malloc.h>
#include < memory.h>
#include < tchar.h>

// ȫ�ֱ���:
HINSTANCE hInst;        // ��ǰʵ��
TCHAR szTitle[20];        // �������ı�
TCHAR szWindowClass[20];      // ����������
LPDIRECT3D9 g_pD3D = NULL;      // D3Dָ��
LPDIRECT3DDEVICE9 g_pD3DDevice = NULL;   // D3D�豸ָ��
LPDIRECT3DVERTEXBUFFER9 g_pVertexBuffer = NULL; // ���㻺��ָ��
//���嶥����Ϣ�Ľṹ��
struct CUSTOMVERTEX
{
    FLOAT x, y, z, rhw;  
    DWORD colour;  
};
//�������ɶ����ʽ
#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZRHW|D3DFVF_DIFFUSE)
//�����ͷ�COM����ĺ�
#define SafeRelease(pObject) if(pObject != NULL) {pObject->Release(); pObject=NULL;}
// �˴���ģ���а����ĺ�����ǰ������:
ATOM    MyRegisterClass(HINSTANCE hInstance);
BOOL    InitInstance(HINSTANCE, int);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK About(HWND, UINT, WPARAM, LPARAM);
//��ʼ��D3D�豸
HRESULT InitialiseD3D(HWND hWnd)
{
 //ȡ��D3D9�Ķ���
    g_pD3D = Direct3DCreate9(D3D_SDK_VERSION);
    if(g_pD3D == NULL)
    {
        return E_FAIL;
    }
 //�õ���ǰ����ʾģʽ
    D3DDISPLAYMODE d3ddm;
    if(FAILED(g_pD3D->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &d3ddm)))
    {
        return E_FAIL;
    }
 //����һ��D3D�豸
    D3DPRESENT_PARAMETERS d3dpp;
    ZeroMemory(&d3dpp, sizeof(d3dpp));
    d3dpp.Windowed = TRUE;//ȫ��ģʽ���Ǵ���ģʽ
    d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;//��̨���������Ƶ�ǰ̨ʱ,�����̨����������
    d3dpp.BackBufferFormat = d3ddm.Format;//��Ļ����ʾģʽ
	d3dpp.PresentationInterval = 33;
 //����һ��Direct3D�豸
    if(FAILED(g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd,D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &g_pD3DDevice)))
    {
        return E_FAIL;
    }
   
    return S_OK;
}
HRESULT InitialiseVertexBuffer()
{
 VOID* pVertices;
 
 //������Ϣ����
 CUSTOMVERTEX cvVertices[] =
 {
  {250.0f, 100.0f, 0.5f, 1.0f, D3DCOLOR_XRGB(255, 0, 0),},
  {400.0f, 350.0f, 0.5f, 1.0f, D3DCOLOR_XRGB(0, 255, 0),},
  {100.0f, 350.0f, 0.5f, 1.0f, D3DCOLOR_XRGB(0, 0, 255),},
 };
 //ͨ���豸�������㻺��
 if(FAILED(g_pD3DDevice->CreateVertexBuffer(3 * sizeof(CUSTOMVERTEX),
                                               0, D3DFVF_CUSTOMVERTEX,
                                               D3DPOOL_DEFAULT, &g_pVertexBuffer,NULL)))
 {
  return E_FAIL;
 }
 //�������㻺�壬���õ�һ����Ŷ�����Ϣ�Ļ�������ָ��
 if(FAILED(g_pVertexBuffer->Lock(0, sizeof(cvVertices), (void**)&pVertices, 0)))
 {
  return E_FAIL;
 }
 //���ƶ�����Ϣ
 memcpy(pVertices, cvVertices, sizeof(cvVertices));
 //�������㻺����
 g_pVertexBuffer->Unlock();
    return S_OK;
}
void Render()
{
    if(g_pD3DDevice == NULL)
    {
        return;
    }
 //��պ󱸻�����Ϊ��ɫ
    g_pD3DDevice->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);   
 //��ʼ���Ƴ���
    g_pD3DDevice->BeginScene();
 //��Ⱦ������
 g_pD3DDevice->SetStreamSource(0, g_pVertexBuffer, 0, sizeof(CUSTOMVERTEX));
 g_pD3DDevice->SetFVF(D3DFVF_CUSTOMVERTEX);
 g_pD3DDevice->DrawPrimitive(D3DPT_TRIANGLELIST, 0, 1);
 //�������Ƴ���
    g_pD3DDevice->EndScene();
   
 //��ҳ��ʾ
    g_pD3DDevice->Present(NULL, NULL, NULL, NULL);
}
//�ͷ���ʹ�õ�������COM����
void CleanUp()
{
 SafeRelease(g_pVertexBuffer);
 SafeRelease(g_pD3DDevice);
 SafeRelease(g_pD3D);
}
//��Ϸѭ��
void GameLoop()
{
    //������Ϸѭ��
    MSG msg;
    BOOL fMessage;
    PeekMessage(&msg, NULL, 0U, 0U, PM_NOREMOVE);
   
    while(msg.message != WM_QUIT)
    {
        fMessage = PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE);
        if(fMessage)
        {
            //������Ϣ
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
        {
   //���û����Ϣ������Ⱦ��ǰ�ĳ���
            Render();
        }
    }
}
int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
 //ע��Windows�Ĵ�����
    WNDCLASSEX wc = {sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L,
                     GetModuleHandle(NULL), NULL, NULL, NULL, NULL,
                     "D3DDrawGraphics", NULL};
    RegisterClassEx(&wc);
 //����һ������
    HWND hWnd = CreateWindow("D3DDrawGraphics", "D3D���Ƽ�ͼ��",
                              WS_OVERLAPPEDWINDOW, 50, 50, 500, 500,
                              GetDesktopWindow(), NULL, wc.hInstance, NULL);
 //��ʼ��Direct3D
    if(SUCCEEDED(InitialiseD3D(hWnd)))
    {
  //��ʾ����
        ShowWindow(hWnd, SW_SHOWDEFAULT);
        UpdateWindow(hWnd);
  //��ʼ�����㻺��
  if(SUCCEEDED(InitialiseVertexBuffer()))
  {
   //��ʼ��Ϸ: ������Ϸѭ��
   GameLoop();
  }
    }
   
    CleanUp();
 //�����������ע��
    UnregisterClass("D3DDrawGraphics", wc.hInstance);
   
    return 0;
}
 

// ע�ᴰ����
ATOM MyRegisterClass(HINSTANCE hInstance)
{
 WNDCLASSEX wcex;
 wcex.cbSize = sizeof(WNDCLASSEX);
 wcex.style   = CS_HREDRAW | CS_VREDRAW;
 wcex.lpfnWndProc = (WNDPROC)WndProc;
 wcex.cbClsExtra  = 0;
 wcex.cbWndExtra  = 0;
 wcex.hInstance  = hInstance;
 //wcex.hIcon   = LoadIcon(hInstance, (LPCTSTR)IDI_MY);
 wcex.hCursor  = LoadCursor(NULL, IDC_ARROW);
 wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
// wcex.lpszMenuName = (LPCTSTR)IDC_MY;
 wcex.lpszClassName = szWindowClass;
 //wcex.hIconSm  = LoadIcon(wcex.hInstance, (LPCTSTR)IDI_SMALL);
 return RegisterClassEx(&wcex);
}

//   ����ʵ�����������������
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;
   hInst = hInstance; // ��ʵ������洢��ȫ�ֱ�����
   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);
   if (!hWnd)
   {
      return FALSE;
   }
   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);
   return TRUE;
}

//  ���ڻص�����
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch(message)
    {
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        break;
        case WM_KEYUP:
            switch (wParam)
            {
   case VK_ESCAPE:
                    DestroyWindow(hWnd);
                    return 0;
                break;
            }
        break;
    }
    return DefWindowProc(hWnd, message, wParam, lParam);
}