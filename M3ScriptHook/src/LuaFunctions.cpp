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
typedef int(__cdecl *luaL_loadbuffer_t)(lua_State *L, char *buff, size_t size, char *name);
luaL_loadbuffer_t		pluaL_loadbuffer = nullptr;

__declspec(dllexport) int luaL_loadbuffer_(lua_State *L, char *buff, size_t size, char *name)
{
	return pluaL_loadbuffer(L, buff, size, name);
}

/************************************************************************/
/* Lua pcall implementation                                             */
/************************************************************************/
typedef int(__cdecl *lua_pcall_t)(lua_State *L, int32_t nargs, int32_t nresults, int32_t errfunc);
lua_pcall_t				plua_pcall2 = nullptr;

__declspec(dllexport) int lua_pcall_(lua_State *L, int32_t nargs, int32_t nresults, int32_t errfunc)
{
	return plua_pcall2(L, nargs, nresults, errfunc);
}

/************************************************************************/
/* Lua tolstring implementation                                         */
/************************************************************************/
typedef const char *	(__cdecl *lua_tolstring_t) (lua_State *L, int32_t idx);
lua_tolstring_t			plua_tolstring = nullptr;

__declspec(dllexport) const char *lua_tolstring_(lua_State *L, int32_t idx)
{
	return plua_tolstring(L, idx);
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

//
int32_t lua_gettop_(lua_State *L)
{
	return (int32_t)((*(uintptr_t *)((uintptr_t)L + 72) - *(uintptr_t *)((uintptr_t)L + 80)) >> 4);
}

//
__declspec(dllexport) void logPointer(std::string name, uint64_t pointer)
{
	std::stringstream ss;
	ss << name << " (" << std::hex << pointer << ")";
	M3ScriptHook::instance()->log(ss.str());
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
	MessageBoxA(NULL, "works", "works", 0);
	int32_t numargs = lua_gettop_(L);
	M3ScriptHook::instance()->log(std::to_string(numargs));
	if (numargs != 2)
		return 0;

	const char *logFile = lua_tolstring_(L, 1);
	const char *message = lua_tolstring_(L, 2);

	M3ScriptHook::instance()->log(message);
	M3ScriptHook::instance()->log(logFile);

	const char * msg1 = (const char*)*(uintptr_t *)((uintptr_t)L + 72) + 16 * 1;
	M3ScriptHook::instance()->log(msg1);

	std::fstream file(logFile, std::ios::out | std::ios::app);
	file << message;
	file << "\n";
	file.close();

	return 0;
}

//
LuaFunctions::LuaFunctions()
{
	M3ScriptHook::instance()->log(__FUNCTION__);
	// Yep, it's thread blocking, but that's what I want, no processing of other stuff until this shit's ready..
	do {
		M3ScriptHook::instance()->log(__FUNCTION__ " Game is not ready, script engine not initialized, retry");
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

	//
	plua_tolstring = (lua_tolstring_t)GetPointerFromPattern("lua_tolstring", "48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC 40 8B DA 48 8B F9"); // "4C 8B C9 81 FA ? ? ? ? 7E 37"); // 
	if (!plua_tolstring) {
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

	//
	this->m_mainScriptMachineReady = true;
	return m_mainScriptMachineReady;
}

bool LuaFunctions::Setup()
{
#define lua_register_(L,n,f) (lua_pushcfunction_(L, (f)), plua_setfield(L, -10002, (n))) //plua_setglobal(L, (n)))

#define lua_pushcfunction_(L,f)	plua_pushcclosure(L, (f), 0)

	lua_register_(GetL(), "PrintToLog", LuaFunctions::PrintToLog);
	return true;
}
