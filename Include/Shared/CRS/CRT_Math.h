//////////////////////////////////////////////////////////////////////////
///�㷨��ţ����ɽ����
//////////////////////////////////////////////////////////////////////////
#ifndef __CRT_MATH_H__
#define __CRT_MATH_H__

#include<iostream>
#include<cmath>

//using namespace std;
namespace IS_GIS 
{

	class CCRTMath
	{
	public:
		CCRTMath();
		virtual ~CCRTMath();

	public:
		//ţ�ٷ����
		//���û����������߳�����false��
		//yΪ����ֵ��
		//init_x Ϊ��ʼ��xֵ��һ��Ϊmin_x, 
		//min_x��max_x Ϊ-1.0+1e-04��1.0-1e-04, 
		//*outputΪ����ĸ�
		static bool newtonCalRoot(double e, double y, double init_x, double min_x, 
				   double max_x, double *output);
	protected:
		// ����Ϊ:
		// f(x) = ((1+e*x)/(1-e*x))^e * (1-x)/(1+x) - y;
		// ����f(x)����ֵ
		static double calFunValue(double e, double x, double y);
		//f '(x) =  e * [((1+e*x)/(1-e*x))^(e-1)] * [2*e/((1-e*x)^2)] * [(1-x)/(1+x)]
		//		 + [((1+e*x)/(1-e*x))^e] * [-2/((1+x)^2)];
		// ����f(x)һ������ֵ��Ϊ�������Ѿ���������ĺ���ֵ����ʽ�Ѿ������
		static double calDFunValue(double e, double x, double y, double fun_val);
	};

//
////test...
//void main()
//{
//	double e, input_y;
//	double min_x, max_x;
//	double output_x;
//
//	min_x = -1.0 + 1e-04;
//	max_x = 1.0 - 1e-04;
//
//	while(cin >> e >> input_y)
//	{
//		if(e <= 0.0 || e >= 1.0) break;
//		
//		if(!newtonCalRoot(e, input_y, min_x, min_x, max_x, &output_x))
//			break;
//
//		cout.setf(ios::fixed);
//		cout.precision(10);
//		cout << output_x << endl;
//	}
//}
}

#endif// __CRT_MATH_H__
