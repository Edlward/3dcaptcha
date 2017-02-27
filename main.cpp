#include <stdio.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <string>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

using namespace std;
using namespace cv;

//temp������0Ϊ��ʼ����1Ϊ�߽��ַ���2Ϊ��׼������,3Ϊ�ַ�����
void initTemp(Mat& src)
{
	for (int i = 0; i < src.rows; i++)
	{
		for (int j = 0; j < src.cols; j++)
		{
			src.at<uchar>(i, j) = 0;
		}
	}
}
//ѡ������ı�����
//��ȡ�ַ��߽�
void selectTextFirst(Mat& src, Mat& dst,Mat& temp)
{
	for (int i = 1; i < src.rows-1; i++)
	{
		const uchar * previous = src.ptr<const uchar>(i - 1); // ��ǰ�е���һ��
		const uchar * current = src.ptr<const uchar>(i); //��ǰ��
		const uchar * next = src.ptr<const uchar>(i + 1); //��ǰ�е���һ��
		for (int j = 1; j < src.cols-1; j++)
		{
			//ѡ��P��
			//���õ�P������ֵΪ0(����ɫ)
			if ((int)current[j] == 0)
			{
				//����������������3�����ص�Ϊ��ɫ,��������
				int domain[4];
				domain[0] = previous[j];
				domain[1] = next[j];
				domain[2] = current[j - 1];
				domain[3] = current[j + 1];

				if ((domain[0] == 0 && domain[1] == 0 && domain[2] == 0) || (domain[0] == 0 && domain[1] == 0 && domain[3] == 0)
					|| (domain[0] == 0 && domain[2] == 0 && domain[3] == 0) || (domain[1] == 0 && domain[2] == 0 && domain[3] == 0) ||
					(domain[0] == 0 && domain[1] == 0 && domain[2] == 0 && domain[3] == 0))
				{
					//�򽫵�P��Ϊ�ı��ϵ����أ��ҽ�����Ϊ��ɫ
					dst.at<cv::Vec3b>(i, j)[0] = 255;
					dst.at<cv::Vec3b>(i, j)[1] = 0;
					dst.at<cv::Vec3b>(i, j)[2] = 0;
					temp.at<uchar>(i, j) = 1;

					if (domain[0] == 0)
					{
						//�򽫵�P��Ϊ�ı��ϵ����أ��ҽ�����Ϊ��ɫ
						dst.at<cv::Vec3b>(i-1, j)[0] = 255;
						dst.at<cv::Vec3b>(i-1, j)[1] = 0;
						dst.at<cv::Vec3b>(i-1, j)[2] = 0;

						temp.at<uchar>(i-1, j) = 1;
					}
					if (domain[1] == 0)
					{
						//�򽫵�P��Ϊ�ı��ϵ����أ��ҽ�����Ϊ��ɫ
						dst.at<cv::Vec3b>(i + 1, j)[0] = 255;
						dst.at<cv::Vec3b>(i + 1, j)[1] = 0;
						dst.at<cv::Vec3b>(i + 1, j)[2] = 0;

						temp.at<uchar>(i+1, j) = 1;

					}
					if (domain[2] == 0)
					{
						//�򽫵�P��Ϊ�ı��ϵ����أ��ҽ�����Ϊ��ɫ
						dst.at<cv::Vec3b>(i, j-1)[0] = 255;
						dst.at<cv::Vec3b>(i, j-1)[1] = 0;
						dst.at<cv::Vec3b>(i, j-1)[2] = 0;

						temp.at<uchar>(i, j-1) = 1;

					}
					if (domain[3] == 0)
					{
						//�򽫵�P��Ϊ�ı��ϵ����أ��ҽ�����Ϊ��ɫ
						dst.at<cv::Vec3b>(i, j+1)[0] = 255;
						dst.at<cv::Vec3b>(i, j+1)[1] = 0;
						dst.at<cv::Vec3b>(i, j+1)[2] = 0;

						temp.at<uchar>(i, j+1) = 1;
					}
				}
			}
		}
	}
}

