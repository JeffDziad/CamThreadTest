#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include <iostream>
#include <thread>
#include <stdlib.h>

#include <ctime>
#include <ratio>
#include <chrono>

static bool running = true;
static std::chrono::high_resolution_clock::time_point last_delete = std::chrono::high_resolution_clock::now();

/*
cv::Mat resize_frame(cv::Mat frame, int height, int width) {
	cv::Mat resized;

	return resized;
}

cv::Mat preprocess_frame(cv::Mat frame) {
	cv::Mat processed = resize_frame(frame, 720, 1280);
	return processed;
}
*/

// MAYBE USE std::chrono::system_clock

int getFPS(std::chrono::high_resolution_clock::time_point t1, int frame_count) {
	std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double, std::milli> time_span = t2 - t1;
	return time_span.count() / frame_count;
}
 
bool isTimeFor(std::chrono::high_resolution_clock::time_point last_time, int ms) {
	std::chrono::high_resolution_clock::time_point now = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double, std::milli> diff = now - last_time;
	return diff.count() > ms;
}

void Cam_Thread(std::string url, std::string cam_name) {
	std::chrono::high_resolution_clock::time_point init_time = std::chrono::high_resolution_clock::now();
	std::chrono::high_resolution_clock::time_point last_save = std::chrono::high_resolution_clock::now();
	cv::VideoCapture* stream = new cv::VideoCapture(url, cv::CAP_FFMPEG);
	int frames_captured = 0;
	while (running) {
		if (stream->isOpened()) {
			cv::namedWindow(cam_name);
			cv::Mat unprocessed_frame;
			if (stream->read(unprocessed_frame)) {
				cv::Mat processed_frame = unprocessed_frame;

				// Resizing leads to slow processing times, leave at native size
				//cv::resize(unprocessed_frame, processed_frame, cv::Size(1280, 720), cv::INTER_LINEAR);
				cv::putText(processed_frame, "FPS : " + std::to_string(getFPS(init_time, frames_captured)),
					cv::Point(processed_frame.cols / 20, processed_frame.rows / 10),
					cv::FONT_HERSHEY_DUPLEX, 1.0, (std::sin(frames_captured * 0.2) > 0) ? CV_RGB(255, 255, 255) : CV_RGB(255, 0, 0), 2);
				cv::imshow(cam_name, processed_frame);
				frames_captured++;
				cv::waitKey(1);
			}

			// 1. Save Frame


		}
	}
}

int main() {

#if WIN32
	_putenv_s("OPENCV_FFMPEG_CAPTURE_OPTIONS", "rtsp_transport;udp");
#endif

	// Start video capture with a new thread
	std::thread cam1(Cam_Thread, "rtsp://beverly1:0FtYard1@192.168.1.245/live", "Beverly_Front");
	std::thread cam2(Cam_Thread, "rtsp://admin:jeffjadd@192.168.1.246/live", "Beverly_Back");

	while (running) {
		std::string input;
		std::cin >> input;
		if (input == "stop") {
			running = false;
		}
	}

	// Wait for threads to finish
	cam1.join();
	cam2.join();

	return 1;
}