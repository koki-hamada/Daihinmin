#pragma once
#include "ReLU_layer.hpp"
#include "convolution_layer.hpp"
#include "inner_product_layer.hpp"
#include "layer.hpp"
#include "max_pooling_layer.hpp"
#include "sigmoid_layer.hpp"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>

const int MAX_INPUT_NUM = 4096;
const int MAX_LAYER_NUM = 30;

class Net {
  public:
	int layer_num_;
	Layer* layers_[MAX_LAYER_NUM];
	float data_[2][MAX_INPUT_NUM];

	Net() {
		layer_num_ = 0;
	};
	~Net() {
		Clear();
	};

	void Read(std::string filename) {
		Clear();

		std::ifstream ifs(filename.c_str());
		if(ifs.bad()) {
			std::cout << "not found " + filename << std::endl;
			exit(1);
		}

		std::string s;
		while(ifs >> s) {
			if(s == "[InnerProductLayer]") {
				int num_input, num_output;
				ifs >> s >> num_input >> s >> num_output;
				InnerProductLayer* iplayer = new InnerProductLayer(num_input, num_output);
				for(int i = 0; i < num_output * num_input; i++) {
					ifs >> iplayer->weight_[i];
				}
				for(int i = 0; i < num_output; i++) {
					ifs >> iplayer->bias_[i];
				}
				layers_[layer_num_++] = iplayer;
			} else if(s == "[ConvolutionLayer]") {
				int input[11]; //convolution_layer�̃p�����[�^���d�݂̂ق���11���� pooling�����l
				for(int i = 0; i < 11; i++)
					ifs >> s >> input[i];
				ConvolutionLayer* convlayer = new ConvolutionLayer(input[0], input[1], input[2], input[3], input[4], input[5], input[6], input[7], input[8], input[9], input[10]);
				for(int i = 0; i < convlayer->num_input_ * convlayer->num_output_ * convlayer->kernel_h_ * convlayer->kernel_w_; i++)
					ifs >> convlayer->weight_[i];
				for(int i = 0; i < convlayer->num_output_; i++)
					ifs >> convlayer->bias_[i];
				layers_[layer_num_++] = convlayer;
			} else if(s == "[PoolingLayer]") {
				int method;
				ifs >> s >> method;
				int input[11];
				for(int i = 0; i < 11; i++)
					ifs >> s >> input[i];
				//if(method == 0)
				MaxPoolingLayer* poollayer = new MaxPoolingLayer(input[0], input[1], input[2], input[3], input[4], input[5], input[6], input[7], input[8], input[9], input[10]);
				layers_[layer_num_++] = poollayer;
			} else if(s == "[SigmoidLayer]") {
				layers_[layer_num_] = new SigmoidLayer(layers_[layer_num_ - 1]->num_output_, layers_[layer_num_ - 1]->output_height_, layers_[layer_num_ - 1]->output_width_);
				layer_num_++;
			} else if(s == "[ReLULayer]") {
				layers_[layer_num_] = new ReLULayer(layers_[layer_num_ - 1]->num_output_, layers_[layer_num_ - 1]->output_height_, layers_[layer_num_ - 1]->output_width_);
				layer_num_++;
			} else {
				std::cout << "read error " + s + "?" << std::endl;
			}
		}
	}

	void Clear() {
		for(int i = 0; i < layer_num_; i++) {
			delete layers_[i];
		}
		layer_num_ = 0;
	}

	void Forward() {
		float* in = data_[0];
		float* out = data_[1];
		for(int i = 0; i < layer_num_; i++) {
			layers_[i]->Forward(in, out);
			std::swap(in, out);
			//std::swap(data_[0], data_[1]);
		}
	}

	inline float* Input() {
		return data_[0];
	}

	inline float* Output() {
		return data_[layer_num_ % 2];
	}
};
