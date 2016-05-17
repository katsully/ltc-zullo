#pragma once
#include "CinderOpenCV.h"
class Shape
{
public:
	Shape();
	
	int ID;
	double area;
	float depth;
	cv::Point centroid;	// center point of the shape
	bool matchFound;
	bool moving;
	bool background;
	cinder::Color color;
	int stillness;
	float motion;
	cv::vector<cv::Point> hull;	// stores point representing the hull of the shape
	int lastFrameSeen;
};

