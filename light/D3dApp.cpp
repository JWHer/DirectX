#include "D3dApp.h"

//Forward declarations
//init전 수행되는 작업에 대해
namespace { D3dApp* BOC = 0; }
LRESULT CALLBACK MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	return BOC->MsgProc(hwnd, msg, wParam, lParam);
}

D3dApp::D3dApp(HINSTANCE hInstance)
	: mHInst(hInstance), mainWndCaption(L"D3dAppWindow"), appClassName(L"D3dAppClass"),
	mClientHeight(600), mClientWidth(800), mHWnd(0),
	mEnable4xMsaa(false), m4xMsaaQuality(0),
	mPaused(false), mMin(false), mMax(false), mResizing(false),
	mDevice(0), mImmediateContext(0), mSwapChain(0),
	mDepthStencilBuffer(0), mRenderTargetView(0), mDepthStencilView(0)
{
	ZeroMemory(&mScreenViewport, sizeof(D3D11_VIEWPORT));
	BOC = this;
}

D3dApp::~D3dApp() {
	//if (mainRenderTargetView)mainRenderTargetView->Release();
	if (mDepthStencilView)mDepthStencilView->Release();
	if (mSwapChain) mSwapChain->Release();
	if (mDepthStencilBuffer) mDepthStencilBuffer->Release();
	if (mImmediateContext) { 
		mImmediateContext->ClearState(); 
		mImmediateContext->Release();
	}
	if (mDevice) mDevice->Release();
}

HINSTANCE D3dApp::GetMainInst()const {
	return mHInst;
}

HWND D3dApp::GetMainWnd()const {
	return mHWnd;
}

bool D3dApp::Init() {
	if (!InitMainWindow())
		return false;
	if (!InitDirect3D())
		return false;
	return true;
}

bool D3dApp::InitMainWindow() {
	//register class
	WNDCLASS wc;
	//wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;//수평 수직 변경허용
	wc.lpfnWndProc = MainWndProc;
	wc.cbClsExtra = 0;//할당 여분
	wc.cbWndExtra = 0;//할당 여분
	wc.hInstance = mHInst;
	wc.hIcon = LoadIcon(mHInst, IDI_APPLICATION);//아이콘
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);//마우스
	wc.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);//배경색
	wc.lpszMenuName = NULL;
	wc.lpszClassName = appClassName.c_str();//타이틀
	//wc.hIconSm = LoadIcon(wc.hInstance, (LPCTSTR)IDI_APPLICATION);
	if (!RegisterClass(&wc)) {
		MessageBox(0, L"RegisterClass Failed.", 0, 0);
		return false;
	}

	//Create
	RECT rc = { 0,0,mClientWidth, mClientHeight };
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);//사각형, 윈도스타일, 메뉴
	mHWnd = CreateWindow(appClassName.c_str(), mainWndCaption.c_str(),
		WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top,
		NULL, NULL, mHInst, NULL);
	if (!mHWnd) {
		wchar_t *errorString;
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL, GetLastError(), MAKELANGID(LANG_ENGLISH, SUBLANG_DEFAULT),
			(LPTSTR)&errorString, 0, NULL);
		MessageBox(0, errorString, 0, 0);
		LocalFree(errorString);
		return false;
	}
	
	ShowWindow(mHWnd, SW_SHOW);
	UpdateWindow(mHWnd);

	return true;
}

