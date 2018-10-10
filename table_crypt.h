
#ifndef __TABLE_CRYPT_H__
#define __TABLE_CRYPT_H__

#pragma once

#define CRYPT_SWITCH


///*********************************************************************************///
///***********************************init data*************************************///

extern unsigned int u_crypt_xor_i;
extern unsigned int u_crypt_xor_f;
extern unsigned int u_crypt_shift;

#define	__SIZE__	32
#define ROTATE_LEFT(x) (((x) << (u_crypt_shift)) | ((x) >> ((__SIZE__) - (u_crypt_shift)))) 
#define ROTATE_RIGHT(x) (((x) >> (u_crypt_shift)) | ((x) << ((__SIZE__) - (u_crypt_shift))))


///*********************************************************************************///

// table_int hack表格内存用
struct table_crypt_int
{
//#define u_crypt_xor_i 0xeeaa2015
	int m_value;
	table_crypt_int()
	{
		set_value(0);
	}
	table_crypt_int(int _value)
	{
		set_value(_value);
	}
	
	int get_value() const
	{
		//return (int)(m_value^u_crypt_xor_i);
#if defined(CRYPT_SWITCH)
		return (int)((ROTATE_RIGHT( (unsigned int)(m_value) ))^u_crypt_xor_i);
#else
		return m_value;
#endif
	}
	void set_value(int _value)
	{
		//m_value = _value^u_crypt_xor_i;
#if defined(CRYPT_SWITCH)
		m_value = (int)(ROTATE_LEFT((unsigned int)(_value)^u_crypt_xor_i));
#else
		m_value = _value;
#endif
    }
    operator int()
    {
        return get_value();
    }

	int operator = (const int _value)
	{
		set_value(_value);
		return get_value();
	}
	// int
	int operator + (const int _value)
	{
		return get_value()+_value;
	}
	int operator - (const int _value)
	{
		return get_value()-_value;
	}
	int operator * (const int _value)
	{
		return get_value()*_value;
	}
	int operator / (const int _value)
	{
		return get_value()/_value;
	}

	// float
	float operator + (const float& _value)
	{
		return (float)((float)get_value()+_value);
	}
	float operator - (const float& _value)
	{
		return (float)((float)get_value()-_value);
	}
	float operator * (const float& _value)
	{
		return (float)((float)get_value()*_value);
	}
	float operator / (const float _value)
	{
		return (float)((float)get_value()/_value);
	}
	float operator / (const double _value)
	{
		return (float)((float)get_value()/_value);
	}
	// bool
	bool operator != (const int _value)
	{
		return get_value() != _value;
	}
	bool operator > (const int _value)
	{
		return get_value() > _value;
	}
	bool operator < (const int _value)
	{
		return get_value() < _value;
	}
	bool operator >= (const int _value)
	{
		return get_value() >= _value;
	}
	bool operator <= (const int _value)
	{
		return get_value() <= _value;
	}
	void operator += (const int& _value)
	{
		set_value(get_value()+_value);
	}

};
	
// table_float hack表格内存用
struct table_crypt_float
{
//#define u_crypt_xor_f 0x2015abcd
	
	typedef union
	{
		unsigned int _i;
		float	_f;
	}uCF;
	uCF m_value;

	table_crypt_float()
	{
		set_value(0);
	}
	table_crypt_float(float _value)
	{
		set_value(_value);
	}
	
	float get_value() const
	{
#if defined(CRYPT_SWITCH)
		uCF temp;

		//temp._i = m_value._i^u_crypt_xor_f;
		temp._i = ROTATE_LEFT(m_value._i)^u_crypt_xor_f;
		return (float)temp._f;
#else
		return m_value._f;
#endif
	}
	void set_value(float _value)
	{
		m_value._f = _value;
#if defined(CRYPT_SWITCH)
		//m_value._i^=u_crypt_xor_f;
		m_value._i = ROTATE_RIGHT((unsigned int)(m_value._i^u_crypt_xor_f));
#endif
	}
	operator float()
	{
		return get_value();
	}
	float operator = (const float _value)
	{
		set_value(_value);
		return get_value();
	}
	//// int
	//float operator + (const int _value)
	//{
	//	return get_value()+(float)_value;
	//}
	//float operator - (const int _value)
	//{
	//	return get_value()-(float)_value;
	//}
	//float operator * (const int _value)
	//{
	//	return get_value()*(float)_value;
	//}
	//float operator / (const int _value)
	//{
	//	return get_value()/(float)_value;
	//}

	//// float
	//float operator + (const float _value)
	//{
	//	return ((float)get_value()+_value);
	//}
	//float operator - (const float _value)
	//{
	//	return ((float)get_value()-_value);
	//}
	//float operator * (const float _value)
	//{
	//	return ((float)get_value()*_value);
	//}
	//float operator / (const float _value)
	//{
	//	return ((float)get_value()/_value);
	//}
	//float operator / (const double _value)
	//{
	//	return ((float)get_value()/(float)_value);
	//}
	// bool
	/*bool operator != (const int _value)
	{
		return get_value() != _value;
	}
	bool operator > (const int _value)
	{
		return get_value() > _value;
	}
	bool operator < (const int _value)
	{
		return get_value() < _value;
	}
	bool operator >= (const int _value)
	{
		return get_value() >= _value;
	}
	bool operator <= (const int _value)
	{
		return get_value() <= _value;
	}
*/
};



#endif


