

#include <assert.h>

#if defined(_MSC_VER)

#endif
#include "Excel.h"
#include "ExcelParser.h"

#if defined(__ANDROID__)
#include <cstdlib>
#endif
#include "../log/log.h"


///****************************************************************************************///
std::string GetLanguage();
///****************************************************************************************///

/////////////////////////////////////////////////////////
////**������Ҫ����g_Excel,��ΪTableInfo��ͷ�ļ����õ�**////
////////////////////////////////////////////////////////
__EXCEL_SPACE_BEG__
	Excel* g_Excel=NULL;

	// ����table����ڵ��ַ����Ĺ���洢
	struct table_string_cmp
	{
		bool operator () (const char* x, const char* y) const
		{
			return strcmp(x, y) < 0;
		}
	};
	typedef std::set<char*, table_string_cmp> table_string_set;
	table_string_set g_table_string_set;
	::nr_heap	g_table_string_heap(32000);
	char g_table_string_null[] = { 0 };

	// By Jiangli: ����ʹ��nr_heap��֮����Ҫ�ͷŵ�g_table_string_set
	// Ŀ��1������set<string>�������ڴ����
	// Ŀ��2��string����char*��Ҳ���Լ�������
	// ʵ��Debug�汾������2~3MB�ڴ�
	// ����22200���ڵ�Release�汾���ٿɽ�Լ1MB�ڴ档
	char* table_string_insert(const char* t)
	{
		if(t == 0) return g_table_string_null;

		size_t c = strlen(t);
		if(c == 0) return g_table_string_null;

		table_string_set::iterator i = g_table_string_set.find((char*)t);
		if(i != g_table_string_set.end())
		{
			char* p = *i;
			return p;
		}

		char* p = (char*)g_table_string_heap.allocate(c + 1);
		memcpy(p, t, c);
		p[c] = 0;
		g_table_string_set.insert(p);
		return p;
	}
__EXCEL_SPACE_END__

#include "TableInfo.h"


__EXCEL_SPACE_BEG__

// ��ȡĬ��ֵ
void _GetTitleDefValue(const std::string& strType, std::string& defValue)
{
	size_t idStart = strType.find_first_of(':');
	if (idStart == std::string::npos)
	{
		return;
	}
	size_t idEnd = strType.find_first_of('>');
	if (idEnd != std::string::npos && idStart < idEnd)
	{
		defValue = strType.substr(idStart + 1, idEnd - idStart - 1);
	}
}
// ��ȡ������
void _GetTitleValueName(const std::string& strType, std::string& valueName)
{
	size_t idStart = strType.find_first_of('>');
	if (idStart == std::string::npos)
	{
		return;
	}
	valueName = strType.substr(idStart + 1);
}

// ���������������߻���ͷ�е����ݣ����ɶ�Ӧ��t_v_title
bool ParserTitle(excel_parser::text_data * text_data,t_v_title &titles)
{	
	unsigned int offset = 0;
	unsigned int column = 0;
	while( text_data )
	{
		std::string strType = text_data->m_text;

		// ������ͷ����

		if (!strType.empty())
		{
			if (strType.find("<I") < strType.length())
			{
				title_info t;
				t.value_type = type_int;
				_GetTitleValueName(strType, t.value_name);
				t.column = column;
				_GetTitleDefValue(strType,t.defValue);
				titles.push_back(t);
			}else if(strType.find("<VI") < strType.length())
			{
				title_info t;
				t.value_type = type_vint;
				_GetTitleValueName(strType, t.value_name);
				t.column = column;
				titles.push_back(t);
			}else if(strType.find("<S") < strType.length())
			{
				title_info t;
				t.value_type = type_string;
				_GetTitleValueName(strType, t.value_name);
				t.column = column;
				_GetTitleDefValue(strType,t.defValue);
				titles.push_back(t);
			}else if(strType.find("<VS") < strType.length())
			{
				title_info t;
				t.value_type = type_vstring;
				_GetTitleValueName(strType, t.value_name);
				t.column = column;
				titles.push_back(t);
			}else if(strType.find("<F") < strType.length())
			{
				title_info t;
				t.value_type = type_float;
				_GetTitleValueName(strType, t.value_name);
				t.column = column;
				_GetTitleDefValue(strType,t.defValue);
				titles.push_back(t);
			}else if(strType.find("<VF") < strType.length())
			{
				title_info t;
				t.value_type = type_vfloat;
				_GetTitleValueName(strType, t.value_name);
				t.column = column;
				titles.push_back(t);
			}else if(strType.find("<L") < strType.length())
			{
				title_info t;
				t.value_type = type_idx_string;
				_GetTitleValueName(strType, t.value_name);
				t.column = column;
				_GetTitleDefValue(strType, t.defValue);
				titles.push_back(t);
			}
		}
		// �����ͷ�ֶ��в���������<>������Ϊһ��ע���У�����Ϊ�ڴ��еĽṹ��ĳ�Ա����
		column++;
		text_data = text_data->m_next;			
	}

	return true;
}
title_info* GetTitleValueName(const char* value_name, t_v_title &titles)
{
	for (auto &i : titles)
	{
		if (i.value_name == std::string(value_name))
		{
			return &i;
		}
	}
	return nullptr;
}
void line_index::insert(int id, line* n)
{
	size_t c = m_cells.size();
	if(c == 0 || m_cells[c-1].m_id < id)
	{
		cell cl = {id, n};
		m_cells.push_back(cl);
		return;
	}
	cell* b = &m_cells[0];
	cell* p = b;
	while(c > 0)
	{
        size_t half = c/2;
        cell* mid = p+half;

		int id_m = mid->m_id;
        if(id_m < id)
        {
            p = ++ mid;
            c -= half+1;
        }
        else if(id_m > id)
        {
            c = half;
        }
		else
		{
			mid->m_line = n;
			return;
		}
	}
	cell cl = {id, n};
	m_cells.insert(m_cells.begin()+(p-b), cl);
}

