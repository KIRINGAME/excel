
#ifndef __TABLE_INFO_H__
#define __TABLE_INFO_H__

#pragma once
#include "Excel.h"
#include <vector>
#include <assert.h>
#include "SBCMN/cmn/cmndef.h"
///////////////////////////////////////////////////////
////*******����������Ϣ��ͨ���궨������*************////
///////////////////////////////////////////////////////

#define VNAME(name) (#name)

__EXCEL_SPACE_BEG__

	
// �ֶ�����
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

// ��ͷ������
struct title_info
{
	// ���������е�
	e_value_type		value_type;
	std::string			value_name;	//��Ӧ�ı�������
	// ������ר�е�
	int					offset;
	// �ı���ר�е�
	int					column;		//������ǰ�ǵڼ��У������м�û��<>��ע���й���
	std::string			defValue;
};

// ��ṹ������
typedef void (*mb_table_info_init_f)	(mb_table_info&);
typedef std::vector<title_info>			t_v_title;
typedef std::vector<std::string>		t_v_value_name;
typedef void (*ctor_fn_t)(void*);
typedef void (*dtor_fn_t)(void*);

// �У��������ݴ洢��ָ�룩
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

// ������ �����к����������ӳ�����֣�
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

// ��ģ���࣬����ʵ���������궨�����ɵĽṹ�壩
template<typename T>
class mb_table : public table  // **ÿ�ű���mb_tableģ�����һ��ʵ����
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
	// ��Էֱ�
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

// ��Ҫ��ÿ��������Ҫ����Ϣ
struct mb_table_info
{
	std::string			m_name;
	table*				m_table;		// ���ָ��
	ctor_fn_t			m_ctor;			// �ṹ����Ҫ�Ĺ��캯��(��������)
    dtor_fn_t			m_dtor;			// �ṹ����Ҫ����������(��������)
	size_t				m_struct_size;	// ÿ�нṹ����Ҫmalloc�Ŀռ��С
	t_v_title			m_title_list;	// ͨ���궨�����ɵı�ͷ��Ϣ

	 mb_table_info(const char* name , mb_table_info_init_f fn)
	 {
		 // ���g_Excel �����ڣ�����
		 if(g_Excel == NULL)
		 {
			 g_Excel = new Excel;
		 }

		 m_name = name;
		 g_Excel->m_map_table_info[m_name]=this; // **��ÿ�ű�ľ�̬ȫ�ֱ����Ϣ����ӳ�䵽excel����map��
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
		
		// ����g_Excel
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

	// ���͵�ӳ��
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
//************����Ϊ�궨�����ɴ��벿��**************////
//////////////////////////////////////////////////////

//////////////////////////////////////////////////////
//************���ṹ�������нṹ����������*********////
//////////////////////////////////////////////////////
#define MB_DATA_ENTER(type) \
	struct l_##type {
#define MB_DATA_LEAVE() };
#define MB_DATA_VAR(type, var) type var;
#define MB_DATA_ARRAY(type, var) std::vector<type> var;

#pragma pack(push) //�������״̬
#pragma pack(1)
#include "Def.h"
#pragma pack(pop)//�ָ�����״̬
#include "Undef.h"

///////////////////////////////////////////////////////
//*******************table����*********************////
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


