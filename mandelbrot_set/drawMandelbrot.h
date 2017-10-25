#pragma once
#include <queue>
#include <mutex>

template <typename T>
void processPixels(std::queue<std::pair<int, int>>& work_q, std::mutex& guard, const int width, const int height, const T x_min, const T x_max, const T y_min, const T y_max, sf::Image& out_img) {
	T x_range = x_max - x_min;
	T y_range = y_max - y_min;
	while (1) {
		std::pair<int, int> job;
		{
			std::lock_guard<std::mutex> lock(guard);
			if (work_q.size()) {
				job = work_q.front();
				work_q.pop();
			}
			else { break; }
		}
		int x = job.first;
		int y = job.second;

		T real_0 = (T(x) / width) * x_range + x_min;
		T imag_0 = (T(y) / height) * y_range + y_min;
		//if (last_real == real_0) {
		//	fp_e = true;
		//}
		// Fill with black first, in case it's part of the set
		out_img.setPixel(x, y, sf::Color(0, 0, 0));
		T real = real_0;
		T imag = imag_0;
		T real_sq = real_0 * real_0;
		T imag_sq = imag_0 * imag_0;
		for (int iter = 0; iter < MAX_ITERS; ++iter) {
			// Perform next iteration. c(n+1) = c(n)*c(n) + c(1); c(0) = 0
			T new_real = (real_sq - imag_sq) + real_0;
			// imag = (2 * real * imag) + imag_0
			imag = real * imag;
			imag += imag;
			imag += imag_0;
			real = new_real;
			real_sq = real*real;
			imag_sq = imag*imag;
			if (real_sq + imag_sq > CUTOFF_DIST) {
				// Not in the set.
				//uint8_t prop = 255 - 255 * (float(iter) / MAX_ITERS);
				hsv desired_color{
					240 - 240 * (double(iter) / MAX_ITERS),
					1.0,
					1 - (double(iter) / MAX_ITERS)
				};
				rgb rgb_color = hsv2rgb(desired_color);
				out_img.setPixel(x, y, sf::Color(255*rgb_color.r, 255*rgb_color.g, 255*rgb_color.b));
				break;
			}
		}
	}
}

template <typename T>
void drawMandelbrot(const int width, const int height, const T x_min, const T x_max, const T y_min, const T y_max, sf::Image& out_img) {
	std::queue<std::pair<int,int>> work_q;

	bool fp_e = false;
	for (int y = 0; y < height; ++y) {
		T last_real = x_min - 1;
		for (int x = 0; x < width; ++x) {
			work_q.push(std::make_pair(x, y));
			//last_real = real_0;
		}
	}

	std::vector<std::thread> workers;
	int n_threads = std::thread::hardware_concurrency();
	std::mutex lock;
	for (int i = 0; i < n_threads; ++i) {
		workers.push_back(std::thread(processPixels<T>, std::ref(work_q), std::ref(lock), width, height, x_min, x_max, y_min, y_max, std::ref(out_img)));
	}
	for (std::thread& t : workers) {
		t.join();
	}

	if (fp_e) {
		cout << "Fp Error" << endl;
	}
}