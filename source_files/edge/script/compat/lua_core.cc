#include "i_defs.h"

#include "file.h"
#include "filesystem.h"
#include "path.h"

#include "main.h"

#include "vm_coal.h"
#include "dm_state.h"
#include "e_main.h"
#include "g_game.h"
#include "version.h"

#include "e_player.h"
#include "hu_font.h"
#include "hu_draw.h"
#include "r_modes.h"
#include "w_wad.h"

#include "m_random.h"

#include "lua_compat.h"

//------------------------------------------------------------------------
//  SYSTEM MODULE
//------------------------------------------------------------------------

// sys.error(str)
//
static int SYS_error(lua_State *L)
{
    const char *s = luaL_checkstring(L, 1);
    I_Error("%s\n", s);
    return 0;
}

// sys.print(str)
//
static int SYS_print(lua_State *L)
{
    const char *s = luaL_checkstring(L, 1);
    I_Printf("%s\n", s);
    return 0;
}

// sys.debug_print(str)
//
static int SYS_debug_print(lua_State *L)
{
    const char *s = luaL_checkstring(L, 1);
    I_Debugf("%s\n", s);
    return 0;
}

// sys.edge_version()
//
static int SYS_edge_version(lua_State *L)
{
    lua_pushnumber(L, edgeversion.f);
    return 1;
}

#ifdef WIN32
static bool console_allocated = false;
#endif
static int SYS_AllocConsole(lua_State *L)
{
#ifdef WIN32
    if (console_allocated)
    {
        return 0;
    }

    console_allocated = true;
    AllocConsole();
    freopen("CONIN$", "r", stdin);
    freopen("CONOUT$", "w", stdout);
    freopen("CONOUT$", "w", stderr);
#endif

    return 0;
}

//------------------------------------------------------------------------
//  MATH EXTENSIONS
//------------------------------------------------------------------------

// math.rint(val)
static int MATH_rint(lua_State* L)
{
    double val = luaL_checknumber(L, 1);
    lua_pushinteger(L, I_ROUND(val));
    return 1;
}

// SYSTEM
static const luaL_Reg syslib[] = {{"error", SYS_error},
                                  {"print", SYS_print},
                                  {"debug_print", SYS_debug_print},
                                  {"edge_version", SYS_edge_version},
                                  {"allocate_console", SYS_AllocConsole},
                                  {NULL, NULL}};

static int luaopen_sys(lua_State *L)
{
    luaL_newlib(L, syslib);
    return 1;
}

static void RegisterGlobal(lua_State *L, const char *name, const char *module_name)
{
    int top = lua_gettop(L);
    int ret = luaL_loadstring(L, epi::STR_Format("return require \"%s\"", module_name).c_str());
    if (ret != LUA_OK)
    {        
        I_Warning("LUA: RegisterGlobal: %s", lua_tostring(L, -1));
        lua_settop(L, top);
        return;
    }
    
    ret = lua_pcall(L, 0, LUA_MULTRET, 0);
    if (ret != LUA_OK)
    {
        I_Warning("LUA: RegisterGlobal: %s", lua_tostring(L, -1));
        lua_settop(L, top);
        return;
    }    
    // pop loader data from
    lua_pop(L, 1);
    lua_setglobal(L, name);
    SYS_ASSERT(top == lua_gettop(L));
}

const luaL_Reg loadlibs[] = {{"sys", luaopen_sys}, {NULL, NULL}};

void LUA_RegisterCoreLibraries(lua_State *L)
{
    const luaL_Reg *lib;
    /* "require" functions from 'loadedlibs' and set results to global table */
    for (lib = loadlibs; lib->func; lib++)
    {
        luaL_requiref(L, lib->name, lib->func, 1);
        lua_pop(L, 1); /* remove lib */
    }

    // globally available core classes
    RegisterGlobal(L, "vec2", "core.vec2");
    RegisterGlobal(L, "vec3", "core.vec3");

    // add rint to math
    lua_getglobal(L, "math");
    lua_pushcfunction(L, MATH_rint);
    lua_setfield(L, -2, "rint");
    lua_pop(L, 1);
}