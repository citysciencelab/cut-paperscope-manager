/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	INCLUDES
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


	#include "PSCandidate.h"



/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	CONSTRUCTOR
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


	PSCandidate::PSCandidate(std::vector<cv::Point> contour, PSShapeType shapeType)
		: contour(contour),
		  shapeType(shapeType)
	{

	}



/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	POINTS
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


	std::vector<cv::Point> PSCandidate::getPoints() {
		
		std::vector<cv::Point> points;
		
		if(shapeType == PSShapeType::Rectangle) {
            
			cv::RotatedRect rect = cv::minAreaRect(contour);
			cv::Point2f vertices[4];
			rect.points(vertices);
			for(int i = 0; i < 4; i++) { points.push_back(vertices[i]); }
		}
		else if(shapeType == PSShapeType::Circle) {

			if(contour.size() < 5) { return points; }
            
			cv::RotatedRect ellipse = cv::fitEllipse(contour);
            cv::ellipse2Poly(ellipse.center, cv::Size(ellipse.size.width / 2, ellipse.size.height / 2), ellipse.angle, 0, 360, 20, points);
		}
		else if(shapeType == PSShapeType::Triangle) {
			
			cv::minEnclosingTriangle(contour, points);
		}
		else if(shapeType == PSShapeType::Cross) {
			
			cv::RotatedRect rect = cv::minAreaRect(contour);
			points.push_back(rect.center);
		}
        else if(shapeType == PSShapeType::Organic) {
			
			cv::approxPolyDP(contour, points, 0.002 * cv::arcLength(contour, true), true);
		}
		else if(shapeType == PSShapeType::Street) {

			cv::approxPolyDP(contour, points ,1, true);
		}
		
		return points;
	}



/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	DRAW
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


	void PSCandidate::drawBoundingBox(cv::Mat *matRender) {

		cv::Scalar color = this->shapeType == PSShapeType::Street ? cv::Scalar(0, 0, 255) : cv::Scalar(0, 255, 0);

		cv::Rect rect = cv::boundingRect(contour);
		cv::rectangle(*matRender, rect, color);

		drawShapeType(matRender);
	}


	void PSCandidate::drawShapeType(cv::Mat *matRender) {

		std::string label;
		cv::Scalar color = this->shapeType == PSShapeType::Street ? cv::Scalar(0, 0, 255) : cv::Scalar(0, 255, 0);

		switch(shapeType) {
			case PSShapeType::Rectangle: 	label = "rectangle"; break;
			case PSShapeType::Circle: 		label = "circle"; break;
			case PSShapeType::Triangle: 	label = "triangle"; break;
			case PSShapeType::Cross: 		label = "cross"; break;
            case PSShapeType::Organic: 		label = "organic"; break;
            case PSShapeType::Street: 		label = "street"; break;
		}

		// draw label
		cv::Rect rect = cv::boundingRect(contour);
		cv::putText(*matRender, label, cv::Point(rect.x, rect.y - 10.0), cv::FONT_HERSHEY_SIMPLEX, 0.75, color, 2, cv::LINE_AA);
	}
