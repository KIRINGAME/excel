#include "TableScript.h"
#include "../script/script_engine.h"

__EXCEL_SPACE_BEG__

namespace
{
#define GetCellMagic 230189
#define GetCellByLineMagic 230190
#define GetCountMagic 230191

	int getIntArraySize(lua_State* L)
	{
		if (lua_gettop(L) >= 1 && lua_type(L, 1) == LUA_TLIGHTUSERDATA)
		{
			std::vector<int> *pv = (std::vector<int>*)lua_touserdata(L, 1);
			lua_pushinteger(L, (lua_Integer)pv->size());
			return 1;
		}
		return 0;
	}
	int getIntArrayElement(lua_State* L)
	{
		if (lua_gettop(L) >= 2 && lua_type(L, 1) == LUA_TLIGHTUSERDATA && lua_type(L, 2) == LUA_TNUMBER)
		{
			std::vector<int> *pv = (std::vector<int>*)lua_touserdata(L, 1);
			int idx = (int)lua_tointeger(L, 2);
			if (idx >= 0 && idx < (int)pv->size())
			{
				lua_pushinteger(L, (lua_Integer)(*pv)[idx]);
				return 1;
			}
		}
		return 0;
	}
	int getFloatArraySize(lua_State* L)
	{
		if (lua_gettop(L) >= 1 && lua_type(L, 1) == LUA_TLIGHTUSERDATA)
		{
			std::vector<float> *pv = (std::vector<float>*)lua_touserdata(L, 1);
			lua_pushinteger(L, (lua_Integer)pv->size());
			return 1;
		}
		return 0;
	}

	int getFloatArrayElement(lua_State* L)
	{
		if (lua_gettop(L) >= 2 && lua_type(L, 1) == LUA_TLIGHTUSERDATA && lua_type(L, 2) == LUA_TNUMBER)
		{
			std::vector<float> *pv = (std::vector<float>*)lua_touserdata(L, 1);
			int idx = (int)lua_tointeger(L, 2);
			if (idx >= 0 && idx < (int)pv->size())
			{
				lua_pushnumber(L, (lua_Number)(*pv)[idx]);
				return 1;
			}
		}
		return 0;
	}
	int getStringArraySize(lua_State* L)
	{
		if (lua_gettop(L) >= 1 && lua_type(L, 1) == LUA_TLIGHTUSERDATA)
		{
			// modified by wusitong
			// std::vector<int> *pv = (std::vector<int>*)lua_touserdata(L, 1);
			std::vector<const char*> *pv = (std::vector<const char*>*)lua_touserdata(L, 1);
			lua_pushinteger(L, (lua_Integer)pv->size());
			return 1;
		}
		return 0;
	}
	int getStringArrayElement(lua_State* L)
	{
		if (lua_gettop(L) >= 2 && lua_type(L, 1) == LUA_TLIGHTUSERDATA && lua_type(L, 2) == LUA_TNUMBER)
		{
			std::vector<const char*> *pv = (std::vector<const char*>*)lua_touserdata(L, 1);
			int idx = (int)lua_tointeger(L, 2);
			if (idx >= 0 && idx < (int)pv->size())
			{
				lua_pushstring(L, (*pv)[idx]);
				return 1;
			}
		}
		return 0;
	}

#define MB_DATA_ENTER(TableName) \
	class script_##TableName { \
	public: \
		static const tb_##TableName & table; \
		enum FieldIndex { \
			__fieldStart,
#define MB_DATA_VAR(type, var) \
			var,
#define MB_DATA_ARRAY(type, var) \
			var,
#define MB_DATA_LEAVE() \
			__filedEnd \
		}; \
		static void regClass(lua_State *L); \
		static int getCell(lua_State *L); \
		static int getCellByLine(lua_State *L); \
		static int getCount(lua_State *L); \
	};
#include "Def.h"
#include "Undef.h"

