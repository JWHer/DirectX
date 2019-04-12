#ifndef __D3DAPP_H
#define __D3DAPP_H

#include <WindowsX.h>
#include <d3d11.h>
#include <sstream>
#include "GameTimer.h"

class D3dApp {
public:
	D3dApp(HINSTANCE hInstance);
	virtual ~D3dApp();

	HINSTANCE GetMainInst()const;
	HWND GetMainWnd()const;
	int Run();//���� ������

	virtual bool Init();//������ ����, ���̷�Ʈ ����
	virtual bool OnResize();//ȭ�� ����
	virtual void UpdateScene(float deltaTime) = 0;//ȭ�� ����
	virtual void DrawScene() = 0;//ȭ�� ����
	virtual LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);//���� �޽���

	virtual void OnMouseDown(WPARAM btnState, int x, int y) { }
	virtual void OnMouseUp(WPARAM btnState, int x, int y) { }
	virtual void OnMouseMove(WPARAM btnState, int x, int y) { }
	virtual void OnMouseWheel(WPARAM btnState, int x, int y, int delta) { }
	virtual void OnKeyDown(WPARAM btnState, int x, int y) { }

protected:
	bool InitMainWindow();//������ ����
	bool InitDirect3D();//���̷�Ʈ ����
	void CalculateFrameStats();//������ ���

	//�ʵ�
	HINSTANCE mHInst;//���� �ν��Ͻ�, �Ű�����
	HWND mHWnd;//���� �ڵ�, ������ �־����

	ID3D11Device* mDevice;
	ID3D11DeviceContext* mImmediateContext;//����
	IDXGISwapChain* mSwapChain;
	ID3D11Texture2D* mDepthStencilBuffer;

	ID3D11RenderTargetView* mRenderTargetView;//������Ʈ
	ID3D11DepthStencilView* mDepthStencilView;
	D3D11_VIEWPORT mScreenViewport;

	D3D_DRIVER_TYPE mDriverType = D3D_DRIVER_TYPE_NULL;
	D3D_FEATURE_LEVEL mFeatureLevel = D3D_FEATURE_LEVEL_11_0;
	
	UINT m4xMsaaQuality;
	GameTimer mTimer;

	std::wstring mainWndCaption;
	std::wstring appClassName;
	int mClientWidth;
	int mClientHeight;

	bool mPaused;
	bool mMin;
	bool mMax;
	bool mResizing;
	bool mEnable4xMsaa;
};

#endif