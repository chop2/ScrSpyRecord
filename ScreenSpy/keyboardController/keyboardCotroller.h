#pragma once
#include <vector>
#include <string>
#include <Windows.h>
using namespace std;

class KeyboardController
{
public:
	static KeyboardController* GetInstance();
	void SendKey(const string& key, bool shift = false);
	void SendKeys(const vector<string>& keys);
	void SendStr(const string& key);

	//press key by PostMessage
	void SendKey(HWND hWnd, char key);

	void LeftClick();
	void RightClick();
	void MouseMove(int x, int y);
	void MiddleClick(int mouseData = 500);
private:
	KeyboardController();
	KeyboardController(const KeyboardController&);
	KeyboardController& operator=(const KeyboardController&);
	~KeyboardController();
	static KeyboardController* m_pInstance;
};