#include "filter.h"
#include "test.h"

using namespace std;
using namespace cv;

namespace grf
{
	void exTest_GRF_SSE()
	{
		string fname;
		fname = "lena.png";
		//fname = "lena256.png";
		//fname = "lena1024.png";
		//fname = "flower.png";
		//fname = "kodak/kodim04.png";

		const string wname = "image BF SSE";
		namedWindow(wname);

		int r = 3;
		int sr = 16;

		int data = 0;
		createTrackbar("data", wname, &data, 24);
		int impl = 0;
		createTrackbar("implementation", wname, &impl, 4);
		int datatype = 0;
		createTrackbar("8U, 32F, 64F", wname, &datatype, 2);
		int isGray = 0;
		createTrackbar("isGray", wname, &isGray, 1);

		createTrackbar("r", wname, &r, 64);
		createTrackbar("sr", wname, &sr, 128);

		Mat src;
		if (isGray == 1)
		{
			src = imread(fname, 0);
		}
		else
		{
			src = imread(fname, 1);
		}


		Mat dest, ref;
		src.copyTo(dest);
		int d = 2 * r + 1;
#if CV_SSE4_1
		gaussianRangeFilter_Scalar(src, ref, r, sr, BORDER_REPLICATE);
		//gaussianRangeFilter_SSE(src, ref, Size(d, d), (float)sr, FILTER_RECTANGLE, BORDER_REPLICATE, WEIGHT_VECTOR_EXP);
#endif

		int key = 0;
		Stat st;
		ConsoleImage ci(Size(640, 480), "BF SSE");

		while (true)
		{
			//fname = format("kodak/kodim%02d.png",data);
			if (data == 0) fname = "img/lena.png";
			if (data == 1) fname = "img/lena256.png";
			if (data == 2) fname = "img/lena128.png";
			if (data == 3) fname = "img/lena64.png";
			if (data == 4) fname = "img/lena32.png";
			if (data == 5) fname = "img/lena1024.png";

			d = 2 * r + 1;
			if (isGray == 1)
			{
				src = imread(fname, 0);
			}
			else
			{
				src = imread(fname, 1);
			}

			if (datatype == 0)
			{
				src.convertTo(src, CV_8U);
			}
			else if (datatype == 1)
			{
				src.convertTo(src, CV_32F);
			}
			else
			{
				src.convertTo(src, CV_64F);
			}

#if CV_SSE4_1
			gaussianRangeFilter_Scalar(src, ref, r, sr, BORDER_REPLICATE);
			//gaussianRangeFilter_SSE(src, ref, Size(d, d), (float)sr, FILTER_RECTANGLE, BORDER_REPLICATE, WEIGHT_VECTOR_EXP);
#endif

			// test SIMD implementation
			switch (impl)
			{
			case WEIGHT_VECTOR_EXP:
			{
				const string title = " EXP";
				ci((datatype == 0 ? "8U" : (datatype == 1 ? "32F" : "64F")) + title);
#if CV_SSE4_1
				CalcTime t;
				// normal gaussianRange filter
				gaussianRangeFilter_SSE(src, dest, Size(d, d), (float)sr, FILTER_RECTANGLE, BORDER_REPLICATE, WEIGHT_VECTOR_EXP);
				st.push_back(t.getTime());
				ci(t.getTimeString());
#else
				ci("invalid impl");
#endif
				break;
			}
			case WEIGHT_RANGE_LUT_SET:
			{
				const string title = " LUT set";
				ci((datatype == 0 ? "8U" : (datatype == 1 ? "32F" : "64F")) + title);
#if CV_SSE4_1
				CalcTime t;
				// normal gaussianRange filter
				gaussianRangeFilter_SSE(src, dest, Size(d, d), (float)sr, FILTER_RECTANGLE, BORDER_REPLICATE, WEIGHT_RANGE_LUT_SET);
				st.push_back(t.getTime());
				ci(t.getTimeString());
#else
				ci("invalid impl");
#endif
				break;
			}
			case WEIGHT_RANGE_QUANTIZATION_LUT_SET:
			{
				const string title = " quantization LUT set";
				ci((datatype == 0 ? "8U" : (datatype == 1 ? "32F" : "64F")) + title);
#if CV_SSE4_1
				CalcTime t;
				// normal gaussianRange filter
				gaussianRangeFilter_SSE(src, dest, Size(d, d), (float)sr, FILTER_RECTANGLE, BORDER_REPLICATE, WEIGHT_RANGE_QUANTIZATION_LUT_SET);
				st.push_back(t.getTime());
				ci(t.getTimeString());
#else
				ci("invalid impl");
#endif
				break;
			}
			case WEIGHT_RANGE_LUT_GATHER:
			case WEIGHT_RANGE_QUANTIZATION_LUT_GATHER:
			default:
			{
				ci("invalid impl");
				break;
			}
			}

			ci(format("median: %f ", st.getMedian()));
			ci(format("num   : %d ", st.num_data()));
			if (!dest.empty() && !ref.empty())
			{
				ci(format("PSNR: %f ", calcPSNR(dest, ref)));
			}
			ci.flush();


			Mat show;
			dest.convertTo(show, CV_8U);
			imshow(wname, show);

			Mat show_ref;
			ref.convertTo(show_ref, CV_8U);
			imshow("ref", show_ref);

			const Mat show_diff(10 * (show - show_ref));
			imshow("diff", show_diff);


			key = waitKey(1);
			if (key == 'q')
			{
				break;
			}
			if (key == 'r')
			{
				system("clear");
				st.clear();
			}
		}
	}
}