line* line_index::find(int id) const
{
	size_t c = m_cells.size();
	if(c == 0) return 0;

	const cell* b = &m_cells[0];
	const cell* p = b;
	while(c > 0)
	{
        size_t half = c/2;
		const cell* mid = p+half;

		int id_m = mid->m_id;
        if(id_m < id)
        {
            p = ++ mid;
            c -= half+1;
        }
        else if(id_m > id)
        {
            c = half;
        }
		else
		{
			return mid->m_line;
		}
	}
	return 0;
}

Excel::Excel(): m_archive(nullptr)
{
}

Excel::~Excel()
{
	m_map_table_info.clear();
}

bool Excel::PushData(std::string&table_dir, const std::string& table_name, const std::string& sub_table_file)
{
	//����buffer
	IGxBlob* pBuff = m_archive->GetBlob(table_dir.c_str());
	if (pBuff == NULL)
	{
		CERROR(u8"the mb does not exist. table : %s, path : %s", table_name.c_str(), table_dir.c_str());
		return false;
	}
	// ��ȡmap�е�ǰ������Ϣ
	type_map_table_info::iterator it_map_table_info = m_map_table_info.find(table_name);
	if(it_map_table_info == m_map_table_info.end())
	{
		CERROR(u8"can't find table info for %s.", table_name.c_str());
		return false;
	}
	mb_table_info* table_info = it_map_table_info->second;
	if(table_info->m_table == NULL )
	{
		CERROR(u8"table info for %s is null", table_name.c_str());
		return false;
	}

	// ���ӱ�����ӱ�
	table* _outTable = nullptr;
	if (sub_table_file.empty())
	{
		_outTable = table_info->m_table;
	}
	else
	{
		map<string, table*>& sub_table = table_info->m_table->m_sub_table;
		map<string, table*>::iterator fIt = sub_table.find(sub_table_file);
		if (fIt != sub_table.end())
		{
			CERROR(u8"duplicate sub table %s for table %s", sub_table_file.c_str(), table_name.c_str());
			return false;
		}
		else
		{
			size_t size = sub_table_file.find("cf");
			if (size != sub_table_file.npos)
			{
				string tb = sub_table_file.substr(0, size);
				if (sub_table.find(tb) != sub_table.end())
				{
					_outTable = sub_table[tb];
				}
				else
				{
					_outTable = new table;
					sub_table[tb] = _outTable;
				}
			}
			else
			{
				_outTable = new table;
				sub_table[sub_table_file] = _outTable;
			}
		}
	}


	// �Ƚ������
	excel_parser ep;
	if (ep.load(pBuff) == false)
	{
		CERROR(u8"%s encoding is not supported!",table_dir.c_str());
		return false;
	}
	// �ͷ�buffer
	delete pBuff;

	int line_cnt = (int)ep.m_lines.m_size;
	if (line_cnt < 1)
	{
		return false;
	}
	excel_parser::line_data* line_data = ep.m_lines.m_head;
	
	line_data = line_data->m_next;
	// �ı���ͷ
	// ��ȡ�ı������<>�趨�ı�ṹ��������궨�����ɵı�ṹ����֤
	excel_parser::text_data* text_data = line_data->m_texts.m_head;
	t_v_title text_titles;
	ParserTitle(text_data, text_titles);

	// ��֤*

	// ����е���֯�Ͷ������֯�Ƿ���ͬ
	t_v_title & macro_table_titles = table_info->m_title_list;
	t_v_title::iterator it_m = macro_table_titles.begin();
	t_v_title::iterator it_m_end = macro_table_titles.end();
	
	//t_v_title::iterator it_t = _titles.begin();
	//t_v_title::iterator it_t_end = _titles.end();

	// ��ʱ����������assert��ʱ��鿴�ڼ��еڼ���
	//int _temp_col = -1;
	int _temp_line = 5;

	//for(;it_m != it_m_end && it_t != it_t_end;it_m++,it_t++)
	//{
	//	_temp_col++;
	//	if(!(it_m->value_type == it_t->value_type || (it_m->value_type == type_string && it_t->value_type == type_idx_string)) || it_m->offset != it_t->offset)
	//	{
	//		// �궨��Ͳ߻����ƥ��
	//		CERROR("the macro does not match the excel table. table : %s, path : %s, column : %d", table_name.c_str(), table_dir.c_str(), _temp_col);
	//		return false;
	//	}
	//}	
	//if(it_m != it_m_end || it_t != it_t_end)
	//{
	//	// ������ͬ
	//	CERROR("column num does not match table : %s, path : %s", table_name.c_str(), table_dir.c_str() );
	//	return false;
	//}

	// Խ����ͷ
	line_data = line_data->m_next;
	line_data = line_data->m_next;
	line_data = line_data->m_next;
	line_data = line_data->m_next;

	// ѭ��ÿ������
	_outTable->m_lines_v.reserve(_outTable->m_lines_v.size()+line_cnt-1);
	_outTable->m_lines_i.m_cells.reserve(_outTable->m_lines_i.m_cells.size()+line_cnt-1);
	char temp[512];
	int lastId = -1;

	while( line_data )
	{
		_temp_line++;
		// ѭ��ÿ����ÿ������
		excel_parser::text_data* text_data = line_data->m_texts.m_head;
		excel_parser::text_data text_data_null = {0,""};	//�����ݣ�������������

		// #����ע����
		if (text_data->m_text[0]=='#')
		{
			// ��һ������
			line_data = line_data->m_next;
			continue;
		}

		// ����ռ䲢���ù��캯��
		line* p_line = (line*)malloc(table_info->m_struct_size);
		table_info->m_ctor(p_line);

		// ����ƫ����д������
		unsigned int i = 0;
		// idx�����⵱ǰ�е���������һ�����������֣�
		int idx = -1;
		char* it_string;		

		// ���ҵ�ID
		//if(i< macro_table_titles.size())
		//{
		//	excel_parser::text_data* text_data_idx = line_data->m_texts.m_head;
		//	for(int title_i = 0;title_i < (int)(macro_table_titles.size());title_i++)
		//	{
		//		title_info& t = macro_table_titles[title_i];
		//		if(t.value_type == type_int)
		//		{
		//			if(idx == -1)
		//			{
		//				// <I>ID �������ַ���Ҳ�ᱻ����������ֵ��0
		//				title_info* txt_t = GetTitleValueName(t.value_name.c_str(), text_titles);
		//				if (txt_t != nullptr)
		//				{
		//					for (int c_i = 0; c_i < txt_t->column&&text_data_idx; c_i++)
		//					{
		//						//�ҵ��ڼ���
		//						text_data_idx = text_data_idx->m_next;
		//					}
		//					if (text_data_idx)
		//					{
		//						idx = atoi(text_data_idx->m_text);
		//						if (idx <= lastId)
		//						{
		//							CERROR(u8"Non monotone increasing ID: %d (line:%d) in table, %s", idx, _temp_line, table_dir.c_str());
		//						}
		//						lastId = idx;
		//						break;
		//					}
		//				}
		//				CERROR(u8"Non idxID in table,line:%d, %s", _temp_line, table_dir.c_str());
		//				break;
		//			}
		//		}
		//	}
		//}
		
		for (int title_i = 0; title_i < (int)(macro_table_titles.size()); title_i++)
		{
			title_info& t = macro_table_titles[title_i];
			title_info* txt_t = GetTitleValueName(t.value_name.c_str(), text_titles);
			if (txt_t == nullptr)
			{
				CERROR(u8"can't find the value_name in excel table. table : %s, path : %s, column : %s", table_name.c_str(), table_dir.c_str(), t.value_name);
				continue;
			}
			if (txt_t->value_type != t.value_type)
			{
				CERROR(u8"the macro value_type does not match the excel table. table : %s, path : %s, column : %s", table_name.c_str(), table_dir.c_str(), t.value_name);
			}
			// ÿ�δ�ͷ��
			text_data = line_data->m_texts.m_head;
			// �ҵ���Ӧ���ֵ���һ��
			for (int _column = 0; _column < txt_t->column; _column++)
			{
				text_data = text_data->m_next;
				if (text_data == 0)
				{
					text_data = &text_data_null;
					break;
				}
			}
			switch (t.value_type)
			{
			case type_int:
			{
				if (strlen(text_data->m_text) == 0 && !t.defValue.empty())
				{
					*(int*)((char*)(p_line)+t.offset) = atoi(t.defValue.c_str());
				}
				else
				{
					int _i_data = atoi(text_data->m_text);
					*(int*)((char*)(p_line)+t.offset) = _i_data;
					if (idx == -1)
					{
						idx = _i_data;
						if (idx <= lastId)
						{
							CERROR(u8"Non monotone increasing ID: %d (line:%d) in table, %s", idx, _temp_line, table_dir.c_str());
						}
						lastId = idx;
					}
				}
			}
			break;
			case type_float:
			{
				if (strlen(text_data->m_text) == 0 && !t.defValue.empty())
				{
					*(float*)((char*)(p_line)+t.offset) = (float)atof(t.defValue.c_str());
				}
				else
				{
					*(float*)((char*)(p_line)+t.offset) = (float)atof(text_data->m_text);
				}
			}
			break;
			case type_string:
			{
				if (strlen(text_data->m_text) == 0 && !t.defValue.empty())
				{
					// �ַ�������
					//*(string*)((char*)(p_line) +  t.offset) = t.defValue;
					it_string = table_string_insert(t.defValue.c_str());
					*(char**)((char*)(p_line)+t.offset) = it_string;
				}
				else
				{
					// �ַ�������
					//*(string*)((char*)(p_line) +  t.offset) = text_data->m_text;
					it_string = table_string_insert(text_data->m_text);
					*(char**)((char*)(p_line)+t.offset) = it_string;
				}
			}
			break;
			case type_vint:
			{
				const char* pos = text_data->m_text;
				const char* beg = text_data->m_text;

				for (; *beg != '\0'; pos++)
				{
					if (*pos == '*' || *pos == '|' || *pos == '\0')
					{
						memset(temp, 0, 512);
						if (pos - beg > 0)
						{
							strncpy(temp, beg, pos - beg);

							(*(std::vector<int>*)((char*)(p_line)+t.offset)).push_back(atoi(temp));
						}
						if (*pos != '\0')
						{
							beg = pos + 1;
						}
						else
						{
							break;
						}
					}
				}
			}
			break;
			case type_vfloat:
			{
				const char* pos = text_data->m_text;
				const char* beg = text_data->m_text;

				for (; *beg != '\0'; pos++)
				{
					if (*pos == '*' || *pos == '|' || *pos == '\0')
					{
						memset(temp, 0, 512);
						if (pos - beg > 0)
						{
							strncpy(temp, beg, pos - beg);

							(*(std::vector<float>*)((char*)(p_line)+t.offset)).push_back((float)atof(temp));
						}
						if (*pos != '\0')
						{
							beg = pos + 1;
						}
						else
						{
							break;
						}
					}
				}
			}
			break;
			case type_vstring:
			{
				const char* pos = text_data->m_text;
				const char* beg = text_data->m_text;
				for (; *beg != '\0'; pos++)
				{
					if (*pos == '*' || *pos == '|' || *pos == '\0')
					{
						if (pos - beg > 0)
						{
							std::string szStr(beg, pos - beg);

							// ԭʼ�汾
							//(*(vector<string>*)((char*)(p_line) +  t.offset)).push_back( szStr );

							// �ַ�������汾
							it_string = table_string_insert(szStr.c_str());
							(*(std::vector<char*>*)((char*)(p_line)+t.offset)).push_back(it_string);
						}
						if (*pos != '\0')
						{
							beg = pos + 1;
						}
						else
						{
							break;
						}
					}
				}
			}
			break;
			case type_idx_string:
			{
				int idx_string = -1;
				*(char**)((char*)(p_line)+t.offset) = NULL;
				if (strlen(text_data->m_text) == 0 && !t.defValue.empty())
				{
					idx_string = atoi(t.defValue.c_str());
				}
				else
				{
					idx_string = atoi(text_data->m_text);
				}
				if (idx_string >= 0)
				{
					l_lang_txt *txt = gt_lang_txt->find(idx_string);
					if (txt != NULL)
						*(const char**)((char*)(p_line)+t.offset) = txt->str;
				}
			}
			break;
			default:
				assert(false && "invalid type");
				break;
			}
		}

		// �����������
		_outTable->m_lines_v.push_back(p_line);
		_outTable->m_lines_i.insert(idx,p_line);

		// ��һ������
		line_data = line_data->m_next;
	}

	return true;
}