bool D3dApp::InitDirect3D() {
	HRESULT hr = S_OK;
	UINT createDeviceFlags = 0;
#ifdef DEBUG
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif // DEBUG

	D3D_DRIVER_TYPE driverTypes[] = {
		D3D_DRIVER_TYPE_HARDWARE,
		D3D_DRIVER_TYPE_WARP,
		D3D_DRIVER_TYPE_REFERENCE,
	};
	UINT numDriverTypes = ARRAYSIZE(driverTypes);

	D3D_FEATURE_LEVEL featureLevels[] = {
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
	};
	UINT numFeatureLevels = ARRAYSIZE(featureLevels);

	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferCount = 1;
	sd.BufferDesc.Width = (UINT)mClientWidth;
	sd.BufferDesc.Height = (UINT)mClientHeight;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = mHWnd;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;

	//가능한 드라이버 타입 생성
	for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++) {
		mDriverType = driverTypes[driverTypeIndex];
		hr = D3D11CreateDeviceAndSwapChain(NULL, mDriverType, NULL, createDeviceFlags,
			featureLevels, numFeatureLevels, D3D11_SDK_VERSION,
			&sd, &mSwapChain, &mDevice, &mFeatureLevel, &mImmediateContext);

		if (SUCCEEDED(hr))
			break;
	}
	if (FAILED(hr))
	{
		MessageBox(0, L"D3D11CreateDevice Failed.", 0, 0);
		return false;
	}
	if (mFeatureLevel != D3D_FEATURE_LEVEL_11_0) {
		MessageBox(0, L"Direct3d Feature Level 11 unsupported.", 0, 0);
	}

	hr = mDevice->CheckMultisampleQualityLevels(DXGI_FORMAT_R8G8B8A8_UNORM, 4, &m4xMsaaQuality);
	if (SUCCEEDED(hr)) {
		//4X MSAA support
		if (mEnable4xMsaa) {
			sd.SampleDesc.Count = 4;
			sd.SampleDesc.Quality = m4xMsaaQuality - 1;
		}
	}
	else {
		mEnable4xMsaa = false;
	}

	if (!OnResize())
		return false;

	return true;
}

int D3dApp::Run() {
	//Main message loop
	MSG msg = { 0 };
	mTimer.Reset();
	while (msg.message != WM_QUIT) {
		//윈도 메시지 실행
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		//아닐 때 게임 실행
		else {
			mTimer.Tick();
			if (!mPaused) {
				CalculateFrameStats();
				UpdateScene(mTimer.DeltaTime());
				DrawScene();
			}
			else {
				Sleep(100);
			}
		}
	}

	return (int)msg.wParam;
}

void D3dApp::DrawScene() {
	//default
	float ClearColor[4] = { 0.0f, 0.125f, 0.3f, 1.0f };//R, G, B, Alpha
	mImmediateContext->ClearRenderTargetView(mRenderTargetView, ClearColor);
	mSwapChain->Present(0, 0);
}

void D3dApp::UpdateScene(float deltaTime) {

}

void D3dApp::CalculateFrameStats() {
	static int frameCnt = 0;
	static float timeElapsed = 0.0f;

	frameCnt++;

	if ((mTimer.TotalTime() - timeElapsed) >= 1.0f) {
		float fps = (float)frameCnt;
		float mspf = 1000.0f / fps;

		std::wostringstream outs;
		outs.precision(6);
		outs << mainWndCaption << L"    "
			<< L"FPS: " << fps << L"    "
			<< L"Frame Time: " << mspf << L" (ms)";
		SetWindowText(mHWnd, outs.str().c_str());

		//reset
		frameCnt = 0;
		timeElapsed += 1.0f;
	}
}

