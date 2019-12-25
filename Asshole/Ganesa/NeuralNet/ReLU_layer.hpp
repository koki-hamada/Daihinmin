#pragma once
#include "layer.hpp"
#include <math.h>
class ReLULayer : public Layer {
public:
	ReLULayer(int n, int height, int width) : Layer(n,n,width,height,width,height){
		/*num_input_ = num_output_ = n;
		input_height_ = output_height_ = height;
		input_width_ = output_width_ = width;*/
	}
	virtual void Forward(const float *input, float* output) {
		for (int o = 0; o < num_output_*output_height_ * output_width_; o++) {
			output[o] = (0.0f < input[o]) ? input[o] : 0.0f;
		}
	}
};