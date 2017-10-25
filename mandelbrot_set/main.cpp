#include <stdint.h>
#include <iostream>
#include <vector>
#include <chrono>
#include <SFML/Graphics.hpp>
#include <mpreal.h>

#include "color_conversion.h"
#include "drawMandelbrot.h"

using namespace std;
using mpfr::mpreal;

// TODO: Use stdint fast_types
const int MAX_ITERS = 500;
const double CUTOFF_DIST = 4;

//void drawMandelbrot(const int width, const int height, const mpreal x_min, const mpreal x_max, const mpreal y_min, const mpreal y_max, sf::Image& out_img) {
//	mpreal x_range = x_max - x_min;
//	mpreal y_range = y_max - y_min;
//	bool fp_e = false;
//	for (int y = 0; y < height; ++y) {
//		mpreal last_real = x_min - 1;
//		for (int x = 0; x < width; ++x) {
//			mpreal real_0 = (mpreal(x) / width) * x_range + x_min;
//			mpreal imag_0 = (mpreal(y) / height) * y_range + y_min;
//			if (last_real == real_0) {
//				fp_e = true;
//			}
//			// Fill with black first, in case it's part of the set
//			out_img.setPixel(x, y, sf::Color(0, 0, 0));
//			mpreal real = real_0;
//			mpreal imag = imag_0;
//			mpreal real_sq = real_0 * real_0;
//			mpreal imag_sq = imag_0 * imag_0;
//			for (int iter = 0; iter < MAX_ITERS; ++iter) {
//				// Perform next iteration. c(n+1) = c(n)*c(n) + c(1); c(0) = 0
//				mpreal new_real = (real_sq - imag_sq) + real_0;
//				// imag = (2 * real * imag) + imag_0
//				imag = real * imag;
//				imag += imag;
//				imag += imag_0;
//				real = new_real;
//				real_sq = real*real;
//				imag_sq = imag*imag;
//				if (real_sq + imag_sq > CUTOFF_DIST) {
//					// Not in the set.
//					//uint8_t prop = 255 - 255 * (float(iter) / MAX_ITERS);
//					hsv desired_color{
//						240 - 240 * (double(iter) / MAX_ITERS),
//						1.0,
//						1 - (double(iter) / MAX_ITERS)
//					};
//					rgb rgb_color = hsv2rgb(desired_color);
//					out_img.setPixel(x, y, sf::Color(255*rgb_color.r, 255*rgb_color.g, 255*rgb_color.b));
//					break;
//				}
//			}
//			last_real = real_0;
//		}
//	}
//	if (fp_e) {
//		cout << "Fp Error" << endl;
//	}
//}

typedef mpreal FloatType;
//typedef double FloatType;

int main() {
	const int img_w = 720;
	const int img_h = 720;
	auto inWindow = [img_w, img_h](int x, int y) {return x >= 0 && y >= 0 && x < img_w && y < img_h; };
	sf::RenderWindow window(sf::VideoMode(img_w, img_h), "I's a window");
	window.setFramerateLimit(60);
	sf::Sprite sprite;
	sf::Texture texture;
	texture.create(img_w, img_h);
	sf::Image image;
	image.create(img_w, img_h, sf::Color(255, 255, 0));
	sprite.setTexture(texture);

	FloatType min_x = -1.5;
	FloatType max_x = 0.5;
	FloatType orig_width = max_x - min_x;
	FloatType min_y = -1;
	FloatType max_y = 1;
	bool redraw = true;
	bool dragging = false;
	int down_x, down_y;

	// Set precision in bits
	mpreal::set_default_prec(64);

	while (window.isOpen()) {
		sf::Event event;
		while (window.waitEvent(event)) {
			if (event.type == sf::Event::Closed) {
				window.close();
			}
			if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
				down_x = event.mouseButton.x;
				down_y = event.mouseButton.y;
				if (inWindow(down_x, down_y)) {
					dragging = true;
					std::cout << "Mouse down: " << down_x << ", " << down_y << std::endl;
				}
			}
			if (event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Left) {
				int up_x = event.mouseButton.x;
				int up_y = event.mouseButton.y;
				if (inWindow(up_x, up_y)) {
					FloatType x_range = max_x - min_x;
					FloatType y_range = max_y - min_y;
					FloatType x_scale = x_range / img_w;
					FloatType y_scale = y_range / img_h;
					min_x = min_x + min(down_x, up_x) * x_scale;
					max_x = min_x + abs(down_x - up_x) * x_scale;
					min_y = min_y + min(down_y, up_y) * y_scale;
					max_y = min_y + abs(down_y - up_y) * y_scale;
					redraw = true;
					dragging = false;
					std::cout << "Mouse up: " << up_x << ", " << up_y << std::endl;
				}
			}
			if (redraw) {
				std::cout << "Zoom level = " << orig_width / (max_x - min_x) << std::endl;
				auto start_time = std::chrono::steady_clock::now();
				drawMandelbrot(img_w, img_h, min_x, max_x, min_y, max_y, image);
				auto end_time = std::chrono::steady_clock::now();
				std::cout << "Took " << std::chrono::duration<double, milli>(end_time - start_time).count() << " ms to draw" << std::endl;
				redraw = false;
			}


			window.clear();
			texture.update(image);
			window.draw(sprite);
			if (dragging) {
				sf::Vector2i mouse_pos = sf::Mouse::getPosition(window);
				sf::Vector2f rect_size(abs(mouse_pos.x - down_x), abs(mouse_pos.y - down_y));
				sf::RectangleShape rect(rect_size);
				rect.setPosition(min(mouse_pos.x, down_x), min(mouse_pos.y, down_y));
				rect.setOutlineColor(sf::Color::Green);
				rect.setFillColor(sf::Color(0, 0, 0, 0));
				rect.setOutlineThickness(2);
				window.draw(rect);
			}
			window.display();
		}
	}
	return 0;
}
