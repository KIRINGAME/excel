
MB_DATA_ENTER(index_list)
	MB_DATA_VAR(int,				ID)
	MB_DATA_VAR(char const*,		name)			// ������	
	MB_DATA_VAR(int,				type)			// ������	
	MB_DATA_VAR(char const*,		dir1)			// 
	MB_DATA_VAR(char const*,		dir2)			
	MB_DATA_VAR(char const*,		dir3)			
	MB_DATA_VAR(char const*,		dir4)			
	MB_DATA_VAR(char const*,		dir5)			
	MB_DATA_VAR(char const*,		dir6)			
	MB_DATA_VAR(char const*,		dir7)			
	MB_DATA_VAR(char const*,		dir8)			
	MB_DATA_VAR(char const*,		dir9)			
	MB_DATA_VAR(char const*,		dir10)		
	MB_DATA_VAR(char const*,		dir11)	
	MB_DATA_VAR(char const*,		dir12)	
	MB_DATA_VAR(char const*,		dir13)	
	MB_DATA_VAR(char const*,		dir14)	
	MB_DATA_VAR(char const*,		dir15)	
MB_DATA_LEAVE()

MB_DATA_ENTER(define)
	MB_DATA_VAR(int, id)
	MB_DATA_VAR(float, float_value)			//float��ֵ
	MB_DATA_VAR(char const*, string_value)	//string��ֵ
	MB_DATA_VAR(int, int_value)				//int��ֵ
	MB_DATA_ARRAY(int, int_array_value)		//int����
MB_DATA_LEAVE()
