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
#pragma once

#include <Export.h>
#include <PluginSystem.h>
#include <LuaStateManager.h>
#include <singleton.h>
#include <lua.h>

#define SCRIPTMACHINE_MAX 12

class M3ScriptHook : public singleton<M3ScriptHook>
{
private:
	bool m_bEnded = false;

public:
	M3ScriptHook();
	virtual ~M3ScriptHook() = default;

	void log(std::string message);
	void EndThreads();
	void LoadScript(const std::string &file);
	void LoadLuaFile(lua_State *L, const std::string &name);
	bool ExecuteLua(lua_State *L, const std::string &lua);
	static uint32_t WINAPI mainThread(LPVOID);
	void StartThreads();
	bool HasEnded();
	void Shutdown();
};

class C_ScriptGameMachineWrapper // Total size: 24 bytes
{
public:
	uint64_t    VTABLE;         // 00-08
	uint64_t    unkpointer;     // 08-16
	uint8_t     unkbyte;        // 16-17
	uint8_t		gap0[7];

	virtual ~C_ScriptGameMachineWrapper();

	virtual void Function001();
	virtual void Function002();
	virtual void Function003(); // nullsub
	virtual void Function004();
	virtual void Function005();
	virtual void Function006();
};

class C_ScriptMachine // Total size: 412 bytes
{
public:
	uint64_t qword0; // VTABLE
	uint64_t qword8;
	uint64_t qword10;
	uint64_t qword18;
	uint32_t qword20;
	uint64_t qword28;
	uint64_t qword30;
	uint8_t gap0[0x28]; // seems to be a std::vector
	uint64_t qword60;
	uint64_t qword68;
	uint64_t qword70;
	uint64_t qword78;
	uint64_t qword80;
	uint64_t qword88;
	uint64_t qword90;
	uint64_t qword98;
	uint64_t qwordA0;
	uint64_t qwordA8;
	uint64_t qwordB0;
	uint64_t qwordB8;
	uint8_t gapC0[16];
	lua_State* qwordD0; // LUA STATE
	uint64_t qwordD8;
	uint64_t qwordE0;
	uint8_t byteE8;
	uint8_t gapE9[15];
	uint64_t qwordF8;
	uint64_t qword100;
	uint8_t gap108[16];
	uint32_t uint32_t118;
	void *pvoid120;
	uint64_t qword128;
	uint64_t qword130;
	uint32_t uint32_t138;
	uint64_t qword140;
	uint64_t qword148;
	uint64_t qword150;
	uint32_t uint32_t158;
	uint32_t uint32_t15C;
	uint32_t uint32_t160;
	uint8_t gap164[20];
	uint32_t uint32_t178;
	uint64_t qword180;
	uint64_t qword188;
	uint64_t qword190;
	uint64_t qword198;

	virtual ~C_ScriptMachine();

	virtual void Function0001();
	virtual void Function0002();
	virtual void Function0003();
	virtual void Function0004();
	virtual void Function0005();
	virtual void Function0006();
	virtual void Function0007();
	virtual void Function0008();
	virtual void Function0009();
	virtual void Function0010();
	virtual void Function0011();
	virtual void Function0012();
	virtual void Function0013();
	virtual void Function0014();
	virtual void Function0015();
	virtual void Function0016();
	virtual void Function0017();
	virtual void Function0018();
	virtual void Function0019();  // nullsub
	virtual void Function0020();
	virtual void Function0021();
};

class C_ScriptGameMachine : public C_ScriptMachine // size: (start at offset 416, end at 560)
{
public:
	uint64_t qword1A0; // ptr to 40(0x28) size big object, as down in C_ScriptMachine, seems to be a vector
	uint64_t qword1A8;
	uint64_t qword1B0;
	uint64_t qword1B8;
	uint64_t qword1C0;
	uint64_t qword1C8;
	uint64_t qword1D0;
	uint64_t qword1D8;
	uint64_t qword1E0;
	uint64_t qword1E8;
	uint64_t qword1F0;
	uint64_t qword1F8;
	uint64_t qword200;
	uint64_t qword208;
	uint8_t byte210;
	uint64_t qword218;
	uint64_t qword220;
	uint64_t qword228;

	virtual ~C_ScriptGameMachine();

	virtual void Function0001();
	virtual void Function0002();
	virtual void Function0003();
	virtual void Function0004();
	virtual void Function0005();
	virtual void Function0006();
	virtual void Function0007();
	virtual void Function0008();
	virtual void Function0009();
	virtual void Function0010();
	virtual void Function0011();
	virtual void Function0012();
	virtual void Function0013();
	virtual void Function0014();
	virtual void Function0015();
	virtual void Function0016();
	virtual void Function0017();
	virtual void Function0018();
	virtual void Function0019(); // nullsub
	virtual void Function0020();
	virtual void Function0021();
};

lua_State *GetL(C_ScriptGameMachine *pEngine = nullptr);