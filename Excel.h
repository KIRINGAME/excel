/********************************************************
Copyright 1996-2005, Pixel Software Corporation, 
All rights reserved.

Module name:	Excel manager
Purpose:		deal with table frome txt
Author: jiangli
Editor:xuequn
Compiler: Microsoft Visual C++ 2010
History: 2013-5-28

Remarks: 2018.1.9 wuzhichang 增加分表

*********************************************************/
#pragma  once
#ifndef __EXCEL_H__
#define __EXCEL_H__


#include <string>
#include <map>

#include <GxBase/GxBase.h>

#define __EXCEL_SPACE_BEG__	namespace excel {
#define __EXCEL_SPACE_END__	}


__EXCEL_SPACE_BEG__



/////////////////////////////////////////////////////////
////********辅助建立表格系统的两个临时列表********////
////////////////////////////////////////////////////////

class table;
class line;
struct mb_table_info;
enum
{
	client = 1,
	server = 2
};
enum
{
	all_load	= 0x00,
	client_load = 0x01,
	server_load	= 0x02
};
enum
{
	file_type	= 0x00,
	dir_type	= 0x10
};
class Excel
{
private:
	IGxArchive*							m_archive;
	std::string							m_szdir;

	bool PushData(std::string&table_dir, const std::string& table_name, const std::string& sub_table="");
public:
	Excel();
	~Excel();

	static bool onSetLanguage();

	typedef std::map<std::string,mb_table_info*>	type_map_table_info;
	type_map_table_info					m_map_table_info;

	bool Load(IGxArchive*	_pArch ,const char* szDir,int iType);
	// 载入表格 _table_name和_table_dir不同，_table_name是同一的表名如:ItemList ;_table_dir为真正的分表名,如ItemList_Cmn
	bool LoadTable(const std::string& _table_name, const std::string& _table_dir);
	// 载入文件夹<20180109 xuequn copy from wuzhichang>
	bool LoadTableDir(const std::string& _table_name, const std::string& _table_dir);
    
    void unloadAllTables();

	bool Reload(const std::string& table_name, int iType);
	bool Reload(int id, int iType);

	void RLink_MB();
	
};

extern Excel* g_Excel;

__EXCEL_SPACE_END__

extern const char* get_idx_text(int id);

#endif




