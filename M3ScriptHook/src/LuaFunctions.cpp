// Base Application taken from Klusark (GPLv2)
// https://code.google.com/archive/p/mafia2injector/

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

#include <Export.h>
#include <Common.h>
#include <sstream>
#include <LuaFunctions.h>
#include <M3ScriptHook.h>
#include <cstdint>
#include <hooking/hooking.h>
#include <fstream>
#include <thread>

 /************************************************************************/
 /* Find pattern implementation											*/
 /************************************************************************/
uint64_t GetPointerFromPattern(const char *name, const char *pattern)
{
	auto pat = hooking::pattern(pattern).get(0).origaddr();
	logPointer(name, pat);
	return pat;
}

/************************************************************************/
/* Lua Load Buffer impl													*/
/************************************************************************/
typedef int32_t(__cdecl *luaL_loadbuffer_t)(lua_State *L, char *buff, size_t size, char *name);
luaL_loadbuffer_t		pluaL_loadbuffer = nullptr;

__declspec(dllexport) int32_t luaL_loadbuffer_(lua_State *L, char *buff, size_t size, char *name)
{
	return pluaL_loadbuffer(L, buff, size, name);
}

/************************************************************************/
/* Lua pcall implementation                                             */
/************************************************************************/
typedef int32_t(__cdecl *lua_pcall_t)(lua_State *L, int32_t nargs, int32_t nresults, int32_t errfunc);
lua_pcall_t				plua_pcall2 = nullptr;

__declspec(dllexport) int32_t lua_pcall_(lua_State *L, int32_t nargs, int32_t nresults, int32_t errfunc)
{
	return plua_pcall2(L, nargs, nresults, errfunc);
}

/************************************************************************/
/* Lua tolstring implementation                                         */
/************************************************************************/
typedef const char *	(__cdecl *lua_tostring_t) (lua_State *L, int32_t idx);
lua_tostring_t			plua_tostring = nullptr;

__declspec(dllexport) const char *lua_tostring_(lua_State *L, int32_t idx)
{
	return plua_tostring(L, idx);
}

/************************************************************************/
/* Lua isstring implementation                                         */
/************************************************************************/
typedef uint32_t(__cdecl *lua_isstring_t) (lua_State *L, int32_t idx);
lua_isstring_t			plua_isstring = nullptr;

__declspec(dllexport) uint32_t lua_isstring_(lua_State *L, int32_t idx)
{
	return plua_isstring(L, idx);
}

/************************************************************************/
/* Lua newthread implementation                                         */
/************************************************************************/
typedef	lua_State *		(__cdecl *lua_newthread_t) (lua_State *L);
lua_newthread_t		plua_newthread = nullptr;

__declspec(dllexport) lua_State *lua_newthread_(lua_State *L)
{
	return plua_newthread(L);
}

/************************************************************************/
/* Lua pushcclosure implementation                                      */
/************************************************************************/
typedef	lua_State *		(__cdecl *lua_pushcclosure_t) (lua_State *L, lua_CFunction fn, int n);
lua_pushcclosure_t		plua_pushcclosure = nullptr;

__declspec(dllexport) lua_State *lua_pushcclosure_(lua_State *L, lua_CFunction fn, int n)
{
	return plua_pushcclosure(L, fn, n);
}

/************************************************************************/
/* Lua setglobal implementation		                                    */
/************************************************************************/
typedef	lua_State *		(__cdecl *lua_setglobal_t) (lua_State *L, const char *var);
lua_setglobal_t		  plua_setglobal = nullptr;

__declspec(dllexport) lua_State *lua_setglobal_(lua_State *L, const char *var)
{
	return plua_setglobal(L, var);
}

/************************************************************************/
/* Lua setfield implementation		                                    */
/************************************************************************/
typedef	lua_State *		(__cdecl *lua_setfield_t) (lua_State *L, int idx, const char *k);
lua_setfield_t		  plua_setfield = nullptr;

__declspec(dllexport) lua_State *lua_setglobal_(lua_State *L, int idx, const char *k)
{
	return plua_setfield(L, idx, k);
}

/************************************************************************/
/* (Havok) Lua gettop implementation			                        */
/************************************************************************/
int32_t lua_gettop_(lua_State *L)
{
	return (int32_t)((*(uintptr_t *)((uintptr_t)L + 72) - *(uintptr_t *)((uintptr_t)L + 80)) >> 4);
}

//
__declspec(dllexport) void logPointer(std::string name, uint64_t pointer)
{
	std::stringstream ss;
	ss << name << " (" << std::hex << pointer << ")";
	M3ScriptHook::instance()->Log(ss.str().c_str());
}

