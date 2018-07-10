////Matrix.h  myMatrix  Class

#ifndef __COUNTSEVENPARAM_H__
#define __COUNTSEVENPARAM_H__

#include <vector>
#include <utility>

#define int_32 int
#define uint_32 unsigned int

const double DOUBLE_ZERO = 0.000000001;//TODO:ͳһ����0�ĸ��㾫��
typedef std::vector<std::vector<double> >  MATRIX;

///////////////////////////////////////////////////////////////////////////////////
//int_32 sweapLine( double ** data, uint_32 startLine, uint_32 N );
//int_32 elimLine( double ** data, uint_32 startLine, uint_32 N );
//int_32 elimination( double ** data, uint_32 startLine, uint_32 N );
//int_32 substituteBack( double ** data, uint_32 startLine, uint_32 N );


//////////////////////////////////////////////////////////////////////////////////
class myMatrix
{
public:
	myMatrix();
	myMatrix(MATRIX matrix);
	virtual ~myMatrix(){}

	int_32 set_Matrix(MATRIX matrix);
	MATRIX get_Matrix();

	//�õ�m_matrix��ת�þ���
	MATRIX get_TransposeMatrix();
	//�õ�m_matrix * matrix�ĳ˻�����
    MATRIX get_multiplyMatrix(MATRIX matrix);

private:
	MATRIX m_matrix;
};

//ʵ�ֽⷽ�̵���
class ComputeEquation
{
public:
	ComputeEquation();
	virtual ~ComputeEquation();

	int_32 set_data(double** m_data);
	double** get_data();
	
	//ƽ�������󷽳�
	int_32 sqrt_computeEquation(int_32 startLine, int_32 N);
	//����Ԫ�ط��󷽳�
	int_32 colm_computeEquation(int_32 startLine, int_32 N);

private:
	int_32 sweapLine(int_32 startLine, int_32 N );
	int_32 elimLine(int_32 startLine, int_32 N );

	int_32 elimination( int_32 startLine, int_32 N );//���ո�˹��Ԫ��������Ϊ�����Ǿ���
	int_32 sqrtElimination( int_32 startLine, int_32 N );	//ƽ������Ԫ���õ������Ǿ���
	int_32 topSubstituteBack( int_32 startLine, int_32 N );	//�����ǻش��󷽳�
	int_32 downSubstituteBack( int_32 startLine, int_32 N );	//�����ǻش��󷽳�
private:
	double** m_data;
};

////////////////////////////////////////////////////////////////////////////////
#endif //__COUNTSEVENPARAM_H__