//////////////////////////////////////////////////////////////////////////////
// Main function of temporal adaptive scaling
//
/////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <math.h>

#include "../Common/video.h"
#include "../Common/utils.h"

double *MotionEnergy(Video *src, double threshold)
{
#ifdef USE_TRACEBACK
	Trace->Add(__FILE__, __LINE__);
#endif
	double *total_dt = new double[src->GetTime()];

	Matrix *tgradient = NULL;
	for (int t=0; t<src->GetTime(); t++)
	{
		//results[t] = *(new Matrix(0,0));
		// results[t] = *(Gradient(src->GetFrame(t)));
		if (tgradient)
			delete tgradient;
		
		double tdt = 0.0;
		if (t>0 && t<(src->GetTime()-1))
		{			
			//results[t] += *(FrameDifference(src->GetFrame(t-1),src->GetFrame(t+1)));
			tgradient = FrameDifference(src->GetFrame(t-1),src->GetFrame(t+1),tdt,threshold);
		}
		else
		{
			int lower_t = max(t-1,0);
			int upper_t = min(t+1,src->GetTime()-1);
			//results[t] += *(FrameDifference(src->GetFrame(lower_t),src->GetFrame(upper_t)));
			tgradient = FrameDifference(src->GetFrame(lower_t),src->GetFrame(upper_t),tdt,threshold);
		}

		//total_dt[t] = tdt;

		double std = 0.0;
		double mean = tdt/(tgradient->NumOfRows()*tgradient->NumOfCols());
		for (int x = 0; x < tgradient->NumOfRows(); x++)
		{
			for (int y = 0; y < tgradient->NumOfCols(); y++)
			{
				std += pow(tgradient->Get(x+1,y+1)-mean,2.0);
			}
		}
		std /= tgradient->NumOfRows()*tgradient->NumOfCols()-1;
		std = sqrt(std);
		total_dt[t] = std*tdt;
		
	}

	return total_dt;
}

double *SmoothMotionEnergy(double *energy, int time, int window_size)
{
	double *result = new double[time];

	window_size = 11;
	//double weight[5] = { 0.05, 0.25, 0.4, 0.25, 0.05 };
	double weight[11] = { 1,1,1,1,1,1,1,1,1,1,1 };

	for (int t = 0; t < time; t++)
	{
		double sEnergy = 0.0;
		for (int m = -floor((double)(window_size/2)); m < floor((double)(window_size/2))+1; m++)
		{
			int t_idx = t+m;
			if (t_idx < 0)
				t_idx = abs(t_idx);
			if (t_idx >= time)
				t_idx = t_idx-abs(time-t_idx);

			sEnergy += energy[t_idx];
		}

		result[t] = sEnergy/11;
	}

	return result;
}