#define MB_DATA_ENTER(TableName) \
	void script_##TableName::regClass(lua_State *L) { \
		lua_pushstring(L, #TableName); \
		lua_newtable(L); \
		lua_newtable(L); \
		lua_pushstring(L, "__index"); \
		lua_newtable(L);
#define MB_DATA_VAR(type, var) \
		lua_pushstring(L, #var); \
		lua_pushinteger(L, var); \
		lua_rawset(L, -3);
#define MB_DATA_ARRAY(type, var) MB_DATA_VAR(type, var)
#define MB_DATA_LEAVE() \
		lua_pushinteger(L, GetCellMagic); \
		lua_pushcfunction(L, &getCell); \
		lua_rawset(L, -3); \
		lua_pushinteger(L, GetCellByLineMagic); \
		lua_pushcfunction(L, &getCellByLine); \
		lua_rawset(L, -3); \
		lua_pushinteger(L, GetCountMagic); \
		lua_pushcfunction(L, &getCount); \
		lua_rawset(L, -3); \
		lua_rawset(L, -3); \
		lua_setmetatable(L, -2); \
		lua_rawset(L, -3); \
	}
#include "Def.h"
#include "Undef.h"

#define MB_DATA_ENTER(TableName) \
	const tb_##TableName & script_##TableName::table = *gt_##TableName; \
	int script_##TableName::getCell(lua_State *L) { \
		if (lua_gettop(L) >= 2 && lua_type(L, -2) == LUA_TNUMBER && lua_type(L, -1) == LUA_TNUMBER) { \
			auto line = table.find((int)lua_tointeger(L, -2)); \
			if (line != NULL) { \
				switch(lua_tointeger(L, -1)) \
				{
#define MB_DATA_VAR(type, var) \
				case var: \
					push<type>(L, line->var); \
					return 1;
#define MB_DATA_ARRAY(type, var) \
				case var: \
					lua_pushlightuserdata(L, &line->var); \
					return 1;
#define MB_DATA_LEAVE() \
				default: \
					break; \
				}; \
			} \
		}; \
		return 0; \
	}
#include "Def.h"
#include "Undef.h"

#define MB_DATA_ENTER(TableName) \
	int script_##TableName::getCellByLine(lua_State *L) { \
		if (lua_gettop(L) >= 2 && lua_type(L, -2) == LUA_TNUMBER && lua_type(L, -1) == LUA_TNUMBER) { \
			auto line = table.find_line((int)lua_tointeger(L, -2)); \
			if (line != NULL) { \
				switch(lua_tointeger(L, -1)) \
				{
#define MB_DATA_VAR(type, var) \
				case var: \
					push<type>(L, line->var); \
					return 1;
#define MB_DATA_ARRAY(type, var) \
				case var: \
					lua_pushlightuserdata(L, &line->var); \
					return 1;
#define MB_DATA_LEAVE() \
				default: \
					break; \
				}; \
			} \
		}; \
		return 0; \
	}
#include "Def.h"
#include "Undef.h"

#define MB_DATA_ENTER(TableName) \
	int script_##TableName::getCount(lua_State *L) { \
		lua_pushinteger(L, table.count()); \
		return 1; \
	}
#define MB_DATA_VAR(type, var)
#define MB_DATA_ARRAY(type, var)
#define MB_DATA_LEAVE()
#include "Def.h"
#include "Undef.h"

