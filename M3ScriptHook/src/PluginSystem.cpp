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

#include <PluginSystem.h>
#include <Windows.h>
#include <string>
#include <Common.h>
#include <M3ScriptHook.h>

void PluginSystem::LoadPlugins()
{
	M3ScriptHook::instance()->log(__FUNCTION__);

	WIN32_FIND_DATA data;
	HANDLE file = FindFirstFileEx("plugins\\*.dll", FindExInfoStandard, &data, FindExSearchNameMatch, 0, 0);

	if (file >= (HANDLE)0xFFFFFFFFFFFFFFFF) {
		return;
	}

	do {
		std::string path = "plugins\\";
		path += data.cFileName;

		HMODULE lib = LoadLibraryA(path.c_str());
		if (!lib) {
			M3ScriptHook::instance()->log(__FUNCTION__ " failed to load plugin (LoadLibrary) " + path);
			continue;
		}

		StartPlugin_t pStartPlugin = (StartPlugin_t)GetProcAddress(lib, "StartPlugin");
		if (!pStartPlugin) {
			M3ScriptHook::instance()->log(__FUNCTION__ " failed to find start routine in plugin " + path);
			continue;
		}

		StopPlugin_t pStopPlugin = (StopPlugin_t)GetProcAddress(lib, "StopPlugin");
		if (!pStopPlugin) {
			M3ScriptHook::instance()->log(__FUNCTION__ " failed to find stop routine in plugin " + path);
			continue;
		}

		Plugin plugin;
		plugin.name = data.cFileName;
		plugin.pStartPlugin = pStartPlugin;
		plugin.pStopPlugin = pStopPlugin;
		plugins.push_back(plugin);

		M3ScriptHook::instance()->log(__FUNCTION__ " loaded plugin " + path);

	} while (file && FindNextFile(file, &data));
}

void PluginSystem::UnloadPlugins()
{
	this->StopPlugins();

	for (auto& plugin : this->plugins) {
		FreeLibrary(GetModuleHandleA(plugin.name.c_str()));
	}

	this->plugins.clear();
}

void PluginSystem::ReloadPlugins()
{
	if (this->plugins.size()) {
		this->UnloadPlugins();
	}

	this->LoadPlugins();
}


void PluginSystem::StartPlugins()
{
	M3ScriptHook::instance()->log(__FUNCTION__);
	for (auto& plugin : this->plugins) {
		plugin.pStartPlugin(LuaStateManager::instance()->GetState());
	}
}


void PluginSystem::StopPlugins()
{
	M3ScriptHook::instance()->log(__FUNCTION__);
	for (auto& plugin : this->plugins) {
		plugin.pStopPlugin();
	}
}