//
lua_State* GetL(C_ScriptGameMachine *pMainScriptMachine)
{
	if (!pMainScriptMachine) {
		pMainScriptMachine = LuaFunctions::instance()->GetMainGameScriptMachine();
	}

	auto luaPtr = *(lua_State **)((uintptr_t)pMainScriptMachine + 208);
	return luaPtr;
}

int32_t LuaFunctions::PrintToLog(lua_State *L)
{
	//static auto pat = hooking::pattern("40 53 48 83 EC 20 48 8B 41 48 48 8B D9 48 2B 41 50 48 C1 F8 04 FF C8").get(0).origaddr();
	//static auto checkStruct = pat;// + *(int32_t *)(pat + 1) + 5;

	//static auto pat2 = hooking::pattern("E8 ? ? ? ? 85 C0 74 5E 8B D3").get(0).origaddr();
	//static auto isString = pat2 + *(int32_t *)(pat2 + 1) + 5;

	//static auto pat3 = hooking::pattern("4C 8B C9 81 FA ? ? ? ? 7E 37").get(0).origaddr();
	//static auto toString = pat3; //+ *(int32_t *)(pat3 + 1) + 5;

	const char *logFile;
	const char *message;

	//auto context = hooking::func_call<uintptr_t, lua_State*, int32_t, uintptr_t, uintptr_t>(checkStruct, L, 2, 2, 0);
	//M3ScriptHook::instance()->log("0x%p", (uintptr_t)context);

	if (plua_isstring(L, 1))
	{
		logFile = plua_tostring(L, 1);
	}

	if (plua_isstring(L, 2))
	{
		message = plua_tostring(L, 2);
	}

	M3ScriptHook::instance()->LogToFile(logFile, message);
	return 0;
}

int32_t LuaFunctions::BindKey(lua_State *L)
{
	M3ScriptHook::instance()->Log(__FUNCTION__);
	const char *key = "";
	const char *context = "";

	if (plua_isstring(L, 1))
	{
		key = plua_tostring(L, 1);
	}

	if (plua_isstring(L, 2))
	{
		context = plua_tostring(L, 2);
	}

	M3ScriptHook::instance()->CreateKeyBind(key, context);
	return 0;
}

int32_t LuaFunctions::UnbindKey(lua_State *L)
{
	M3ScriptHook::instance()->Log(__FUNCTION__);
	const char *key = "";
	const char *context = "";

	if (plua_isstring(L, 1))
	{
		key = plua_tostring(L, 1);
	}

	if (plua_isstring(L, 2))
	{
		context = plua_tostring(L, 2);
	}

	M3ScriptHook::instance()->DestroyKeyBind(key, context);
	return 0;
}

int32_t LuaFunctions::DelayBuffer(lua_State *L)
{
	M3ScriptHook::instance()->Log(__FUNCTION__);
	const char *time = "";
	const char *context = "";

	if (plua_isstring(L, 1))
	{
		time = plua_tostring(L, 1);
	}

	if (plua_isstring(L, 2))
	{
		context = plua_tostring(L, 2);
	}

	// Pretty hacky implementation, we should consider using a Job queue instead and checking time?
	std::thread th = std::thread([L, time, context]() {
		M3ScriptHook::instance()->Log(__FUNCTION__);
		auto mtime = std::stoi(time);
		std::this_thread::sleep_for(std::chrono::milliseconds(mtime));
		M3ScriptHook::instance()->ExecuteLua(L, context);
	});
	th.detach();

	return 0;
}

//
LuaFunctions::LuaFunctions()
{
	M3ScriptHook::instance()->Log(__FUNCTION__);
	// Yep, it's thread blocking, but that's what I want, no processing of other stuff until this shit's ready..
	do {
		M3ScriptHook::instance()->Log(__FUNCTION__ " Game is not ready, script engine not initialized, retry");
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
		std::this_thread::yield();
	} while (!this->LoadPointers());
}

C_ScriptGameMachine *LuaFunctions::GetMainGameScriptMachine()
{
	return this->m_pMainGameScriptMachine;
}

bool LuaFunctions::IsMainScriptMachineReady()
{
	return this->m_mainScriptMachineReady;
}

