#include "BOCEngine.h"
#include <fstream>
#include <vector>

BOCEngine::BOCEngine(HINSTANCE hInstacne)
:D3dApp(hInstacne), mTheta(1.5f*Pi), mPhi(0.25f*Pi), mRadius(5.0f), mSensitivity(1.0f),
mVertexBuffer(0), mIndexBuffer(0), mConstantBuffer(0),
mInputLayout(0), mVertexShader(0), mPixelShader(0), mPixelShaderSolid(0)
{
	D3dApp::mainWndCaption = L"Book Of Change";
	D3dApp::appClassName = L"BOCClass";

	mLastMousePos.x = 0;
	mLastMousePos.y = 0;
	mWorld = XMMatrixIdentity();
	//mWorldLight = XMFLOAT4(500.0f, 500.0f, -500.0f, 1.0f);
}

BOCEngine::~BOCEngine()
{
	D3dApp::~D3dApp();
	if (mVertexBuffer) mVertexBuffer->Release();
	if (mIndexBuffer) mIndexBuffer->Release();
	if (mConstantBuffer) mConstantBuffer->Release();
	if (mInputLayout) mInputLayout->Release();
	if (mVertexShader) mVertexShader->Release();
	if (mPixelShader) mPixelShader->Release();
}

bool BOCEngine::init() {
	if (!D3dApp::Init())
		return false;

	HRESULT hr;
	
	//Build FX
	//Á¤Á¡ ¼ÎÀÌ´õ, Áö¿À¸ÞÆ®¸® ¼ÎÀÌÅÍ, ÇÈ¼¿ ¼ÎÀÌ´õ
	ID3DBlob* pVSBlob = NULL;
	//¿¡·¯°¡ ÀÖÀ»¶§
	if (!CompileShaderFromFile(L"FX/color.fx", "VS", "vs_5_0", &pVSBlob)) {
		MessageBox(NULL,
			L"The FX file cannot be compiled. Please run this executable from the directory that contains the FX file.",
			L"Error", MB_OK);
		return false;
	}

	//create the vertex shader
	hr = mDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, &mVertexShader);
	if (FAILED(hr)) {
		pVSBlob->Release();
		return false;
	}

	//Build Vertex Layout
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA,0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA,0 }
	};
	UINT numElements = ARRAYSIZE(layout);

	//create the input layout
	hr = mDevice->CreateInputLayout(layout, numElements, pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), &mInputLayout);
	pVSBlob->Release();
	if (FAILED(hr))
		return false;

	//set the input layout->DrawScene
	//mImmediateContext->IASetInputLayout(mInputLayout);

	//Compile the pixel shader
	ID3DBlob* pPSBlob = NULL;
	if (!CompileShaderFromFile(L"FX/color.fx", "PS", "ps_5_0", &pPSBlob)) {
		MessageBox(NULL, L"The FX file cannot be compiled. Please run this executable from the directory that contains the FX file", L"Error", MB_OK);
		return false;
	}

	//create the pixel shader
	hr = mDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &mPixelShader);
	pPSBlob->Release();
	if (FAILED(hr))
		return false;





	///////////////////////////////////////////////////////////////////////////////box

	//create vertex buffer
	Vertex vertices[] =
	{
		{ XMFLOAT3(-1.0f, +1.0f, -1.0f), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f) },//blue
		{ XMFLOAT3(+1.0f, +1.0f, -1.0f), XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f) },//green
		{ XMFLOAT3(+1.0f, +1.0f, +1.0f), XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f) },//cyan
		{ XMFLOAT3(-1.0f, +1.0f, +1.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) },//red
		{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f) },//magenta
		{ XMFLOAT3(+1.0f, -1.0f, -1.0f), XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f) },//yelow
		{ XMFLOAT3(+1.0f, -1.0f, +1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f) },//white
		{ XMFLOAT3(-1.0f, -1.0f, +1.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) },//black
	};

	//create the index buffer
	UINT indices[] =
	{	//¾Õ
		3,1,0,
		2,1,3,
		//µÚ
		0,5,4,
		1,5,0,
		//ÁÂ
		3,4,7,
		0,4,3,
		//¿ì
		1,6,5,
		2,6,1,
		//»ó
		2,7,6,
		3,7,2,
		//ÇÏ
		6,4,5,
		7,4,6,
	};
	if (!BuildGeometryBuffers(vertices, 8, indices, 36, &box))
		return false;

	///////////////////////////////////////////////////////////////////////////////triangle

	//create vertex buffer
	//x z y
	Vertex tVertices[] =
	{
		{ XMFLOAT3(+5.0f, +1.5f, +5.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) },//red
		{ XMFLOAT3(+6.0f, +0.0f, +6.0f), XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f) },//green
		{ XMFLOAT3(+6.0f, +0.0f, +4.0f), XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f) },
		{ XMFLOAT3(+4.0f, +0.0f, +4.0f), XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f) },
		{ XMFLOAT3(+4.0f, +0.0f, +6.0f), XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f) },
	};

	//create the index buffer
	UINT tIndices[] =
	{	
		0,1,2,
		0,2,3,
		0,3,4,
		0,4,1,
		1,3,2,
		1,4,3,
	};
	if (!BuildGeometryBuffers(tVertices, 5, tIndices, 18, &triangle))
		return false;

	//skull

	XMFLOAT4 black(0.0f, 0.0f, 0.0f, 1.0f);
	if(!BuildGeometryBuffers(L"Models/skull.txt", black, &skull))
		return false;

	//wirefram ·»´õ¸µ ¼³Á¤
	D3D11_RASTERIZER_DESC wireframeDesc;
	ZeroMemory(&wireframeDesc, sizeof(D3D11_RASTERIZER_DESC));
	wireframeDesc.FillMode = D3D11_FILL_WIREFRAME; //D3D11_FILL_SOLID;
	wireframeDesc.CullMode = D3D11_CULL_NONE;//front, back
	wireframeDesc.FrontCounterClockwise = false;
	wireframeDesc.DepthClipEnable = true;
	hr = mDevice->CreateRasterizerState(&wireframeDesc, &mWireframeRS);
	if (FAILED(hr))
		return false;
	//wire
	mImmediateContext->RSSetState(mWireframeRS);

	return true;
}

