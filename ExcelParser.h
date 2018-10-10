#ifndef _EXCEL_PARSER_H_
#define _EXCEL_PARSER_H_

#include "../encoding/encoding.h"
#include "../log/log.h"
namespace
{	
	// quick single list
	template<class T>
	struct qs_list
	{
		T*      m_head;
		T*      m_tail;
		size_t  m_size;

		qs_list()
		{
			m_head  = 0;
			m_tail  = 0;
			m_size  = 0;
		}
		void push_back(T* p)
		{
			if(m_head == 0)
			{
				m_head = p;
			}
			else
			{
				m_tail->m_next = p;
			}
			p->m_next = 0;
			m_tail = p;
			++ m_size;
		}
	};
	// no release heap
	class nr_heap
	{
	protected:
		struct page;

		page*   m_pages;
		char*   m_pos;
		char*   m_end;
		size_t  m_page_size;
		size_t  m_huge_size;

		bool m_zero_memory;
	public:
		nr_heap(size_t page_size = 4096);
		~nr_heap();

		void* allocate(size_t s);
		void clear();
		void create(size_t pageSize, bool zero_memory);
	};


	// nr_heap
	struct nr_heap::page
	{
		page*   m_next;
	};

	nr_heap::nr_heap(size_t page_size)
	{
		m_pages     = 0;
		m_pos       = 0;
		m_end       = 0;
		m_page_size = page_size;
		m_huge_size = m_page_size/2;
		m_zero_memory = false;
	}

	void nr_heap::create(size_t page_size, bool zero_memory)
	{
		m_page_size = page_size;
		m_huge_size = m_page_size / 2;
		m_zero_memory = zero_memory;
	}

	nr_heap::~nr_heap()
	{
		clear();
	}

	void nr_heap::clear()
	{
		while(m_pages)
		{
			void* p = m_pages;
			m_pages = m_pages->m_next;
			::free(p);
		}
		m_pos = 0;
		m_end = 0;
	}

	void* nr_heap::allocate(size_t s)
	{
		s = (s+3)&(~3);
		size_t r = m_end-m_pos;
		if(r >= s)
		{
			char* p = m_pos;
			m_pos += s;
			return p;
		}
		if(s >= m_huge_size)
		{
			size_t n = sizeof(page) + s;
			page* pg = (page*)::malloc(n);
			if (m_zero_memory) memset(pg, 0, n);
			pg->m_next = m_pages;
			m_pages = pg;
			return pg+1;
		}
		page* pg = (page*)::malloc(m_page_size);
		if (m_zero_memory) memset(pg, 0, m_page_size);
		pg->m_next = m_pages;
		m_pages = pg;
		char* p = (char*)(pg+1);
		m_pos = p+s;
		m_end = p+m_page_size-sizeof(page);
		return p;
	}
	struct excel_parser
	{
		struct text_data
		{
			text_data*  m_next;
			const char* m_text;
		};
		typedef qs_list<text_data> text_list;

		struct line_data
		{
			line_data*  m_next;
			text_list   m_texts;
		};
		typedef qs_list<line_data> line_list;
		

		nr_heap		m_heap;
		line_list   m_lines;
		line_data*  m_line;
		char*       m_pos;
		char*		m_mem_data;

		excel_parser();
		~excel_parser();

		bool load(IGxBlob * pBuff);

		inline void allocate_text(char* p, char* e)
		{
			size_t c = e-p;
			text_data* d = (text_data*)m_heap.allocate(sizeof(text_data));
			d->m_next    = 0;
			d->m_text	 = p;
			p[c]		 = 0;	// 注意，这里会修改内存！
			m_line->m_texts.push_back(d);
		}
		inline void allocate_line()
		{
			if(m_line)
			{
				m_lines.push_back(m_line);
			}
			m_line = (line_data*)m_heap.allocate(sizeof(line_data));
			memset(m_line, 0, sizeof(line_data));
		}
	};

	excel_parser::excel_parser()
	{
		m_line  = 0;
		m_pos   = 0;
		m_mem_data = NULL;
	}

	excel_parser::~excel_parser()
	{
		free(m_mem_data);
		m_mem_data = nullptr;
	}

