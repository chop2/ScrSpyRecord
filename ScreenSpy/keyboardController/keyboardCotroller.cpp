#include "keyboardCotroller.h"
#include <WinUser.h>
#include <atlconv.h>

#pragma comment(lib,"User32.lib")  
#pragma comment(lib,"shlwapi.lib") 

CRITICAL_SECTION cs;

KeyboardController* KeyboardController::m_pInstance = NULL;
void sendAscii(wchar_t data, BOOL shift);
void sendUnicode(wchar_t data);
void sendKeys(char *strMsg);

KeyboardController::KeyboardController()
{
}

KeyboardController::~KeyboardController()
{
}

KeyboardController::KeyboardController(const KeyboardController&)
{
}

KeyboardController& KeyboardController::operator = (const KeyboardController& controller)
{
	*this = controller;
	return *this;
}

KeyboardController* KeyboardController::GetInstance()
{
	::InitializeCriticalSection(&cs);
	if (m_pInstance == NULL)
	{
		::EnterCriticalSection(&cs);
		m_pInstance = new KeyboardController();
		::LeaveCriticalSection(&cs);
	}
	return m_pInstance;
}


void KeyboardController::SendKey(const string& key, bool shift)
{
	wstring wstr;
	USES_CONVERSION;
	wstr = A2W(key.c_str());
	sendAscii(wstr[0], shift);
}

void KeyboardController::SendKeys(const vector<string>& keys)
{
	for (size_t i = 0; i < keys.size(); i++)
	{
		SendKey(keys[i], false);
	}
}

void KeyboardController::SendStr(const string& key)
{
	sendKeys((char*)key.c_str());
}

void KeyboardController::SendKey(HWND hWnd, char key)
{
	keybd_event(VK_CONTROL, 0, 0, 0);
	PostMessage(hWnd, WM_KEYDOWN, key, 0);
	PostMessage(hWnd, WM_KEYUP, key, 0);
	keybd_event(VK_CONTROL, 0, KEYEVENTF_KEYUP, 0);
	CloseHandle(hWnd);
}

void KeyboardController::LeftClick()
{
	INPUT Input = { 0 };
	// 左键按下
	Input.type = INPUT_MOUSE;
	Input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
	::SendInput(1, &Input, sizeof(INPUT));
	// 左键抬起
	::ZeroMemory(&Input, sizeof(INPUT));
	Input.type = INPUT_MOUSE;
	Input.mi.dwFlags = MOUSEEVENTF_LEFTUP;
	::SendInput(1, &Input, sizeof(INPUT));
}

void KeyboardController::RightClick()
{
	INPUT Input = { 0 };
	// 右键按下
	Input.type = INPUT_MOUSE;
	Input.mi.dwFlags = MOUSEEVENTF_RIGHTDOWN;
	::SendInput(1, &Input, sizeof(INPUT));
	// 右键抬起
	::ZeroMemory(&Input, sizeof(INPUT));
	Input.type = INPUT_MOUSE;
	Input.mi.dwFlags = MOUSEEVENTF_RIGHTUP;
	::SendInput(1, &Input, sizeof(INPUT));
}

void KeyboardController::MouseMove(int x, int y)
{
	double fScreenWidth = ::GetSystemMetrics(SM_CXSCREEN) - 1;
	double fScreenHeight = ::GetSystemMetrics(SM_CYSCREEN) - 1;
	double fx = x*(65535.0f / fScreenWidth);
	double fy = y*(65535.0f / fScreenHeight);
	INPUT  Input = { 0 };
	Input.type = INPUT_MOUSE;
	Input.mi.dwFlags = MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE;
	Input.mi.dx = fx;
	Input.mi.dy = fy;
	::SendInput(1, &Input, sizeof(INPUT));
}

void KeyboardController::MiddleClick(int mouseData)
{
	INPUT Input = { 0 };
	// 设置滚轮量
	Input.type = INPUT_MOUSE;
	Input.mi.dwFlags = MOUSEEVENTF_WHEEL;
	Input.mi.mouseData = mouseData;
	::SendInput(1, &Input, sizeof(INPUT));
}

void sendAscii(wchar_t data, BOOL shift)
{
	INPUT input[2];
	memset(input, 0, 2 * sizeof(INPUT));

	if (shift)
	{
		input[0].type = INPUT_KEYBOARD;
		input[0].ki.wVk = VK_SHIFT;
		SendInput(1, input, sizeof(INPUT));
	}

	input[0].type = INPUT_KEYBOARD;
	input[0].ki.wVk = data;

	input[1].type = INPUT_KEYBOARD;
	input[1].ki.wVk = data;
	input[1].ki.dwFlags = KEYEVENTF_KEYUP;

	SendInput(2, input, sizeof(INPUT));

	if (shift)
	{
		input[0].type = INPUT_KEYBOARD;
		input[0].ki.wVk = VK_SHIFT;
		input[0].ki.dwFlags = KEYEVENTF_KEYUP;
		SendInput(1, input, sizeof(INPUT));
	}
}

void sendUnicode(wchar_t data)
{
	INPUT input[2];
	memset(input, 0, 2 * sizeof(INPUT));

	input[0].type = INPUT_KEYBOARD;
	input[0].ki.wVk = 0;
	input[0].ki.wScan = data;
	input[0].ki.dwFlags = 0x4;//KEYEVENTF_UNICODE;  

	input[1].type = INPUT_KEYBOARD;
	input[1].ki.wVk = 0;
	input[1].ki.wScan = data;
	input[1].ki.dwFlags = KEYEVENTF_KEYUP | 0x4;//KEYEVENTF_UNICODE;  

	SendInput(2, input, sizeof(INPUT));
}

//为方便使用，下面这个函数包装了前两个函数。  
void sendKeys(char *strMsg)
{
	short vk;
	BOOL shift;

	USES_CONVERSION;
	wchar_t* data = A2W(strMsg);
	int len = wcslen(data);

	for (int i = 0; i<len; i++)
	{
		if (data[i] >= 0 && data[i]<256) //ascii字符  
		{
			vk = VkKeyScanW(data[i]);

			if (vk == -1)
			{
				sendUnicode(data[i]);
			}
			else
			{
				if (vk < 0)
				{
					vk = ~vk + 0x1;
				}

				shift = vk >> 8 & 0x1;

				if (GetKeyState(VK_CAPITAL) & 0x1)
				{
					if (data[i] >= 'a' && data[i] <= 'z' || data[i] >= 'A' && data[i] <= 'Z')
					{
						shift = !shift;
					}
				}

				sendAscii(vk & 0xFF, shift);
			}
		}
		else //unicode字符  
		{
			sendUnicode(data[i]);
		}
	}
}