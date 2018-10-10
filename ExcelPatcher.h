#pragma  once
#ifndef EXCEL_PATCHER_H_
#define EXCEL_PATCHER_H_

#include "ExcelParser.h"
#include <map>
//#include <GxEngine.h>
#include "EchoArchive.h"

class excel_patcher
{
private:
	//	 map<列---		map<索引，字串值>>
	std::map<int ,std::map<int,const char *> > m_content_map;
	excel_parser * m_parser;
public:	
	excel_patcher( Echo::DataStream * buffer)
	{
		//m_parser = new excel_parser();
		//m_parser->load(buffer);
		//excel_parser::line_data * lineData = m_parser->m_lines.m_head;
		//int col_n = 0;
		//while (lineData)
		//{
		//	int id__ = atoi(lineData->m_texts.m_head->m_text);
		//	// 如果第一个是-1证明第二个是列值
		//	if(id__ == -1)
		//	{
		//		col_n = atoi(lineData->m_texts.m_head->m_next->m_text);
		//	}
		//	// 不然就添加到语言替换map里
		//	else
		//	{
		//		m_content_map[col_n][id__] = lineData->m_texts.m_head->m_next->m_text;
		//	}
		//	lineData=lineData->m_next;
		//}
	}

	const char * get_cell_string(int col,int index)
	{
		std::map<int,std::map<int,const char *> >::iterator col_iter = m_content_map.find(col);
		if (col_iter == m_content_map.end())
		{
			return NULL;
		}
		std::map<int,const char *>::iterator index_iter = col_iter->second.find(index);
		if(index_iter == col_iter->second.end())
		{
			return NULL;
		}
		return index_iter->second;
	}

	~excel_patcher()
	{
		delete m_parser;
	}
};

#endif