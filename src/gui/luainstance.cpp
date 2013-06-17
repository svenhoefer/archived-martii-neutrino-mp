/*
 * neutrino-mp lua to c++ bridge
 *
 * (C) 2013 Stefan Seyfried <seife@tuxboxcvs.slipkontur.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <config.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>

#include <global.h>
#include <system/settings.h>
#include <gui/widget/msgbox.h>
#include <gui/widget/messagebox.h>
#ifdef MARTII
#include <gui/filebrowser.h>
#include <driver/pictureviewer/pictureviewer.h>
#endif
#include <neutrino.h>

#include "luainstance.h"

/* the magic color that tells us we are using one of the palette colors */
#define MAGIC_COLOR 0x42424200
#define MAGIC_MASK  0xFFFFFF00

struct table_key {
	const char *name;
	long code;
};

struct lua_envexport {
	const char *name;
	table_key *t;
};

static void set_lua_variables(lua_State *L)
{
	/* keyname table created with
	 * sed -n '/^[[:space:]]*RC_0/,/^[[:space:]]*RC_analog_off/ {
	 * 	s#^[[:space:]]*RC_\([^[:space:]]*\).*#	{ "\1",	CRCInput::RC_\1 },#p
	 * }' driver/rcinput.h
	 */
	static table_key keyname[] =
	{
		{ "0",			CRCInput::RC_0 },
		{ "1",			CRCInput::RC_1 },
		{ "2",			CRCInput::RC_2 },
		{ "3",			CRCInput::RC_3 },
		{ "4",			CRCInput::RC_4 },
		{ "5",			CRCInput::RC_5 },
		{ "6",			CRCInput::RC_6 },
		{ "7",			CRCInput::RC_7 },
		{ "8",			CRCInput::RC_8 },
		{ "9",			CRCInput::RC_9 },
		{ "backspace",		CRCInput::RC_backspace },
		{ "up",			CRCInput::RC_up },
		{ "left",		CRCInput::RC_left },
		{ "right",		CRCInput::RC_right },
		{ "down",		CRCInput::RC_down },
		{ "spkr",		CRCInput::RC_spkr },
		{ "minus",		CRCInput::RC_minus },
		{ "plus",		CRCInput::RC_plus },
		{ "standby",		CRCInput::RC_standby },
		{ "help",		CRCInput::RC_help },
		{ "home",		CRCInput::RC_home },
		{ "setup",		CRCInput::RC_setup },
		{ "topleft",		CRCInput::RC_topleft },
		{ "topright",		CRCInput::RC_topright },
		{ "page_up",		CRCInput::RC_page_up },
		{ "page_down",		CRCInput::RC_page_down },
		{ "ok",			CRCInput::RC_ok },
		{ "red",		CRCInput::RC_red },
		{ "green",		CRCInput::RC_green },
		{ "yellow",		CRCInput::RC_yellow },
		{ "blue",		CRCInput::RC_blue },
		{ "top_left",		CRCInput::RC_top_left },
		{ "top_right",		CRCInput::RC_top_right },
		{ "bottom_left",	CRCInput::RC_bottom_left },
		{ "bottom_right",	CRCInput::RC_bottom_right },
		{ "audio",		CRCInput::RC_audio },
		{ "video",		CRCInput::RC_video },
		{ "tv",			CRCInput::RC_tv },
		{ "radio",		CRCInput::RC_radio },
		{ "text",		CRCInput::RC_text },
		{ "info",		CRCInput::RC_info },
		{ "epg",		CRCInput::RC_epg },
		{ "recall",		CRCInput::RC_recall },
		{ "favorites",		CRCInput::RC_favorites },
		{ "sat",		CRCInput::RC_sat },
		{ "sat2",		CRCInput::RC_sat2 },
		{ "record",		CRCInput::RC_record },
		{ "play",		CRCInput::RC_play },
		{ "pause",		CRCInput::RC_pause },
		{ "forward",		CRCInput::RC_forward },
		{ "rewind",		CRCInput::RC_rewind },
		{ "stop",		CRCInput::RC_stop },
		{ "timeshift",		CRCInput::RC_timeshift },
		{ "mode",		CRCInput::RC_mode },
		{ "games",		CRCInput::RC_games },
		{ "next",		CRCInput::RC_next },
		{ "prev",		CRCInput::RC_prev },
		{ "www",		CRCInput::RC_www },
		{ "power_on",		CRCInput::RC_power_on },
		{ "power_off",		CRCInput::RC_power_off },
		{ "standby_on",		CRCInput::RC_standby_on },
		{ "standby_off",	CRCInput::RC_standby_off },
		{ "mute_on",		CRCInput::RC_mute_on },
		{ "mute_off",		CRCInput::RC_mute_off },
		{ "analog_on",		CRCInput::RC_analog_on },
		{ "analog_off",		CRCInput::RC_analog_off },
#ifdef MARTII
/* SPARK keys */
		{ "find",		CRCInput::RC_find },
		{ "pip",		CRCInput::RC_pip },
		{ "folder",		CRCInput::RC_archive },
		{ "forward",		CRCInput::RC_fastforward },
		{ "slow",		CRCInput::RC_slow },
		{ "playmode",		CRCInput::RC_playmode },
		{ "usb",		CRCInput::RC_usb },
		{ "f1",			CRCInput::RC_f1 },
		{ "f2",			CRCInput::RC_f2 },
		{ "f3",			CRCInput::RC_f3 },
		{ "f4",			CRCInput::RC_f4 },
		{ "prog1",		CRCInput::RC_prog1 },
		{ "prog2",		CRCInput::RC_prog2 },
		{ "prog3",		CRCInput::RC_prog3 },
		{ "prog4",		CRCInput::RC_prog4 },
#endif
		/* to check if it is in our range */
		{ "MaxRC",		CRCInput::RC_MaxRC },
		{ NULL, 0 }
	};

	/* list of colors, exported e.g. as COL['INFOBAR_SHADOW'] */
	static table_key colorlist[] =
	{
		{ "COLORED_EVENTS_CHANNELLIST",	MAGIC_COLOR | (COL_COLORED_EVENTS_CHANNELLIST) },
		{ "COLORED_EVENTS_INFOBAR",	MAGIC_COLOR | (COL_COLORED_EVENTS_INFOBAR) },
		{ "INFOBAR_SHADOW",		MAGIC_COLOR | (COL_INFOBAR_SHADOW) },
		{ "INFOBAR",			MAGIC_COLOR | (COL_INFOBAR) },
		{ "MENUHEAD",			MAGIC_COLOR | (COL_MENUHEAD) },
		{ "MENUCONTENT",		MAGIC_COLOR | (COL_MENUCONTENT) },
		{ "MENUCONTENTDARK",		MAGIC_COLOR | (COL_MENUCONTENTDARK) },
		{ "MENUCONTENTSELECTED",	MAGIC_COLOR | (COL_MENUCONTENTSELECTED) },
		{ "MENUCONTENTINACTIVE",	MAGIC_COLOR | (COL_MENUCONTENTINACTIVE) },
		{ "BACKGROUND",			MAGIC_COLOR | (COL_BACKGROUND) },
		{ "DARK_RED",			MAGIC_COLOR | (COL_DARK_RED0) },
		{ "DARK_GREEN",			MAGIC_COLOR | (COL_DARK_GREEN0) },
		{ "DARK_BLUE",			MAGIC_COLOR | (COL_DARK_BLUE0) },
		{ "LIGHT_GRAY",			MAGIC_COLOR | (COL_LIGHT_GRAY0) },
		{ "DARK_GRAY",			MAGIC_COLOR | (COL_DARK_GRAY0) },
		{ "RED",			MAGIC_COLOR | (COL_RED0) },
		{ "GREEN",			MAGIC_COLOR | (COL_GREEN0) },
		{ "YELLOW",			MAGIC_COLOR | (COL_YELLOW0) },
		{ "BLUE",			MAGIC_COLOR | (COL_BLUE0) },
		{ "PURP",			MAGIC_COLOR | (COL_PURP0) },
		{ "LIGHT_BLUE",			MAGIC_COLOR | (COL_LIGHT_BLUE0) },
		{ "WHITE",			MAGIC_COLOR | (COL_WHITE0) },
		{ "BLACK",			MAGIC_COLOR | (COL_BLACK0) },
		{ NULL, 0 }
	};

	/* font list, the _TYPE_ is redundant, exported e.g. as FONT['MENU'] */
	static table_key fontlist[] =
	{
		{ "MENU",		SNeutrinoSettings::FONT_TYPE_MENU },
		{ "MENU_TITLE",		SNeutrinoSettings::FONT_TYPE_MENU_TITLE },
		{ "MENU_INFO",		SNeutrinoSettings::FONT_TYPE_MENU_INFO },
		{ "EPG_TITLE",		SNeutrinoSettings::FONT_TYPE_EPG_TITLE },
		{ "EPG_INFO1",		SNeutrinoSettings::FONT_TYPE_EPG_INFO1 },
		{ "EPG_INFO2",		SNeutrinoSettings::FONT_TYPE_EPG_INFO2 },
		{ "EPG_DATE",		SNeutrinoSettings::FONT_TYPE_EPG_DATE },
		{ "EVENTLIST_TITLE",	SNeutrinoSettings::FONT_TYPE_EVENTLIST_TITLE },
		{ "EVENTLIST_ITEMLARGE",SNeutrinoSettings::FONT_TYPE_EVENTLIST_ITEMLARGE },
		{ "EVENTLIST_ITEMSMALL",SNeutrinoSettings::FONT_TYPE_EVENTLIST_ITEMSMALL },
		{ "EVENTLIST_DATETIME",	SNeutrinoSettings::FONT_TYPE_EVENTLIST_DATETIME },
		{ "GAMELIST_ITEMLARGE",	SNeutrinoSettings::FONT_TYPE_GAMELIST_ITEMLARGE },
		{ "GAMELIST_ITEMSMALL",	SNeutrinoSettings::FONT_TYPE_GAMELIST_ITEMSMALL },
		{ "CHANNELLIST",	SNeutrinoSettings::FONT_TYPE_CHANNELLIST },
		{ "CHANNELLIST_DESCR",	SNeutrinoSettings::FONT_TYPE_CHANNELLIST_DESCR },
		{ "CHANNELLIST_NUMBER",	SNeutrinoSettings::FONT_TYPE_CHANNELLIST_NUMBER },
		{ "CHANNELLIST_EVENT",	SNeutrinoSettings::FONT_TYPE_CHANNELLIST_EVENT },
		{ "CHANNEL_NUM_ZAP",	SNeutrinoSettings::FONT_TYPE_CHANNEL_NUM_ZAP },
		{ "INFOBAR_NUMBER",	SNeutrinoSettings::FONT_TYPE_INFOBAR_NUMBER },
		{ "INFOBAR_CHANNAME",	SNeutrinoSettings::FONT_TYPE_INFOBAR_CHANNAME },
		{ "INFOBAR_INFO",	SNeutrinoSettings::FONT_TYPE_INFOBAR_INFO },
		{ "INFOBAR_SMALL",	SNeutrinoSettings::FONT_TYPE_INFOBAR_SMALL },
		{ "FILEBROWSER_ITEM",	SNeutrinoSettings::FONT_TYPE_FILEBROWSER_ITEM },
		{ "MENU_HINT",		SNeutrinoSettings::FONT_TYPE_MENU_HINT },
		{ NULL, 0 }
	};

	table_key corners[] =
	{
		{ "TOP_LEFT",		CORNER_TOP_LEFT },
		{ "TOP_RIGHT",		CORNER_TOP_RIGHT },
		{ "BOTTOM_LEFT",	CORNER_BOTTOM_LEFT },
		{ "BOTTOM_RIGHT",	CORNER_BOTTOM_RIGHT },
		{ "RADIUS_LARGE",	RADIUS_LARGE },	/* those depend on g_settings.rounded_corners */
		{ "RADIUS_MID",		RADIUS_MID },
		{ "RADIUS_SMALL",	RADIUS_SMALL },
		{ "RADIUS_MIN",		RADIUS_MIN },
		{ NULL, 0 }
	};

	/* screen offsets, exported as e.g. SCREEN['END_Y'] */
	table_key screenopts[] =
	{
		{ "OFF_X", g_settings.screen_StartX },
		{ "OFF_Y", g_settings.screen_StartY },
		{ "END_X", g_settings.screen_EndX },
		{ "END_Y", g_settings.screen_EndY },
		{ NULL, 0 }
	};
#ifdef MARTII
	table_key menureturn[] =
	{
		{ "NONE", menu_return::RETURN_NONE },
		{ "REPAINT", menu_return::RETURN_REPAINT },
		{ "EXIT", menu_return::RETURN_EXIT },
		{ "EXIT_ALL", menu_return::RETURN_EXIT_ALL },
		{ "EXIT_REPAINT", menu_return::RETURN_EXIT_REPAINT },
		{ NULL, 0 }
	};
#endif

	/* list of environment variable arrays to be exported */
	lua_envexport e[] =
	{
		{ "RC",		keyname },
		{ "COL",	colorlist },
		{ "SCREEN",	screenopts },
		{ "FONT",	fontlist },
		{ "CORNER",	corners },
#ifdef MARTII
		{ "MENU_RETURN", menureturn },
#endif
		{ NULL, NULL }
	};

	int i = 0;
	while (e[i].name) {
		int j = 0;
		lua_newtable(L);
		while (e[i].t[j].name) {
			lua_pushstring(L, e[i].t[j].name);
			lua_pushinteger(L, e[i].t[j].code);
			lua_settable(L, -3);
			j++;
		}
		lua_setglobal(L, e[i].name);
		i++;
	}
}

