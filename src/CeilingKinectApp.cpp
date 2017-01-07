#include "cinder/app/App.h"
#include "cinder/params/Params.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "Kinect2.h"
#include "CinderOpenCV.h"
#include "Shape.h"

#include<math.h>

using namespace ci;
using namespace ci::app;
using namespace std;


class CeilingKinectApp : public App {
  public:

	void setup() override;
	void prepareSettings(Settings* settings);
	void keyDown(KeyEvent event) override;
	void update() override;
	void draw() override;

	// all pixels below near limit and above far limit are set to far limit depth
	short mNearLimit;
	short mFarLimit;

	// threshold for the camera
	double mThresh;
	double mMaxVal;

	// colors for the shapes
	vector<Color> shapeColors;
	int colorIdx;

private:
	Kinect2::DeviceRef mDevice;

	ci::Channel8uRef mChannelBodyIndex;
	ci::Channel16uRef mChannelDepth;
	ci::Channel16uRef mChannelDepth2;
	ci::Channel16uRef mChannelInfrared;
	ci::Surface8uRef mSurfaceColor;

	float mFrameRate;
	bool mFullScreen;
	ci::params::InterfaceGlRef mParams;

	cv::Mat mInput;

	ci::Surface8u mSurface;
	ci::Surface8u mSurfaceDepth;
	ci::Surface8u mSurfaceDepth2;
	ci::Surface8u mSurfaceBlur;
	ci::Surface8u mSurfaceSubtract;
	gl::TextureRef mTexture;
	gl::TextureRef mTextureDepth;
	gl::TextureRef mTextureDepth2;

	cv::Mat mPreviousFrame;
	cv::Mat mBackground;

	typedef vector< vector<cv::Point> > ContourVector;
	ContourVector mContours;
	ContourVector mApproxContours;
	int mStepSize;
	int minArea;
	int mBlurAmount;
	int shapeUID;

	cv::vector<cv::Vec4i> mHierarchy;
	vector<Shape> mShapes;
	vector<Shape> mTrackedShapes;	// store tracked shapes

	ivec2 mCurrentMousePosition;

	cv::Mat removeBlack(cv::Mat input, short nearLimit, short farLimit);
	vector<Shape> getEvaluationSet(ContourVector rawContours, int minimalArea, int maxArea);
	Shape* findNearestMatch(Shape trackedShape, vector<Shape> &shapes, float maximumDistance);
	float mapNum(float value, float istart, float istop, float ostart, float ostop);
};

void CeilingKinectApp::prepareSettings(Settings* settings)
{
	settings->setFrameRate(60.0f);
	settings->setWindowSize(800, 800);
}

void CeilingKinectApp::setup()
{
	console() << "x window size " << getWindowSize().x << endl;
	console() << "y windows size " << getWindowSize().y << endl;

	mFrameRate = 0.0f;
	mFullScreen = false;
	minArea = 190;

	mDevice = Kinect2::Device::create();
	mDevice->start();
	mDevice->connectBodyIndexEventHandler([&](const Kinect2::BodyIndexFrame& frame)
	{
		mChannelBodyIndex = frame.getChannel();
	});
	mDevice->connectColorEventHandler([&](const Kinect2::ColorFrame& frame)
	{
		mSurfaceColor = frame.getSurface();
	});
	mDevice->connectDepthEventHandler([&](const Kinect2::DepthFrame& frame)
	{
		mChannelDepth = frame.getChannel();
	});
	mDevice->connectInfraredEventHandler([&](const Kinect2::InfraredFrame& frame)
	{
		mChannelInfrared = frame.getChannel();
	});

	mParams = params::InterfaceGl::create("Params", ivec2(255, 200));
	mParams->addParam("Thresh", &mThresh, "min=0.0f max=255.0f step=1.0 keyIncr=a keyDecr=s");
	mParams->addParam("Maxval", &mMaxVal, "min=0.0f max=255.0f step=1.0 keyIncr=q keyDecr=w");
	mParams->addParam("Min Area", &minArea, "min=0.0f max=200.0f step=1.0 keyIncr=k keyDecr=l");
	//mParams->addParam("Max", &mFarLimit, "min=0.0f max=4000.0f step=10.0 keyIncr=p keyDecr=o");
	mStepSize = 10;
	mBlurAmount = 10;

	shapeUID = 0;

	// set the threshold to ignore all black pixels and pixels that are far away from the camera
	mNearLimit = 30;
	mFarLimit = 2446;
	mThresh = 10.0;
	mMaxVal = 255.0;

	// colors for each shape
	shapeColors.push_back(Color(1, 0, 0));	// red
	shapeColors.push_back(Color(1, 1, 0));	// yellow
	shapeColors.push_back(Color(0, 1, 0));	// green
	shapeColors.push_back(Color(0, 0, 1));	// blue
	shapeColors.push_back(Color(1, 0, 1));	// purple
	shapeColors.push_back(Color(1, 1, 1));	// white

	colorIdx = 0;
}

