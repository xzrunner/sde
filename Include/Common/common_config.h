#ifndef IS_GIS_COMMON_CONFIG_H_
#define IS_GIS_COMMON_CONFIG_H_

#pragma warning(disable: 4018) // signed/unsigned mismatch
#pragma warning(disable: 4267) // conversion from size_t to long, etc
#pragma warning(disable: 4244) // conversion from double to int, etc
#pragma warning(disable: 4099) // type name declare mismatch, struct/class, etc
#pragma warning(disable: 4010) // comment containing back slash
#pragma warning(error:	 4715) // not all control paths return a value
#pragma warning(error:	 4150) // deletion of pointer to incomplete type(no destructor called)
#pragma warning(error:	 4305) // truncation from 'int' to 'bool'
#pragma warning(error:	 4706) // assignment within conditional expression 'if(a = b)...'
#pragma warning(error:	 4390) // empty controlled statement found; is this the intent?
#pragma warning(error:	 4056) // Floating-point constant arithmetic generates a result that exceeds the maximum allowable value
#pragma warning(default: 4061) // The enumerate has no associated handler in a switch statement
#pragma warning(default: 4062) // same as above, when there is no 'default'
//4191, 4217
//#pragma warning(3:		 4242) // conversion from 'type1' to 'type2', possible loss of data
//4254, 4255, 
#pragma warning(default: 4263) // A class function definition has the same name as a virtual function in a base class but not the same number or type of arguments. This effectively hides the virtual function in the base class.
#pragma warning(default: 4264) // see above
#pragma warning(default: 4265) // class has virtual functions, but destructor is not virtual
//4287, 
#pragma warning(error:	 4288) // nonstandard extension used : loop control variable declared in the for-loop is used outside the for-loop scope; it conflicts with the declaration in the outer scope
#pragma warning(error:	 4289) // loop control variable declared in the for-loop is used outside the for-loop scope
#pragma warning(default: 4296) //* expression is always false or true, An unsigned variable was used in a comparison operation with zero.
#pragma warning(default: 4302) // 'conversion' : truncation from 'type 1' to 'type 2'
//4339, 4347, 4514, 4529, 4545, 4546, 4547, 4548, 4549, 4536, 4555, 
#pragma warning(default: 4557) // '__assume' contains effect 
//#pragma warning(3:		 4619) // #pragma warning : there is no warning number 'number'
//#pragma warning(error:	 4623) // 'derived class' : default constructor could not be generated because a base class default constructor is inaccessible
//#pragma warning(error:	 4625) // 'derived class' : copy constructor could not be generated because a base class copy constructor is inaccessible
//#pragma warning(error:	 4626) // 'derived class' : operator = ...(see above)
// 4628, 
#pragma warning(default: 4640) // 'instance' : construction of local static object is not thread-safe
#pragma warning(default: 4668) // δ����ķ�����Ԥ������ָ��һ��ʹ�á����Ž�����Ϊ�١�
// 4682, 4686, 4710, 4786, 4793, 
#pragma warning(default: 4820) // Ԫ�ص����ͺ�˳��ʹ��������ṹ��β�������䡣
#pragma warning(default: 4905) // ���ַ���ǿ��ת��Ϊ��LPSTR��
#pragma warning(default: 4906) // �ַ���ǿ��ת��Ϊ��LPWSTR��
// 4917, 4928, 4931, 
#pragma warning(default: 4946) // reinterpret_cast �������֮��ʹ��:��class1���͡�class2��

#pragma warning(disable: 4996) // function was declared deprecated

#define MACRO_NUM_CONV_2_STR2(num) #num
#define MACRO_NUM_CONV_2_STR(num) MACRO_NUM_CONV_2_STR2(num)
#define CMPL_MESSAGE(desc) message(__FILE__ "( " MACRO_NUM_CONV_2_STR(__LINE__) "): " "warning: "  #desc)

#endif //IS_GIS_COMMON_CONFIG_H_