//#define DBG printf
#define DBG(...)

#define lua_boxpointer(L, u) \
	(*(void **)(lua_newuserdata(L, sizeof(void *))) = (u))

#define lua_unboxpointer(L, i) \
	(*(void **)(lua_touserdata(L, i)))

const char CLuaInstance::className[] = "neutrino";

CLuaInstance::CLuaInstance()
{
	/* Create the intepreter object.  */
	lua = luaL_newstate();

	/* register standard + custom functions. */
	registerFunctions();
}

CLuaInstance::~CLuaInstance()
{
	if (lua != NULL)
	{
		lua_close(lua);
		lua = NULL;
	}
}

#define SET_VAR1(NAME) \
	lua_pushinteger(lua, NAME); \
	lua_setglobal(lua, #NAME);
#define SET_VAR2(NAME, VALUE) \
	lua_pushinteger(lua, VALUE); \
	lua_setglobal(lua, #NAME);

/* Run the given script. */
void CLuaInstance::runScript(const char *fileName)
{
	// luaL_dofile(lua, fileName);
	/* run the script */
	int status = luaL_loadfile(lua, fileName);
	if (status) {
		fprintf(stderr, "[CLuaInstance::%s] Can't load file: %s\n", __func__, lua_tostring(lua, -1));
		ShowMsg2UTF("Lua script error:", lua_tostring(lua, -1), CMsgBox::mbrBack, CMsgBox::mbBack);
		return;
	}
	set_lua_variables(lua);
	status = lua_pcall(lua, 0, LUA_MULTRET, 0);
	if (status)
	{
		fprintf(stderr, "[CLuaInstance::%s] error in script: %s\n", __func__, lua_tostring(lua, -1));
		ShowMsg2UTF("Lua script error:", lua_tostring(lua, -1), CMsgBox::mbrBack, CMsgBox::mbBack);
	}
}

const luaL_Reg CLuaInstance::methods[] =
{
	{ "PaintBox", CLuaInstance::PaintBox },
	{ "RenderString", CLuaInstance::RenderString },
	{ "PaintIcon", CLuaInstance::PaintIcon },
	{ "GetInput", CLuaInstance::GetInput },
	{ "FontHeight", CLuaInstance::FontHeight },
#ifdef MARTII
	{ "GetSize", CLuaInstance::GetSize },
	{ "DisplayImage", CLuaInstance::DisplayImage },
	{ "Blit", CLuaInstance::Blit },
	{ "GetLanguage", CLuaInstance::GetLanguage },
#endif
	{ NULL, NULL }
};

#ifndef DYNAMIC_LUAPOSIX
/* hack: we link against luaposix, which is included in our
 * custom built lualib */
extern "C" { LUAMOD_API int (luaopen_posix_c) (lua_State *L); }
#else
static int dolibrary (lua_State *L, const char *name)
{
	int status = 0;
	const char *msg = "";
	lua_getglobal(L, "require");
	lua_pushstring(L, name);
	status = lua_pcall(L, 1, 0, 0);
	if (status && !lua_isnil(L, -1))
	{
		msg = lua_tostring(L, -1);
		if (NULL == msg)
		{
			msg = "(error object is not a string)";
		}
		fprintf(stderr, "[CLuaInstance::%s] error in dolibrary: %s (%s)\n", __func__, name,msg);
		lua_pop(L, 1);
	}
	else
	{
		printf("[CLuaInstance::%s] loaded library: %s\n", __func__, name);
	}
	return status;
}
#endif
/* load basic functions and register our own C callbacks */
void CLuaInstance::registerFunctions()
{
	luaL_openlibs(lua);
	luaopen_table(lua);
	luaopen_io(lua);
	luaopen_string(lua);
	luaopen_math(lua);
#ifndef DYNAMIC_LUAPOSIX
	luaopen_posix_c(lua);
#else
	dolibrary(lua,"posix");
#endif

	lua_newtable(lua);
	int methodtable = lua_gettop(lua);
	luaL_newmetatable(lua, className);
	int metatable = lua_gettop(lua);
	lua_pushliteral(lua, "__metatable");
	lua_pushvalue(lua, methodtable);
	lua_settable(lua, metatable);

	lua_pushliteral(lua, "__index");
	lua_pushvalue(lua, methodtable);
	lua_settable(lua, metatable);

	lua_pushliteral(lua, "__gc");
	lua_pushcfunction(lua, GCWindow);
	lua_settable(lua, metatable);

	lua_pop(lua, 1);

	luaL_setfuncs(lua, methods, 0);
	lua_pop(lua, 1);

	lua_register(lua, className, NewWindow);
#ifdef MARTII
	MenueRegister(lua);
	HintboxRegister(lua);
	MessageboxRegister(lua);
#endif
}

CLuaData *CLuaInstance::CheckData(lua_State *L, int narg)
{
	luaL_checktype(L, narg, LUA_TUSERDATA);
	void *ud = luaL_checkudata(L, narg, className);
	if (!ud)
		fprintf(stderr, "[CLuaInstance::%s] wrong type %p, %d, %s\n", __func__, L, narg, className);
	return *(CLuaData **)ud;  // unbox pointer
}

int CLuaInstance::NewWindow(lua_State *L)
{
	int count = lua_gettop(L);
	int x = g_settings.screen_StartX;
	int y = g_settings.screen_StartY;
	int w = g_settings.screen_EndX - x;
	int h = g_settings.screen_EndY - y;
	CLuaData *D = new CLuaData();
	if (count > 0)
		x = luaL_checkint(L, 1);
	if (count > 1)
		y = luaL_checkint(L, 2);
	if (count > 2)
		w = luaL_checkint(L, 3);
	if (count > 3)
		h = luaL_checkint(L, 4);
	CFBWindow *W = new CFBWindow(x, y, w, h);
	D->fbwin = W;
	D->rcinput = g_RCInput;
	lua_boxpointer(L, D);
	luaL_getmetatable(L, className);
	lua_setmetatable(L, -2);
	return 1;
}

int CLuaInstance::PaintBox(lua_State *L)
{
	int count = lua_gettop(L);
	DBG("CLuaInstance::%s %d\n", __func__, count);
	int x, y, w, h, radius = 0, corner = CORNER_ALL;
	unsigned int c;

	CLuaData *W = CheckData(L, 1);
	if (!W || !W->fbwin)
		return 0;
	x = luaL_checkint(L, 2);
	y = luaL_checkint(L, 3);
	w = luaL_checkint(L, 4);
	h = luaL_checkint(L, 5);
	/* luaL_checkint does not like e.g. 0xffcc0000 on powerpc (returns INT_MAX) instead */
	c = (unsigned int)luaL_checknumber(L, 6);
	if (count > 6)
		radius = luaL_checkint(L, 7);
	if (count > 7)
		corner = luaL_checkint(L, 8);
	/* those checks should be done in CFBWindow instead... */
	if (x < 0)
		x = 0;
	if (y < 0)
		y = 0;
	if (w < 0 || x + w > W->fbwin->dx)
		w = W->fbwin->dx - x;
	if (h < 0 || y + h > W->fbwin->dy)
		h = W->fbwin->dy - y;
	/* use the color constants */
	if ((c & MAGIC_MASK) == MAGIC_COLOR)
		c = CFrameBuffer::getInstance()->realcolor[c & 0x000000ff];
	W->fbwin->paintBoxRel(x, y, w, h, c, radius, corner);
	return 0;
}

int CLuaInstance::PaintIcon(lua_State *L)
{
	DBG("CLuaInstance::%s %d\n", __func__, lua_gettop(L));
	int x, y, h;
	unsigned int o;
	const char *fname;

	CLuaData *W = CheckData(L, 1);
	if (!W || !W->fbwin)
		return 0;
	fname = luaL_checkstring(L, 2);
	x = luaL_checkint(L, 3);
	y = luaL_checkint(L, 4);
	h = luaL_checkint(L, 5);
	o = luaL_checkint(L, 6);
	W->fbwin->paintIcon(fname, x, y, h, o);
	return 0;
}

#ifdef MARTII
extern CPictureViewer * g_PicViewer;

int CLuaInstance::DisplayImage(lua_State *L)
{
	DBG("CLuaInstance::%s %d\n", __func__, lua_gettop(L));
	int x, y, w, h;
	const char *fname;

	fname = luaL_checkstring(L, 2);
	x = luaL_checkint(L, 3);
	y = luaL_checkint(L, 4);
	w = luaL_checkint(L, 5);
	h = luaL_checkint(L, 6);
	int trans = 0;
	if (lua_isnumber(L, 7))
		trans = luaL_checkint(L, 7);
	g_PicViewer->DisplayImage(fname, x, y, w, h, trans);
	return 0;
}

int CLuaInstance::GetSize(lua_State *L)
{
	DBG("CLuaInstance::%s %d\n", __func__, lua_gettop(L));
	int w = 0, h = 0;
	const char *fname;

	fname = luaL_checkstring(L, 2);
	g_PicViewer->getSize(fname, &w, &h);
	lua_pushinteger(L, w);
	lua_pushinteger(L, h);
	return 2;
}
#endif

int CLuaInstance::RenderString(lua_State *L)
{
	int x, y, w, boxh, f, center;
	unsigned int c;
	const char *text;
	int numargs = lua_gettop(L);
	DBG("CLuaInstance::%s %d\n", __func__, numargs);
	c = COL_MENUCONTENT;
	boxh = 0;
	center = 0;

	CLuaData *W = CheckData(L, 1);
	if (!W || !W->fbwin)
		return 0;
	f = luaL_checkint(L, 2);	/* font number, use FONT_TYPE_XXX in the script */
	text = luaL_checkstring(L, 3);	/* text */
	x = luaL_checkint(L, 4);
	y = luaL_checkint(L, 5);
	if (numargs > 5)
		c = luaL_checkint(L, 6);
	if (numargs > 6)
		w = luaL_checkint(L, 7);
	else
		w = W->fbwin->dx - x;
	if (numargs > 7)
		boxh = luaL_checkint(L, 8);
	if (numargs > 8)
		center = luaL_checkint(L, 9);
	if (f >= SNeutrinoSettings::FONT_TYPE_COUNT || f < 0)
		f = SNeutrinoSettings::FONT_TYPE_MENU;
	int rwidth = g_Font[f]->getRenderWidth(text, true);
	if (center) { /* center the text inside the box */
		if (rwidth < w)
			x += (w - rwidth) / 2;
	}
	c &= 0x000000FF; /* TODO: colors that are not in the palette? */
	if (boxh > -1) /* if boxh < 0, don't paint string */
		W->fbwin->RenderString(g_Font[f], x, y, w, text, c, boxh, true);
	lua_pushinteger(L, rwidth); /* return renderwidth */
	return 1;
}

int CLuaInstance::GetInput(lua_State *L)
{
	int numargs = lua_gettop(L);
	int timeout = 0;
	neutrino_msg_t msg;
	neutrino_msg_data_t data;
	CLuaData *W = CheckData(L, 1);
	if (!W)
		return 0;
	if (numargs > 1)
		timeout = luaL_checkint(L, 2);
	W->rcinput->getMsg_ms(&msg, &data, timeout);
	/* TODO: I'm not sure if this works... */
	if (msg != CRCInput::RC_timeout && msg > CRCInput::RC_MaxRC)
	{
		DBG("CLuaInstance::%s: msg 0x%08lx data 0x%08lx\n", __func__, msg, data);
		CNeutrinoApp::getInstance()->handleMsg(msg, data);
	}
	/* signed int is debatable, but the "big" messages can't yet be handled
	 * inside lua scripts anyway. RC_timeout == -1, RC_nokey == -2 */
	lua_pushinteger(L, (int)msg);
	lua_pushunsigned(L, data);
	return 2;
}

int CLuaInstance::FontHeight(lua_State *L)
{
	int f;
	DBG("CLuaInstance::%s %d\n", __func__, lua_gettop(L));

	CLuaData *W = CheckData(L, 1);
	if (!W)
		return 0;
	f = luaL_checkint(L, 2);	/* font number, use FONT['xxx'] for FONT_TYPE_xxx in the script */
	if (f >= SNeutrinoSettings::FONT_TYPE_COUNT || f < 0)
		f = SNeutrinoSettings::FONT_TYPE_MENU;
	lua_pushinteger(L, (int)g_Font[f]->getHeight());
	return 1;
}

int CLuaInstance::GCWindow(lua_State *L)
{
	DBG("CLuaInstance::%s %d\n", __func__, lua_gettop(L));
	CLuaData *w = (CLuaData *)lua_unboxpointer(L, 1);
	delete w->fbwin;
	w->rcinput = NULL;
	delete w;
	return 0;
}
#ifdef MARTII
int CLuaInstance::Blit(lua_State *L)
{
	CLuaData *W = CheckData(L, 1);
	if (W && W->fbwin) {
		if (lua_isnumber(L, 2))
			W->fbwin->mayBlit = (int)lua_tonumber(L, 2); // enable/disable automatic blit
		else
			W->fbwin->blit();
	}
	return 0;
}

int CLuaInstance::GetLanguage(lua_State *L)
{
	// FIXME -- should conform to ISO 639-1/ISO 3166-1
	lua_pushstring(L, g_settings.language.c_str());

	return 1;
}

#if 0
local m = menue.new{name="mytitle", icon="myicon", hide_parent=true}			       
m:addItem{type="back"}									     
m:addItem{type="separator"}									
												   
function talk(a, b)										   
	print(">talk")									     
	print(a .. " => " b)										   
	print("<talk")									     
	return MENU_RETURN["RETURN_REPAINT"]						       
end												
												   
function anotherMenue()									    
	print(">>anothermenue");								   
	local m = menue.new{name="anothermenue", icon="settings"}				  
	m:addItem{type="back"}								     
	m:addItem{type="separator"}								
	m:addItem{type="numeric", name="testnumeric"}					      
	m:exec()										   
	print("<<anothermenue");								   
end												
												   
												   
m:addItem{type="chooser", name="testchooser", action="talk", options={ "on", "off", "auto" }, icon=
m:addItem{type="forwarder", name="testforwarder", action="anotherMenue", icon="network"}	   
m:addItem{type="separator"}									
m:addItem{type="numeric", name="testnumeric", id="a", action="talk"}				       
m:addItem{type="separator"}									
m:addItem{type="filebrowser", name="fbrowser", id="b", action="talk"}				      
m:addItem{type="separator"}									
m:addItem{type="stringinput", name="stringinput", id="c", action="talk"}				   
m:exec()
#endif

bool CLuaInstance::tableLookup(lua_State *L, const char *what, std::string &value)
{
	bool res = false;
	lua_pushstring(L, what);
	lua_gettable(L, -2);
	res = lua_isstring(L, -1);
	if (res)
		value = lua_tostring(L, -1);
	lua_pop(L, 1);
	return res;
}

bool CLuaInstance::tableLookup(lua_State *L, const char *what, int &value)
{
	bool res = false;
	lua_pushstring(L, what);
	lua_gettable(L, -2);
	res = lua_isnumber(L, -1);
	if (res)
		value = (int) lua_tonumber(L, -1);
	lua_pop(L, 1);
	return res;
}

bool CLuaMenueChangeObserver::changeNotify(lua_State *L, const std::string &luaAction, const std::string &luaId, void *Data)
{
	const char *optionValue = (const char *) Data;
	lua_pushglobaltable(L);
	lua_getfield(L, -1, luaAction.c_str());
	lua_remove(L, -2);
	lua_pushstring(L, luaId.c_str());
	lua_pushstring(L, optionValue);
	lua_pcall(L, 2 /* two args */, 1 /* one result */, 0);
	double res = lua_isnumber(L, -1) ? lua_tonumber(L, -1) : 0;
	lua_pop(L, 2);
	return res == menu_return::RETURN_REPAINT || res == menu_return::RETURN_EXIT_REPAINT;
}

void CLuaInstance::MenueRegister(lua_State *L)
{
	luaL_Reg meth[] = {
		{ "new", CLuaInstance::MenueNew },
		{ "addKey", CLuaInstance::MenueAddKey },
		{ "addItem", CLuaInstance::MenueAddItem },
		{ "exec", CLuaInstance::MenueExec },
		{ "hide", CLuaInstance::MenueHide },
		{ "__gc", CLuaInstance::MenueDelete },
		{ NULL, NULL }
	};

	luaL_newmetatable(L, "menue");
	luaL_setfuncs(L, meth, 0);
	lua_pushvalue(L, -1);
	lua_setfield(L, -1, "__index");
	lua_setglobal(L, "menue");
}

CLuaMenue *CLuaInstance::MenueCheck(lua_State *L, int n)
{
	return *(CLuaMenue **) luaL_checkudata(L, n, "menue");
}

CLuaMenue::CLuaMenue()
{
	m = NULL;
	observ = new CLuaMenueChangeObserver();
}

CLuaMenue::~CLuaMenue()
{
	delete m;
	delete observ;
}

CLuaMenueForwarder::CLuaMenueForwarder(lua_State *_L, std::string _luaAction, std::string _luaId)
{
	L = _L;
	luaAction = _luaAction;
	luaId = _luaId;
}

CLuaMenueForwarder::~CLuaMenueForwarder()
{
}

int CLuaMenueForwarder::exec(CMenuTarget* /*parent*/, const std::string & /*actionKey*/)
{
	int res = menu_return::RETURN_REPAINT;
	if (!luaAction.empty()){
		lua_pushglobaltable(L);
		lua_getfield(L, -1, luaAction.c_str());
		lua_remove(L, -2);
		lua_pushstring(L, luaId.c_str());
		int status = lua_pcall(L, 1 /* one arg */, 1 /* one result */, 0);
		if (status) {
			fprintf(stderr, "[CLuaInstance::%s] error in script: %s\n", __func__, lua_tostring(L, -1));
			ShowMsg2UTF("Lua script error:", lua_tostring(L, -1), CMsgBox::mbrBack, CMsgBox::mbBack);
		}
		if (lua_isnumber(L, -1))
			res = (int) lua_tonumber(L, -1);
		lua_pop(L, 1);
	}
	return res;
}

CLuaMenueFilebrowser::CLuaMenueFilebrowser(lua_State *_L, std::string _luaAction, std::string _luaId, std::string *_value, bool _dirMode) : CLuaMenueForwarder(_L, _luaAction, _luaId)
{
	value = _value;
	dirMode = _dirMode;
}

int CLuaMenueFilebrowser::exec(CMenuTarget* /*parent*/, const std::string& /*actionKey*/)
{
	CFileBrowser fileBrowser;
	fileBrowser.Dir_Mode = dirMode;
	
	CFileFilter fileFilter;
	for (std::vector<std::string>::iterator it = filter.begin(); it != filter.end(); ++it)
		fileFilter.addFilter(*it);
	if (!filter.empty())
		fileBrowser.Filter = &fileFilter;

	if (fileBrowser.exec(value->c_str()) == true)
		*value = fileBrowser.getSelectedFile()->Name;

	if (!luaAction.empty()){
		lua_pushglobaltable(L);
		lua_getfield(L, -1, luaAction.c_str());
		lua_remove(L, -2);
		lua_pushstring(L, value->c_str());
		lua_pcall(L, 1 /* one arg */, 1 /* one result */, 0);
		lua_pop(L, 1);
	}
	return menu_return::RETURN_REPAINT;
}

CLuaMenueStringinput::CLuaMenueStringinput(lua_State *_L, std::string _luaAction, std::string _luaId, std::string _name, std::string *_value, int _size, std::string _valid_chars, CChangeObserver *_observ, std::string _icon, bool _sms) : CLuaMenueForwarder(_L, _luaAction, _luaId)
{
	name = _name;
	value = _value;
	size = _size;
	valid_chars = _valid_chars;
	icon = _icon;
	observ = _observ;
	sms = _sms;
}

int CLuaMenueStringinput::exec(CMenuTarget* /*parent*/, const std::string & /*actionKey*/)
{
	CStringInput *i;
	if (sms)
		i = new CStringInputSMS(name, value, size, NONEXISTANT_LOCALE, NONEXISTANT_LOCALE, valid_chars.c_str(), observ, icon.c_str());
	else
		i = new CStringInput(name, value, size, NONEXISTANT_LOCALE, NONEXISTANT_LOCALE, valid_chars.c_str(), observ, icon.c_str());
	i->exec(NULL, "");
	delete i;
	if (!luaAction.empty()){
		lua_pushglobaltable(L);
		lua_getfield(L, -1, luaAction.c_str());
		lua_remove(L, -2);
		lua_pushstring(L, luaId.c_str());
		lua_pushstring(L, value->c_str());
		lua_pcall(L, 2 /* two args */, 1 /* one result */, 0);
		lua_pop(L, 2);
	}
	return menu_return::RETURN_REPAINT;
}

int CLuaInstance::MenueNew(lua_State *L)
{
	CMenuWidget *m;

	if (lua_istable(L, 1)) {
		std::string name, icon;
		tableLookup(L, "name", name) || tableLookup(L, "title", name);
		tableLookup(L, "icon", icon);
		int mwidth;
		if(tableLookup(L, "mwidth", mwidth))
			m = new CMenuWidget(name.c_str(), icon.c_str(), mwidth);
		else
			m = new CMenuWidget(name.c_str(), icon.c_str());
	} else
		m = new CMenuWidget();

	CLuaMenue **udata = (CLuaMenue **) lua_newuserdata(L, sizeof(CLuaMenue *));
	*udata = new CLuaMenue();
	(*udata)->m = m;
	luaL_getmetatable(L, "menue");
	lua_setmetatable(L, -2);
	return 1;
}

int CLuaInstance::MenueDelete(lua_State *L)
{
	CLuaMenue *m = MenueCheck(L, 1);
	if (!m)
		return 0;

	while(!m->targets.empty()) {
		delete m->targets.back();
		m->targets.pop_back();
	}
	while(!m->tofree.empty()) {
		free(m->tofree.back());
		m->tofree.pop_back();
	}

	delete m;
	return 0;
}

int CLuaInstance::MenueAddKey(lua_State *L)
{
	CLuaMenue *m = MenueCheck(L, 1);
	if (!m)
		return 0;
	lua_assert(lua_istable(L, 2));

	std::string action;	tableLookup(L, "action", action);
	std::string id;		tableLookup(L, "id", id);
	int directkey = CRCInput::RC_nokey; tableLookup(L, "directkey", directkey);
	if (action != "" && directkey != (int) CRCInput::RC_nokey) {
		CLuaMenueForwarder *forwarder = new CLuaMenueForwarder(L, action, id);
		m->m->addKey(directkey, forwarder, action);
		m->targets.push_back(forwarder);
	}
	return 0;
}

int CLuaInstance::MenueAddItem(lua_State *L)
{
	CLuaMenue *m = MenueCheck(L, 1);
	if (!m)
		return 0;
	lua_assert(lua_istable(L, 2));

	CLuaMenueItem i;
	m->items.push_back(i);
	CLuaMenueItem *b = &m->items.back();

	tableLookup(L, "name", b->name);
	std::string icon;	tableLookup(L, "icon", icon);
	std::string type;	tableLookup(L, "type", type);
	if (type == "back") {
		m->m->addItem(GenericMenuBack);
	} else if (type == "separator") {
		if (!b->name.empty()) {
			m->m->addItem(new CMenuSeparator(CMenuSeparator::LINE|CMenuSeparator::STRING, b->name));
		} else {
			m->m->addItem(GenericMenuSeparatorLine);
		}
	} else {
		std::string right_icon;	tableLookup(L, "right_icon", right_icon);
		std::string action;	tableLookup(L, "action", action);
		std::string value;	tableLookup(L, "value", value);
		std::string id;		tableLookup(L, "id", id);
		std::string tmp;
		int directkey = CRCInput::RC_nokey; tableLookup(L, "directkey", directkey);
		int pulldown = false; 	tableLookup(L, "pulldown", pulldown);
		tmp = "true";
		tableLookup(L, "enabled", tmp) || tableLookup(L, "active", tmp);
		bool enabled = (tmp == "true" || tmp == "1" || tmp == "yes");
		tableLookup(L, "range", tmp);
		int range_from = 0, range_to = 99;
		sscanf(tmp.c_str(), "%d,%d", &range_from, &range_to);

		if (type == "forwarder") {
			b->val_s = value;
			CLuaMenueForwarder *forwarder = new CLuaMenueForwarder(L, action, id);
			CMenuItem *mi = new CMenuForwarder(
				b->name, enabled, NULL, forwarder, NULL/*ActionKey*/, directkey, icon.c_str(), right_icon.c_str());
			mi->setLua(L, action, id);
			m->m->addItem(mi);
			m->targets.push_back(forwarder);
		} else if (type == "chooser") {
			int options_count = 0;
			lua_pushstring(L, "options");
			lua_gettable(L, -2);
			if (lua_istable(L, -1))
				for (lua_pushnil(L); lua_next(L, -2); lua_pop(L, 2)) {
					lua_pushvalue(L, -2);
					options_count++;
				}
			lua_pop(L, 1);

			CMenuOptionChooser::keyval_ext *kext = (CMenuOptionChooser::keyval_ext *)calloc(options_count, sizeof(CMenuOptionChooser::keyval_ext));
			m->tofree.push_back(kext);
			lua_pushstring(L, "options");
			lua_gettable(L, -2);
			b->val_i = 0;
			int j = 0;
			if (lua_istable(L, -1))
				for (lua_pushnil(L); lua_next(L, -2); lua_pop(L, 2)) {
					lua_pushvalue(L, -2);
					const char *key = lua_tostring(L, -1);
					const char *val = lua_tostring(L, -2);
					kext[j].key = atoi(key);
					kext[j].value = NONEXISTANT_LOCALE;
					kext[j].valname = strdup(val);
					m->tofree.push_back((void *)kext[j].valname);
					if (!strcmp(value.c_str(), kext[j].valname))
						b->val_i = kext[j].key;
					j++;
				}
			lua_pop(L, 1);
			CMenuItem *mi = new CMenuOptionChooser(b->name, &b->val_i, kext, options_count, enabled, m->observ, directkey, icon.c_str(), pulldown);
			mi->setLua(L, action, id);
			m->m->addItem(mi);
		} else if (type == "numeric") {
			b->val_i = range_from;
			sscanf(value.c_str(), "%d", &b->val_i);
			CMenuItem *mi = new CMenuOptionNumberChooser(b->name, &b->val_i, enabled, range_from, range_to, m->observ,
				0, 0, NONEXISTANT_LOCALE, pulldown);
			mi->setLua(L, action, id);
			m->m->addItem(mi);
		} else if (type == "string") {
			b->val_s = value;
			CMenuItem *mi = new CMenuOptionStringChooser(b->name, &b->val_s, enabled, m->observ, directkey, icon.c_str(), pulldown);
			mi->setLua(L, action, id);
			m->m->addItem(mi);
		} else if (type == "stringinput") {
			b->val_s = value;
			std::string valid_chars = "abcdefghijklmnopqrstuvwxyz0123456789!\"§$%&/()=?-. ";
			tableLookup(L, "valid_chars", valid_chars);
			int sms = 0;	tableLookup(L, "sms", sms);
			int size = 30;	tableLookup(L, "size", size);
			CLuaMenueStringinput *stringinput = new CLuaMenueStringinput(L, action, id, b->name, &b->val_s, size, valid_chars, m->observ, icon.c_str(), sms);
			CMenuItem *mi = new CMenuForwarder(b->name, enabled, NULL, stringinput, NULL/*ActionKey*/, directkey, icon.c_str(), right_icon.c_str());
			mi->setLua(L, action, id);
			m->m->addItem(mi);
			m->targets.push_back(stringinput);
		} else if (type == "filebrowser") {
			b->val_s = value;
			int dirMode = 0; tableLookup(L, "dir_mode", dirMode);
			CLuaMenueFilebrowser *filebrowser = new CLuaMenueFilebrowser(L, action, id, &b->val_s, dirMode);
			lua_pushstring(L, "filter");
			lua_gettable(L, -2);
			if (lua_istable(L, -1))
				for (lua_pushnil(L); lua_next(L, -2); lua_pop(L, 2)) {
					lua_pushvalue(L, -2);
					const char *val = lua_tostring(L, -2);
					filebrowser->addFilter(val);
				}
			lua_pop(L, 1);

			CMenuItem *mi = new CMenuForwarder( b->name, enabled, NULL, filebrowser, NULL/*ActionKey*/, directkey, icon.c_str(), right_icon.c_str());
			mi->setLua(L, action, id);
			m->m->addItem(mi);
			m->targets.push_back(filebrowser);
		}
	}
	return 0;
}

int CLuaInstance::MenueHide(lua_State *L)
{
	CLuaMenue *m = MenueCheck(L, 1);
	if (!m)
		return 0;
	m->m->hide();
	return 0;
}

int CLuaInstance::MenueExec(lua_State *L)
{
	CLuaMenue *m = MenueCheck(L, 1);
	if (!m)
		return 0;
	m->m->exec(NULL, "");
	m->m->hide();
	return 0;
}

void CLuaInstance::HintboxRegister(lua_State *L)
{
	luaL_Reg meth[] = {
		{ "new", CLuaInstance::HintboxNew },
		{ "exec", CLuaInstance::HintboxExec },
		{ "paint", CLuaInstance::HintboxPaint },
		{ "hide", CLuaInstance::HintboxHide },
		{ "__gc", CLuaInstance::HintboxDelete },
		{ NULL, NULL }
	};

	luaL_newmetatable(L, "hintbox");
	luaL_setfuncs(L, meth, 0);
	lua_pushvalue(L, -1);
	lua_setfield(L, -1, "__index");
	lua_setglobal(L, "hintbox");
}

CLuaHintbox *CLuaInstance::HintboxCheck(lua_State *L, int n)
{
	return *(CLuaHintbox **) luaL_checkudata(L, n, "hintbox");
}

CLuaHintbox::CLuaHintbox()
{
	caption = NULL;
	b = NULL;
}

CLuaHintbox::~CLuaHintbox()
{
	if (caption)
		free(caption);
	delete b;
}

int CLuaInstance::HintboxNew(lua_State *L)
{
	lua_assert(lua_istable(L,1));

	std::string name, text, icon = std::string(NEUTRINO_ICON_INFO);
	tableLookup(L, "name", name) || tableLookup(L, "title", name) || tableLookup(L, "caption", name);
	tableLookup(L, "text", text);
	tableLookup(L, "icon", icon);
	int width = 450;
	tableLookup(L, "width", width);

	CLuaHintbox **udata = (CLuaHintbox **) lua_newuserdata(L, sizeof(CLuaHintbox *));
	*udata = new CLuaHintbox();
	(*udata)->caption = strdup(name.c_str());
	(*udata)->b = new CHintBox((*udata)->caption, text.c_str(), width, icon.c_str());
	luaL_getmetatable(L, "hintbox");
	lua_setmetatable(L, -2);
	return 1;
}

int CLuaInstance::HintboxDelete(lua_State *L)
{
	CLuaHintbox *m = HintboxCheck(L, 1);
	delete m;
	return 0;
}

int CLuaInstance::HintboxPaint(lua_State *L)
{
	CLuaHintbox *m = HintboxCheck(L, 1);
	if (!m)
		return 0;
	m->b->paint();
	return 0;
}

int CLuaInstance::HintboxHide(lua_State *L)
{
	CLuaHintbox *m = HintboxCheck(L, 1);
	m->b->hide();
	return 0;
}

int CLuaInstance::HintboxExec(lua_State *L)
{
	CLuaHintbox *m = HintboxCheck(L, 1);
	if (!m)
		return 0;
	int timeout = -1;
	if (lua_isnumber(L, -1))
		timeout = (int) lua_tonumber(L, -1);
	m->b->paint();

	// copied from gui/widget/hintbox.cpp

	neutrino_msg_t msg;
	neutrino_msg_data_t data;
	if ( timeout == -1 )
		timeout = 5; /// default timeout 5 sec
		//timeout = g_settings.timing[SNeutrinoSettings::TIMING_INFOBAR];

	uint64_t timeoutEnd = CRCInput::calcTimeoutEnd( timeout );

	int res = messages_return::none;

	while ( ! ( res & ( messages_return::cancel_info | messages_return::cancel_all ) ) )
	{
		g_RCInput->getMsgAbsoluteTimeout( &msg, &data, &timeoutEnd );

		if ((msg == CRCInput::RC_timeout) || (msg == CRCInput::RC_ok))
			res = messages_return::cancel_info;
		else if(msg == CRCInput::RC_home)
			res = messages_return::cancel_all;
		else if ((m->b->has_scrollbar()) && ((msg == CRCInput::RC_up) || (msg == CRCInput::RC_down)))
		{
			if (msg == CRCInput::RC_up)
				m->b->scroll_up();
			else
				m->b->scroll_down();
		}
		else if((msg == CRCInput::RC_sat) || (msg == CRCInput::RC_favorites)) {
		}
		else if(msg == CRCInput::RC_mode) {
			res = messages_return::handled;
			break;
		}
		else if((msg == CRCInput::RC_next) || (msg == CRCInput::RC_prev)) {
			res = messages_return::cancel_all;
			g_RCInput->postMsg(msg, data);
		}
		else
		{
			res = CNeutrinoApp::getInstance()->handleMsg(msg, data);
			if (res & messages_return::unhandled)
			{

				// leave here and handle above...
				g_RCInput->postMsg(msg, data);
				res = messages_return::cancel_all;
			}
		}
	}
	m->b->hide();
	return 0;
}

void CLuaInstance::MessageboxRegister(lua_State *L)
{
	luaL_Reg meth[] = {
		{ "exec", CLuaInstance::MessageboxExec },
		{ NULL, NULL }
	};

	luaL_newmetatable(L, "messagebox");
	luaL_setfuncs(L, meth, 0);
	lua_pushvalue(L, -1);
	lua_setfield(L, -1, "__index");
	lua_setglobal(L, "messagebox");
}

// messagebox.exec{caption="Title", text="text", icon="settings", width=500,timeout=-1,return_default_on_timeout=0,
//	default = "yes", buttons = { "yes", "no", "cancel", "all", "back", "ok" }, align="center1|center2|left|right" }
int CLuaInstance::MessageboxExec(lua_State *L)
{
	lua_assert(lua_istable(L,1));

	std::string name, text, icon = std::string(NEUTRINO_ICON_INFO);
	tableLookup(L, "name", name) || tableLookup(L, "title", name) || tableLookup(L, "caption", name);
	tableLookup(L, "text", text);
	tableLookup(L, "icon", icon);
	int timeout = -1, width = 450, return_default_on_timeout = 0, show_buttons = 0, default_button = 0;
	tableLookup(L, "timeout", timeout);
	tableLookup(L, "width", width);
	tableLookup(L, "return_default_on_timeout", return_default_on_timeout);

	std::string tmp;
	if (tableLookup(L, "align", tmp)) {
		lua_pushvalue(L, -2);
		const char *val = lua_tostring(L, -2);
		table_key mb[] = {
			{ "center1",	CMessageBox::mbBtnAlignCenter1 },
			{ "center2",	CMessageBox::mbBtnAlignCenter1 },
			{ "left",	CMessageBox::mbBtnAlignLeft },
			{ "right",	CMessageBox::mbBtnAlignRight },
			{ NULL,		0 }
		};
		for (int i = 0; mb[i].name; i++)
			if (!strcmp(mb[i].name, val)) {
				show_buttons |= mb[i].code;
				break;
			}
	}
	lua_pushstring(L, "buttons");
	lua_gettable(L, -2);
	for (lua_pushnil(L); lua_next(L, -2); lua_pop(L, 2)) {
		lua_pushvalue(L, -2);
		const char *val = lua_tostring(L, -2);
		table_key mb[] = {
			{ "yes",	CMessageBox::mbYes },
			{ "no",		CMessageBox::mbNo },
			{ "cancel",	CMessageBox::mbCancel },
			{ "all",	CMessageBox::mbAll },
			{ "back",	CMessageBox::mbBack },
			{ "ok",		CMessageBox::mbOk },
			{ NULL,		0 }
		};
		for (int i = 0; mb[i].name; i++)
			if (!strcmp(mb[i].name, val)) {
				show_buttons |= mb[i].code;
				break;
			}
	}
	lua_pop(L, 1);

	table_key mbr[] = {
		{ "yes",	CMessageBox::mbrYes },
		{ "no",		CMessageBox::mbrNo },
		{ "cancel",	CMessageBox::mbrCancel },
		{ "back",	CMessageBox::mbrBack },
		{ "ok",		CMessageBox::mbrOk },
		{ NULL,		0 }
	};
	if (tableLookup(L, "default", tmp)) {
		lua_pushvalue(L, -2);
		const char *val = lua_tostring(L, -2);
		for (int i = 0; mbr[i].name; i++)
			if (!strcmp(mbr[i].name, val)) {
				default_button = mbr[i].code;
				break;
			}
	}

	int res = ShowMsgUTF(name.c_str(), text.c_str(), (CMessageBox::result_) default_button, (CMessageBox::buttons_) show_buttons, icon.empty() ? NULL : icon.c_str(), width, timeout, return_default_on_timeout);

	tmp = "cancel";
	for (int i = 0; mbr[i].name; i++)
		if (res == mbr[i].code) {
			tmp = mbr[i].name;
			break;
		}
	lua_pushstring(L, tmp.c_str());

	return 1;
}
#endif
