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
	IUnknown* pAdapter,//�ЂƂ܂���nullptrOK
	D3D_FEATURE_LEVEL			MinimumFeatureLevel,//�Œ���K�v�ȃt�B�[�`���[���x��
	REFIID									riid,//��q
	void** ppDevice//��q
);
D3D_FEATURE_LEVEL levels[] =
{
	D3D_FEATURE_LEVEL_12_1,
	D3D_FEATURE_LEVEL_12_0,
	D3D_FEATURE_LEVEL_11_1,
	D3D_FEATURE_LEVEL_11_0,
};

D3D12_COMMAND_QUEUE_DESC cmdQueueDesc = {};




// @brief �R���\�[����ʂɃt�H�[�}�b�g�t���������\��
// @parm format�t�H�}�[�h�i%d�Ƃ�%f�Ƃ��́j
// @param �ϒ�����
// @remarks ���̊֐��̓f�o�b�O�p�ł��B�f�o�b�O���ɂ������삵�܂���
void DebugOutputFormatString(const char* format, ...)
{
#ifdef _DEBUG
	va_list valist;
	va_start(valist, format);
	printf(format, valist);
	va_end(valist);
#endif // DEBUG
}
//�����Ȃ���΂����Ȃ��֐�
LRESULT WindowProcedure(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	//�E�C���h�E���j�����ꂽ��Ă΂��
	if (msg == WM_DESTROY)
	{
		PostQuitMessage(0);//OS�ɑ΂��āu	�������̃A�v���͏I���v�Ɠ`����
		return 0;
	}
	return DefWindowProc(hwnd, msg, wparam, lparam);;//����̏������s��
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

	vector<function<void(void)>> commandlist;//�R�}���h���X�g��͂�������

	commandlist.push_back([]() {cout << "GPU Set RTV-1" << endl; });//���߂P
	cout << "CPU�@Set����-2" << endl;

	commandlist.push_back([]() {cout << "GPU Clear RTV-3" << endl; });//����2
	cout << "CPU�@Clear����-4" << endl;

	commandlist.push_back([]() {cout << "GPU Close RTV-5" << endl; });//����3
	cout << "CPU�@Close����-6" << endl;

	cout << endl;

	//�R�}���h�L���[��ExecuteCommand�͂���
	for (auto& cmd : commandlist)
	{
		cmd();
	}

	getchar();

	return 0;
	//Direct3D�f�o�C�X�̏�����
	D3D_FEATURE_LEVEL featureLevel;

	for (auto& lv : levels)
	{
		if (D3D12CreateDevice(nullptr, lv, IID_PPV_ARGS(&_dev)) == S_OK)
		{
			featureLevel = lv;
			break;//�����\�ȃo�[�W���������������烋�[�v��ł��؂�
		}
	}

	//�^�C���A�E�g�Ȃ�
	cmdQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

	//�A�_�v�^�[���P�����g��Ȃ��Ƃ��͂O�ł悢
	cmdQueueDesc.NodeMask = 0;

	//�v���C�I���e�B�͓��Ɏw��Ȃ�
	cmdQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;

	//�R�}���h���X�g�ƍ��킹��
	cmdQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;


	auto result = CreateDXGIFactory1(IID_PPV_ARGS(&_dxgiFactory));
	result = _dev->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(&_cmdAllocator));

	result = _dev->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
		_cmdAllocator, nullptr,
		IID_PPV_ARGS(&_cmdList));
	//�L���[����
	result = _dev->CreateCommandQueue(&cmdQueueDesc, IID_PPV_ARGS(&_cmdQueue));
	//�A�_�v�^�[�̗񋓗p
	vector<IDXGIAdapter*> adapters;

	//�����ɓ���̖��O�����A�_�v�^�[�I�u�W�F�N�g������
	IDXGIAdapter* tmpAdapter = nullptr;

	for (int i = 0;
		_dxgiFactory->EnumAdapters(i, &tmpAdapter) != DXGI_ERROR_NOT_FOUND; ++i)
	{
		adapters.push_back(tmpAdapter);
	}
	for (auto adpt : adapters)
	{
		DXGI_ADAPTER_DESC adesc = {};
		adpt->GetDesc(&adesc);//�A�_�v�^�[�̐����I�u�W�F�N�g�擾

		wstring strDesc = adesc.Description;

		//�T�������A�_�v�^�[�̖��O���m�F
		if (strDesc.find(L"NVIDIA") != std::string::npos)
		{
			tmpAdapter = adpt;
			break;
		}
	}


	WNDCLASSEX w = {};
	w.cbSize = sizeof(WNDCLASSEX);
	w.lpfnWndProc = (WNDPROC)WindowProcedure;//�R�[���o�b�N�֐��̎w��
	w.lpszClassName = TEXT("DX12Sample");//�A�v���P�[�V�����N���X���i�K���ł悢�j
	w.hInstance = GetModuleHandle(nullptr);//�n���h���̎擾

	RegisterClassEx(&w);//�A�v���P�[�V�����N���X�i�E�B���h�E�N���X�̎w���OS�ɓ`����j

	RECT wrc = { 0,0,WINDOW_WIDTH,WINDOW_HEIGHT };//�E�B���h�E�T�C�Y�����߂�

	//�֌W���g���ăE�B���h�E�̃T�C�Y��␳����
	AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);

	//�E�B���h�E�I�u�W�F�N�g�̐���
	HWND hwnd = CreateWindow(w.lpszClassName,
		TEXT("DX12�e�X�g"),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		wrc.right - wrc.left,
		wrc.bottom - wrc.top,
		nullptr,
		nullptr,
		w.hInstance,
		nullptr);

	//�E�B���h�E�\��
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
	//�����N���X�͎g��Ȃ��̂œo�^��������
	UnregisterClass(w.lpszClassName, w.hInstance);
}
