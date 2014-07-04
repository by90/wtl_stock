#include "parse_iterator.h"
#ifndef dad_parse_iterator_h
#define dad_parse_iterator_h

//DadÎÄ¼şµÄÖ¤¾í´úÂë¡¢Ãû³Æ¶ÔÕÕ±í½á¹¹
struct id_of_dad
{
	char id[16];
	char title[16];
};

//DadÎÄ¼şµÄĞĞÇé¼ÇÂ¼½á¹¹
struct quote_of_dad
{
	long quoteTime; //Ê¹ÓÃ_mktime32×ªÎª__time32t´æ´¢£¬Ê¹ÓÃ_localtime32¶ÁÈ¡
	float open, high, low, close;
	float volume, amount;
};

//DadÎÄ¼şµÄÊ¶±ğ½á¹¹
struct parse_of_dad
{
	id_of_dad *idOfDad;
	quote_of_dad *quoteOfDad;
};

//ÓÃÓÚÊ¶±ğdad,ÓÉ´ËÌá¹©parse_of_dad½á¹¹
class dad_parse_iterator :public parse_iterator<parse_of_dad>
{
public:
	using self_type = dad_parse_iterator;
	dad_parse_iterator() = default;
	dad_parse_iterator(char * _begin, char * _end, parse_of_dad _value):
		parse_iterator<parse_of_dad>(_begin, _end, _value)
	{

	}

	self_type& operator++()
	{
		//Èç¹ûÒÑ¾­µ½end£¬Ôò++²Ù×÷Ê²Ã´¶¼²»×ö
		if (value_.quoteOfDad == nullptr)
			return *this;
		value_.quoteOfDad=(quote_of_dad *)((char *)value_.quoteOfDad + 32);
		if (*(long *)(value_.quoteOfDad) == 0xffffffff) //Èç¹ûÊ±¼äÎª0xffffffff£¬±íÊ¾´ËÊ±½øÈëÏÂÒ»Ö»¹ÉÆ±
		{
			//ÎÄ¼şµÄ×îºó£¬0xffffffff¿ªÊ¼£¬È»ºóÈ«0.»»ÑÔÖ®£¬ÏÂÒ»Ö»¹ÉÆ±£¬µ«idÎª¿Õ£¬±íÊ¾ÎÄ¼ş½áÊø¡£
			//×îºó  FF FF FF FF 00 00 00 00  00 00 00 00 00 00 00 00   ÿÿÿÿ
			//      00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00
			if ((*((char *)value_.quoteOfDad + 4) == 0x00))  //Î²²¿ÊÇ0xffffffffËÄ¸ö×Ö½ÚºóÒ»¶Ñ0.
			{
				value_.idOfDad = nullptr; //ÕâÑùÊÇ·ñÓĞÄÚ´æĞ¹Â©£¿
				value_.quoteOfDad = nullptr;
			}
			else  //´ËÊ±È·¶¨ÎªÏÂÒ»Ö»¹ÉÆ±
			{
				value_.idOfDad = (id_of_dad *)((char *)value_.quoteOfDad + 4);  //ĞŞ¸Äid
				value_.quoteOfDad = (quote_of_dad *)((char *)value_.quoteOfDad + 32);//ÒÆ¶¯32Î»
			}
		}
		return *this;
	};

	//¸²¸Ç£¬»ùÀà²»ÄÜÊÇvirtual
	bool  operator==(const self_type& _next) 
	{
		return (value_.quoteOfDad == _next.value_.quoteOfDad);
	};

	bool  operator!=(const self_type& _next)
	{
		return (value_.quoteOfDad != _next.value_.quoteOfDad);
	};
};


#endif