//����ȥ��
void dislodgeBackGround(Mat& src, Mat& temp, Mat& dst1, Mat& dst2, int T)
{
	const int channels = src.channels();
	//��׼����ȡ
	for (int i = 1; i < src.rows - 1; i++)
	{
		const uchar * previous = src.ptr<const uchar>(i - 1); // ��ǰ�е���һ��
		const uchar * current = src.ptr<const uchar>(i); //��ǰ��
		const uchar * next = src.ptr<const uchar>(i + 1); //��ǰ�е���һ��
		for (int j = 1; j < src.cols - 1; j++)
		{
			//����������֤��ͼ�еĺ�ɫ���أ����õ㲻������3.3.1��������ȡ���ַ��߽磬������Ϊ��P
			if ((int)current[j] == 0 && (int)temp.at<uchar>(i, j) != 1)
			{
				int P[9];
				P[0] = current[j];
				P[1] = previous[j - 1];
				P[2] = previous[j];
				P[3] = previous[j + 1];
				P[4] = current[j + 1];
				P[5] = next[j + 1];
				P[6] = next[j];
				P[7] = next[j - 1];
				P[8] = current[j - 1];

				//��P2��P3��P6��P7��������ĸ�λ�ö��ǰ�ɫ
				if (P[2] == 255 && P[3] == 255 && P[6] == 255 && P[7] == 255)
				{
					//��P4��P5λ��ֻ��һ�������к�ɫ���أ�P1��P8λ��Ҳֻ��һ�������к�ɫ����
					if (((P[4] == 0 && P[5] != 0) || (P[4] != 0 && P[5] == 0)) &&
						((P[1] == 0 && P[8] != 0) || (P[1] != 0 && P[8] == 0)))
					{
						//�򽫵�P��Ϊ��׼���ϵ����أ��ҽ�����Ϊ��ɫ
						dst1.at<Vec3b>(i, j)[0] = 0;
						dst1.at<Vec3b>(i, j)[1] = 255;
						dst1.at<Vec3b>(i, j)[2] = 0;

						temp.at<uchar>(i, j) = 2;
					}
				}
				else
				{
					//��P3��P5ͬʱΪ��ɫ����P1��P7ͬʱΪ��ɫ
					if ((P[3] == 0 && P[5] == 0) || (P[1] == 0 && P[7] == 0))
					{
						//�򽫵�P��Ϊ��׼���ϵ����أ��ҽ�����Ϊ��ɫ
						dst1.at<Vec3b>(i, j)[0] = 0;
						dst1.at<Vec3b>(i, j)[1] = 255;
						dst1.at<Vec3b>(i, j)[2] = 0;

						temp.at<uchar>(i, j) = 2;
					}
				}
			}
		}
	}

	dst1.copyTo(dst2);

	//�������
	for (int i = 0; i < src.rows; i++)
	{
		for (int j = 0; j < src.cols; j++)
		{
			//��ͬһ����������׼���ϵĵ�֮��ľ���D<T
			int D = 1000;
			if ((int)temp.at<uchar>(i, j) == 2)
			{
				int first=j;
				int second=0; 
				bool isBack = true;
				for (int k = j+1; k < (j + T) && k<src.cols; k++)
				{
					if ((int)temp.at<uchar>(i, k) == 2)
					{
						second = k;
						break;
					}
				}
				//������֮��û�к�ɫ���ػ��ַ��߽�����
				if (second<(j + T) && second<src.cols && second>first)
				{
					for (int k = first+1; k < second; k++)
					{
						//if (((int)dst1.at<Vec3b>(i, k)[0] == 0 && (int)dst1.at<Vec3b>(i, k)[1] == 0 && (int)dst1.at<Vec3b>(i, k)[2] == 0) 
						//	|| (int)temp.at<uchar>(i, k) == 1)
						if ((int)src.at<uchar>(i, k) == 0 && (int)temp.at<uchar>(i, k) == 1)
						{
							isBack = false;
							break;
						}
					}
				}
				else
				{
					isBack = false;
				}
				//��������֮������е����ض����Ϊͼ�񱳾�,��Ϊ��ɫ
				if (isBack == true)
				{
					for (int k = first; k <= second; k++)
					{
						dst2.at<Vec3b>(i, k)[0] = 96;
						dst2.at<Vec3b>(i, k)[1] = 96;
						dst2.at<Vec3b>(i, k)[2] = 96;
					}
				}
			}
		}
	}
}

//ȥ��
void denoise(Mat& src, Mat &dst)
{
	morphologyEx(src, dst, MORPH_OPEN, NULL,Point(-1,-1),5);
}