bool Excel::LoadTable(const std::string& table_name, const std::string& _table_dir)
{
	if (_table_dir.empty())
		return false;
	std::string table_dir;
	table_dir.append(m_szdir);
	table_dir.append("/");
	table_dir.append(_table_dir);
	table_dir.append(".txt");

	// ����*
	return PushData(table_dir, table_name);
}

bool Excel::LoadTableDir(const std::string& table_name, const std::string& _table_dir)
{
	if (_table_dir.empty())
		return false;
	std::string table_dir;
	table_dir.append(m_szdir);
	table_dir.append("/");
	table_dir.append(_table_dir);
	const std::set<std::string>& list = m_archive->ListDir(table_dir.c_str());
	for (auto _file :list)
	{
		std::string file_name = _file.substr(table_dir.length() + 1, _file.length() - 5 - table_dir.length());
		PushData(_file, table_name, file_name);
	}
	return true;
}

std::string _getfilepathname(const std::string& _path)
{
	size_t _start = _path.find_last_of('/');
    if (_start == std::string::npos)
        _start = 0;
    else
        ++_start;
	size_t _end = _path.find_last_of('.');
    if (_end == std::string::npos)   _end = _path.length();
	return _path.substr(_start,_end - _start);
}

bool Excel::Load(IGxArchive*_pArch ,const char* szDir,int iType)
{
	m_archive = _pArch;
	m_szdir = szDir;
	
	// ����index��
	LoadTable("index_list","index_list");

	auto line = gt_index_list->find(0);
	static std::string langFile;
	langFile.assign(line->dir1);
	langFile.append(GetLanguage());
	if (!_pArch->TestExist(("mb/" + langFile + ".txt").c_str()))
	{
		langFile.assign(line->dir1);
		langFile.append("chs");
	}
	line->dir1 = langFile.c_str();

	 //�����ӱ�
	unsigned int count = gt_index_list->count();
	for(unsigned int i = 0 ;i < count; i++)
	{
		l_index_list * p = gt_index_list->find_line(i);
		if ((p->type & 0xF) > 0 && ((p->type & 0xF) & iType) == 0)
		{
			continue;
		}
		//�ж����ļ�����Ŀ¼
		if ((p->type & 0xF0) == 0)
		{
			LoadTable(p->name, p->dir1);
			LoadTable(p->name, p->dir2);
			LoadTable(p->name, p->dir3);
			LoadTable(p->name, p->dir4);
			LoadTable(p->name, p->dir5);
			LoadTable(p->name, p->dir6);
			LoadTable(p->name, p->dir7);
			LoadTable(p->name, p->dir8);
			LoadTable(p->name, p->dir9);
			LoadTable(p->name, p->dir10);
			LoadTable(p->name, p->dir11);
			LoadTable(p->name, p->dir12);
			LoadTable(p->name, p->dir13);
			LoadTable(p->name, p->dir14);
			LoadTable(p->name, p->dir15);
		}
		else 
		{
			LoadTableDir(p->name, p->dir1);
			LoadTableDir(p->name, p->dir2);
			LoadTableDir(p->name, p->dir3);
			LoadTableDir(p->name, p->dir4);
			LoadTableDir(p->name, p->dir5);
			LoadTableDir(p->name, p->dir6);
			LoadTableDir(p->name, p->dir7);
			LoadTableDir(p->name, p->dir8);
			LoadTableDir(p->name, p->dir9);
			LoadTableDir(p->name, p->dir10);
			LoadTableDir(p->name, p->dir11);
			LoadTableDir(p->name, p->dir12);
			LoadTableDir(p->name, p->dir13);
			LoadTableDir(p->name, p->dir14);
			LoadTableDir(p->name, p->dir15);
		}
	}
	// By Jiangli: �����ͷŵ�g_table_string_set������ʡ���ڴ档
	g_table_string_set.clear();

	return false;

}