int getCell(lua_State *L)
	{
		if (lua_gettop(L) >= 3 && lua_type(L, 1) == LUA_TSTRING && lua_type(L, 2) == LUA_TNUMBER && lua_type(L, 3) == LUA_TSTRING)
		{
			lua_getglobal(L, "excel");
			lua_pushvalue(L, 1);
			lua_rawget(L, -2);
			if (lua_type(L, -1) == LUA_TTABLE)
			{
				lua_pushinteger(L, GetCellMagic);
				lua_gettable(L, -2);
				lua_CFunction f = lua_tocfunction(L, -1);
				lua_copy(L, 3, -1);
				lua_gettable(L, -2);
				if (lua_type(L, -1) == LUA_TNUMBER)
				{
					lua_copy(L, -1, 3);
					lua_pop(L, 3);
					return f(L);
				}
			}
		}
		return 0;
	}

	int getCellByLine(lua_State *L)
	{
		if (lua_gettop(L) >= 3 && lua_type(L, 1) == LUA_TSTRING && lua_type(L, 2) == LUA_TNUMBER && lua_type(L, 3) == LUA_TSTRING)
		{
			lua_getglobal(L, "excel");
			lua_pushvalue(L, 1);
			lua_rawget(L, -2);
			if (lua_type(L, -1) == LUA_TTABLE)
			{
				lua_pushinteger(L, GetCellByLineMagic);
				lua_gettable(L, -2);
				lua_CFunction f = lua_tocfunction(L, -1);
				lua_copy(L, 3, -1);
				lua_gettable(L, -2);
				if (lua_type(L, -1) == LUA_TNUMBER)
				{
					lua_copy(L, -1, 3);
					lua_pop(L, 3);
					return f(L);
				}
			}
		}
		return 0;
	}

	int getCount(lua_State *L)
	{
		if (lua_gettop(L) >= 1 && lua_type(L, 1) == LUA_TSTRING)
		{
			lua_getglobal(L, "excel");
			lua_pushvalue(L, 1);
			lua_rawget(L, -2);
			if (lua_type(L, -1) == LUA_TTABLE)
			{
				lua_pushinteger(L, GetCountMagic);
				lua_gettable(L, -2);
				lua_CFunction f = lua_tocfunction(L, -1);
				lua_pop(L, 3);
				return f(L);
			}
		}
		return 0;
	}

	// ******************************
	// newVersion
	// ******************************
	template <typename T> void pushCell(lua_State *L, const T& cell);
	
	template<> void pushCell<int>(lua_State *L, const int& cell)
	{
		lua_pushinteger(L, cell);
	}
	template<> void pushCell<float>(lua_State *L, const float& cell)
	{
		lua_pushnumber(L, cell);
	}
	template<> void pushCell<char const*>(lua_State *L, char const* const& cell)
	{
		lua_pushstring(L, cell);
	}
	template<> void pushCell<std::vector<int>>(lua_State *L, const std::vector<int> &cell)
	{
		lua_newtable(L);
		LUA_INTEGER i = 0;
		for (auto iter = cell.begin(), iter_end = cell.end(); iter != iter_end; ++iter)
		{
			lua_pushinteger(L, *iter);
			lua_rawseti(L, -2, ++i);
		}
	}
	template<> void pushCell<std::vector<float>>(lua_State *L, const std::vector<float> &cell)
	{
		lua_newtable(L);
		LUA_INTEGER i = 0;
		for (auto iter = cell.begin(), iter_end = cell.end(); iter != iter_end; ++iter)
		{
			lua_pushnumber(L, *iter);
			lua_rawseti(L, -2, ++i);
		}
	}
	template<> void pushCell<std::vector<const char*>>(lua_State *L, const std::vector<const char*> &cell)
	{
		lua_newtable(L);
		LUA_INTEGER i = 0;
		for (auto iter = cell.begin(), iter_end = cell.end(); iter != iter_end; ++iter)
		{
			lua_pushstring(L, *iter);
			lua_rawseti(L, -2, ++i);
		}
	}

#define MB_DATA_ENTER(TableName) \
	class Script_##TableName { \
	public: \
		static void regClass(lua_State *L); \
	private: \
		static const tb_##TableName * pTable; \
		static int onCall(lua_State *L); \
		static int getCount(lua_State *L); \
		static int enumerateLines(lua_State *L); \
		static int nextLine(lua_State *L); \
		typedef Script_##TableName ThisType; \
		typedef l_##TableName LineType; \
		typedef void (*PushCellFunc)(lua_State *L, LineType* line);
#define MB_DATA_VAR(type, var) \
		static void push_##var(lua_State *L, LineType* line) \
		{ \
			pushCell(L, line->var); \
		}
#define MB_DATA_ARRAY(type, var) MB_DATA_VAR(type, var)
#define MB_DATA_LEAVE() \
	};
#include "Def.h"
#include "Undef.h"

