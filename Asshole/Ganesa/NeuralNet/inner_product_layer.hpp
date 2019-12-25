#ifndef INNER_PRODUCT_LAYER_HPP_
#define INNER_PRODUCT_LAYER_HPP_
#include <iostream>
#include "layer.hpp"

class InnerProductLayer : public Layer {
public:
	float *weight_;
	float *bias_;
	// コンストラクタ
	InnerProductLayer(int num_input, int num_output) : Layer(num_output, num_input) {
	//	num_input_ = num_input;
		//num_output_ = num_output;
//		input_width_ = input_height_ = 1;
	//	output_width_ = output_height_ = 1;
		weight_ = new float[num_output * num_input];
		bias_ = new float[num_output];
	}

	// デストラクタ
	~InnerProductLayer() {
		delete[] weight_;
		delete[] bias_;
	}

	//順伝播. 
	virtual void Forward(const float *input, float* output) {
		for (int o = 0; o < num_output_; o++) {
			output[o] = 0.0f;
			for (int i = 0; i < num_input_; i++) {
				output[o] += weight_[num_input_ * o + i] * input[i];
			}
			output[o] += bias_[o];
		}
	}
}; // class InnerProductLayer
#endif //INNER_PRODUCT_LAYER_HPP_