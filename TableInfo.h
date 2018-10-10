
#ifndef __TABLE_INFO_H__
#define __TABLE_INFO_H__

#pragma once
#include "Excel.h"
#include <vector>
#include <assert.h>
#include "SBCMN/cmn/cmndef.h"
///////////////////////////////////////////////////////
////*******表格的描述信息，通过宏定义生成*************////
///////////////////////////////////////////////////////

#define VNAME(name) (#name)

__EXCEL_SPACE_BEG__

	
// 字段类型
enum e_value_type
{
	type_int		= 0,
	type_float		= 1,
	type_string		= 2,
	type_vint		= 3,
	type_vfloat		= 4,
	type_vstring	= 5,
	type_idx_string = 6,
	type_max		
};

// 表头的描述
struct title_info
{
	// 公共都会有的
	e_value_type		value_type;
	std::string			value_name;	//对应的变量名字
	// 代码里专有的
	int					offset;
	// 文本里专有的
	int					column;		//计数当前是第几列，用作中间没有<>的注释列功能
	std::string			defValue;
};

// 表结构的描述
typedef void (*mb_table_info_init_f)	(mb_table_info&);
typedef std::vector<title_info>			t_v_title;
typedef std::vector<std::string>		t_v_value_name;
typedef void (*ctor_fn_t)(void*);
typedef void (*dtor_fn_t)(void*);

// 行（用作数据存储的指针）
class line 
{
};

class line_index
{
public:
	struct cell
	{
		int					m_id;
		line*				m_line;
	};
	std::vector<cell>	m_cells;

	void insert(int id, line* n);
	line* find(int id) const;

	void clear() { m_cells.clear(); }
	size_t size() const { return m_cells.size(); }
};

// 表（基类 包含行号数组和索引映射两种）
class table 
{
public:
	std::vector<line*>					m_lines_v;
	line_index							m_lines_i;
	//quick_plex*						m_plex;
	table() /*: m_plex(0)*/ {}
	std::map<string, table*>			m_sub_table;

	void destroy(dtor_fn_t _dtor);
};

// 表（模版类，用来实例化各个宏定义生成的结构体）
template<typename T>
class mb_table : public table  // **每张表都是mb_table模版类的一个实例化
{
public:	
    T* find(int _id) const
    {
		T* p = (T*)m_lines_i.find(_id);
		return p;
	}
	T* find_line(unsigned int line_num) const
	{
		if(m_lines_v.size() == 0 || line_num >= m_lines_v.size()) return 0;
		T* p = (T*)m_lines_v[line_num];
		return p;
	}
	T* find_line(unsigned int line_num, int &id) const
	{
		if (m_lines_i.size() > line_num)
		{
			const line_index::cell &cell = m_lines_i.m_cells[line_num];
			id = cell.m_id;
			return (T*)cell.m_line;
		}
		else
			return nullptr;
	}
	unsigned int count() const
	{
		return (unsigned int)m_lines_v.size();
	}
	// 针对分表
	mb_table<T>* operator[](const char* sub_table_file)const
	{
		auto it = m_sub_table.find(sub_table_file);
		if (it != m_sub_table.end())
		{
			return (mb_table<T>*)(it->second);
		}
		return nullptr;
	}
};

// 主要是每行数据需要的信息
struct mb_table_info
{
	std::string			m_name;
	table*				m_table;		// 表的指针
	ctor_fn_t			m_ctor;			// 结构体需要的构造函数(析构行用)
    dtor_fn_t			m_dtor;			// 结构体需要的析构函数(构造行用)
	size_t				m_struct_size;	// 每行结构体需要malloc的空间大小
	t_v_title			m_title_list;	// 通过宏定义生成的表头信息

	 mb_table_info(const char* name , mb_table_info_init_f fn)
	 {
		 // 如果g_Excel 不存在，则建立
		 if(g_Excel == NULL)
		 {
			 g_Excel = new Excel;
		 }

		 m_name = name;
		 g_Excel->m_map_table_info[m_name]=this; // **将每张表的静态全局表格信息变量映射到excel管理map中
		 fn(*this);
	 }
	 ~mb_table_info()
	 {
		m_title_list.clear();
		if(m_table != NULL)
		{
			m_table->destroy(m_dtor);
			m_table = NULL;
		}
		
		// 析构g_Excel
		if(g_Excel != NULL)
		{
			delete g_Excel;
			g_Excel = NULL;
		}
	 }
    template<class T>
    static size_t ptr_cvt(T v)
    {
        union
        {
            T       v;
            size_t  s;
        } u;
        u.s = 0;
        u.v = v;
        return u.s;
	}

	// 类型的映射
	inline e_value_type type_map(const char* _type)
	{
		std::string type = _type;
		if (type == "int")
		{
			return type_int;
		}
		else if (type == "float")
		{
			return type_float;
		}
		else if (type == "char const*")
		{
			return type_string;
		}
		else if (type == "vint")
		{
			return type_vint;
		}
		else if (type == "vfloat")
		{
			return type_vfloat;
		}
		else if (type == "vchar const*")
		{
			return type_vstring;
		}
		assert(0);
		return type_max;
	}
	template<class T, class V>
	mb_table_info& def(const char* type,V T::* ptr,const char* var)
	 {
		 title_info t;
		 t.value_type = type_map(type);
		 t.offset = (int)ptr_cvt(ptr);
		 t.value_name = var;
		 m_title_list.push_back(t);
		 return *this;
	 }
};

//////////////////////////////////////////////////////
//************以下为宏定义生成代码部分**************////
//////////////////////////////////////////////////////

//////////////////////////////////////////////////////
//************表格结构声明；行结构体类型声明*********////
//////////////////////////////////////////////////////
#define MB_DATA_ENTER(type) \
	struct l_##type {
#define MB_DATA_LEAVE() };
#define MB_DATA_VAR(type, var) type var;
#define MB_DATA_ARRAY(type, var) std::vector<type> var;

#pragma pack(push) //保存对齐状态
#pragma pack(1)
#include "Def.h"
#pragma pack(pop)//恢复对齐状态
#include "Undef.h"

///////////////////////////////////////////////////////
//*******************table定义*********************////
///////////////////////////////////////////////////////
#define MB_DATA_ENTER(type) \
	typedef mb_table<l_##type> tb_##type;\
	extern tb_##type* const gt_##type ;

#define MB_DATA_LEAVE()
#define MB_DATA_VAR(type, var)
#define MB_DATA_ARRAY(type, var)

#include "Def.h"
#include "Undef.h"

__EXCEL_SPACE_END__

#endif