void Excel::unloadAllTables()
{
    for (Excel::type_map_table_info::iterator iter = m_map_table_info.begin(), iter_end = m_map_table_info.end();
         iter != iter_end; ++iter)
    {
        mb_table_info* table_info = iter->second;
        if (table_info != NULL) {
            table_info->m_table->destroy(table_info->m_dtor);
        }
    }
}

bool _Reload(l_index_list * p, int iType)
{
	// ��ȡmap�е�ǰ������Ϣ
	Excel::type_map_table_info::iterator it_map_table_info = g_Excel->m_map_table_info.find(p->name);
	if(it_map_table_info == g_Excel->m_map_table_info.end())
	{
		return false;
	}
	mb_table_info* table_info = it_map_table_info->second;
	if(table_info->m_table == NULL )
	{
		return false;
	}
	table_info->m_table->destroy(table_info->m_dtor);

	if ((p->type & 0xF) != 0 && ((p->type & 0xF) & iType) == 0)
	{
		return false;
	}
	if ((p->type & 0xF0) == 0)
	{
		g_Excel->LoadTable(p->name, p->dir1);
		g_Excel->LoadTable(p->name, p->dir2);
		g_Excel->LoadTable(p->name, p->dir3);
		g_Excel->LoadTable(p->name, p->dir4);
		g_Excel->LoadTable(p->name, p->dir5);
		g_Excel->LoadTable(p->name, p->dir6);
		g_Excel->LoadTable(p->name, p->dir7);
		g_Excel->LoadTable(p->name, p->dir8);
		g_Excel->LoadTable(p->name, p->dir9);
		g_Excel->LoadTable(p->name, p->dir10);
		g_Excel->LoadTable(p->name, p->dir11);
		g_Excel->LoadTable(p->name, p->dir12);
		g_Excel->LoadTable(p->name, p->dir13);
		g_Excel->LoadTable(p->name, p->dir14);
		g_Excel->LoadTable(p->name, p->dir15);
	}
	else
	{
		g_Excel->LoadTableDir(p->name, p->dir1);
		g_Excel->LoadTableDir(p->name, p->dir2);
		g_Excel->LoadTableDir(p->name, p->dir3);
		g_Excel->LoadTableDir(p->name, p->dir4);
		g_Excel->LoadTableDir(p->name, p->dir5);
		g_Excel->LoadTableDir(p->name, p->dir6);
		g_Excel->LoadTableDir(p->name, p->dir7);
		g_Excel->LoadTableDir(p->name, p->dir8);
		g_Excel->LoadTableDir(p->name, p->dir9);
		g_Excel->LoadTableDir(p->name, p->dir10);
		g_Excel->LoadTableDir(p->name, p->dir11);
		g_Excel->LoadTableDir(p->name, p->dir12);
		g_Excel->LoadTableDir(p->name, p->dir13);
		g_Excel->LoadTableDir(p->name, p->dir14);
		g_Excel->LoadTableDir(p->name, p->dir15);
	}
	return true;

}
bool Excel::Reload(const std::string& table_name, int iType)
{	
	// ѭ�������
	for(int i = 0 ,imax = gt_index_list->count();i<imax;i++)
	{
		l_index_list * p = gt_index_list->find_line(i);

		if(p->name == table_name)
		{
			return _Reload(p,iType);
			break;
		}
	}
	return false;
}
bool Excel::Reload(int id, int iType)
{
	// ����ID�����
	l_index_list * p = gt_index_list->find_line(id);
	if(p != NULL)
	{
		return _Reload(p, iType);
	}
	return false;
}