	bool excel_parser::load(IGxBlob * pBuff)
	{
		// read file
		size_t buff_size = pBuff->GetSize();
		unsigned char *buff_data = (unsigned char *)pBuff->GetData();
		if(buff_size >= 3 && buff_data[0] == 0xEF && buff_data[1] == 0xBB && buff_data[2] == 0xBF)
		{	
			// UTF8 BOM
			buff_data += 3;
			buff_size -= 3;

			m_mem_data = (char*)(malloc(buff_size + 1));
			memcpy(m_mem_data, buff_data, buff_size);
			m_mem_data[buff_size] = 0;
		}
		else
		{
			//assert(false);
			return false;
		}
		m_pos = (char *)m_mem_data;
		
		// 开始填充数据
		allocate_line();

		// parse begin
		char* p = m_pos;
		for(;;)
		{
			switch(*p)
			{
			case 0:
				goto lb_parse_exit;
			case '\r':
lb_parse_r:
				if(*(p+1) == 0)
				{
					goto lb_parse_exit;
				}
				allocate_text(m_pos, p);
				allocate_line();
				if(*(p+1) == '\n')
				{
					p += 2;
				}
				else
				{
					++ p;
				}
				m_pos = p;
				break;
			case '\n':
lb_parse_n:
				allocate_text(m_pos, p);
				allocate_line();
				++ p;
				m_pos = p;
				break;
			case '\t':
lb_parse_t:
				allocate_text(m_pos, p);
				m_pos = ++p;
				break;
			case '"':
				{
					char* d = ++p;
					m_pos = d;
					for(; ; ++ p, ++ d)
					{
						if(*p == 0)
						{
							// bad format
							p = d;
							-- m_pos;
							goto lb_parse_exit;
						}
						if(*p == '"')
						{
							++ p;
							if(*p != '"')
							{
								break;
							}
						}
						*d = *p;
					}
					for(; ; ++ p, ++d)
					{
						if(*p == 0)
						{
							p = d;
							goto lb_parse_exit;
						}
						switch(*p)
						{
						case '\r':
							if(*(p+1) == 0)
							{
								p = d;
								goto lb_parse_exit;
							}
							allocate_text(m_pos, d);
							allocate_line();
							if(*(p+1) == '\n')
							{
								p += 2;
							}
							else
							{
								++ p;
							}
							m_pos = p;
							goto lb_parse_cell_exit;
						case '\n':
							allocate_text(m_pos, d);
							allocate_line();
							++ p;
							m_pos = p;
							goto lb_parse_cell_exit;
						case '\t':
							{
								allocate_text(m_pos, d);
								++ p;
								m_pos = p;
							}
							goto lb_parse_cell_exit;
						default:
							*d = *p;
						}
					}
				}
				break;
			default:
				++ p;
				for(; ; ++ p)
				{
					switch(*p)
					{
					case '\t':
						goto lb_parse_t;
					case '\n':
						goto lb_parse_n;
					case '\r':
						goto lb_parse_r;
					case 0:
						goto lb_parse_exit;
					}
				}
				break;
			}
lb_parse_cell_exit:;
		}

lb_parse_exit:

		if(m_pos < p)
		{
			allocate_text(m_pos, p);
			m_lines.push_back(m_line);
		}

		return true;
	}

	//long q_atol(const char* p)
	//{
	//	bool neg = false;
	//	for(;;++p)
	//	{
	//		switch(*p)
	//		{
	//			// skip space
	//		case '\t': case '\r': case '\n': case ' ':
	//			continue;
	//		case '-':
	//			neg = true;
	//			++ p;
	//			break;
	//		case '+':
	//			++ p;
	//			break;
	//		case 0:
	//			return 0;
	//		}
	//		break;
	//	}
	//	unsigned long c = 0;
	//	for(;;++p)
	//	{
	//		unsigned long d = (unsigned char)(*p-'0');
	//		if(d >= 10)
	//		{
	//			break;
	//		}
	//		c = c*10+d;
	//	}
	//	if(neg)
	//	{
	//		return -(long)c;
	//	}
	//	return (long)c;
	//}
} // namespace
#endif
