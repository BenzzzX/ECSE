#pragma once
#include "ECS\EEC.hpp"
#include <Windows.h>

struct KeyEvent {
	char key;
	bool isDown;
};


namespace Util
{
	bool is_key_down(char key) {
		return static_cast<bool>(GetKeyState(key) & (1 << 8));
	}
}

namespace Systems {

	class InputSystem {
	std::thread thread;

	public:
		template<typename World>
		InputSystem(World& world) {
			thread = std::thread([&]() {
				HANDLE handle = GetStdHandle(STD_INPUT_HANDLE);
				INPUT_RECORD records[128];
				DWORD nReads;
				while (1) {
					ReadConsoleInput(handle, records, 128, &nReads);
					for (size_t i = 0; i < nReads; i++) {
						if (records[i].EventType == KEY_EVENT) {
							auto Event = records[i].Event.KeyEvent;
							world.broadcast(
								KeyEvent{ Event.uChar.AsciiChar, (bool)Event.bKeyDown });
						}
					}
				}
			});
		}
		~InputSystem() { thread.join(); }
	};
}