bool BOCEngine::CompileShaderFromFile(WCHAR* fileName, LPCSTR entryPoint, LPCSTR shaderModel, ID3DBlob** blobOut) {
	HRESULT hr = S_OK;

	DWORD shaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined ( _DEBUG)
	shaderFlags |= D3DCOMPILE_DEBUG;
#endif

	ID3DBlob * errorBlob;
	//D3DX11CompileFromFile(L"FX/color.fx", NULL, NULL, "VS", "vs_4_0", shaderFlags, 0, NULL, &pVSBlob, &pErrorBlob, NULL);
	hr = D3DCompileFromFile(fileName, NULL, NULL, entryPoint, shaderModel, shaderFlags, 0, blobOut, &errorBlob);
	if (errorBlob != NULL) {
		MessageBoxA(0, (char*)errorBlob->GetBufferPointer(), 0, 0);
		if (errorBlob)errorBlob->Release();
		return false;
	}
	if (errorBlob)errorBlob->Release();

	return true;
}

bool BOCEngine::OnResize() {
	if (!D3dApp::OnResize())
		return false;

	mProj= XMMatrixPerspectiveLH(0.25f*Pi, mClientWidth / (float)mClientHeight,
		1.0f, 1000.0f);
	return true;
}


//SCENE
void BOCEngine::UpdateScene(float deltaTime) {
	//±¸ÁÂÇ¥°è
	float x = mRadius * sinf(mPhi) * cosf(mTheta);
	float y = mRadius * sinf(mPhi) * sinf(mTheta);
	float z = mRadius * cosf(mPhi);

	XMVECTOR pos = XMVectorSet(x, z, y, 1.0f);	//eye
	XMVECTOR target = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);	//at
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);		//up

	//World View Projection
	if(lotationFlag)
		mWorld = XMMatrixRotationY(mTimer.TotalTime());
	else
		mWorld = XMMatrixRotationY(0);
	mView = XMMatrixLookAtLH(pos, target, up);
	mProj = XMMatrixPerspectiveFovLH(XM_PIDIV2, mClientWidth / (float)mClientHeight, 0.01f, 100.0f);

}