void CeilingKinectApp::update()
{
	if (mChannelDepth) {
		mInput = toOcv(mChannelDepth->clone(true));

		cv::Mat thresh;
		cv::Mat eightBit;
		cv::Mat withoutBlack;

		// remove black pixels from frame which get detected as noise
		withoutBlack = removeBlack(mInput, mNearLimit, mFarLimit);

		// convert matrix from 16 bit to 8 bit with some color compensation
		withoutBlack.convertTo(eightBit, CV_8UC3, 0.1 / 1.0);

		// invert the image
		cv::bitwise_not(eightBit, eightBit);

		mContours.clear();
		mApproxContours.clear();

		// using a threshold to reduce noise
		cv::threshold(eightBit, thresh, mThresh, mMaxVal, CV_8U);

		// draw lines around shapes
		cv::findContours(thresh, mContours, mHierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

		vector<cv::Point> approx;
		// approx number of points per contour
		for (int i = 0; i < mContours.size(); i++) {
			cv::approxPolyDP(mContours[i], approx, 3, true);
			mApproxContours.push_back(approx);
		}

		mShapes.clear();
		// get data that we can later compre
		mShapes = getEvaluationSet(mApproxContours, minArea, 100000);

		// find the nearest match for each shape
		for (Shape &s: mTrackedShapes) {
			Shape* nearestShape = findNearestMatch(s, mShapes, 9000);

			// if a tracked shape was found, update that tracked shape with the new shape
			if (nearestShape != NULL) {
				nearestShape->matchFound = true;
				s.centroid = nearestShape->centroid;
				s.lastFrameSeen = ci::app::getElapsedFrames();
				s.hull.clear();
				s.hull = nearestShape->hull;
				if (nearestShape->moving) {
					s.movementCounter++;
					if (s.movementCounter > 5) {
						s.background = false;
					}
				}
			}
		}

		// if shape->matchFound is false, add it as a new shape
		for (int i = 0; i < mShapes.size(); i++) {
			if (mShapes[i].matchFound == false) {
				// assign it an unique ID
				mShapes[i].ID = shapeUID;
				mShapes[i].lastFrameSeen = ci::app::getElapsedFrames();
				mShapes[i].color = shapeColors[colorIdx];
				mShapes[i].background = false;
				colorIdx++;
				// add this new shape to tracked shapes
				mTrackedShapes.push_back(mShapes[i]);
				//ci::app::console() << "NEW SHAPE" << endl;
				shapeUID++;
			}
		}

		// if we didn't find a match for x frames, delete the tracked shape
		for (vector<Shape>::iterator it = mTrackedShapes.begin(); it != mTrackedShapes.end();) {
			if (ci::app::getElapsedFrames() - it->lastFrameSeen > 20) {
				 // remove the tracked shape
				it = mTrackedShapes.erase(it);
				//cinder::app::console() << "ERASE IT BLAHHH" << endl;
			}
			else {
				++it;
			}
		}

		cv::Mat gray8Bit;
		withoutBlack.convertTo(gray8Bit, CV_8UC3, 0.1 / 1.0);

		mSurfaceDepth = Surface8u(fromOcv(mInput));
		mSurfaceBlur = Surface8u(fromOcv(withoutBlack));
		mSurfaceSubtract = Surface8u(fromOcv(eightBit));
	}
}

void CeilingKinectApp::draw()
{
	// clear out the window with black
	gl::clear(Color(0, 0, 0));
	//ci::app::console() << mTrackedShapes.size() << endl;


	/*if (mSurfaceSubtract.getWidth() > 0) {
		if (mTextureDepth) {
			mTextureDepth->update(Channel32f(mSurfaceSubtract));
		}
		else {
			mTextureDepth = gl::Texture::create(Channel32f(mSurfaceSubtract));
		}
		gl::draw(mTextureDepth, mTextureDepth->getBounds());
	}*/

	gl::viewport(getWindowSize());
	gl::clear();
	gl::setMatricesWindow(getWindowSize());
	gl::enableAlphaBlending();

	// draw the RGB image in top left corner
	/*gl::color(Color::white());
	if (mSurfaceColor) {
		gl::TextureRef tex = gl::Texture::create(*mSurfaceColor);
		gl::draw(tex, tex->getBounds(), Rectf(vec2(0.0f), getWindowCenter()));
	}*/


	// draws in the shape's outline in red, tends to add more noise
	/*int idx = 0;
	for (ContourVector::iterator iter = mContours.begin(); iter != mContours.end(); ++iter) {
		gl::begin(GL_LINE_LOOP);
		for (vector<cv::Point>::iterator pt = iter->begin(); pt != iter->end(); ++pt) {
			//gl::color(shapeColors[idx]);
			gl::color(Color(1.0, 0.0, 0.0));
			gl::vertex(fromOcv(*pt));
		}
		gl::end();
		idx++;
	}*/

	// draw shapes
	for (Shape s: mTrackedShapes) {
		if (!s.background) {
			gl::begin(GL_TRIANGLE_STRIP);
			for (int j = 0; j < s.hull.size(); j++) {
				gl::color(s.color);
				gl::vertex(fromOcv(s.hull[j]));
			}
			gl::end();
		}
	}

	mParams->draw();
}

void CeilingKinectApp::keyDown( KeyEvent event )
{
	// remove all background shapes
	ci::app::console() << event.getChar() << endl;
	if (event.getChar() == 'x') {
		cinder::app::console() << mTrackedShapes.size() << endl;
		for (Shape &s : mTrackedShapes) {
			ci::app::console() << "BACKGROUND" << endl;
			ci::app::console() << std::to_string(s.ID) + " background state: " + std::to_string(s.background) << endl;
			s.background = true;
		}
	}
}

cv::Mat CeilingKinectApp::removeBlack(cv::Mat input, short nearLimit, short farLimit)
{
	for (int y = 0; y < input.rows; y++) {
		for (int x = 0; x < input.cols; x++) {
			// if a shape is too close or too far away, set the depth to a fixed number
			if (input.at<short>(y, x) < nearLimit || input.at<short>(y, x) > farLimit) {
				input.at<short>(y, x) = farLimit;
			}
		}
	}
	return input;
}

vector <Shape> CeilingKinectApp::getEvaluationSet(ContourVector rawContours, int minimalArea, int maxArea) 
{
	vector <Shape> vec;
	for (vector<cv::Point> &c : rawContours) {
		// create a matrix for the contour
		cv::Mat matrix = cv::Mat(c);

		// extract the data from contour
		cv::Scalar center = mean(matrix);
		double area = cv::contourArea(matrix);

		// reject it if too small
		if (area < minimalArea) {
			continue;
		}

		// reject it if too big
		if (area > maxArea) {
			continue;
		}

		// store data
		Shape shape;
		shape.area = area;
		shape.centroid = cv::Point(center.val[0], center.val[1]);

		// store points around shape
		shape.hull = c;
		shape.matchFound = false;
		vec.push_back(shape);
	}
	return vec;
}

Shape* CeilingKinectApp::findNearestMatch(Shape trackedShape, vector<Shape> &shapes, float maximumDistance)
{
	Shape* closestShape = NULL;
	float nearestDist = 1e5;
	if (shapes.empty()) {
		return NULL;
	}

	//finalDist keeps track of the distance between the trackedShape and the chosen candidate
	float finalDist;

	for (Shape &candidate : shapes) {
		// find dist between the center of the contour and the shape
		//cv::Point distPoint = trackedShape.centroid - candidate.centroid;
		//float dist = cv::sqrt(distPoint.x * distPoint.x + distPoint.y * distPoint.y);
		float dist = cv::sqrt(pow(trackedShape.centroid.x - candidate.centroid.x, 2) + pow(trackedShape.centroid.y - candidate.centroid.y, 2));
		//cinder::app::console() << dist << endl;
		if (dist > maximumDistance) {
			continue;
		}
		if (candidate.matchFound) {
			continue;
		}
		if (dist < nearestDist) {
			nearestDist = dist;
			closestShape = &candidate;
			finalDist = dist;
		}
	}

	// if shape is moving, don't make it the background
	if (closestShape) {
		if (finalDist > 20) {
			closestShape->moving = true;
		}
	}
	return closestShape;
}

//192.168.1.2

float CeilingKinectApp::mapNum(float value, float istart, float istop, float ostart, float ostop) {
	return ostart + (ostop - ostart) * ((value - istart) / (istop - istart));
}

CINDER_APP( CeilingKinectApp, RendererGl )
