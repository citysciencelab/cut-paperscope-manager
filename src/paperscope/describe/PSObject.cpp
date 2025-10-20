/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	INCLUDES
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


	#include "PSObject.h"
	#include <cmath>

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
		confidence = 15;
		uid = QString::number(QRandomGenerator::global()->bounded(100000,999999)).toStdString();

		// init member
        normalizeCandidates();
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
			confidence += shapeType != targetShape ? 5 : 10;
			if(confidence > 100) { confidence = 100; }
            // if(shapeType == targetShape) { lerp(targetPoints); }
			detectColor();
			return true;
		}

		// overlapping bounding boxes
        if(targetShape != shapeType) { return false; }

		cv::Rect intersection = rect & rectTarget;
		float overlap = (float) intersection.area() / (float) rect.area();
		if(overlap > 0.5) { 
			confidence += shapeType != targetShape ? 5 : 10;
			if(confidence > 100) { confidence = 100; }
            // if(shapeType == targetShape) { lerp(targetPoints); }
			detectColor();
			return true;
		}

		return false;
	}


    void PSObject::normalizeCandidates() {

        points.clear();

        // normalize points to fit in 0-1 range for tracking area
		for(cv::Point &p : candidatePoints) {
			cv::Point2f point;
			point.x = (float) p.x / (float) matTracking->cols;
			point.y = (float) p.y / (float) matTracking->rows;
			points.push_back(point);
		}
    }


    void PSObject::lerp(std::vector<cv::Point> targetPoints) {

        // lerp points - ensure we don't go out of bounds
        size_t minSize = std::min(candidatePoints.size(), targetPoints.size());
        for(size_t i = 0; i < minSize; i++) {
            candidatePoints[i].x = candidatePoints[i].x * 0.99 + targetPoints[i].x * 0.01;
            candidatePoints[i].y = candidatePoints[i].y * 0.99 + targetPoints[i].y * 0.01;
        }

        normalizeCandidates();
    }



