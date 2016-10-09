/*
 * Copyright (c) 2010 Barzakh (martinjk 'at' outlook 'dot' com)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 * claim that you wrote the original software. If you use this software
 * in a product, an acknowledgment in the product documentation would be
 * appreciated but is not required.

 * 2. Altered source versions must be plainly marked as such, and must not be
 * misrepresented as being the original software.

 * 3. This notice may not be removed or altered from any source
 * distribution.

 * 4. THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <windows.h>
#include <stdio.h>
#include <string>
#include <sstream>
#include <vector>
#include <fstream>
#include <thread>
#include <chrono>

#include <M3ScriptHook.h>
#include <Common.h>
#include <LuaFunctions.h>
#include <ScriptSystem.h>
#include <hooking/hooking.h>

M3ScriptHook::M3ScriptHook()
{
	log(__FUNCTION__);
	hooking::hooking_helpers::SetExecutableAddress((uintptr_t)GetModuleHandle(0)); 
	hooking::ExecutableInfo::instance()->EnsureExecutableInfo();
	hooking::ExecutableInfo::instance()->GetExecutableInfo().SetSSEPatternSearching(false);
}

void M3ScriptHook::log(std::string message)
{
	std::fstream file("ScriptHook.log", std::ios::out | std::ios::app);
	file << message;
	file << "\n";
	file.close();
}

void M3ScriptHook::EndThreads()
{
	this->log(__FUNCTION__);
	this->m_bEnded = true;
	PluginSystem::instance()->StopPlugins();
	delete LuaStateManager::instance();
}

void M3ScriptHook::LoadScript(const std::string &file)
{
	this->log(__FUNCTION__);
	auto threadState = LuaStateManager::instance()->GetState();
	this->LoadLuaFile(threadState, file);
}

void M3ScriptHook::LoadLuaFile(lua_State *L, const std::string &name)
{
	this->log(__FUNCTION__);
	std::string file = "function dofile (filename)local f = assert(loadfile(filename)) return f() end dofile(\"";
	file.append(name);
	file.append("\")");
	this->ExecuteLua(L, file);
}

// Export
LUA_API bool ExecuteLua(lua_State *L, const std::string &lua) 
{
	M3ScriptHook::instance()->log(__FUNCTION__);
	return M3ScriptHook::instance()->ExecuteLua(L, lua);
}

bool M3ScriptHook::ExecuteLua(lua_State *L, const std::string &lua)
{
	this->log(std::string("Trying to execute: " + lua).c_str());

	if (!L) {
		this->log("BadState");
		return false;
	}

	luaL_loadbuffer(L, const_cast<char*>(lua.c_str()), lua.length(), "test");

	int32_t result = lua_pcall2(L, 0, LUA_MULTRET, 0);

	if (result != 0)
	{
		if (LUA_ERRSYNTAX == result)
		{
			this->log("Error loading Lua code into buffer with (Syntax Error)");
			return false;
		}
		else if (LUA_ERRMEM == result)
		{
			this->log("Error loading Lua code into buffer with (Memory Allocation Error)");
			return false;
		}
		else
		{
			std::stringstream ss;
			ss << "Error loading Lua code into buffer. Error ";
			ss << result;
			this->log(ss.str());
			size_t size = 0;
			const char *error = lua_tolstring(L, -1, &size);
			this->log(error);
			return false;
		}
	}
	return true;
}


uint32_t WINAPI M3ScriptHook::mainThread(LPVOID) {
	static M3ScriptHook *instance = M3ScriptHook::instance();

	instance->log(__FUNCTION__);

	while (!instance->HasEnded()) {
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
		std::this_thread::yield(); // Process other threads

		if (GetAsyncKeyState(VK_F1) & 1) {
			ScriptSystem::instance()->LoadScripts();
		}

		if (GetAsyncKeyState(VK_F3) & 1) {
			instance->Shutdown();
		}
	}

	return 0;
}

void M3ScriptHook::StartThreads()
{
	this->log(__FUNCTION__);
	CreateThread(0, 0, (LPTHREAD_START_ROUTINE)M3ScriptHook::mainThread, 0, 0, 0);
	//auto thread = std::thread(M3ScriptHook::mainThread);
	//thread.detach();

	LuaStateManager::instance()->StartThread();
}

bool M3ScriptHook::HasEnded()
{
	return this->m_bEnded;
}

void M3ScriptHook::Shutdown()
{
	this->log(__FUNCTION__);
	this->m_bEnded = true;
	this->EndThreads();
	FreeLibraryAndExitThread((HMODULE)GetModuleHandle("M3ScriptHook.dll"), 0);
}

BOOL APIENTRY DllMain(HMODULE, DWORD code, LPVOID) {
	switch (code) {
	case DLL_PROCESS_ATTACH:
		PluginSystem::instance()->LoadPlugins();
		M3ScriptHook::instance()->StartThreads();
		break;
	case DLL_PROCESS_DETACH:
		M3ScriptHook::instance()->EndThreads();
		break;
	}
	return TRUE;
}


