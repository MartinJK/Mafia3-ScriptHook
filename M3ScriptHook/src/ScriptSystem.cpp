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

#include <ScriptSystem.h>
#include <Windows.h>
#include <string>
#include <Common.h>
#include <M3ScriptHook.h>

void ScriptSystem::LoadScripts()
{
	//scripts.clear(); // only add this when we unload scripts..

	M3ScriptHook::instance()->log(__FUNCTION__);

	WIN32_FIND_DATA data;
	HANDLE file = FindFirstFileEx("scripts\\*.lua", FindExInfoStandard, &data, FindExSearchNameMatch, 0, 0);

	if (file >= (HANDLE)0xFFFFFFFFFFFFFFFF) {
		return;
	}

	do {
		std::string path = "scripts/";
		path += data.cFileName;

		scripts.push_back(path);

		M3ScriptHook::instance()->LoadScript(path);

		M3ScriptHook::instance()->log(__FUNCTION__ " loaded script " + path);
	} while (file && FindNextFile(file, &data));
}

void ScriptSystem::UnloadScripts()
{
	// TBD
}

void ScriptSystem::ReloadScripts()
{
	if (this->scripts.size()) {
		this->UnloadScripts();
	}

	this->LoadScripts();
}