bool LuaFunctions::LoadPointers()
{
	M3ScriptHook::instance()->Log(__FUNCTION__);
	uint64_t engineAssignAddress = hooking::pattern("48 89 05 ? ? ? ? 48 8B 10 FF 92 ? ? ? ?").get(0).origaddr();
	uint64_t engine = engineAssignAddress + *(int32_t *)(engineAssignAddress + 3) + 7;
	logPointer("m_pMainGameScriptMachine", engine);
	this->m_pMainGameScriptMachine = *(C_ScriptGameMachine **)engine;

	if (!this->m_pMainGameScriptMachine) {
		return this->m_mainScriptMachineReady;
	}

	//
	plua_pcall2 = (lua_pcall_t)GetPointerFromPattern("lua_pcall2", "48 89 5C 24 ? 55 56 57 41 54 41 55 41 56 41 57 48 83 EC 60 4C 63 FA");
	if (!plua_pcall2) {
		return this->m_mainScriptMachineReady;
	}

	//static auto pat = hooking::pattern("40 53 48 83 EC 20 48 8B 41 48 48 8B D9 48 2B 41 50 48 C1 F8 04 FF C8").get(0).origaddr();
	//static auto checkStruct = pat;// + *(int32_t *)(pat + 1) + 5;

	//
	plua_tostring = (lua_tostring_t)GetPointerFromPattern("lua_tostring", "4C 8B C9 81 FA ? ? ? ? 7E 37");
	if (!plua_tostring) {
		return this->m_mainScriptMachineReady;
	}

	//
	auto isStringAddr = GetPointerFromPattern("lua_isstring", "E8 ? ? ? ? 85 C0 74 5E 8B D3");
	auto isString = isStringAddr + *(int32_t *)(isStringAddr + 1) + 5;
	logPointer("lua_isstring", isStringAddr);
	plua_isstring = (lua_isstring_t)isString;
	if (!plua_isstring) {
		return this->m_mainScriptMachineReady;
	}

	//
	auto loadBufferAddr = GetPointerFromPattern("luaL_loadbuffer", "E8 ? ? ? ? 8B F8 85 FF 74 17");
	auto loadBuffer = loadBufferAddr + *(int32_t *)(loadBufferAddr + 1) + 5;
	logPointer("plua_loadBuffer", loadBuffer);
	pluaL_loadbuffer = (luaL_loadbuffer_t)loadBuffer;
	if (!pluaL_loadbuffer) {
		return this->m_mainScriptMachineReady;
	}

	//
	plua_newthread = (lua_newthread_t)GetPointerFromPattern("lua_newthread", "48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 41 56 41 57 48 83 EC 40 48 8B 79 10 48 8B F1");
	if (!plua_newthread) {
		return this->m_mainScriptMachineReady;
	}

	//
	plua_pushcclosure = (lua_pushcclosure_t)GetPointerFromPattern("lua_pushcclosure", "48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 48 89 7C 24 ? 41 54 41 56 41 57 48 83 EC 40 49 63 F0");
	if (!plua_pushcclosure) {
		return this->m_mainScriptMachineReady;
	}

	//
	/*plua_setglobal = (lua_setglobal_t)GetPointerFromPattern("lua_setglobal", "");
	if (!plua_setglobal) {
		return this->m_mainScriptMachineReady;
	}*/

	//
	plua_setfield = (lua_setfield_t)GetPointerFromPattern("lua_setfield", "48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 48 83 EC 20 48 8B 41 48 49 8B F8 ");
	if (!plua_setfield) {
		return this->m_mainScriptMachineReady;
	}

	/*
	static auto addr = hooking::pattern("F3 0F 10 0D ? ? ? ? 4C 8D 44 24 ? F3 48 0F 2C D0").get(0).origaddr();
	static auto inc = hooking::inject_call<uintptr_t, lua_State*, int32_t>((addr + 0x62));
	inc.inject([](lua_State *L, int32_t a2) {
		const char *test;
		if (plua_isstring(L, 1))
		{
			test = plua_tostring(L, 1);
		}

		M3ScriptHook::instance()->Log("Entity: %s", test);

		return inc.call(L, a2);
	});*/

	//
	this->m_mainScriptMachineReady = true;
	return m_mainScriptMachineReady;
}

bool LuaFunctions::Setup()
{
	M3ScriptHook::instance()->Log(__FUNCTION__);
#define lua_register_(L,n,f) (lua_pushcfunction_(L, (f)), plua_setfield(L, -10002, (n))) //plua_setglobal(L, (n)))

#define lua_pushcfunction_(L,f)	plua_pushcclosure(L, (f), 0)

	auto L = GetL();
	lua_register_(L, "printToLog", LuaFunctions::PrintToLog);

	auto tmp = [L]() {
		M3ScriptHook::instance()->Log(__FUNCTION__);
		lua_register_(L, "bindKey", LuaFunctions::BindKey);
	};
	tmp();

	auto tmp2 = [L]() {
		M3ScriptHook::instance()->Log(__FUNCTION__);
		lua_register_(L, "unbindKey", LuaFunctions::UnbindKey);
	};
	tmp2();

	auto tmp3 = [L]() {
		M3ScriptHook::instance()->Log(__FUNCTION__);
		lua_register_(L, "setTimeout", LuaFunctions::DelayBuffer);
	};
	tmp3();
	return true;
}