#define MB_DATA_ENTER(TableName) \
	void Script_##TableName::regClass(lua_State *L) \
	{ \
		lua_pushstring(L, #TableName); \
		tb_##TableName **userdata = (tb_##TableName**)lua_newuserdata(L, sizeof(tb_##TableName*)); \
		*userdata = gt_##TableName; \
		lua_newtable(L); \
		lua_pushstring(L, "__call"); \
		lua_pushcfunction(L, onCall); \
		lua_rawset(L, -3); \
		lua_pushstring(L, "__len"); \
		lua_pushcfunction(L, getCount); \
		lua_rawset(L, -3); \
		lua_pushstring(L, "__index"); \
		lua_newtable(L); \
		lua_pushstring(L, "enum"); \
		lua_pushcfunction(L, enumerateLines); \
		lua_rawset(L, -3);
#define MB_DATA_VAR(type, var) \
		{ \
			lua_pushstring(L, #var); \
			PushCellFunc *memPtr = (PushCellFunc*)lua_newuserdata(L, sizeof(PushCellFunc)); \
			*memPtr = &ThisType::push_##var; \
			lua_rawset(L, -3); \
		}
#define MB_DATA_ARRAY(type, var) MB_DATA_VAR(type, var)
#define MB_DATA_LEAVE() \
		lua_rawset(L, -3); \
		lua_setmetatable(L, -2); \
		lua_rawset(L, -3); \
	}
#include "Def.h"
#include "Undef.h"

#define MB_DATA_ENTER(TableName) \
	int Script_##TableName::onCall(lua_State *L) \
	{ \
		if (lua_type(L, 1) == LUA_TUSERDATA && lua_type(L, 2) == LUA_TNUMBER) \
		{ \
			tb_##TableName * pTable = gt_##TableName; \
			int id = (int)lua_tointeger(L, 2); \
			auto line = pTable->find(id); \
			if (line != nullptr) \
			{ \
				int maxParam = lua_gettop(L); \
				for (int i = 3; i <= maxParam && lua_type(L, i) == LUA_TSTRING; ++i) \
				{ \
					lua_pushvalue(L, i); \
					lua_gettable(L, 1); \
					if (lua_type(L, -1) == LUA_TUSERDATA) \
					{ \
						PushCellFunc func = *(PushCellFunc*)lua_touserdata(L, -1); \
						lua_pop(L, 1); \
						func(L, line); \
					} \
					else \
					{ \
						lua_pop(L, 1); \
						lua_pushnil(L); \
					} \
				} \
				return maxParam - 2; \
			} \
		} \
		return 0; \
	} \
	int Script_##TableName::getCount(lua_State *L) \
	{ \
		if (lua_type(L, 1) == LUA_TUSERDATA) \
		{ \
			tb_##TableName * pTable = gt_##TableName; \
			lua_pushinteger(L, (LUA_INTEGER)pTable->count()); \
		} \
		else \
			lua_pushinteger(L, -1); \
		return 1; \
	}
#define MB_DATA_VAR(type, var)
#define MB_DATA_ARRAY(type, var)
#define MB_DATA_LEAVE()
#include "Def.h"
#include "Undef.h"

#define MB_DATA_ENTER(TableName) \
	int Script_##TableName::enumerateLines(lua_State *L) \
	{ \
		int countParam = lua_gettop(L); \
		if (countParam >= 1 && lua_type(L, 1) == LUA_TUSERDATA) \
		{ \
			lua_pushinteger(L, 0); \
			for (int i = 2; i <= countParam; ++i) \
			{ \
				if (lua_type(L, i) == LUA_TSTRING) \
					lua_pushvalue(L, i); \
				else \
				{ \
					lua_pushstring(L, "column name must be a string"); \
					lua_error(L); \
				} \
			} \
			lua_pushcclosure(L, nextLine, countParam); \
			lua_pushvalue(L, 1); \
			return 2; \
		} \
		return 0; \
	}
#define MB_DATA_VAR(type, var)
#define MB_DATA_ARRAY(type, var)
#define MB_DATA_LEAVE()
#include "Def.h"
#include "Undef.h"

#define MB_DATA_ENTER(TableName) \
	int Script_##TableName::nextLine(lua_State *L) \
	{ \
		if (lua_type(L, 1) == LUA_TUSERDATA) \
		{ \
			tb_##TableName * pTable = gt_##TableName; \
			int nextLineNo = (int)lua_tointeger(L, lua_upvalueindex(1)); \
			int id; \
			auto line = pTable->find_line(nextLineNo, id); \
			if (line != nullptr) \
			{ \
				lua_pushinteger(L, id); \
				int curIdx = 1; \
				int upvalueIndex = lua_upvalueindex(++curIdx); \
				while(lua_type(L, upvalueIndex) == LUA_TSTRING) \
				{ \
					lua_pushvalue(L, upvalueIndex); \
					lua_gettable(L, 1); \
					if (lua_type(L, -1) == LUA_TUSERDATA) \
					{ \
						PushCellFunc func = *(PushCellFunc*)lua_touserdata(L, -1); \
						lua_pop(L, 1); \
						func(L, line); \
					} \
					else \
					{ \
						lua_pop(L, 1); \
						lua_pushnil(L); \
					} \
					upvalueIndex = lua_upvalueindex(++curIdx); \
				} \
				lua_pushinteger(L, nextLineNo + 1); \
				lua_replace(L, lua_upvalueindex(1)); \
				return curIdx - 1; \
			} \
		} \
		return 0; \
	}
#define MB_DATA_VAR(type, var)
#define MB_DATA_ARRAY(type, var)
#define MB_DATA_LEAVE()
#include "Def.h"
#include "Undef.h"

	static int excel_ref = 0;
	int getMB(lua_State *L)
	{
		if (lua_gettop(L) >= 1 && lua_type(L, 1) == LUA_TSTRING)
		{
			lua_rawgetp(L, LUA_REGISTRYINDEX, &excel_ref);
			lua_pushvalue(L, 1);
			lua_rawget(L, -2);
			lua_replace(L, -2);
			return 1;
		}
		else
			luaL_error(L, "wrong parameters for getMB");
		return 0;
	}
}

void regTableFunction()
{
	lua_State *L = script::GetScriptEngine()->m_L;
	CheckLuaStackBalance(L);

	lua_newtable(L);
#define MB_DATA_ENTER(TableName) \
	Script_##TableName::regClass(L);
#define MB_DATA_VAR(type, var)
#define MB_DATA_ARRAY(type, var)
#define MB_DATA_LEAVE()
#include "Def.h"
#include "Undef.h"

	lua_rawsetp(L, LUA_REGISTRYINDEX, &excel_ref);
	lua_pushcfunction(L, getMB);
	lua_setglobal(L, "getMB");
}

void reg_table_function_to_script()
{
	lua_State *L = script::GetScriptEngine()->m_L;	
	{
		int t = lua_getglobal(L, "excel");
		if (t == LUA_TNIL) {
			lua_pop(L, 1);
			lua_newtable(L);
			lua_pushvalue(L, -1);
			lua_setglobal(L, "excel");
		}
		else if (t != LUA_TTABLE) {
				lua_pop(L, 1);
				return;
		}
	}
	lua_pushstring(L, "getIntArraySize");
	lua_pushcfunction(L, getIntArraySize);
	lua_rawset(L, -3);
	lua_pushstring(L, "getIntArrayElement");
	lua_pushcfunction(L, getIntArrayElement);
	lua_rawset(L, -3);
	lua_pushstring(L, "getFloatArraySize");
	lua_pushcfunction(L, getFloatArraySize);
	lua_rawset(L, -3);
	lua_pushstring(L, "getFloatArrayElement");
	lua_pushcfunction(L, getFloatArrayElement);
	lua_rawset(L, -3);
	lua_pushstring(L, "getStringArraySize");
	lua_pushcfunction(L, getStringArraySize);
	lua_rawset(L, -3);
	lua_pushstring(L, "getStringArrayElement");
	lua_pushcfunction(L, getStringArrayElement);
	lua_rawset(L, -3);
	lua_pushstring(L, "getCell");
	lua_pushcfunction(L, getCell);
	lua_rawset(L, -3);
	lua_pushstring(L, "getCellByLine");
	lua_pushcfunction(L, getCellByLine);
	lua_rawset(L, -3);
	lua_pushstring(L, "getCount");
	lua_pushcfunction(L, getCount);
	lua_rawset(L, -3);

#define MB_DATA_ENTER(TableName) \
	script_##TableName::regClass(L);
#define MB_DATA_VAR(type, var)
#define MB_DATA_ARRAY(type, var)
#define MB_DATA_LEAVE()
#include "Def.h"
#include "Undef.h"

	lua_pop(L, 1);
}

__EXCEL_SPACE_END__