LRESULT D3dApp::MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg)
	{
	case WM_ACTIVATE:
		if (LOWORD(wParam) == WA_INACTIVE) {
			mPaused = true;
			mTimer.Stop();
		}
		else {
			mPaused = false;
			mTimer.Start();
		}
		return 0;

	case WM_SIZE:
		mClientWidth = LOWORD(lParam);
		mClientHeight = HIWORD(lParam);
		if (mDevice) {
			if (wParam == SIZE_MINIMIZED) {
				mPaused = true;
				mMin = true;
				mMax = false;
			}
			else if (wParam == SIZE_MAXIMIZED) {
				mPaused = false;
				mMin = false;
				mMax = true;
				OnResize();
			}
			else if (wParam == SIZE_RESTORED) {
				//최소화 해제
				if (mMin) {
					mPaused = false;
					mMin = false;
					OnResize();
				}
				//최대화 해제
				else if (mMax) {
					mPaused = false;
					mMax = false;
					OnResize();
				}
				else if (mResizing)
				{
					 
				}
				else {
					OnResize();
				}
			}
		}
		return 0;

	case WM_ENTERSIZEMOVE:
		mPaused = true;
		mResizing = true;
		mTimer.Stop();
		return 0;

	case WM_EXITSIZEMOVE:
		mPaused = false;
		mResizing = false;
		mTimer.Start();
		OnResize();
		return 0;

	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

	case WM_MENUCHAR:
		//비프음 방지
		return MAKELRESULT(0, MNC_CLOSE);

	case WM_GETMINMAXINFO:
		//최소 크기
		((MINMAXINFO*)lParam)->ptMinTrackSize.x = 200;
		((MINMAXINFO*)lParam)->ptMinTrackSize.y = 200;
		return 0;

	case WM_LBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_RBUTTONDOWN:
		OnMouseDown(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;
	case WM_LBUTTONUP:
	case WM_MBUTTONUP:
	case WM_RBUTTONUP:
		OnMouseUp(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;
	case WM_MOUSEMOVE:
		OnMouseMove(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;
	case WM_MOUSEWHEEL:
		OnMouseWheel(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), GET_WHEEL_DELTA_WPARAM(wParam));
		return 0;
	case WM_KEYDOWN:
		OnKeyDown(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;
	}

	return DefWindowProc(hWnd, msg, wParam, lParam);
}

bool D3dApp::OnResize() {
	//Release the old views
	if (mRenderTargetView) mRenderTargetView->Release();
	if (mDepthStencilView) mDepthStencilView->Release();
	if (mDepthStencilBuffer) mDepthStencilBuffer->Release();

	HRESULT hr;
	//target view
	hr = mSwapChain->ResizeBuffers(1, mClientWidth, mClientHeight, DXGI_FORMAT_R8G8B8A8_UNORM, 0);
	if (FAILED(hr)) {
		MessageBox(0, L"ResizeBuffer Failed.", 0, 0);
		return false;
	}
	ID3D11Texture2D* backBuffer = NULL;
	hr = mSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBuffer);
	if (FAILED(hr)) {
		MessageBox(0, L"GetBuffer Failed.", 0, 0);
		return false;
	}
	hr = mDevice->CreateRenderTargetView(backBuffer, NULL, &mRenderTargetView);
	if (FAILED(hr)) {
		MessageBox(0, L"CreateRenderTargetView Failed.", 0, 0);
		return false;
	}
	if (backBuffer) backBuffer->Release();

	//depth/stencil buffer
	D3D11_TEXTURE2D_DESC depthStencilDesc;
	depthStencilDesc.Width = mClientWidth;
	depthStencilDesc.Height = mClientHeight;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;
	if (mEnable4xMsaa) {
		depthStencilDesc.SampleDesc.Count = 4;
		depthStencilDesc.SampleDesc.Quality = m4xMsaaQuality - 1;
	}
	else {
		depthStencilDesc.SampleDesc.Count = 1;
		depthStencilDesc.SampleDesc.Quality = 0;
	}
	hr = mDevice->CreateTexture2D(&depthStencilDesc, 0, &mDepthStencilBuffer);
	if (FAILED(hr)) {
		MessageBox(0, L"CreateTexture2D Failed.", 0, 0);
		return false;
	}
	hr = mDevice->CreateDepthStencilView(mDepthStencilBuffer, 0, &mDepthStencilView);
	if (FAILED(hr)) {
		MessageBox(0, L"CreateDepthStencilView Failed.", 0, 0);
		return false;
	}

	//bind
	mImmediateContext->OMSetRenderTargets(1, &mRenderTargetView, mDepthStencilView);

	//viewport
	mScreenViewport.Width = static_cast<float>(mClientWidth);
	mScreenViewport.Height = static_cast<float>(mClientHeight);
	mScreenViewport.MinDepth = 0.0f;
	mScreenViewport.MaxDepth = 1.0f;
	mScreenViewport.TopLeftX = 0;
	mScreenViewport.TopLeftY = 0;
	mImmediateContext->RSSetViewports(1, &mScreenViewport);

	return true;
}
