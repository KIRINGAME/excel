#include "TableInfo.h"

__EXCEL_SPACE_BEG__

///////////////////////////////////////////////////////
//*******************table����*********************////
///////////////////////////////////////////////////////
#define MB_DATA_ENTER(type) \
	typedef mb_table<l_##type> tb_##type;\
	static tb_##type s_table_##type;\
	tb_##type* const gt_##type = &s_table_##type;
#define MB_DATA_LEAVE()
#define MB_DATA_VAR(type, var)
#define MB_DATA_ARRAY(type, var)

#include "Def.h"
#include "Undef.h"
///////////////////////////////////////////////////////
//*******************�нṹ����*******************////
///////////////////////////////////////////////////////

//#define MB_DATA_ENTER(type) \
//	static l_##type gl_##type;
//#define MB_DATA_LEAVE()
//#define MB_DATA_VAR(type, var)
//#define MB_DATA_ARRAY(type, var)
//
//#include "Def.h"
//#include "Undef.h"

///////////////////////////////////////////////////////
//*****************���������Ļص�����***************////
///////////////////////////////////////////////////////

// �ṹ��Ĺ�����������������
template<typename T>
void construct(T* p)
{
    new(p) T();
}

template<typename T>
void destruct(T* p)
{
	(void)p;
	p->~T();
}
template<class T>
static void s_construct(void* p)
{
	construct((T*)p);
}
template<class T>
static void s_destruct(void* p)
{
	destruct((T*)p);
}

#define MB_DATA_ENTER(type) \
	static void s_table_info_init_fun_##type(mb_table_info& li)\
	{\
		typedef l_##type mb_data_t;\
		li.m_struct_size = sizeof(l_##type);\
		li.m_ctor  = &s_construct<l_##type>;\
		li.m_dtor  = &s_destruct<l_##type>;\
		li.m_table = (table*)gt_##type; // **ÿ�ű�ľ�̬ȫ�ֱ����Ϣ��������һ��ָ��ȫ�־�̬���ṹ���ָ��

#define MB_DATA_VAR(type, var) li.def(#type,&mb_data_t::var,#var);
#define MB_DATA_ARRAY(type, var) li.def("v"#type,&mb_data_t::var,#var);
#define MB_DATA_LEAVE() }
#include "Def.h"
#include "Undef.h"

///////////////////////////////////////////////////////
//*****************��������������*******************////
///////////////////////////////////////////////////////

#define MB_DATA_ENTER(type) \
	 static mb_table_info s_table_info_##type(#type,s_table_info_init_fun_##type); // **��ÿ�ű�����һ����̬ȫ�ֱ����Ϣ����
#define MB_DATA_LEAVE()
#define MB_DATA_VAR(type, var)
#define MB_DATA_ARRAY(type, var)

#include "Def.h"
#include "Undef.h"

__EXCEL_SPACE_END__
