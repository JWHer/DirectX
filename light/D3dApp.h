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
	int Run();//메인 진입점

	virtual bool Init();//윈도우 생성, 다이렉트 생성
	virtual bool OnResize();//화면 조정
	virtual void UpdateScene(float deltaTime) = 0;//화면 갱신
	virtual void DrawScene() = 0;//화면 생성
	virtual LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);//윈도 메시지

	virtual void OnMouseDown(WPARAM btnState, int x, int y) { }
	virtual void OnMouseUp(WPARAM btnState, int x, int y) { }
	virtual void OnMouseMove(WPARAM btnState, int x, int y) { }
	virtual void OnMouseWheel(WPARAM btnState, int x, int y, int delta) { }
	virtual void OnKeyDown(WPARAM btnState, int x, int y) { }

protected:
	bool InitMainWindow();//윈도우 생성
	bool InitDirect3D();//다이렉트 생성
	void CalculateFrameStats();//프레임 계산

	//필드
	HINSTANCE mHInst;//윈도 인스턴스, 매개변수
	HWND mHWnd;//윈도 핸들, 생성해 주어야함

	ID3D11Device* mDevice;
	ID3D11DeviceContext* mImmediateContext;//내용
	IDXGISwapChain* mSwapChain;
	ID3D11Texture2D* mDepthStencilBuffer;

	ID3D11RenderTargetView* mRenderTargetView;//뷰포인트
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