//�ַ���ȡ,������ȡ
//templateSize���ڶ����ж��ַ��Ƿ�Ϊ�߽�ʱʹ�õ�ģ�巶Χ
//morphSize����̬ѧ�ղ����ĺ˴�С
//loDiffΪ��ˮ���ʱ����ʾ��ǰ�۲�����ֵ���䲿����������ֵ���ߴ�����ò�������������֮������Ȼ���ɫ֮���lower brightness/color difference�������ֵ��  
//upDiffΪ��ˮ���ʱ����ʾ��ǰ�۲�����ֵ���䲿����������ֵ���ߴ�����ò�������������֮������Ȼ���ɫ֮���lower brightness/color difference�������ֵ��
void extractCharacter(Mat& src, Mat& temp, Mat& dst1,Mat& dst2, int T1, int templateSize,int morphSize,int loDiff,int upDiff)
{
	src.copyTo(dst1);

	//��ˮ����㷨
	//���߽�
	Rect ccomp;
	//floodFill(dst, Point(0, 0), Scalar(96, 96, 96),&ccomp);
	//floodFill(dst, Point(dst.cols / 2, dst.rows / 2), Scalar(96, 96, 96), &ccomp);
	floodFill(dst1, Point(0, 0), Scalar(96, 96, 96), &ccomp, Scalar(loDiff, loDiff, loDiff), Scalar(upDiff, upDiff, upDiff));
	//floodFill(dst, Point(dst.cols/2, dst.rows/2), Scalar(96, 96, 96), &ccomp, Scalar(20, 20, 20), Scalar(20, 20, 20));


	//������ȡ�ַ����棬��Ϊȥ���ַ��߽�����������
	//�����ַ������ϵĿ����Ϊ��ɫ
	for (int i = 0; i < src.rows; i++)
	{
		for (int j = 0; j < src.cols; j++)
		{
			if ((int)dst1.at<Vec3b>(i, j)[0] == 0 && (int)dst1.at<Vec3b>(i, j)[1] == 0 && (int)dst1.at<Vec3b>(i, j)[2] == 0)
			{
				dst1.at<Vec3b>(i, j)[0] = 255;
				dst1.at<Vec3b>(i, j)[1] = 255;
				dst1.at<Vec3b>(i, j)[2] = 255;
			}
		}
	}

	//ˮƽ�������ͼ����ͬһ��������Ϊ��ɫ���ص������� �����Ƚ���������Ϊ�ַ����档
	for (int i = 0; i < src.rows; i++)
	{
		for (int j = 0; j < src.cols; j++)
		{
			if (dst1.at<Vec3b>(i, j)[0] == 255 && dst1.at<Vec3b>(i, j)[1] == 255 && dst1.at<Vec3b>(i, j)[2] == 255)
			{
				int first = j;
				int second;
				bool isCharacter = true;
				for (int k = first + 1; k <= (first + T1) &&k<src.cols; k++)
				{
					if (dst1.at<Vec3b>(i, k)[0] != 255 || dst1.at<Vec3b>(i, k)[1] != 255 || dst1.at<Vec3b>(i, k)[2] != 255)
					{
						isCharacter = false;
						break;
					}
				}
				if (isCharacter)
				{
					for (int k = first; k <= (first + T1)&& k<=src.cols; k++)
					{
						temp.at<uchar>(i, k) = 3;
						
						/*
						dst.at<cv::Vec3b>(i, k)[0] = 0;
						dst.at<cv::Vec3b>(i, k)[1] = 0;
						dst.at<cv::Vec3b>(i, k)[2] = 255;
						*/
					}
				}
			}
		}
	}

	dst1.copyTo(dst2);
	//������ȡ�߽�
	for (int i = templateSize; i < temp.rows - templateSize; i++)
	{
		for (int j = templateSize; j < temp.cols - templateSize; j++)
		{
			if ((int)temp.at<uchar>(i, j) == 1)
			{
				bool isCharacterEdge = false;
				//�жϵ�A�����Ƿ�����ַ���������
				for (int n = i - templateSize; n < (i + templateSize); n++)
				{
					for (int m = j - templateSize; m < (j + templateSize); m++)
					{
						if ((int)temp.at<uchar>(n, m) == 3)
						{
							isCharacterEdge = true;
							break;
						}
					}
				}
				
				//������ڣ�ȷ�ϵ�A�����ַ��߽�
				if (isCharacterEdge)
				{
					(int)temp.at<uchar>(i, j) == 1;
					dst1.at<cv::Vec3b>(i, j)[0] = 255;
					dst1.at<cv::Vec3b>(i, j)[1] = 0;
					dst1.at<cv::Vec3b>(i, j)[2] = 0;

					dst2.at<cv::Vec3b>(i, j)[0] = 255;
					dst2.at<cv::Vec3b>(i, j)[1] = 255;
					dst2.at<cv::Vec3b>(i, j)[2] = 255;
				}
				//�����A����ͼ�񱳾���
				else
				{
					(int)temp.at<uchar>(i, j) == 0;
					dst1.at<cv::Vec3b>(i, j)[0] = 96;
					dst1.at<cv::Vec3b>(i, j)[1] = 96;
					dst1.at<cv::Vec3b>(i, j)[2] = 96;

					dst2.at<cv::Vec3b>(i, j)[0] = 96;
					dst2.at<cv::Vec3b>(i, j)[1] = 96;
					dst2.at<cv::Vec3b>(i, j)[2] = 96;
				}
			}
		}
	}
	//��ͼ��תΪ�������ַ�
	for (int i = 0; i < temp.rows; i++)
	{
		for (int j = 0; j < temp.cols; j++)
		{
			if ((int)temp.at<uchar>(i, j) == 2 )
			{
				dst1.at<cv::Vec3b>(i, j)[0] = 96;
				dst1.at<cv::Vec3b>(i, j)[1] = 96;
				dst1.at<cv::Vec3b>(i, j)[2] = 96;

				dst2.at<cv::Vec3b>(i, j)[0] = 96;
				dst2.at<cv::Vec3b>(i, j)[1] = 96;
				dst2.at<cv::Vec3b>(i, j)[2] = 96;
			}
		}
	}

	//��ֵ�˲�
	medianBlur(dst2, dst2, 5);

	//��̬ѧ�˲�,�ղ���
	Mat element = getStructuringElement(MORPH_RECT, Size(morphSize, morphSize));
	//morphologyEx(dst2, dst2, MORPH_OPEN, element);//������
	morphologyEx(dst2, dst2, MORPH_CLOSE, element);//������
	//morphologyEx(dst2, dst2, MORPH_GRADIENT, element);//��̬ѧ�ݶ�
	//morphologyEx(dst2, dst2, MORPH_ERODE, element);//��ʴ
	//morphologyEx(dst2, dst2, MORPH_DILATE, element);//����

	//��ֵ��
	threshold(dst2, dst2, 128, 255, cv::THRESH_BINARY);
}