void BOCEngine::DrawScene() {
	//Clear the back buffer
	float LightStillBlue[4] = { 0.69f, 0.77f, 0.87f, 1.0f };
	mImmediateContext->ClearRenderTargetView(mRenderTargetView, LightStillBlue);
	//clear the depth buffer(max)
	mImmediateContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	//set the input layout
	mImmediateContext->IASetInputLayout(mInputLayout);
	//set primitive topology
	mImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	
	//for setting vertex buffer
	UINT stride = sizeof(Vertex);
	UINT offset = 0;

	
	XMFLOAT4 vLightDirs[2] =
	{
		XMFLOAT4(-0.577f, 0.577f, -0.577f, 1.0f),
		XMFLOAT4(0.0f, 0.0f, -1.0f, 1.0f),
	};
	XMFLOAT4 vLightColors[2] =
	{
		XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f),
		XMFLOAT4(0.5f, 0.0f, 0.0f, 1.0f)
	};

	
	XMMATRIX mRotate = XMMatrixRotationY(-2.0f*mTimer.TotalTime());
	XMVECTOR vLightDir = XMLoadFloat4(&vLightDirs[1]);
	vLightDir = XMVector3Transform(vLightDir, mRotate);
	XMStoreFloat4(&vLightDirs[1], vLightDir);
	

	//set contants
	ConstantBuffer cb;
	cb.mWorld = XMMatrixTranspose(mWorld);
	cb.mView = XMMatrixTranspose(mView);
	cb.mProj = XMMatrixTranspose(mProj);

	
	cb.vLightDir[0] = vLightDirs[0];
	cb.vLightDir[1] = vLightDirs[1];
	cb.vLightColor[0] = vLightColors[0];
	cb.vLightColor[1] = vLightColors[1];
	cb.vOutputColor = XMFLOAT4(0, 0, 0, 0);
	mImmediateContext->UpdateSubresource(box.mConstantBuffer, 0, NULL, &cb, 0, 0);
	

	//set shader
	mImmediateContext->VSSetShader(mVertexShader, NULL, 0);
	mImmediateContext->PSSetShader(mPixelShader, NULL, 0);

	//box

	//set vertex buffer
	mImmediateContext->IASetVertexBuffers(0, 1, &box.mVertexBuffer, &stride, &offset);
	//set Index buffer
	mImmediateContext->IASetIndexBuffer(box.mIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	mImmediateContext->UpdateSubresource(box.mConstantBuffer, 0, NULL, &cb, 0, 0);
	//Render a triangle
	mImmediateContext->VSSetConstantBuffers(0, 1, &box.mConstantBuffer);
    mImmediateContext->PSSetConstantBuffers(0, 1, &box.mConstantBuffer);
	mImmediateContext->DrawIndexed(box.iCount, 0, 0);

	//triangle
	/*
	//set vertex buffer
	mImmediateContext->IASetVertexBuffers(0, 1, &triangle.mVertexBuffer, &stride, &offset);
	//set Index buffer
	mImmediateContext->IASetIndexBuffer(triangle.mIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	mImmediateContext->UpdateSubresource(triangle.mConstantBuffer, 0, NULL, &cb, 0, 0);
	//Render a triangle
	mImmediateContext->VSSetConstantBuffers(0, 1, &triangle.mConstantBuffer);
	mImmediateContext->DrawIndexed(triangle.iCount, 0, 0);
	*/

	//skull
	//set vertex buffer
	mImmediateContext->IASetVertexBuffers(0, 1, &skull.mVertexBuffer, &stride, &offset);
	//set Index buffer
	mImmediateContext->IASetIndexBuffer(skull.mIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	//set contants
	mImmediateContext->UpdateSubresource(skull.mConstantBuffer, 0, NULL, &cb, 0, 0);
	//Render a triangle
	mImmediateContext->VSSetConstantBuffers(0, 1, &skull.mConstantBuffer);
	mImmediateContext->DrawIndexed(skull.iCount, 0, 0);
	

	mSwapChain->Present(0, 0);
}

bool BOCEngine::BuildGeometryBuffers(WCHAR* fileName, XMFLOAT4 color, D3DObject* object) {
	std::ifstream fin(fileName);

	if (!fin) {
		wchar_t buf[64];
		swprintf_s(buf, L"%s is not found.", fileName);
		MessageBox(0, buf, 0, 0);
		return false;
	}

	object->vCount = 0;
	object->iCount = 0;
	std::string ignore;

	fin >> ignore >> object->vCount;
	fin >> ignore >> object->iCount;
	object->iCount *= 3;
	fin >> ignore >> ignore >> ignore >> ignore;

	float nx, ny, nz;

	std::vector<Vertex> vertices(object->vCount);
	for (UINT i = 0; i < object->vCount; ++i) {
		fin >> vertices[i].Pos.x >> vertices[i].Pos.y >> vertices[i].Pos.z;

		vertices[i].Color = color;
		//Normal not used in this
		fin >> nx >> ny >> nz;
	}

	fin >> ignore;
	fin >> ignore;
	fin >> ignore;

	std::vector<UINT> indices(object->iCount);
	for (UINT i = 0; i < object->iCount; ++i) {
		fin >> indices[i];
	}

	fin.close();

	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_IMMUTABLE;
	bd.ByteWidth = sizeof(Vertex) * object->vCount;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
	bd.StructureByteStride = 0;
	D3D11_SUBRESOURCE_DATA initData;
	ZeroMemory(&initData, sizeof(initData));
	initData.pSysMem = &vertices[0];
	HRESULT hr = mDevice->CreateBuffer(&bd, &initData, &object->mVertexBuffer);
	if (FAILED(hr))
		return false;

	bd.Usage = D3D11_USAGE_IMMUTABLE;
	bd.ByteWidth = sizeof(UINT) * object->iCount;
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;
	initData.pSysMem = &indices[0];
	hr = mDevice->CreateBuffer(&bd, &initData, &object->mIndexBuffer);
	if (FAILED(hr))
		return false;

	//create the constant buffer
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(ConstantBuffer);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
	hr = mDevice->CreateBuffer(&bd, NULL, &object->mConstantBuffer);
	if (FAILED(hr))
		return false;

	return true;
}

bool BOCEngine::BuildGeometryBuffers(Vertex* vertices, UINT vCount, UINT* indices, UINT iCount, D3DObject* object) {
	object->vCount = vCount;
	object->iCount = iCount;

	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_IMMUTABLE;
	bd.ByteWidth = sizeof(Vertex) * object->vCount;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
	bd.StructureByteStride = 0;
	D3D11_SUBRESOURCE_DATA initData;
	ZeroMemory(&initData, sizeof(initData));
	initData.pSysMem = vertices;
	HRESULT hr = mDevice->CreateBuffer(&bd, &initData, &object->mVertexBuffer);
	if (FAILED(hr))
		return false;

	bd.Usage = D3D11_USAGE_IMMUTABLE;
	bd.ByteWidth = sizeof(UINT) * object->iCount;
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;
	initData.pSysMem = indices;
	hr = mDevice->CreateBuffer(&bd, &initData, &object->mIndexBuffer);
	if (FAILED(hr))
		return false;

	//create the constant buffer
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(ConstantBuffer);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
	hr = mDevice->CreateBuffer(&bd, NULL, &object->mConstantBuffer);
	if (FAILED(hr))
		return false;

	return true;
}

//MOUSE
void BOCEngine::OnMouseDown(WPARAM btnState, int x, int y) {
	//wchar_t buf[64];
	//swprintf(buf, L"cliked x:%d->%d y:%d->%d", mLastMousePos.x, x, mLastMousePos.y, y);
	//MessageBox(mHWnd, buf, 0, 0);
	mLastMousePos.x = x;
	mLastMousePos.y = y;
	SetCapture(mHWnd);
}

void BOCEngine::OnMouseUp(WPARAM btnState, int x, int y) {
	mLastMousePos.x = x;
	mLastMousePos.y = y;
	ReleaseCapture();
	//wchar_t buf[64];
	//swprintf(buf, L"T:%lf P:%lf R:%lf", mTheta * 180 / Pi, mPhi * 180 / Pi, mRadius);
	//MessageBox(mHWnd, buf, mainWndCaption.c_str(), 0);
}

void BOCEngine::OnMouseMove(WPARAM btnState, int x, int y) {
	if ((btnState & MK_LBUTTON) != 0) {
		float dx = XMConvertToRadians(0.25f*static_cast<float>(mLastMousePos.x-x));
		float dy = XMConvertToRadians(0.25f*static_cast<float>(mLastMousePos.y-y));

		mTheta += dx * mSensitivity;
		mPhi += dy * mSensitivity;

		//mTheta = mTheta > 2 * Pi ? mTheta - 2 * Pi : mTheta;
		mPhi = mPhi < 0.1f ? 0.1f : (mPhi > Pi - 0.1f ? Pi - 0.1f : mPhi);
		//x < low ? low : (x > high ? high : x); 
	}
	else if ((btnState & MK_RBUTTON) != 0) {
		//float dx = 0.005f*static_cast<float>(x - mLastMousePos.x);
		//float dy = 0.005f*static_cast<float>(y - mLastMousePos.y);
		//mRadius += mSensitivity * (dx - dy);
		//3~15
		//mRadius = mRadius < 3.0f ? 3.0f : (mRadius > 15.0f ? 15.0f : mRadius);
	}
	mLastMousePos.x = x;
	mLastMousePos.y = y;
}

void BOCEngine::OnMouseWheel(WPARAM btnState, int x, int y, int delta) {
	//wchar_t buf[64];
	//swprintf(buf, L"%d", delta);
	//MessageBox(mHWnd, buf, mainWndCaption.c_str(), 0);
	//-120 or 120
	if (delta > 0)
		mRadius -= mRadius * mRadius*mSensitivity*0.02f;
	else
		mRadius += mRadius * mRadius*mSensitivity*0.02f;
	mRadius = mRadius < 3.0f ? 3.0f : (mRadius > 15.0f ? 15.0f : mRadius);
}

void BOCEngine::OnKeyDown(WPARAM btnState, int x, int y) {
	switch (btnState)
	{
	case VK_SPACE:
		if (lotationFlag)
			lotationFlag = false;
		else
			lotationFlag = true;
		break;
	case VK_RIGHT:
		mTheta += 0.1f;
		break;
	case VK_LEFT:
		mTheta -= 0.1f;
		break;
	case VK_UP:
		mPhi -= 0.1f;
		break;
	case VK_DOWN:
		mPhi += 0.1f;
		break;
	default:
		break;
	}
}