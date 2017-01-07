#include "Shape.h"

Shape::Shape() :
	centroid(cv::Point()),
	ID(-1),
	lastFrameSeen(-1),
	matchFound(false),
	moving(false),
	selected(false),
	movementCounter(0),
	background(false),
	stillness(0),
	motion(0.0f)
{
}



