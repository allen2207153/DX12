#include <Windows.h>
#include <iostream>
#include <d3d12.h>
#include<dxgi1_6.h>
#include<vector>
#include<functional>
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#define WINDOW_WIDTH 720
#define WINDOW_HEIGHT 480

using namespace std;


ID3D12Device* _dev = nullptr;
IDXGIFactory6* _dxgiFactory = nullptr;
IDXGISwapChain4* _swapchain = nullptr;

ID3D12CommandAllocator* _cmdAllocator = nullptr;
ID3D12GraphicsCommandList* _cmdList = nullptr;
ID3D12CommandQueue* _cmdQueue = nullptr;

HRESULT D3D12CreateDevice(
	IUnknown* pAdapter,//ひとまずはnullptrOK
	D3D_FEATURE_LEVEL			MinimumFeatureLevel,//最低限必要なフィーチャーレベル
	REFIID									riid,//後述
	void** ppDevice//後述
);
D3D_FEATURE_LEVEL levels[] =
{
	D3D_FEATURE_LEVEL_12_1,
	D3D_FEATURE_LEVEL_12_0,
	D3D_FEATURE_LEVEL_11_1,
	D3D_FEATURE_LEVEL_11_0,
};

D3D12_COMMAND_QUEUE_DESC cmdQueueDesc = {};




// @brief コンソール画面にフォーマット付き文字列を表示
// @parm formatフォマード（%dとか%fとかの）
// @param 可変長引数
// @remarks この関数はデバッグ用です。デバッグ時にしか動作しません
void DebugOutputFormatString(const char* format, ...)
{
#ifdef _DEBUG
	va_list valist;
	va_start(valist, format);
	printf(format, valist);
	va_end(valist);
#endif // DEBUG
}
//書かなければいけない関数
LRESULT WindowProcedure(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	//ウインドウが破棄されたら呼ばれる
	if (msg == WM_DESTROY)
	{
		PostQuitMessage(0);//OSに対して「	もうこのアプリは終わる」と伝える
		return 0;
	}
	return DefWindowProc(hwnd, msg, wparam, lparam);;//既定の処理を行う
}
//#ifdef _DEBUG
int main()
{
	//#else
	//int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
	//{
	//#endif // DEBUG
	//	DebugOutputFormatString("Show window test.");
	//	getchar();
	//	return 0;

	vector<function<void(void)>> commandlist;//コマンドリストを模したもの

	commandlist.push_back([]() {cout << "GPU Set RTV-1" << endl; });//命令１
	cout << "CPU　Set命令-2" << endl;

	commandlist.push_back([]() {cout << "GPU Clear RTV-3" << endl; });//命令2
	cout << "CPU　Clear命令-4" << endl;

	commandlist.push_back([]() {cout << "GPU Close RTV-5" << endl; });//命令3
	cout << "CPU　Close命令-6" << endl;

	cout << endl;

	//コマンドキューのExecuteCommand模した
	for (auto& cmd : commandlist)
	{
		cmd();
	}

	getchar();

	return 0;
	//Direct3Dデバイスの初期化
	D3D_FEATURE_LEVEL featureLevel;

	for (auto& lv : levels)
	{
		if (D3D12CreateDevice(nullptr, lv, IID_PPV_ARGS(&_dev)) == S_OK)
		{
			featureLevel = lv;
			break;//生成可能なバージョンが見つかったらループを打ち切り
		}
	}

	//タイムアウトなし
	cmdQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

	//アダプターを１つしか使わないときは０でよい
	cmdQueueDesc.NodeMask = 0;

	//プライオリティは特に指定なし
	cmdQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;

	//コマンドリストと合わせる
	cmdQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;


	auto result = CreateDXGIFactory1(IID_PPV_ARGS(&_dxgiFactory));
	result = _dev->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(&_cmdAllocator));

	result = _dev->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
		_cmdAllocator, nullptr,
		IID_PPV_ARGS(&_cmdList));
	//キュー生成
	result = _dev->CreateCommandQueue(&cmdQueueDesc, IID_PPV_ARGS(&_cmdQueue));
	//アダプターの列挙用
	vector<IDXGIAdapter*> adapters;

	//ここに特定の名前を持つアダプターオブジェクトが入る
	IDXGIAdapter* tmpAdapter = nullptr;

	for (int i = 0;
		_dxgiFactory->EnumAdapters(i, &tmpAdapter) != DXGI_ERROR_NOT_FOUND; ++i)
	{
		adapters.push_back(tmpAdapter);
	}
	for (auto adpt : adapters)
	{
		DXGI_ADAPTER_DESC adesc = {};
		adpt->GetDesc(&adesc);//アダプターの説明オブジェクト取得

		wstring strDesc = adesc.Description;

		//探したいアダプターの名前を確認
		if (strDesc.find(L"NVIDIA") != std::string::npos)
		{
			tmpAdapter = adpt;
			break;
		}
	}


	WNDCLASSEX w = {};
	w.cbSize = sizeof(WNDCLASSEX);
	w.lpfnWndProc = (WNDPROC)WindowProcedure;//コールバック関数の指定
	w.lpszClassName = TEXT("DX12Sample");//アプリケーションクラス名（適当でよい）
	w.hInstance = GetModuleHandle(nullptr);//ハンドルの取得

	RegisterClassEx(&w);//アプリケーションクラス（ウィンドウクラスの指定をOSに伝える）

	RECT wrc = { 0,0,WINDOW_WIDTH,WINDOW_HEIGHT };//ウィンドウサイズを決める

	//関係を使ってウィンドウのサイズを補正する
	AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);

	//ウィンドウオブジェクトの生成
	HWND hwnd = CreateWindow(w.lpszClassName,
		TEXT("DX12テスト"),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		wrc.right - wrc.left,
		wrc.bottom - wrc.top,
		nullptr,
		nullptr,
		w.hInstance,
		nullptr);

	//ウィンドウ表示
	ShowWindow(hwnd, SW_SHOW);

	MSG msg = {};
	while (true)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		if (msg.message == WM_QUIT)
		{
			break;
		}


	}
	//もうクラスは使わないので登録解除する
	UnregisterClass(w.lpszClassName, w.hInstance);
}
