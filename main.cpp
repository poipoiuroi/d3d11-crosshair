#pragma comment(linker, "/MERGE:.text=Z")
#pragma comment(linker, "/MERGE:.rdata=Z")
#pragma comment(linker, "/MERGE:.data=Z")
#pragma comment(linker, "/MERGE:.bss=Z")
#pragma comment(linker, "/MERGE:.pdata=Z")
#pragma comment(linker, "/MERGE:.tls=Z")
#pragma comment(linker, "/ALIGN:16")
#pragma comment(linker, "/NODEFAULTLIB")
#pragma comment(linker, "/DYNAMICBASE:NO")
#pragma comment(linker, "/ENTRY:dream")
#pragma comment(linker, "/EMITPOGOPHASEINFO")
#pragma comment(linker, "/RELEASE")

#include <windows.h>
#include <d2d1_3.h>
#include <d3d11_2.h>
#include <dwrite_3.h>
#include <dcomp.h>

extern "C" int _fltused = 0;

#pragma optimize("", off)
#pragma function(memset)
__forceinline void* memset(void* dest, int value, size_t num)
{
	__stosb(static_cast<unsigned char*>(dest), static_cast<unsigned char>(value), num);
	return dest;
}

#pragma function(memcpy)
__forceinline void* memcpy(void* dest, const void* src, size_t num)
{
	__movsb(static_cast<unsigned char*>(dest), static_cast<const unsigned char*>(src), num);
	return dest;
}
#pragma optimize("", restore)

HWND hwnd = 0;
IDXGISwapChain1* swap_chain = 0;
ID2D1DeviceContext* dev_context = 0;
IDCompositionTarget* comp_target = 0;
IDWriteFactory* dwrite_factory = 0;

int dream()
{
	int sw = GetSystemMetrics(SM_CXSCREEN), sh = GetSystemMetrics(SM_CYSCREEN);
	auto hInst = GetModuleHandleW(0);
	const int win_w = 50, win_h = 50;

	WNDCLASS wc = {};
	wc.lpfnWndProc = DefWindowProc; wc.hInstance = hInst; wc.lpszClassName = L"OverlayClass";
	RegisterClassW(&wc);

	hwnd = CreateWindowExW(WS_EX_NOREDIRECTIONBITMAP | WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
		wc.lpszClassName, L"Overlay", WS_POPUP | WS_VISIBLE, (sw - win_w) / 2, (sh - win_h) / 2, win_w, win_h, 0, 0, hInst, 0);

	SetLayeredWindowAttributes(hwnd, 0, 0, LWA_COLORKEY);

	ID3D11Device* d3d_device = 0;
	if (FAILED(D3D11CreateDevice(0, D3D_DRIVER_TYPE_HARDWARE, 0, D3D11_CREATE_DEVICE_BGRA_SUPPORT, 0, 0, D3D11_SDK_VERSION, &d3d_device, 0, 0))) return 1;

	IDXGIDevice* dxgi_device = 0;
	if (FAILED(d3d_device->QueryInterface(__uuidof(IDXGIDevice), (void**)&dxgi_device))) return 2;

	IDXGIFactory2* dx_factory = 0;
	if (FAILED(CreateDXGIFactory2(0, __uuidof(IDXGIFactory2), (void**)&dx_factory))) return 3;

	RECT rc; GetClientRect(hwnd, &rc);
	DXGI_SWAP_CHAIN_DESC1 desc{};
	desc.Width = rc.right - rc.left;
	desc.Height = rc.bottom - rc.top;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	desc.BufferCount = 2;
	desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	desc.AlphaMode = DXGI_ALPHA_MODE_PREMULTIPLIED;
	desc.SampleDesc.Count = 1;

	if (FAILED(dx_factory->CreateSwapChainForComposition(dxgi_device, &desc, 0, &swap_chain))) return 4;

	ID2D1Factory2* d2_factory = 0;
	if (FAILED(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, D2D1_FACTORY_OPTIONS{ D2D1_DEBUG_LEVEL_NONE }, &d2_factory))) return 5;

	ID2D1Device1* d2_device = 0;
	if (FAILED(d2_factory->CreateDevice(dxgi_device, &d2_device))) return 6;

	if (FAILED(d2_device->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &dev_context))) return 7;

	IDXGISurface2* surface = 0;
	if (FAILED(swap_chain->GetBuffer(0, __uuidof(IDXGISurface2), (void**)&surface))) return 8;

	D2D1_BITMAP_PROPERTIES1 bp = { { DXGI_FORMAT_R8G8B8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED }, 96, 96, D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW };

	ID2D1Bitmap1* target_bitmap = 0;
	if (FAILED(dev_context->CreateBitmapFromDxgiSurface(surface, &bp, &target_bitmap))) return 9;
	dev_context->SetTarget(target_bitmap);

	IDCompositionDevice* comp_device = 0;
	if (FAILED(DCompositionCreateDevice(dxgi_device, __uuidof(IDCompositionDevice), (void**)&comp_device))) return 10;

	if (FAILED(comp_device->CreateTargetForHwnd(hwnd, TRUE, &comp_target))) return 11;

	IDCompositionVisual* visual = 0;
	if (FAILED(comp_device->CreateVisual(&visual))) return 12;

	if (FAILED(visual->SetContent(swap_chain))) return 13;

	if (FAILED(comp_target->SetRoot(visual))) return 14;

	if (FAILED(comp_device->Commit())) return 15;

	if (FAILED(DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), (IUnknown**)&dwrite_factory))) return 16;

	dev_context->BeginDraw();
	dev_context->Clear();

	constexpr float gap = 0.0f, len = 5.0f, thick = 2.1f;
	
	float cx = win_w / 2.0f, cy = win_h / 2.0f;

	ID2D1SolidColorBrush* brush = 0;
	if (FAILED(dev_context->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Red), &brush))) return 17;

	dev_context->DrawLine({ cx, cy - gap - len }, { cx, cy - gap }, brush, thick);
	dev_context->DrawLine({ cx, cy + gap }, { cx, cy + gap + len }, brush, thick);
	dev_context->DrawLine({ cx - gap - len, cy }, { cx - gap, cy }, brush, thick);
	dev_context->DrawLine({ cx + gap, cy }, { cx + gap + len, cy }, brush, thick);

	if (FAILED(dev_context->EndDraw())) return 18;
	if (FAILED(swap_chain->Present(1, 0))) return 19;

	for (;;) if (GetAsyncKeyState(VK_F9)) break; Sleep(100);
	
	TerminateProcess((void*)-1, 0);
}
