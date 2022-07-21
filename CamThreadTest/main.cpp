#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include <iostream>
#include <thread>
#include <stdlib.h>
#include <algorithm>

#include <ctime>
#include <ratio>
#include <chrono>
#include "date.h"

static bool running = true;
static std::chrono::system_clock::time_point last_delete = std::chrono::system_clock::now();
static const std::string ROOT_SAVE_DIR = "recordings\\";

/*
cv::Mat resize_frame(cv::Mat frame, int height, int width) {
	cv::Mat resized;

	return resized;
}

cv::Mat preprocess_frame(cv::Mat frame) {
	cv::Mat processed = resize_frame(frame, 720, 1280);
	return processed;b 
}
*/ 

std::chrono::duration<double, std::milli> getDiffFromNow(std::chrono::system_clock::time_point t) {
	std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
	auto diff = std::chrono::duration<double, std::milli>(now - t);
	return diff;
}

int getFPS(std::chrono::system_clock::time_point t1, int frame_count) {
	auto diff = getDiffFromNow(t1);
	return diff.count() / frame_count;
}
 
bool isTimeFor(std::chrono::system_clock::time_point last_time, int ms) {
	auto diff = getDiffFromNow(last_time);
	return diff.count() > ms;
}

std::string getFormattedSaveName(std::string cam_name, std::chrono::system_clock::time_point t, std::string extension) {
	std::string out = ROOT_SAVE_DIR;
	std::string formatted = date::format("%F %T %Z", date::floor<std::chrono::milliseconds>(t));
	out += (cam_name + " - ");
	out += formatted;
	out += extension;
	std::replace(out.begin(), out.end(), ':', '_');
	return out;
}

void Cam_Thread(std::string url, std::string cam_name) {
	bool first_start = false;
	const int SAVE_DURATION = 60000;
	const int FPS = 20;
	const std::chrono::system_clock::time_point INIT_TIME = std::chrono::system_clock::now();
	std::chrono::system_clock::time_point last_save = std::chrono::system_clock::now();
	cv::VideoCapture stream = cv::VideoCapture(url, cv::CAP_FFMPEG);
	long frames_captured = 0;
	int stream_width = stream.get(cv::CAP_PROP_FRAME_WIDTH);
	int stream_height = stream.get(cv::CAP_PROP_FRAME_HEIGHT);
	std::string save_destination = getFormattedSaveName(cam_name, last_save, ".avi");

	cv::VideoWriter writer;
	
	//cv::VideoWriter(save_destination, cv::VideoWriter::fourcc('m', 'p', '4', 'v'), FPS, cv::Size(stream_width, stream_height));

	while (running) {
		if (stream.isOpened()) {
			if (!first_start) {
				writer.open(save_destination, cv::VideoWriter::fourcc('m', 'p', '4', 'v'), FPS, cv::Size(stream_width, stream_height));
				std::cout << cam_name << " has Started." << "\n";
				first_start = true;
			}	
			cv::namedWindow(cam_name);
			cv::Mat unprocessed_frame;
			if (stream.read(unprocessed_frame)) {
				cv::Mat processed_frame = unprocessed_frame;
				// Resizing leads to slow processing times, leave at native size
				//cv::resize(unprocessed_frame, processed_frame, cv::Size(1280, 720), cv::INTER_LINEAR);
				cv::putText(processed_frame, "FPS : " + std::to_string(getFPS(INIT_TIME, frames_captured)),
					cv::Point(processed_frame.cols / 20, processed_frame.rows / 10),
					cv::FONT_HERSHEY_DUPLEX, 1.0, (std::sin(frames_captured * 0.2) > 0) ? CV_RGB(255, 255, 255) : CV_RGB(255, 0, 0), 2);
				
				writer.write(processed_frame);
				
				cv::imshow(cam_name, processed_frame);
				frames_captured++;
				cv::waitKey(1);
			}
			if (isTimeFor(last_save, SAVE_DURATION)) {
				last_save = std::chrono::system_clock::now();
				std::string save_destination = getFormattedSaveName(cam_name, last_save, ".avi");
				writer.release();
				writer.open(save_destination, cv::VideoWriter::fourcc('m', 'p', '4', 'v'), FPS, cv::Size(stream_width, stream_height));
				std::cout << cam_name << " - Saving and Starting new video writer." << "\n";
			}
		}
	}
}

int main() {

#if WIN32
	_putenv_s("OPENCV_FFMPEG_CAPTURE_OPTIONS", "rtsp_transport;udp");
#endif

	// Start video capture with a new thread
	
	//std::thread cam1(Cam_Thread, "rtsp://beverly1:0FtYard1@192.168.1.245/live", "Beverly_Front");
	std::thread cam2(Cam_Thread, "rtsp://admin:jeffjadd@192.168.1.246/live", "Beverly_Back");

	while (running) {
		std::string input;
		std::cin >> input;
		if (input == "stop") {
			running = false;
		}
	}

	// Wait for threads to finish
	//cam1.join();
	cam2.join();

	return 1;
}