int main(int argc, char** argv)
{
	Mat image = imread(".././data/~E5]P7]DGBB$OE6(A(5D3VC.png ");//[D3B1L@W31W7U)BJQI$0SCL.png  `QYGD2HE)[P}LS1FQ~)8M1F.png  }`696BF9SVM`HK$1THWA[KK.png  %N@GM8%XY7DTDY]YQ1`}U6A.png {71ZW78PSEXN}T(04OIMAGK.png  D)`}O9B4L07E`(59(PL35]H.png 
	Mat gray;
	Mat binImage;
	Mat temp(image.size(),CV_8U);
	Mat FirstImage,SecondImage,ThirdImage,FourthImgae,FifthImage;

	int T = 8, T1 = 6, templateSize = 5, morphSize=3,loDiff=220,upDiff=140;

	if(image.empty())
	{
		cout<<"image empty!"<<endl;
		return 0;
	}

	//��ʼ�����þ���
	initTemp(temp);

	//�ҶȻ�
	cvtColor(image, gray, CV_BGR2GRAY);
	//��ֵ��
	threshold(gray, binImage, 128, 255, cv::THRESH_BINARY);

	//ѡ������ı�����
	////��ȡ�ַ��߽�
	image.copyTo(FirstImage);
	selectTextFirst(binImage, FirstImage,temp);

	//����ȥ��
	FirstImage.copyTo(SecondImage);
	dislodgeBackGround(binImage, temp, SecondImage, ThirdImage,T);

	//�ַ���ȡ
	extractCharacter(ThirdImage, temp, FourthImgae, FifthImage, T1, templateSize, morphSize, loDiff, upDiff);

	
	//��ʾͼ��
	imshow("����ԭͼ", image);
	imshow("�ַ��߽�������", FirstImage);
	imshow("��׼����ȡ", SecondImage);
	imshow("�������", ThirdImage);
	imshow("������ȡ�ַ��߽�", FourthImgae);
	imshow("�ַ���ȡ�������", FifthImage);
	waitKey(0);


	return 0;
}