void AdaptiveTemporalReduce(Video *src, double *gradient, int time, char *output_folder)
{
	double ratio = (src->GetTime() / time);
	int *selected = new int[src->GetTime()];
	for (int t = 0; t < src->GetTime(); t++)
		selected[t] = 0;

	double *acMotion = NULL;
	double *mapping = NULL;
	int *left_idices = NULL;
	int shot_num = src->GetTime();
	int left_time;
	while (shot_num>0)
	{
		shot_num = 0;
		left_time = time;
		
		int left_frames = 0;		
		for (int t = 0; t < src->GetTime(); t++)
		{
			if (selected[t]==0)
				left_frames++;
		}

		if (left_idices!=NULL)
			delete [] left_idices;
		left_idices = new int[left_frames];		

		if (acMotion!=NULL)
			delete [] acMotion;
		acMotion = new double[left_frames];
		int cur_frame = 0;
		for (int t = 0; t < src->GetTime(); t++)
		{
			if (selected[t]==0)
			{
				left_idices[cur_frame] = t;
				
				if (cur_frame==0)
					acMotion[cur_frame] = gradient[t];
				else
					acMotion[cur_frame] = acMotion[cur_frame-1]+gradient[t];

				cur_frame++;
			}
			else
			{
				left_time--;
			}			
		}
	
		if (mapping!=NULL)
			delete [] mapping;
		mapping = new double[left_frames];
		for (int t = 0; t < left_frames; t++)
		{
			mapping[t] = (acMotion[t]-acMotion[0])/
							(acMotion[left_frames-1]-acMotion[0])*left_time;


			double bound = ceil(mapping[t-1])+1+((ceil(mapping[t-1])-mapping[t-1])+1);
			if (mapping[t]>bound)
			{
				selected[left_idices[t]] = 1;
				shot_num++;
			}
		}

	}

	delete [] acMotion;

	cout << "Reducing video from " << src->GetTime() 
		 << " frames to " << time << " frames..." << endl;
	
	int *frame_ids_left = new int[left_time];
	frame_ids_left[0] = left_idices[0];
	int last_sid = 0;
	
	for (int t = 1; t < left_time; t++)
	{
		for (int tt = last_sid+1; tt < src->GetTime()-time+left_time; tt++)
		{
			if (mapping[tt]==t)
			{
				frame_ids_left[t] = left_idices[tt];
				last_sid = tt-1;
				break;
			}
			if (mapping[tt]>t)
			{
				/*
				double lweight = t-mapping[tt-1];
				double rweight = mapping[tt]-t;
				
				frames[t] = *(InterpolateFrame(src->GetFrame(tt-1),
											   lweight/(lweight+rweight),
											   src->GetFrame(tt),
											   rweight/(lweight+rweight)));

			    last_sid = tt-2;
			    */
				
				if (t-mapping[tt-1]<=mapping[tt]-t)
				{
					frame_ids_left[t] = left_idices[tt-1];
					last_sid = tt-1;
				} else
				{
					frame_ids_left[t] = left_idices[tt];
					last_sid = tt;
				}

				break;
				
			}
		}
	}

	delete [] left_idices;
	delete [] mapping;

	int *frame_ids_selected = new int[time-left_time];
	int cur_frame = 0;
	for (int t = 0; t < src->GetTime(); t++)
	{
		if (selected[t]>0)
		{
			frame_ids_selected[cur_frame] = t;
			cur_frame++;
		}
	}

	delete [] selected;

	int cur_left = 0;
	int cur_selected = 0;
	Picture *frame;
	for (int t = 0; t < time; t++)
	{
		char framename[512] = {'\0'};
		char buf[512] = {'\0'};
		strcat(framename,output_folder);
		strcat(framename,itoa(t,buf,10));
		strcat(framename,".ppm");
	 
		if ((frame_ids_left[cur_left]<frame_ids_selected[cur_selected]) &&
			(cur_left<left_time && cur_selected<time-left_time))
		{
			frame = src->GetFrame(frame_ids_left[cur_left]);
			cur_left++;
		}
		else if ((frame_ids_left[cur_left]>frame_ids_selected[cur_selected]) &&
			(cur_left<left_time && cur_selected<time-left_time))
		{
			frame = src->GetFrame(frame_ids_selected[cur_selected]);
			cur_selected++;
		}
		else if ((cur_left<left_time) && cur_selected==time-left_time)
		{
			frame = src->GetFrame(frame_ids_left[cur_left]);
			cur_left++;
		}
		else if ((cur_left==left_time) && cur_selected<time-left_time)
		{
			frame = src->GetFrame(frame_ids_selected[cur_selected]);
			cur_selected++;
		}

		frame->Save(framename);
	}

	delete [] frame_ids_left;
	delete [] frame_ids_selected;
}


int main(int argc, char **argv)
{
	Video *input = NULL;
	int width, height, time;

	if (argc<4)
	{
		cout << "Usage: t_adaptive_scaling <input_folder> <ratio> <output_folder>" << endl;
		return 0;
		//default parameters
	}
	double ratio = atof(argv[2]);

	// load input video
	cout << "Loading input video..." << endl;
	input = new Video(argv[1]);	
	//vpyramid = VideoPyramid(input);

	// simple calculation of motion energy of every frame
	double *gradient = MotionEnergy(input,0.0);
	//double *energy = SmoothMotionEnergy(gradient,input->GetTime(),5);

	cout << "Adaptively scaling video ..." << endl;
	AdaptiveTemporalReduce(input, gradient, ceil(input->GetTime()*ratio),argv[3]);

	delete gradient;
	delete input;
}