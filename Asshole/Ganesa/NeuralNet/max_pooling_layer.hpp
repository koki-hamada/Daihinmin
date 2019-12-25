#pragma once
#include <iostream>
#include "layer.hpp"
class MaxPoolingLayer : public Layer {
private:
	//float output_temp_[256][256];
	//float input_temp_[256][256];
	float *output_temp_;
	float *input_temp_;
	int w_, h_;
public:
	int kernel_h_;
	int kernel_w_;
	int stride_;
	int pad_h_;
	int pad_w_;

	// コンストラクタ
	MaxPoolingLayer(int num_input, int input_height, int input_width, int num_output, int output_height, int output_width,
		int kernel_height, int kernel_width, int stride, int pad_height, int pad_width) : Layer(num_output,
			num_input,
			input_width,
			input_height,
			output_height,
			output_width),
		kernel_h_(kernel_height),
		kernel_w_(kernel_width),
		stride_(stride),
		pad_h_(pad_height),
		pad_w_(pad_width), 
		h_(input_height + pad_height * 2 + kernel_height - 1),
		w_(input_width + pad_width * 2 + kernel_width - 1) {
			output_temp_ = new float[output_width * output_height];
			input_temp_ = new float[h_ * w_];
	}

	// デストラクタ
	~MaxPoolingLayer() {
		delete[] output_temp_;
		delete[] input_temp_;
	}

	//順伝播. 
	virtual void Forward(const float *input, float* output) {
		int index = 0;
		int index_input = 0;
		for (int m = 0; m < num_output_; m++) {
			for (int i = 0; i < output_height_ * output_width_; i++)
					output_temp_[i] = -1000000.0f;
			for(int i = 0; i < w_ * h_; i++)
				input_temp_[i] = -1000000.0f;
			for (int y = pad_h_; y < input_height_ + pad_h_; y++) {
				for (int x = pad_w_; x < input_width_ + pad_w_; x++) {
					input_temp_[y*w_ + x] = input[index_input++];
				}
			}
			for (int y = 0; y < output_height_; y++)
				for (int x = 0; x < output_width_; x++)
					for (int ky = 0; ky < kernel_h_; ky++)
						for (int kx = 0; kx < kernel_w_; kx++)
							if (input_temp_[(y*stride_ + ky) * w_ + x*stride_ + kx] > output_temp_[y * output_width_ + x])
								output_temp_[y * output_width_ + x] = input_temp_[(y*stride_ + ky) * w_ + x*stride_ + kx];

			for (int i = 0; i < output_height_ * output_width_; i++)
					output[index++] = output_temp_[i];

		}
	}
};