/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	CONTOUR
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// */


	void PSObject::drawContour(int minConfidence) {

		if(shapeType != PSShapeType::Cross) {

			cv::Scalar color = confidence < minConfidence ? cv::Scalar(0, 0, 255) : cv::Scalar(0, 255, 0);
			cv::drawContours(*matRender, std::vector<std::vector<cv::Point>>{candidatePoints}, -1, color, 1);

			// draw vertex points
			for(size_t i = 0; i < candidatePoints.size(); i++) { 
				cv::circle(*matRender, candidatePoints[i], 5, cv::Scalar(0, 255, 255), -1); 
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

		// guards
		if (!matTracking || matTracking->empty() || matTracking->channels() != 3) { return; }
		if (candidatePoints.empty()) { return; }

		// get roi of object
		cv::Rect rect = cv::boundingRect(candidatePoints);
		if(rect.width < 1 || rect.height < 1 || rect.x < 0 || rect.y < 0) { return; }
		if(rect.x + rect.width > matTracking->cols || rect.y + rect.height > matTracking->rows) { return; }

		// roi in hsv color space
		cv::Mat hsv;
		const cv::Mat roiBgr = (*matTracking)(rect);
		cv::cvtColor(roiBgr, hsv, cv::COLOR_BGR2HSV);

		// optional: denoise for stability
		// cv::medianBlur(hsv, hsv, 3);

		// build shape mask in ROI coordinates (shift points by rect.tl())
		std::vector<cv::Point> localPts;
		localPts.reserve(candidatePoints.size());
		for (const auto& p : candidatePoints) {
			localPts.emplace_back(p.x - rect.x, p.y - rect.y);
		}
		cv::Mat shapeMask = cv::Mat::zeros(hsv.size(), CV_8UC1);
		if (localPts.size() >= 3) {
			std::vector<std::vector<cv::Point>> contours = { localPts };
			cv::drawContours(shapeMask, contours, 0, cv::Scalar(255), cv::FILLED);
		} else if (!localPts.empty()) {
			// handle degenerate shapes (single point / line)
			for (const auto& lp : localPts) {
				if (lp.x >= 0 && lp.y >= 0 && lp.x < shapeMask.cols && lp.y < shapeMask.rows) {
					shapeMask.at<uchar>(lp) = 255;
				}
			}
		}

		// split channels
		std::vector<cv::Mat> ch;
		cv::split(hsv, ch); // H:0 (0..179), S:1, V:2

		// mask out dull/dark pixels to improve robustness
		const int satThresh = 30;
		const int valThresh = 30;
		cv::Mat sMask, vMask, chromaMask, validMask;
		cv::threshold(ch[1], sMask, satThresh, 255, cv::THRESH_BINARY);
		cv::threshold(ch[2], vMask, valThresh, 255, cv::THRESH_BINARY);
		cv::bitwise_and(sMask, vMask, chromaMask);
		cv::bitwise_and(chromaMask, shapeMask, validMask);

		// count valid pixels
		int validCount = cv::countNonZero(validMask);
		if (validCount == 0) {
			// fallback to "black"
			avgColor = cv::Scalar(0, 0, 0);
			colorIndexQueue.push_back(0);
			if (colorIndexQueue.size() > 20) { colorIndexQueue.erase(colorIndexQueue.begin()); }
			// smooth to most frequent
			auto colorCandidates = getColorCandidates();
			std::vector<int> colorCount(colorCandidates.size(), 0);
			for (int idx : colorIndexQueue) { colorCount[idx]++; }
			colorIndex = int(std::max_element(colorCount.begin(), colorCount.end()) - colorCount.begin());
			return;
		}

		// average S and V directly with mask
		double sMean = cv::mean(ch[1], validMask)[0];
		double vMean = cv::mean(ch[2], validMask)[0];

		// average H on the circle
		// OpenCV H is 0..179 -> degrees = H*2 -> radians = degrees * pi/180
		double sumSin = 0.0, sumCos = 0.0;
		for (int y = 0; y < hsv.rows; ++y) {
			const uchar* hRow = ch[0].ptr<uchar>(y);
			const uchar* mRow = validMask.ptr<uchar>(y);
			for (int x = 0; x < hsv.cols; ++x) {
				if (mRow[x]) {
					double angleRad = (hRow[x] * 2.0) * M_PI / 180.0;
					sumCos += std::cos(angleRad);
					sumSin += std::sin(angleRad);
				}
			}
		}
		double avgAngle = std::atan2(sumSin, sumCos);            // [-pi, pi]
		if (avgAngle < 0) avgAngle += 2.0 * M_PI;                // [0, 2pi)
		double hMeanDeg = avgAngle * 180.0 / M_PI;               // [0, 360)
		double hMean = hMeanDeg / 2.0;                           // [0, 180)
		if (hMean >= 180.0) hMean -= 180.0;                      // clamp

		// store average HSV
		avgColor = cv::Scalar(hMean, sMean, vMean);

		// classify to nearest candidate using circular hue distance
		auto colorCandidates = getColorCandidates();
		auto hueDist = [](double h1, double h2) -> double {
			double d = std::fabs(h1 - h2);
			return std::min(d, 180.0 - d); // circular distance in OpenCV hue space
		};

		int matchColorIndex = 0;
		double bestScore = std::numeric_limits<double>::infinity();
		// weights: emphasize hue, moderate S, light V
		const double wH = 2.0, wS = 1.0, wV = 0.5;
		for (int i = 0; i < (int)colorCandidates.size(); ++i) {
			for (const auto& cand : colorCandidates[i]) {
				double dh = hueDist(hMean, cand[0]);
				double ds = std::fabs(sMean - cand[1]);
				double dv = std::fabs(vMean - cand[2]);
				double score = std::sqrt((wH*dh)*(wH*dh) + (wS*ds)*(wS*ds) + (wV*dv)*(wV*dv));
				if (score < bestScore) {
					bestScore = score;
					matchColorIndex = i;
				}
			}
		}

		// temporal smoothing
		colorIndexQueue.push_back(matchColorIndex);
		if (colorIndexQueue.size() > 20) { colorIndexQueue.erase(colorIndexQueue.begin()); }

		std::vector<int> colorCount(colorCandidates.size(), 0);
		for (int idx : colorIndexQueue) { colorCount[idx]++; }

		int maxCount = 0;
		for (int i = 0; i < (int)colorCount.size(); i++) {
			if (colorCount[i] > maxCount) {
				maxCount = colorCount[i];
				colorIndex = i;
			}
		}
	}


	std::vector< std::vector<cv::Scalar> > PSObject::getColorCandidates() {

		std::vector< std::vector<cv::Scalar> > colorCandidates;

		// [0] black
		colorCandidates.push_back({
			cv::Scalar(0, 0, 0), 
		});

		// [1] blue
		colorCandidates.push_back({
			cv::Scalar(100, 255, 128),
			cv::Scalar(125, 255, 128),
		});

		// [2] green
		colorCandidates.push_back({
			cv::Scalar(60, 255, 128),
		});

		// [3] yellow
		colorCandidates.push_back({
			cv::Scalar(30, 255, 128), 
		});

		// [4] red
		colorCandidates.push_back({
			cv::Scalar(0, 255, 128), 
			cv::Scalar(180, 255, 128), 
		});

		return colorCandidates;
	}


	void PSObject::drawColor() {

		// guard: candidate points must exist
		if (candidatePoints.empty()) { return; }

		cv::Rect rect = cv::boundingRect(candidatePoints);
        if(rect.width < 1 || rect.height < 1) { return; }

		// render color as circle
		cv::Mat rgbColor;
		cv::cvtColor(cv::Mat(1, 1, CV_8UC3, avgColor), rgbColor, cv::COLOR_HSV2BGR);
		cv::circle(*matRender, cv::Point(rect.x, rect.y + rect.height + 20), 10, cv::Scalar(rgbColor.at<cv::Vec3b>(0, 0)[0], rgbColor.at<cv::Vec3b>(0, 0)[1], rgbColor.at<cv::Vec3b>(0, 0)[2]), -1);

		// render color name
		std::vector<std::string> colorNames = {"black", "blue", "green", "yellow", "red"};
		cv::putText(*matRender, colorNames[colorIndex], cv::Point(rect.x + 16, rect.y + rect.height + 25), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 255, 255), 1, cv::LINE_AA);
	}

