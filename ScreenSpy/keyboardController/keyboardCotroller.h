#pragma once
#include <vector>
#include <string>
using namespace std;

class KeyboardController
{
public:
	static KeyboardController* GetInstance();
	void SendKey(const string& key,bool shift = false);
	void SendKeys(const vector<string>& keys);

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