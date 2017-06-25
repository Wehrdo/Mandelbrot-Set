#include <stdint.h>
#include <iostream>
#include <vector>
#include <SFML/Graphics.hpp>
#include "color_conversion.h"
#include <mpreal.h>

using namespace std;

// TODO: Use stdint fast_types
const int MAX_ITERS = 500;
const double CUTOFF_DIST = 5;

void drawMandelbrot(const int width, const int height, const double x_min, const double x_max, const double y_min, const double y_max, sf::Image& out_img) {
	double x_range = x_max - x_min;
	double y_range = y_max - y_min;
	bool fp_e = false;
	for (int y = 0; y < height; ++y) {
		double last_real = x_min - 1;
		for (int x = 0; x < width; ++x) {
			double real_0 = (double(x) / width) * x_range + x_min;
			double imag_0 = (double(y) / height) * y_range + y_min;
			if (last_real == real_0) {
				fp_e = true;
			}
			// Fill with black first, in case it's part of the set
			out_img.setPixel(x, y, sf::Color(0, 0, 0));
			double real = real_0;
			double imag = imag_0;
			for (int iter = 0; iter < MAX_ITERS; ++iter) {
				// Perform next iteration. c(n+1) = c(n)*c(n) + c(1); c(0) = 0
				double new_real = (real*real - imag*imag) + real_0;
				imag = (2 * real * imag) + imag_0;
				real = new_real;
				if (real*real + imag*imag > CUTOFF_DIST) {
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
			last_real = real_0;
		}
	}
	if (fp_e) {
		cout << "Fp Error" << endl;
	}
}

int main() {
	const int img_w = 840;
	const int img_h = 840;
	sf::RenderWindow window(sf::VideoMode(img_w, img_h), "I's a window");
	window.setFramerateLimit(60);
	sf::Sprite sprite;
	sf::Texture texture;
	texture.create(img_w, img_h);
	sf::Image image;
	image.create(img_w, img_h, sf::Color(255, 255, 0));
	sprite.setTexture(texture);

	double min_x = -1.5;
	double max_x = 0.5;
	double min_y = -1;
	double max_y = 1;
	bool redraw = true;
	bool dragging = false;
	int down_x, down_y;
	while (window.isOpen()) {
		sf::Event event;
		while (window.waitEvent(event)) {
			if (event.type == sf::Event::Closed) {
				window.close();
			}
			if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
				down_x = event.mouseButton.x;
				down_y = event.mouseButton.y;
				dragging = true;
			}
			if (event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Left) {
				double x_range = max_x - min_x;
				double y_range = max_y - min_y;
				int up_x = event.mouseButton.x;
				int up_y = event.mouseButton.y;
				double x_scale = x_range / img_w;
				double y_scale = y_range / img_h;
				min_x = min_x + min(down_x, up_x) * x_scale;
				max_x = min_x + abs(down_x - up_x) * x_scale;
				min_y = min_y + min(down_y, up_y) * y_scale;
				max_y = min_y + abs(down_y - up_y) * y_scale;
				redraw = true;
				dragging = false;
			}
			if (redraw) {
				drawMandelbrot(img_w, img_h, min_x, max_x, min_y, max_y, image);
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
