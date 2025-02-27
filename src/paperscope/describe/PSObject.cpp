/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	INCLUDES
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


	#include "PSObject.h"

	// Qt
	#include <QDebug>
	#include <QRandomGenerator>



/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	CONSTRUCTOR
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


	PSObject::PSObject(std::vector<cv::Point> candidatePoints, PSShapeType shapeType, cv::Mat *matTracking, cv::Mat *matRender)
		: candidatePoints(candidatePoints), 
		  shapeType(shapeType),
		  matTracking(matTracking),
          matRender(matRender)
	{

        // init properties
        colorIndex = 0;

        // set random uid as string
		uid = QString::number(QRandomGenerator::global()->bounded(100000,999999)).toStdString();

		// normalize points
		for(cv::Point &p : candidatePoints) {
			cv::Point2f point;
			point.x = (float) p.x / (float) matTracking->cols;
			point.y = (float) p.y / (float) matTracking->rows;
			points.push_back(point);
		}

		confidence = 4;
	}	

 

/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	TRACKING
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


	bool PSObject::isSame(std::vector<cv::Point> targetPoints, PSShapeType targetShape) {

		cv::Rect rect = cv::boundingRect(candidatePoints);
		cv::Rect rectTarget = cv::boundingRect(targetPoints);

		// same center
		cv::Point center = cv::Point(rect.x + rect.width / 2, rect.y + rect.height / 2);
		cv::Point centerTarget = cv::Point(rectTarget.x + rectTarget.width / 2, rectTarget.y + rectTarget.height / 2);
		float distance = cv::norm(center - centerTarget);
		if(distance < 10) { 
			confidence += shapeType != targetShape ? 2 : 8;
			if(confidence > 100) { confidence = 100; }
			return true;
		}

		// overlapping bounding boxes
		if(targetShape == PSShapeType::Street || shapeType == PSShapeType::Street) { return false; }
		cv::Rect intersection = rect & rectTarget;
		float overlap = (float) intersection.area() / (float) rect.area();
		if(overlap > 0.5) { 
			confidence += shapeType != targetShape ? 2 : 8;
			if(confidence > 100) { confidence = 100; }
			return true;
		}

		return false;
	}



/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	CONTOUR
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


	void PSObject::drawContour() {

		if(shapeType != PSShapeType::Cross) {
			
			cv::Scalar color = this->shapeType == PSShapeType::Street ? cv::Scalar(0, 0, 255) : cv::Scalar(0, 255, 0);
			cv::drawContours(*matRender, std::vector<std::vector<cv::Point>>{candidatePoints}, -1, color, 1);
			
			// draw vertex points
			if(shapeType != PSShapeType::Street) {
				for(size_t i = 0; i < candidatePoints.size(); i++) { 
					cv::circle(*matRender, candidatePoints[i], 5, cv::Scalar(0, 255, 255), -1); 
				}
			}
		}
		// render cross as point
		else {
			cv::circle(*matRender, candidatePoints[0], 5, cv::Scalar(0, 255, 255), -1);
		}

		drawColor();
	}



/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	COLOR DETECTION
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


	void PSObject::detectColor() {

        if(shapeType == PSShapeType::Street) { return; }

		// get roi of object from matTracking
		cv::Rect rect = cv::boundingRect(candidatePoints);
        if(rect.width < 1 || rect.height < 1 || rect.x < 0 || rect.y < 0) { return; }
        if(rect.x + rect.width > matTracking->cols || rect.y + rect.height > matTracking->rows) { return; }
		cv::Mat roi = (*matTracking)(rect);

		// every pixel outside of candidatePoints shape should be black
		cv::Mat mask = cv::Mat::zeros(roi.size(), CV_8UC1);
		std::vector<std::vector<cv::Point>> contours = {candidatePoints};
		cv::drawContours(mask, contours, 0, cv::Scalar(255), -1);
		cv::bitwise_and(roi, roi, roi, mask);
		
		// convert to hsv
		cv::cvtColor(roi, roi, cv::COLOR_BGR2HSV);

		// get average color but ignore black pixels
		int h = 0, s = 0, v = 0, count = 0;
		for(int i = 0; i < roi.rows; i++) {
			for(int j = 0; j < roi.cols; j++) {

				int hue = roi.at<cv::Vec3b>(i, j)[0];
				int sat = roi.at<cv::Vec3b>(i, j)[1];
				int val = roi.at<cv::Vec3b>(i, j)[2];
				
				if(val > 30 && sat > 30) {
					h += hue;
					s += sat;
					v += val;
					count++;
				}
			}
		}

		// calculate average color
		if(count > 0) { h /= count; s /= count; v /= count; }
		avgColor = cv::Scalar(h,s,v);

		// find best matching color
		std::vector< std::vector<cv::Scalar> > colorCandidates = getColorCandidates();
		float minDistance = 1000;
		for(int i = 0; i < (int) colorCandidates.size(); i++) {

			for(int j = 0; j < (int) colorCandidates[i].size(); j++) {
				float distance = cv::norm(avgColor - colorCandidates[i][j]);
				if(distance < minDistance) {
					minDistance = distance;
					colorIndex = i;
				}
			}
		}
    }


	std::vector< std::vector<cv::Scalar> > PSObject::getColorCandidates() {

		std::vector< std::vector<cv::Scalar> > colorCandidates;

		// [0] black
		colorCandidates.push_back({
			cv::Scalar(90, 20, 20), 
		});

		// [1] blue
		colorCandidates.push_back({
			cv::Scalar(110, 165 , 128)
		});

		// [2] green
		colorCandidates.push_back({
			cv::Scalar(75, 153, 128)
		});

		// [3] yellow
		colorCandidates.push_back({
			cv::Scalar(43, 140, 235), 
			cv::Scalar(25, 153, 155), 
		});

		return colorCandidates;
	}


	void PSObject::drawColor() {

		if(shapeType == PSShapeType::Street) { return; }

		cv::Rect rect = cv::boundingRect(candidatePoints);
        if(rect.width < 1 || rect.height < 1) { return; }

		// render color as circle
		cv::Mat rgbColor;
		cv::cvtColor(cv::Mat(1, 1, CV_8UC3, avgColor), rgbColor, cv::COLOR_HSV2BGR);
		cv::circle(*matRender, cv::Point(rect.x, rect.y + rect.height + 20), 10, cv::Scalar(rgbColor.at<cv::Vec3b>(0, 0)[0], rgbColor.at<cv::Vec3b>(0, 0)[1], rgbColor.at<cv::Vec3b>(0, 0)[2]), -1);

		// render color name
		std::vector<std::string> colorNames = {"black", "blue", "green", "yellow"};
		cv::putText(*matRender, colorNames[colorIndex], cv::Point(rect.x + 16, rect.y + rect.height + 25), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 255, 255), 1, cv::LINE_AA);
	}