bool Excel::onSetLanguage()
{
	//bool ret = true;
	//for(std::vector<line*>::iterator iter = gt_index_list->m_lines_v.begin(), iter_end = gt_index_list->m_lines_v.end();
	//	ret && iter != iter_end; ++iter)
	//{
	//	l_index_list * line = (l_index_list*)*iter;
	//	if (line->trans.size() > 0)
	//	{
	//		ret |= _Reload(line);
	//	}
	//}
	//return ret;
	return false;
}

// ai_group���ؽ���������ݸĳɰ�λ���棬�������Ч��
void RLink_AIGroup()
{
	for (unsigned int i = 0; i < excel::gt_ai_group->count(); ++i)
	{
		excel::l_ai_group * pExcel = excel::gt_ai_group->find_line(i);
		if (pExcel->id == 1)
		{
			pExcel->m_friend_camp = 0;
			pExcel->m_enemy_camp = 0;
			continue;
		}
		int camp = pExcel->camp - 1;
		if (camp < 0 || camp>319)
		{
			continue;
		}
		pExcel->m_camp = 1 << camp;
		std::vector<int> v_friend_camp = pExcel->ai_friend;
		for (size_t j = 0; j < v_friend_camp.size(); ++j)
		{
			camp = v_friend_camp[j] - 1;
			if (camp<0 || camp>319)
			{
				continue;
			}
			else if(camp>=0 && camp <=31)
				pExcel->m_friend_camp |= 1 << camp;
			else if (camp >= 32 && camp <= 63)
				pExcel->m_friend_camp1 |= 1 << camp;
			else if (camp >= 64 && camp <= 95)
				pExcel->m_friend_camp2 |= 1 << camp;
			else if (camp >= 96 && camp <= 127)
				pExcel->m_friend_camp3 |= 1 << camp;
			else if (camp >= 128 && camp <= 159)
				pExcel->m_friend_camp4 |= 1 << camp;
			else if (camp >= 160 && camp <= 191)
				pExcel->m_friend_camp5 |= 1 << camp;
			else if (camp >= 192 && camp <= 223)
				pExcel->m_friend_camp6 |= 1 << camp;
			else if (camp >= 224 && camp <= 255)
				pExcel->m_friend_camp7 |= 1 << camp;
			else if (camp >= 256 && camp <= 287)
				pExcel->m_friend_camp8 |= 1 << camp;
			else if (camp >= 288 && camp <= 319)
				pExcel->m_friend_camp9 |= 1 << camp;
		}
		std::vector<int> v_not_friend_camp = pExcel->not_ai_friend;
		if (v_not_friend_camp.size() > 0)
		{
			for (size_t j = 0; j < v_not_friend_camp.size(); ++j)
			{
				camp = v_not_friend_camp[j] - 1;
				if (camp<0 || camp>319)
				{
					continue;
				}
				else if (camp >= 0 && camp <= 31)
					pExcel->m_friend_camp |= 1 << camp;
				else if (camp >= 32 && camp <= 63)
					pExcel->m_friend_camp1 |= 1 << camp;
				else if (camp >= 64 && camp <= 95)
					pExcel->m_friend_camp2 |= 1 << camp;
				else if (camp >= 96 && camp <= 127)
					pExcel->m_friend_camp3 |= 1 << camp;
				else if (camp >= 128 && camp <= 159)
					pExcel->m_friend_camp4 |= 1 << camp;
				else if (camp >= 160 && camp <= 191)
					pExcel->m_friend_camp5 |= 1 << camp;
				else if (camp >= 192 && camp <= 223)
					pExcel->m_friend_camp6 |= 1 << camp;
				else if (camp >= 224 && camp <= 255)
					pExcel->m_friend_camp7 |= 1 << camp;
				else if (camp >= 256 && camp <= 287)
					pExcel->m_friend_camp8 |= 1 << camp;
				else if (camp >= 288 && camp <= 319)
					pExcel->m_friend_camp9 |= 1 << camp;
			}
			pExcel->m_friend_camp ^= 0xFFFFFFFF;
			pExcel->m_friend_camp1 ^= 0xFFFFFFFF;
			pExcel->m_friend_camp2 ^= 0xFFFFFFFF;
			pExcel->m_friend_camp3 ^= 0xFFFFFFFF;
			pExcel->m_friend_camp4 ^= 0xFFFFFFFF;
			pExcel->m_friend_camp5 ^= 0xFFFFFFFF;
			pExcel->m_friend_camp6 ^= 0xFFFFFFFF;
			pExcel->m_friend_camp7 ^= 0xFFFFFFFF;
			pExcel->m_friend_camp8 ^= 0xFFFFFFFF;
			pExcel->m_friend_camp9 ^= 0xFFFFFFFF;
		}

		std::vector<int> v_enemy_camp = pExcel->ai_enemy;
		for (size_t j = 0; j < v_enemy_camp.size(); ++j)
		{
			camp = v_enemy_camp[j] - 1;
			if (camp<0 || camp>319)
			{
				continue;
			}
			else if (camp >= 0 && camp <= 31)
				pExcel->m_enemy_camp |= 1 << camp;
			else if (camp >= 32 && camp <= 63)
				pExcel->m_enemy_camp1 |= 1 << camp;
			else if (camp >= 64 && camp <= 95)
				pExcel->m_enemy_camp2 |= 1 << camp;
			else if (camp >= 96 && camp <= 127)
				pExcel->m_enemy_camp3 |= 1 << camp;
			else if (camp >= 128 && camp <= 159)
				pExcel->m_enemy_camp4 |= 1 << camp;
			else if (camp >= 160 && camp <= 191)
				pExcel->m_enemy_camp5 |= 1 << camp;
			else if (camp >= 192 && camp <= 223)
				pExcel->m_enemy_camp6 |= 1 << camp;
			else if (camp >= 224 && camp <= 255)
				pExcel->m_enemy_camp7 |= 1 << camp;
			else if (camp >= 256 && camp <= 287)
				pExcel->m_enemy_camp8 |= 1 << camp;
			else if (camp >= 288 && camp <= 319)
				pExcel->m_enemy_camp9 |= 1 << camp;
		}
		std::vector<int> v_not_enemy_camp = pExcel->not_ai_enemy;
		if (v_not_enemy_camp.size() > 0)
		{
			for (size_t j = 0; j < v_not_enemy_camp.size(); ++j)
			{
				camp = v_not_enemy_camp[j] - 1;
				if (camp<0 || camp>319)
				{
					continue;
				}
				else if (camp >= 0 && camp <= 31)
					pExcel->m_enemy_camp |= 1 << camp;
				else if (camp >= 32 && camp <= 63)
					pExcel->m_enemy_camp1 |= 1 << camp;
				else if (camp >= 64 && camp <= 95)
					pExcel->m_enemy_camp2 |= 1 << camp;
				else if (camp >= 96 && camp <= 127)
					pExcel->m_enemy_camp3 |= 1 << camp;
				else if (camp >= 128 && camp <= 159)
					pExcel->m_enemy_camp4 |= 1 << camp;
				else if (camp >= 160 && camp <= 191)
					pExcel->m_enemy_camp5 |= 1 << camp;
				else if (camp >= 192 && camp <= 223)
					pExcel->m_enemy_camp6 |= 1 << camp;
				else if (camp >= 224 && camp <= 255)
					pExcel->m_enemy_camp7 |= 1 << camp;
				else if (camp >= 256 && camp <= 287)
					pExcel->m_enemy_camp8 |= 1 << camp;
				else if (camp >= 288 && camp <= 319)
					pExcel->m_enemy_camp9 |= 1 << camp;
			}
			pExcel->m_enemy_camp ^= 0xFFFFFFFF;
			pExcel->m_enemy_camp1 ^= 0xFFFFFFFF;
			pExcel->m_enemy_camp2 ^= 0xFFFFFFFF;
			pExcel->m_enemy_camp3 ^= 0xFFFFFFFF;
			pExcel->m_enemy_camp4 ^= 0xFFFFFFFF;
			pExcel->m_enemy_camp5 ^= 0xFFFFFFFF;
			pExcel->m_enemy_camp6 ^= 0xFFFFFFFF;
			pExcel->m_enemy_camp7 ^= 0xFFFFFFFF;
			pExcel->m_enemy_camp8 ^= 0xFFFFFFFF;
			pExcel->m_enemy_camp9 ^= 0xFFFFFFFF;
		}
	}
}

void Excel::RLink_MB()
{
	RLink_AIGroup();
}

void table::destroy(dtor_fn_t _dtor)
{
	int i = 0,i_max = (int)m_lines_v.size();
	while(i<i_max)
	{
		_dtor(m_lines_v[i]);
		free(m_lines_v[i]);
		i++;
	}
	m_lines_v.clear();
	m_lines_i.clear();

	//�ӱ��Ƕ�̬new������
	for (map<string, table*>::iterator it = m_sub_table.begin(); it != m_sub_table.end(); it++)
	{
		it->second->destroy(_dtor);
		delete it->second;
	}
	m_sub_table.clear();
}

__EXCEL_SPACE_END__

const char* get_idx_text(int id)
{
	auto line = excel::gt_lang_txt->find(id);
	return line ? line->str : NULL;
}