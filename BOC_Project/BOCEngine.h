#ifndef __BOCENGINE_H
#define __BOCENGINE_H

#include "D3dApp.h"

#include <DirectXMath.h>// <xnamath.h>
#include <D3Dcompiler.h>// <D3DX11.h>
#include <DirectXColors.h>

using namespace DirectX;

struct Vertex
{
	XMFLOAT3 Pos;
	XMFLOAT4 Color;
};
struct ConstantBuffer
{
	XMMATRIX mWorld;
	XMMATRIX mView;
	XMMATRIX mProj;
	XMFLOAT4 vLightDir[2];
	XMFLOAT4 vLightColor[2];
	XMFLOAT4 vOutputColor;
};
struct D3DObject
{
	ID3D11Buffer* mVertexBuffer;
	ID3D11Buffer* mIndexBuffer;
	ID3D11Buffer* mConstantBuffer;
	UINT vCount;
	UINT iCount;
};

const float Pi = 3.1415926535f;

class BOCEngine :public D3dApp
{
public:
	BOCEngine(HINSTANCE hInstance);
	~BOCEngine();
	virtual bool init();

protected:
	virtual bool OnResize();
	virtual void DrawScene() override;
	virtual void UpdateScene(float deltaTime) override;

	virtual void OnMouseDown(WPARAM btnState, int x, int y) override;
	virtual void OnMouseUp(WPARAM btnState, int x, int y) override;
	virtual void OnMouseMove(WPARAM btnState, int x, int y) override;
	virtual void OnMouseWheel(WPARAM btnState, int x, int y, int delta) override;
	virtual void OnKeyDown(WPARAM btnState, int x, int y) override;
private:
	bool CompileShaderFromFile(WCHAR* fileName, LPCSTR entryPoint, LPCSTR shaderModel, ID3DBlob** blobOut);
	bool BuildGeometryBuffers(WCHAR* fileName, XMFLOAT4 color, D3DObject* object);
	bool BuildGeometryBuffers(Vertex* vertices, UINT vCount, UINT* indices, UINT iCount, D3DObject* object);

	ID3D11Buffer* mVertexBuffer;
	ID3D11Buffer* mIndexBuffer;
	ID3D11Buffer* mConstantBuffer;

	D3DObject box;
	D3DObject triangle;
	D3DObject skull;
	ID3D11RasterizerState* mWireframeRS;

	ID3D11InputLayout* mInputLayout;
	ID3D11VertexShader* mVertexShader;
	ID3D11PixelShader* mPixelShader;
	ID3D11PixelShader* mPixelShaderSolid;

	XMMATRIX mWorld;
	XMMATRIX mView;
	XMMATRIX mProj;

	XMFLOAT4 mWorldLight;

	float mTheta;
	float mPhi;
	float mRadius;
	float mSensitivity;

	bool lotationFlag;

	POINT mLastMousePos;
};

#endif // __BOCENGINE_H
